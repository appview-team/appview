package run

import (
	"encoding/json"
	"fmt"
	"io"
	"os"
	"path"
	"path/filepath"
	"strconv"
	"strings"
	"syscall"
	"time"

	"github.com/appview-team/appview/internal"
	"github.com/appview-team/appview/util"
)

func environNoAppView() []string {
	env := []string{}
	for _, envVar := range os.Environ() {
		if !strings.HasPrefix(envVar, "APPVIEW") {
			env = append(env, envVar)
		}
	}
	return env
}

// CreateAll outputs all bundled files to a given path
func CreateAll(path string) error {
	files := []string{"libappview.so", "appview.yml"}
	perms := []os.FileMode{0755, 0644}
	for i, f := range files {
		b, err := Asset(fmt.Sprintf("build/%s", f))
		if err != nil {
			return err
		}
		err = os.WriteFile(filepath.Join(path, f), b, perms[i])
		if err != nil {
			return err
		}
	}
	return nil
}

// readBytesStdIn reads the byte data from stdin
func readBytesStdIn() []byte {
	var cfgData []byte

	stdinFs, err := os.Stdin.Stat()
	if err != nil {
		return cfgData
	}

	// Avoid waiting for input from terminal when no data was provided
	if stdinFs.Mode()&os.ModeCharDevice != 0 && stdinFs.Size() == 0 {
		return cfgData
	}

	cfgData, _ = io.ReadAll(os.Stdin)

	return cfgData
}

// LoadConfig loads a config if not yet defined
// Config will be loaded from (first of):
// - File via --userconfig option
// - StdIn if @tryStdIn is true
// - Run options
func (rc *Config) LoadConfig(tryStdIn bool) {
	if rc.sc == nil {
		if rc.UserConfig == "" {
			var stdInData []byte
			if tryStdIn {
				stdInData = readBytesStdIn()
			}
			if len(stdInData) > 0 {
				err := rc.ConfigFromStdin(stdInData)
				util.CheckErrSprintf(err, "%v", err)
			} else {
				err := rc.configFromRunOpts()
				util.CheckErrSprintf(err, "%v", err)
			}
		} else {
			err := rc.ConfigFromFile()
			util.CheckErrSprintf(err, "%v", err)
		}
	}
}

// setupWorkDir sets up a working directory for a given set of args
func (rc *Config) setupWorkDir(args []string, attach bool) {
	// Override to CriblDest if specified
	// TODO still needed?
	if rc.CriblDest != "" {
		rc.EventsDest = rc.CriblDest
		rc.MetricsDest = rc.CriblDest
	}

	// Create session directory
	rc.createWorkDir(args, attach)

	// Build or load config
	// Note: Read the data from stdin only during attach
	rc.LoadConfig(attach)

	// Populate session directory
	rc.populateWorkDir(args, attach)
}

// createWorkDir creates a working directory
func (rc *Config) createWorkDir(args []string, attach bool) {
	var pid string
	var cmd string

	dirPerms := os.FileMode(0755)
	if attach {
		dirPerms = 0777
		oldmask := syscall.Umask(0)
		defer syscall.Umask(oldmask)
	}

	if rc.WorkDir != "" {
		if util.CheckDirExists(rc.WorkDir) {
			// Directory exists, exit
			return
		}
	}

	if rc.now == nil {
		rc.now = time.Now
	}

	// Directories named CMD_SESSIONID_PID_TIMESTAMP
	ts := strconv.FormatInt(rc.now().UTC().UnixNano(), 10)
	sessionID := GetSessionID()
	if attach {
		// When we attach args[0] points to pid of desired process
		pid = args[0]
		pidInt, _ := strconv.Atoi(args[0])
		cmd, _ = util.PidCommand(rc.Rootdir, pidInt)
	} else {
		pid = strconv.Itoa(os.Getpid())
		cmd = path.Base(args[0])
	}

	tmpDirName := cmd + "_" + sessionID + "_" + pid + "_" + ts

	// Create History directory
	histDir := HistoryDir()
	err := os.MkdirAll(histDir, 0755)
	util.CheckErrSprintf(err, "error creating history dir: %v", err)

	// Sudo user warning
	if _, present := os.LookupEnv("SUDO_USER"); present {
		util.Warn("WARNING: Session history will be stored in %s and owned by root\n", histDir)
	}

	// Create Working directory
	if attach {
		// Validate /tmp exists
		if !util.CheckDirExists("/tmp") {
			util.ErrAndExit("/tmp directory does not exist")
		}
		// Create working directory in /tmp (0777 permissions)
		rc.WorkDir = filepath.Join("/tmp", tmpDirName)
		err := os.Mkdir(rc.WorkDir, dirPerms)
		util.CheckErrSprintf(err, "error creating workdir dir: %v", err)

		// Symbolic link between /tmp/tmpDirName and /history/tmpDirName
		rootHistDir := filepath.Join(histDir, tmpDirName)
		os.Symlink(rc.WorkDir, rootHistDir)
	} else {
		// Create working directory in history/
		rc.WorkDir = filepath.Join(HistoryDir(), tmpDirName)
		err = os.Mkdir(rc.WorkDir, 0755)
		util.CheckErrSprintf(err, "error creating workdir dir: %v", err)
	}

	// Create Cmd directory
	cmdDir := filepath.Join(rc.WorkDir, "cmd")
	err = os.Mkdir(cmdDir, dirPerms)
	util.CheckErrSprintf(err, "error creating cmd dir: %v", err)

	// Create Payloads directory
	payloadsDir := filepath.Join(rc.WorkDir, "payloads")
	err = os.MkdirAll(payloadsDir, dirPerms)
	util.CheckErrSprintf(err, "error creating payloads dir: %v", err)
}

