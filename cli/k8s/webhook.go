package k8s

import (
	"context"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net/http"
	"os"
	"strings"

	"github.com/appview-team/appview/internal"
	"github.com/rs/zerolog/log"
	admissionv1 "k8s.io/api/admission/v1"
	corev1 "k8s.io/api/core/v1"
	"k8s.io/apimachinery/pkg/api/errors"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
	"k8s.io/client-go/kubernetes"
	"k8s.io/client-go/rest"
)

// App contains configuration for a webhook server
type App struct {
	*Options
}

// HandleMutate handles the mutate endpoint
func (app *App) HandleMutate(w http.ResponseWriter, r *http.Request) {
	log.Debug().Str("method", r.Method).Str("proto", r.Proto).Str("remoteaddr", r.RemoteAddr).Str("uri", r.RequestURI).Int("length", int(r.ContentLength)).Msg("request received")
	admissionReview := &admissionv1.AdmissionReview{}

	err := json.NewDecoder(r.Body).Decode(admissionReview)
	if err != nil {
		app.HandleError(w, r, fmt.Errorf("invalid JSON input"))
		return
	}

	pod := &corev1.Pod{}
	if err := json.Unmarshal(admissionReview.Request.Object.Raw, pod); err != nil {
		app.HandleError(w, r, fmt.Errorf("unmarshal to pod: %v", err))
		return
	}

	shouldModify := true

	if _, ok := pod.ObjectMeta.Annotations["appview.dev/disable"]; ok {
		shouldModify = false
	}

	// Use appview-pod-init to see if the pod is already mutated
	// if so, don't double-mutate it
	for i := 0; i < len(pod.Spec.InitContainers); i++ {
		if pod.Spec.InitContainers[i].Name == "appview-pod-init" {
			shouldModify = false
		}
	}

	ver := internal.GetNormalizedVersion()

	patch := []JSONPatchEntry{}
	if shouldModify {
		log.Debug().Interface("pod", pod).Msgf("modifying pod")
		// creates the in-cluster config
		config, err := rest.InClusterConfig()
		if err != nil {
			// Only throw an error if we're not in a test
			// OR if we are in a test, and the error is NOT ErrNotInACluster
			if !strings.HasSuffix(os.Args[0], ".test") || err != rest.ErrNotInCluster {
				app.HandleError(w, r, err)
				return
			}
		} else {
			// creates the clientset
			clientset, err := kubernetes.NewForConfig(config)
			if err != nil {
				app.HandleError(w, r, err)
				return
			}

			// Get current namespace from serviceaccount
			namespace, err := ioutil.ReadFile("/var/run/secrets/kubernetes.io/serviceaccount/namespace")
			if err != nil {
				app.HandleError(w, r, err)
				return
			}

			// Get reference configmap in appview's namespace
			cm, err := clientset.CoreV1().ConfigMaps(string(namespace)).Get(context.TODO(), "appview", metav1.GetOptions{})
			if err != nil {
				app.HandleError(w, r, err)
				return
			}

			// If the appview configmap does exists in our namespace, create it from template in appview namespace
			if cmlocal, err := clientset.CoreV1().ConfigMaps(admissionReview.Request.Namespace).Get(context.TODO(), "appview", metav1.GetOptions{}); errors.IsNotFound(err) {
				log.Debug().Interface("cm", cm).Str("namespace", admissionReview.Request.Namespace).Msgf("creating configmap")
				cm.SetResourceVersion("")
				cm.SetNamespace(admissionReview.Request.Namespace)
				_, err := clientset.CoreV1().ConfigMaps(admissionReview.Request.Namespace).Create(context.TODO(), cm, metav1.CreateOptions{})
				if err != nil {
					app.HandleError(w, r, err)
					return
				}
			} else {
				// We do exist, so update the data with current configuration from our appview namespace configmap
				cmlocal.Data = cm.Data
				log.Debug().Interface("cm", cm).Str("namespace", admissionReview.Request.Namespace).Msgf("updating configmap")
				_, err := clientset.CoreV1().ConfigMaps(admissionReview.Request.Namespace).Update(context.TODO(), cmlocal, metav1.UpdateOptions{})
				if err != nil {
					app.HandleError(w, r, err)
					return
				}
			}
		}

		// InitContainer are splitted by 2 phases (appview-pod-init, appview-pod-extract)
		// appview-pod-init: copy appview from cribl:/appview to shared volume /appview/appview
		// appview-pod-extract: create extraction directory (/appview/appview/<id>/) and perform
		// extract operation there
		//
		// Note: appview-pod-extract is performed in context of application container
		// therefore we are able to detect proper loader used in application container

		// appview-pod-init container will copy the appview binary
		pod.Spec.InitContainers = append(pod.Spec.InitContainers, corev1.Container{
			Name:    "appview-pod-init",
			Image:   fmt.Sprintf("cribl/appview:%s", ver),
			Command: []string{"cp", "/usr/local/bin/appview", "/appview/appview"},
			VolumeMounts: []corev1.VolumeMount{{
				Name:      "appview",
				MountPath: "/appview",
			}},
		})

		// Create appview-conf volume
		// assumed to be emptyDir
		pod.Spec.Volumes = append(pod.Spec.Volumes, corev1.Volume{
			Name: "appview",
		}, corev1.Volume{
			Name: "appview-conf",
			VolumeSource: corev1.VolumeSource{
				ConfigMap: &corev1.ConfigMapVolumeSource{
					LocalObjectReference: corev1.LocalObjectReference{
						Name: "appview",
					},
				},
			},
		})

		// add volume mount to all containers in the pod
		for i := 0; i < len(pod.Spec.Containers); i++ {
			// appview-pod-extract container(s) will extract the appview files (library and config files)
			appviewDirPath := fmt.Sprintf("/appview/%d", i)
			cmd := []string{
				"/appview/appview",
				"excrete",
				"--parents",
			}
			if len(app.CriblDest) > 0 {
				cmd = append(cmd,
					"--cribldest",
					app.CriblDest,
				)
			} else {
				cmd = append(cmd,
					"--metricdest",
					app.MetricDest,
					"--metricformat",
					app.MetricFormat,
					"--metricprefix",
					app.MetricPrefix,
					"--eventdest",
					app.EventDest,
				)
			}

			cmd = append(cmd, appviewDirPath)
			pod.Spec.InitContainers = append(pod.Spec.InitContainers, corev1.Container{
				Name:            fmt.Sprintf("appview-pod-extract-%d", i),
				Image:           pod.Spec.Containers[i].Image,
				ImagePullPolicy: pod.Spec.Containers[i].ImagePullPolicy,
				Command:         cmd,
				VolumeMounts: []corev1.VolumeMount{{
					Name:      "appview",
					MountPath: "/appview",
				}},
			})

			pod.Spec.Containers[i].VolumeMounts = append(pod.Spec.Containers[i].VolumeMounts, corev1.VolumeMount{
				Name:      "appview",
				MountPath: "/appview",
			}, corev1.VolumeMount{
				Name:      "appview-conf",
				MountPath: "/appview/appview.yml",
				SubPath:   "appview.yml",
			})
			if len(app.CriblDest) > 0 {
				pod.Spec.Containers[i].Env = append(pod.Spec.Containers[i].Env, corev1.EnvVar{
					Name:  "APPVIEW_CRIBL",
					Value: app.CriblDest,
				})
			}
			// Add environment variables to configure appview
			pod.Spec.Containers[i].Env = append(pod.Spec.Containers[i].Env, corev1.EnvVar{
				Name:  "LD_PRELOAD",
				Value: fmt.Sprintf("%s/libappview.so", appviewDirPath),
			})
			pod.Spec.Containers[i].Env = append(pod.Spec.Containers[i].Env, corev1.EnvVar{
				Name:  "APPVIEW_CONF_PATH",
				Value: fmt.Sprintf("%s/appview.yml", appviewDirPath),
			})
			pod.Spec.Containers[i].Env = append(pod.Spec.Containers[i].Env, corev1.EnvVar{
				Name:  "APPVIEW_EXEC_PATH",
				Value: "/appview/appview",
			})
			pod.Spec.Containers[i].Env = append(pod.Spec.Containers[i].Env, corev1.EnvVar{
				Name:  "LD_LIBRARY_PATH",
				Value: fmt.Sprintf("/tmp/appview/%s/", ver),
			})
			// Get some metadata pushed into appview from the K8S downward API
			pod.Spec.Containers[i].Env = append(pod.Spec.Containers[i].Env, corev1.EnvVar{
				Name: "APPVIEW_TAG_node_name",
				ValueFrom: &corev1.EnvVarSource{
					FieldRef: &corev1.ObjectFieldSelector{
						FieldPath: "spec.nodeName",
					},
				},
			})
			pod.Spec.Containers[i].Env = append(pod.Spec.Containers[i].Env, corev1.EnvVar{
				Name: "APPVIEW_TAG_namespace",
				ValueFrom: &corev1.EnvVarSource{
					FieldRef: &corev1.ObjectFieldSelector{
						FieldPath: "metadata.namespace",
					},
				},
			})
			// Add tags from k8s metadata
			for k, v := range pod.ObjectMeta.Labels {
				if strings.HasPrefix(k, "app.kubernetes.io") {
					parts := strings.Split(k, "/")
					if len(parts) > 1 {
						pod.Spec.Containers[i].Env = append(pod.Spec.Containers[i].Env, corev1.EnvVar{
							Name:  fmt.Sprintf("APPVIEW_TAG_%s", strings.ToLower(parts[1])),
							Value: v,
						})
					}
				}
			}
		}

		if pod.ObjectMeta.Labels != nil {
			pod.ObjectMeta.Labels["appview.dev/appview"] = "true"
		} else {
			pod.ObjectMeta.Labels = map[string]string{
				"appview.dev/appview": "true",
			}
		}

		initContainersBytes, err := json.Marshal(&pod.Spec.InitContainers)
		if err != nil {
			app.HandleError(w, r, fmt.Errorf("error marshaling initContainers: %v", err))
			return
		}

		containersBytes, err := json.Marshal(&pod.Spec.Containers)
		if err != nil {
			app.HandleError(w, r, fmt.Errorf("error marshaling initContainers: %v", err))
			return
		}

		volumesBytes, err := json.Marshal(&pod.Spec.Volumes)
		if err != nil {
			app.HandleError(w, r, fmt.Errorf("marshall volumes: %v", err))
			return
		}

		labelsBytes, err := json.Marshal(&pod.ObjectMeta.Labels)
		if err != nil {
			app.HandleError(w, r, fmt.Errorf("marshal labels: %v", err))
			return
		}

		// build json patch
		patch = []JSONPatchEntry{
			{
				Op:    "replace",
				Path:  "/metadata/labels",
				Value: labelsBytes,
			},
			{
				Op:    "replace",
				Path:  "/spec/initContainers",
				Value: initContainersBytes,
			},
			{
				Op:    "replace",
				Path:  "/spec/containers",
				Value: containersBytes,
			},
			{
				Op:    "replace",
				Path:  "/spec/volumes",
				Value: volumesBytes,
			},
		}
	}

	patchBytes, err := json.Marshal(&patch)
	if err != nil {
		app.HandleError(w, r, fmt.Errorf("marshall jsonpatch: %v", err))
		return
	}
	log.Debug().RawJSON("patch", patchBytes).Msgf("patch")

	patchType := admissionv1.PatchTypeJSONPatch

	// build admission response
	admissionResponse := &admissionv1.AdmissionResponse{
		UID:       admissionReview.Request.UID,
		Allowed:   true,
		Patch:     patchBytes,
		PatchType: &patchType,
	}

	respAdmissionReview := &admissionv1.AdmissionReview{
		TypeMeta: metav1.TypeMeta{
			Kind:       "AdmissionReview",
			APIVersion: "admission.k8s.io/v1",
		},
		Response: admissionResponse,
	}

	w.Header().Set("Content-Type", "application/json")
	outb, err := json.Marshal(respAdmissionReview)
	if err != nil {
		app.HandleError(w, r, fmt.Errorf("json encoding error: %v", err))
		return
	}

	_, err = w.Write(outb)
	if err != nil {
		app.HandleError(w, r, fmt.Errorf("write error: %v", err))
		return
	}
	log.Info().Bool("modified", shouldModify).Msgf("patch returned")
}

// JSONPatchEntry represents a single JSON patch entry
type JSONPatchEntry struct {
	Op    string          `json:"op"`
	Path  string          `json:"path"`
	Value json.RawMessage `json:"value,omitempty"`
}
