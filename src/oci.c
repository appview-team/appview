#define _GNU_SOURCE

#include "oci.h"
#include "cJSON.h"


/*
 * Read the OCI configuration into memory
 *
 * Returns the modified data which should be freed with appview_free, in case of failure returns NULL
 */
void *
ociReadCfgIntoMem(const char *cfgPath) {
    void *buf = NULL;
    struct stat fileStat;

    if (appview_stat(cfgPath, &fileStat) == -1) {
        return NULL;
    }

    FILE *fp = appview_fopen(cfgPath, "r");
    if (!fp) {
        return NULL;
    }

    buf = (char *)appview_malloc(fileStat.st_size);
    if (!buf) {
        goto close_file;
    }

    size_t ret = appview_fread(buf, sizeof(char), fileStat.st_size, fp);
    if (ret != fileStat.st_size ) {
        appview_free(buf);
        buf = NULL;
        goto close_file;
    }

close_file:

    appview_fclose(fp);

    return buf;
}

/*
 * Modify the OCI configuration for the given container.
 * A path to the container specific the location of the configuration file.
 *
 * Refer to the opencontainers Linux runtime-spec for details about the exact JSON struct.
 * The following changes will be performed:
 * - Add a mount point(s)
 *   * `appview` directory will be mounted from the host "/usr/lib/appview/" into the container: "/usr/lib/appview/"
 *   * A UNIX socket directory will be mounted from the host into the container. The path to UNIX socket
 *   will be read from host based on value in the rules file [optionally]
 *
 * - Extend Environment variables
 *   * `LD_PRELOAD` will contain the following entry `/opt/appview/libappview.so`
 *   * `APPVIEW_SETUP_DONE=true` mark that configuration was processed
 *
 * - Add prestart hook
 *   execute a appview extract operation to ensure that the proper library is used in the specific loader context; musl or glibc.
 * 
 * Returns the modified data which should be freed with appview_free, in case of failure returns NULL
 */