func (rc *Config) populateWorkDir(args []string, attach bool) {
	filePerms := os.FileMode(0644)
	if attach {
		filePerms = 0777
		oldmask := syscall.Umask(0)
		defer syscall.Umask(oldmask)
	}

	// Create Log file
	internal.CreateLogFile(filepath.Join(rc.WorkDir, "appview.log"), filePerms)

	// Create Metrics file
	if rc.sc.Metric.Transport.TransportType == "file" {
		f, err := os.OpenFile(rc.sc.Metric.Transport.Path, os.O_CREATE, filePerms)
		if err != nil && !os.IsExist(err) {
			util.ErrAndExit("cannot create metric file %s: %v", rc.sc.Metric.Transport.Path, err)
		}
		f.Close()
	}

	// Create Events file
	if rc.sc.Event.Transport.TransportType == "file" {
		f, err := os.OpenFile(rc.sc.Event.Transport.Path, os.O_CREATE, filePerms)
		if err != nil && !os.IsExist(err) {
			util.ErrAndExit("cannot create metric file %s: %v", rc.sc.Event.Transport.Path, err)
		}
		f.Close()
	}

	// Create event_dest file
	err := os.WriteFile(filepath.Join(rc.WorkDir, "event_dest"), []byte(rc.buildEventsDest()), filePerms)
	util.CheckErrSprintf(err, "error writing event_dest: %v", err)

	// Create metrics_dest file
	err = os.WriteFile(filepath.Join(rc.WorkDir, "metric_dest"), []byte(rc.buildMetricsDest()), filePerms)
	util.CheckErrSprintf(err, "error writing metric_dest: %v", err)

	// Create metrics_format file
	if rc.MetricsFormat != "" {
		err = os.WriteFile(filepath.Join(rc.WorkDir, "metric_format"), []byte(rc.MetricsFormat), filePerms)
		util.CheckErrSprintf(err, "error writing metric_format: %v", err)
	}

	// Create config file
	scYamlPath := filepath.Join(rc.WorkDir, "appview.yml")
	if rc.UserConfig == "" {
		err := rc.WriteAppViewConfig(scYamlPath, filePerms)
		util.CheckErrSprintf(err, "%v", err)
	} else {
		input, err := os.ReadFile(rc.UserConfig)
		if err != nil {
			util.ErrAndExit("cannot read file %s: %v", rc.UserConfig, err)
		}
		if err = os.WriteFile(scYamlPath, input, 0644); err != nil {
			util.ErrAndExit("failed to write file to %s: %v", scYamlPath, err)
		}
	}

	// Create args.json file
	argsJSONPath := filepath.Join(rc.WorkDir, "args.json")
	if attach {
		// When we attach args[0] points to pid of desired process
		pid, _ := strconv.Atoi(args[0])
		cmdLine, _ := util.PidCmdline(rc.Rootdir, pid)
		args = strings.Split(cmdLine, " ")
	}
	argsBytes, err := json.Marshal(args)
	util.CheckErrSprintf(err, "error marshaling JSON: %v", err)
	err = os.WriteFile(argsJSONPath, argsBytes, filePerms)
}

// Open to race conditions, but having a duplicate ID is only a UX bug rather than breaking
// so keeping it simple and avoiding locking etc
func GetSessionID() string {
	countFile := filepath.Join(util.AppViewHome(), "count")
	lastSessionID := 0
	lastSessionBytes, err := os.ReadFile(countFile)
	if err == nil {
		lastSessionID, err = strconv.Atoi(string(lastSessionBytes))
		if err != nil {
			lastSessionID = 0
		}
	}
	sessionID := strconv.Itoa(lastSessionID + 1)
	_ = os.WriteFile(countFile, []byte(sessionID), 0644)
	return sessionID
}

// HistoryDir returns the history directory
func HistoryDir() string {
	return filepath.Join(util.AppViewHome(), "history")
}

