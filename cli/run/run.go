package run

import (
	"os"
	"path/filepath"
	"time"

	"github.com/appview-team/appview/libappview"
	"github.com/appview-team/appview/loader"
	"github.com/appview-team/appview/util"
)

// Config represents options to change how we run appview
type Config struct {
	WorkDir       string
	Verbosity     int
	Payloads      bool
	MetricsDest   string
	EventsDest    string
	MetricsFormat string
	MetricsPrefix string
	CriblDest     string
	Subprocess    bool
	Loglevel      string
	LogDest       string
	LibraryPath   string
	NoBreaker     bool
	AuthToken     string
	UserConfig    string
	CommandDir    string
	Coredump      bool
	Backtrace     bool
	Rootdir       string

	now func() time.Time
	sc  *libappview.AppViewConfig
}

// Run executes a viewed command
func (rc *Config) Run(args []string) {
	env := os.Environ()

	// Disable detection of a appview rules file with this command
	env = append(env, "APPVIEW_RULES=false")

	// Disable cribl event breaker with this command
	if rc.NoBreaker {
		env = append(env, "APPVIEW_CRIBL_NO_BREAKER=true")
	}

	// Normal operation, create a directory for this run.
	// Directory contains appview.yml which is configured to output to that
	// directory and has a command directory configured in that directory.
	rc.setupWorkDir(args, false)
	env = append(env, "APPVIEW_CONF_PATH="+filepath.Join(rc.WorkDir, "appview.yml"))

	// Handle custom library path
	if len(rc.LibraryPath) > 0 {
		if !util.CheckDirExists(rc.LibraryPath) {
			util.ErrAndExit("Library Path does not exist: \"%s\"", rc.LibraryPath)
		}
		args = append([]string{"-f", rc.LibraryPath}, args...)
	}

	ld := loader.New()
	if !rc.Subprocess {
		ld.Passthrough(args, env)
	}
	ld.PassthroughSubProc(args, env)
}