char *
ociModifyCfg(const void *cfgMem, const char *appviewPath, const char *unixSocketPath) {

    if (!cfgMem || !appviewPath ) {
        return NULL;
    }

    cJSON *json = cJSON_Parse(cfgMem);
    if (json == NULL) {
        goto exit;
    }

    /*
    * Handle process environment variables
    *
    "env":[
         "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin",
         "HOSTNAME=6735578591bb",
         "TERM=xterm",
         "LD_PRELOAD=/opt/appview/libappview.so",
         "APPVIEW_SETUP_DONE=true"
      ],
    */
    cJSON *procNode = cJSON_GetObjectItemCaseSensitive(json, "process");
    if (!procNode) {
        procNode = cJSON_CreateObject();
        if (!procNode) {
            cJSON_Delete(json);
            goto exit;
        }
        cJSON_AddItemToObject(json, "process", procNode);
    }

    cJSON *envNodeArr = cJSON_GetObjectItemCaseSensitive(procNode, "env");
    if (envNodeArr) {
        bool ldPreloadPresent = FALSE;
        // Iterate over environment string array
        size_t envSize = cJSON_GetArraySize(envNodeArr);
        for (int i = 0; i < envSize ;++i) {
            cJSON *item = cJSON_GetArrayItem(envNodeArr, i);
            char *strItem = cJSON_GetStringValue(item);

            if (appview_strncmp("LD_PRELOAD=", strItem, C_STRLEN("LD_PRELOAD=")) == 0) {
                size_t itemLen = appview_strlen(strItem);
                size_t newLdprelLen = itemLen + C_STRLEN("/opt/appview/libappview.so:");
                char *newLdPreloadLib = appview_calloc(1, newLdprelLen);
                if (!newLdPreloadLib) {
                    cJSON_Delete(json);
                    goto exit;
                }
                appview_strncpy(newLdPreloadLib, "LD_PRELOAD=/opt/appview/libappview.so:", C_STRLEN("LD_PRELOAD=/opt/appview/libappview.so:"));
                appview_strcat(newLdPreloadLib, strItem + C_STRLEN("LD_PRELOAD="));
                cJSON *newLdPreloadLibObj = cJSON_CreateString(newLdPreloadLib);
                if (!newLdPreloadLibObj) {
                    appview_free(newLdPreloadLib);
                    cJSON_Delete(json);
                    goto exit;
                }
                cJSON_ReplaceItemInArray(envNodeArr, i, newLdPreloadLibObj);
                appview_free(newLdPreloadLib);

                cJSON *appviewEnvNode = cJSON_CreateString("APPVIEW_SETUP_DONE=true");
                if (!appviewEnvNode) {
                    cJSON_Delete(json);
                    goto exit;
                }
                cJSON_AddItemToArray(envNodeArr, appviewEnvNode);
                ldPreloadPresent = TRUE;
                break;
            } else if (appview_strncmp("APPVIEW_SETUP_DONE=true", strItem, C_STRLEN("APPVIEW_SETUP_DONE=true")) == 0) {
                // we are done here
                cJSON_Delete(json);
                goto exit;
            }
        }


        // There was no LD_PRELOAD in environment variables
        if (ldPreloadPresent == FALSE) {
            const char *const envItems[2] =
            {
                "LD_PRELOAD=/opt/appview/libappview.so",
                "APPVIEW_SETUP_DONE=true"
            };
            for (int i = 0; i < ARRAY_SIZE(envItems) ;++i) {
                cJSON *appviewEnvNode = cJSON_CreateString(envItems[i]);
                if (!appviewEnvNode) {
                    cJSON_Delete(json);
                    goto exit;
                }
                cJSON_AddItemToArray(envNodeArr, appviewEnvNode);
            }
        }
    } else {
        const char * envItems[2] =
        {
            "LD_PRELOAD=/opt/appview/libappview.so",
            "APPVIEW_SETUP_DONE=true"
        };
        envNodeArr = cJSON_CreateStringArray(envItems, ARRAY_SIZE(envItems));
        if (!envNodeArr) {
            cJSON_Delete(json);
            goto exit;
        }
        cJSON_AddItemToObject(procNode, "env", envNodeArr);
    }

    /*
    * Handle process mounts for library and rules file and optionally for UNIX socket
    *
    "mounts":[
      {
         "destination":"/proc",
         "type":"proc",
         "source":"proc",
         "options":[
            "nosuid",
            "noexec",
            "nodev"
         ]
      },
      ...
      {
         "destination":"/usr/lib/appview/",
         "type":"bind",
         "source":"/usr/lib/appview/",
         "options":[
            "rbind",
            "rprivate"
         ]
      },
      {
         "destination":"/var/run/appview/",
         "type":"bind",
         "source":"/var/run/appview/",
         "options":[
            "rbind",
            "rprivate"
         ]
      }
    */

    const char *mountPath[2] =
    {
        "/usr/lib/appview/",
        unixSocketPath
    };

    // Skip the Unix mounting point if it is not present
    size_t mountPoints = ARRAY_SIZE(mountPath);
    if (!unixSocketPath) {
        mountPoints -= 1;
    }

    for (int i = 0; i < mountPoints; ++i ) {
        cJSON *mountNodeArr = cJSON_GetObjectItemCaseSensitive(json, "mounts");
        if (!mountNodeArr) {
            mountNodeArr = cJSON_CreateArray();
            if (!mountNodeArr) {
                cJSON_Delete(json);
                goto exit;
            }
            cJSON_AddItemToObject(json, "mounts", mountNodeArr);
        }

        cJSON *mountNode = cJSON_CreateObject();
        if (!mountNode) {
            cJSON_Delete(json);
            goto exit;
        }

        if (!cJSON_AddStringToObjLN(mountNode, "destination", mountPath[i])) {
            cJSON_Delete(mountNode);
            cJSON_Delete(json);
            goto exit;
        }

        if (!cJSON_AddStringToObjLN(mountNode, "type", "bind")) {
            cJSON_Delete(mountNode);
            cJSON_Delete(json);
            goto exit;
        }

        if (!cJSON_AddStringToObjLN(mountNode, "source", mountPath[i])) {
            cJSON_Delete(mountNode);
            cJSON_Delete(json);
            goto exit;
        }

        const char *optItems[2] =
        {
            "rbind",
            "rprivate"
        };

        cJSON *optNodeArr = cJSON_CreateStringArray(optItems, ARRAY_SIZE(optItems));
        if (!optNodeArr) {
            cJSON_Delete(mountNode);
            cJSON_Delete(json);
            goto exit;
        }
        cJSON_AddItemToObject(mountNode, "options", optNodeArr);
        cJSON_AddItemToArray(mountNodeArr, mountNode);
    }

    /*
    * Handle startContainer hooks process
    *
   "hooks":{
      "prestart":[
         {
            "path":"/proc/1513/exe",
            "args":[
               "libnetwork-setkey",
               "-exec-root=/var/run/docker",
               "6735578591bb3c5aebc91e5c702470c52d2c10cea52e4836604bf5a4a6c0f2eb",
               "ec7e49ffc98c"
            ]
         }
      ],
      "startContainer":[
         {
            "path":"/usr/lib/appview/<version>/appview"
            "args":[
               "/usr/lib/appview/<version>/appview",
               "extract",
               "-p",
               "/opt/appview",
            ]
         },
       ]
    */
    cJSON *hooksNode = cJSON_GetObjectItemCaseSensitive(json, "hooks");
    if (!hooksNode) {
        hooksNode = cJSON_CreateObject();
        if (!hooksNode) {
            cJSON_Delete(json);
            goto exit;
        }
        cJSON_AddItemToObject(json, "hooks", hooksNode);
    }

    cJSON *startContainerNodeArr = cJSON_GetObjectItemCaseSensitive(hooksNode, "startContainer");
    if (!startContainerNodeArr) {
        startContainerNodeArr = cJSON_CreateArray();
        if (!startContainerNodeArr) {
            cJSON_Delete(json);
            goto exit;
        }
        cJSON_AddItemToObject(hooksNode, "startContainer", startContainerNodeArr);
    }

    cJSON *startContainerNode = cJSON_CreateObject();
    if (!startContainerNode) {
        cJSON_Delete(json);
        goto exit;
    }

    if (!cJSON_AddStringToObjLN(startContainerNode, "path",  appviewPath)) {
        cJSON_Delete(startContainerNode);
        cJSON_Delete(json);
        goto exit;
    }

    const char *argsItems[4] =
    {
        appviewPath,
        "extract",
        "-p",
        "/opt/appview"
    };
    cJSON *argsNodeArr = cJSON_CreateStringArray(argsItems, ARRAY_SIZE(argsItems));
    if (!argsNodeArr) {
        cJSON_Delete(startContainerNode);
        cJSON_Delete(json);
        goto exit;
    }
    cJSON_AddItemToObject(startContainerNode, "args", argsNodeArr);
    cJSON_AddItemToArray(startContainerNodeArr, startContainerNode);

    char *jsonStr = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);

    return jsonStr;

exit:
    return NULL;
}

/*
 * Write the OCI configuration into the specified file
 * 
 * Returns TRUE in case of success, FALSE otherwise
 */
bool
ociWriteConfig(const char *path, const char *cfg) {
    FILE *fp = appview_fopen(path, "w");
    if (fp == NULL) {
        return FALSE;
    }

    appview_fprintf(fp, "%s\n", cfg);

    appview_fclose(fp);
    return TRUE;
}