func (rc *Config) buildMetricsDest() string {
	var dest string
	if rc.sc.Cribl.Enable == "true" {
		if rc.sc.Cribl.Transport.TransportType == "unix" || rc.sc.Cribl.Transport.TransportType == "file" {
			dest = rc.sc.Cribl.Transport.TransportType + "://" + rc.sc.Cribl.Transport.Path
		} else {
			dest = rc.sc.Cribl.Transport.TransportType + "://" + rc.sc.Cribl.Transport.Host + ":" + fmt.Sprint(rc.sc.Cribl.Transport.Port)
		}
	} else {
		if rc.sc.Metric.Transport.TransportType == "unix" || rc.sc.Metric.Transport.TransportType == "file" {
			dest = rc.sc.Metric.Transport.TransportType + "://" + rc.sc.Metric.Transport.Path
		} else {
			dest = rc.sc.Metric.Transport.TransportType + "://" + rc.sc.Metric.Transport.Host + ":" + fmt.Sprint(rc.sc.Metric.Transport.Port)
		}
	}
	return dest
}

func (rc *Config) buildEventsDest() string {
	var dest string
	if rc.sc.Cribl.Enable == "true" {
		if rc.sc.Cribl.Transport.TransportType == "unix" || rc.sc.Cribl.Transport.TransportType == "file" {
			dest = rc.sc.Cribl.Transport.TransportType + "://" + rc.sc.Cribl.Transport.Path
		} else {
			dest = rc.sc.Cribl.Transport.TransportType + "://" + rc.sc.Cribl.Transport.Host + ":" + fmt.Sprint(rc.sc.Cribl.Transport.Port)
		}
	} else {
		if rc.sc.Event.Transport.TransportType == "unix" || rc.sc.Event.Transport.TransportType == "file" {
			dest = rc.sc.Event.Transport.TransportType + "://" + rc.sc.Event.Transport.Path
		} else {
			dest = rc.sc.Event.Transport.TransportType + "://" + rc.sc.Event.Transport.Host + ":" + fmt.Sprint(rc.sc.Event.Transport.Port)
		}
	}
	return dest
}

func (rc *Config) CreateWorkDirBasic(cmd string) {
	dirPerms := os.FileMode(0755)
	if cmd == "rules" {
		dirPerms = 0777
		oldmask := syscall.Umask(0)
		defer syscall.Umask(oldmask)
	}

	// Directories named CMD_SESSIONID_PID_TIMESTAMP
	ts := strconv.FormatInt(time.Now().UTC().UnixNano(), 10)
	pid := strconv.Itoa(os.Getpid())
	sessionID := GetSessionID()
	tmpDirName := path.Base(cmd + "_" + sessionID + "_" + pid + "_" + ts)

	// Create History directory
	histDir := HistoryDir()
	err := os.MkdirAll(histDir, 0755)
	util.CheckErrSprintf(err, "error creating history dir: %v", err)

	// Sudo user warning
	if _, present := os.LookupEnv("SUDO_USER"); present {
		util.Warn("WARNING: Session logs will be stored in %s and owned by root\n", histDir)
	}

	// Create Working directory
	if cmd == "rules" {
		// Validate /tmp exists
		if !util.CheckDirExists("/tmp") {
			util.ErrAndExit("/tmp directory does not exist")
		}
		// Create working directory in /tmp (0777 permissions)
		rc.WorkDir = filepath.Join("/tmp", tmpDirName)
		err := os.Mkdir(rc.WorkDir, dirPerms)
		util.CheckErrSprintf(err, "error creating workdir dir: %v", err)

		// Symbolic link between /tmp/tmpDirName and /history/tmpDirName
		rootHistDir := filepath.Join(histDir, tmpDirName)
		os.Symlink(rc.WorkDir, rootHistDir)
	} else {
		// Create working directory in history/
		rc.WorkDir = filepath.Join(HistoryDir(), tmpDirName)
		err = os.Mkdir(rc.WorkDir, 0755)
		util.CheckErrSprintf(err, "error creating workdir dir: %v", err)
	}

	// Populate working directory
	// Create Log file
	filePerms := os.FileMode(0644)
	internal.CreateLogFile(filepath.Join(rc.WorkDir, "appview.log"), filePerms)
	internal.SetDebug()
}

// GetAppViewVerDir returns the directory path which will be used for handling the appview file
// /usr/lib/appview/<version>/
// or
// /tmp/appview/<version>/
func GetAppViewVerDir() (string, error) {
	version := internal.GetNormalizedVersion()
	var appviewVersionPath string

	// Check /usr/lib/appview only for official version
	if !internal.IsVersionDev() {
		appviewVersionPath = filepath.Join("/usr/lib/appview/", version)
		if _, err := os.Stat(appviewVersionPath); err != nil {
			if err := os.MkdirAll(appviewVersionPath, 0755); err != nil {
				return appviewVersionPath, err
			}
			err := os.Chmod(appviewVersionPath, 0755)
			return appviewVersionPath, err
		}
		return appviewVersionPath, nil
	}

	appviewVersionPath = filepath.Join("/tmp/appview/", version)
	if _, err := os.Stat(appviewVersionPath); err != nil {
		if err := os.MkdirAll(appviewVersionPath, 0777); err != nil {
			return appviewVersionPath, err
		}
		err := os.Chmod(appviewVersionPath, 0777)
		return appviewVersionPath, err
	}
	return appviewVersionPath, nil

}
