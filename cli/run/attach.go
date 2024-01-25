package run

import (
	"errors"
	"fmt"
	"os"
	"path/filepath"

	"github.com/appview-team/appview/loader"
	"github.com/appview-team/appview/util"
	"github.com/syndtr/gocapability/capability"
)

var (
	errGetLinuxCap       = errors.New("unable to get linux capabilities for current process")
	errLoadLinuxCap      = errors.New("unable to load linux capabilities for current process")
	errMissingPtrace     = errors.New("missing PTRACE capabilities to attach to a process")
	errMissingViewedProc = errors.New("no viewed process found matching that name")
	errNotViewed         = errors.New("detach failed. This process is not being viewed")
	errLibraryNotExist   = errors.New("library Path does not exist")
)

// NOTE: The responsibility of this function is to check if its possible to attach
// and then perform the attach if so
func (rc *Config) Attach(pid int, setupWorkDir bool) (bool, error) {
	env := os.Environ()
	ld := loader.New()

	reattach := false

	status, err := util.PidAppViewStatus(rc.Rootdir, pid)
	if err != nil {
		return false, err
	}
	if status == util.Disable || status == util.Setup {
		if err := util.UserVerifyRootPerm(); err != nil {
			return false, err
		}
		// Validate PTRACE capability
		c, err := capability.NewPid2(0)
		if err != nil {
			return false, errGetLinuxCap
		}

		if err = c.Load(); err != nil {
			return false, errLoadLinuxCap
		}

		if !c.Get(capability.EFFECTIVE, capability.CAP_SYS_PTRACE) {
			return false, errMissingPtrace
		}
	} else {
		// Reattach because process contains our library
		reattach = true
	}

	args := []string{fmt.Sprint(pid)}

	if rc.Rootdir != "" {
		args = append(args, []string{"--rootdir", rc.Rootdir}...)
	}

	// Disable detection of a appview rules file with this command
	env = append(env, "APPVIEW_RULES=false")

	// Disable cribl event breaker with this command
	if rc.NoBreaker {
		env = append(env, "APPVIEW_CRIBL_NO_BREAKER=true")
	}

	// Normal operational, create a directory for this run.
	// Directory contains appview.yml which is configured to output to that
	// directory and has a command directory configured in that directory.
	if setupWorkDir {
		rc.setupWorkDir(args, true)
	}
	env = append(env, "APPVIEW_CONF_PATH="+filepath.Join(rc.WorkDir, "appview.yml"))

	// Check the attached process mnt namespace.
	// If it is different from the CLI mnt namespace:
	// - create working directory in the attached process mnt namespace
	// - replace the working directory in the CLI mnt namespace with symbolic
	//   link to working directory created in previous step
	refNsPid := util.PidGetRefPidForMntNamespace(rc.Rootdir, pid)
	if refNsPid != -1 {
		env = append(env, "APPVIEW_HOST_WORKDIR_PATH="+rc.WorkDir)
	}

	// Handle custom library path
	if len(rc.LibraryPath) > 0 {
		if !util.CheckDirExists(rc.LibraryPath) {
			return reattach, errLibraryNotExist
		}
		args = append([]string{"-f", rc.LibraryPath}, args...)
	}

	if reattach {
		env = append(env, "APPVIEW_CONF_RELOAD="+filepath.Join(rc.WorkDir, "appview.yml"))
	}

	stdoutStderr, err := ld.AttachSubProc(args, env)
	util.Warn(stdoutStderr)
	if err != nil {
		return reattach, err
	}

	// Replace the working directory files with symbolic links in case of successful attach
	// where the target ns is different to the origin ns

	if setupWorkDir {
		eventsFilePath := filepath.Join(rc.WorkDir, "events.json")
		metricsFilePath := filepath.Join(rc.WorkDir, "metrics.json")
		logsFilePath := filepath.Join(rc.WorkDir, "libappview.log")
		payloadsDirPath := filepath.Join(rc.WorkDir, "payloads")

		if rc.Rootdir != "" {
			os.Remove(eventsFilePath)
			os.Remove(metricsFilePath)
			os.Remove(logsFilePath)
			os.RemoveAll(payloadsDirPath)
			os.Symlink(filepath.Join(rc.Rootdir, "/proc", fmt.Sprint(pid), "root", eventsFilePath), eventsFilePath)
			os.Symlink(filepath.Join(rc.Rootdir, "/proc", fmt.Sprint(pid), "root", metricsFilePath), metricsFilePath)
			os.Symlink(filepath.Join(rc.Rootdir, "/proc", fmt.Sprint(pid), "root", logsFilePath), logsFilePath)
			os.Symlink(filepath.Join(rc.Rootdir, "/proc", fmt.Sprint(pid), "root", payloadsDirPath), payloadsDirPath)

		} else if refNsPid != -1 {
			// Child namespace
			os.Remove(eventsFilePath)
			os.Remove(metricsFilePath)
			os.Remove(logsFilePath)
			os.RemoveAll(payloadsDirPath)
			os.Symlink(filepath.Join("/proc", fmt.Sprint(refNsPid), "root", eventsFilePath), eventsFilePath)
			os.Symlink(filepath.Join("/proc", fmt.Sprint(refNsPid), "root", metricsFilePath), metricsFilePath)
			os.Symlink(filepath.Join("/proc", fmt.Sprint(refNsPid), "root", logsFilePath), logsFilePath)
			os.Symlink(filepath.Join("/proc", fmt.Sprint(refNsPid), "root", payloadsDirPath), payloadsDirPath)
		}
	}

	return reattach, nil
}

// NOTE: The responsibility of this function is to check if its possible to detach
// and then perform the detach if so
func (rc *Config) Detach(pid int) error {
	env := os.Environ()
	ld := loader.New()

	status, err := util.PidAppViewStatus(rc.Rootdir, pid)
	if err != nil {
		return err
	}
	if status != util.Active {
		return errNotViewed
	}

	args := []string{fmt.Sprint(pid)}

	if rc.Rootdir != "" {
		args = append(args, []string{"--rootdir", rc.Rootdir}...)
	}

	stdoutStderr, err := ld.DetachSubProc(args, env)
	util.Warn(stdoutStderr)
	if err != nil {
		return err
	}

	return nil
}
