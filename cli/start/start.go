package start

import (
	"errors"
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
	"strconv"

	"github.com/criblio/scope/libscope"
	"github.com/criblio/scope/loader"
	"github.com/criblio/scope/run"
	"github.com/criblio/scope/util"
	"github.com/rs/zerolog/log"
	"gopkg.in/yaml.v2"
)

// startConfig represents a processed configuration
type startConfig struct {
	AllowProc []allowProcConfig `mapstructure:"allow,omitempty" json:"allow,omitempty" yaml:"allow"`
	DenyProc  []denyProcConfig  `mapstructure:"deny,omitempty" json:"deny,omitempty" yaml:"deny"`
}

// denyProcConfig represents a denied process configuration
type denyProcConfig struct {
	Procname string `mapstructure:"procname,omitempty" json:"procname,omitempty" yaml:"procname,omitempty"`
	Arg      string `mapstructure:"arg,omitempty" json:"arg,omitempty" yaml:"arg,omitempty"`
}

// allowProcConfig represents a allowed process configuration
type allowProcConfig struct {
	Procname string               `mapstructure:"procname,omitempty" json:"procname,omitempty" yaml:"procname,omitempty"`
	Arg      string               `mapstructure:"arg,omitempty" json:"arg,omitempty" yaml:"arg,omitempty"`
	Config   libscope.ScopeConfig `mapstructure:"config" json:"config" yaml:"config"`
}

var (
	errMissingData = errors.New("missing filter data")
)

// startAttachSingleProcess attach to the the specific process identifed by pid with configuration pass in cfgData.
// It returns the status of operation.
func startAttachSingleProcess(pid string, cfgData []byte) error {
	tmpFile, err := os.Create(fmt.Sprintf("/tmp/tmpAttachCfg_%s.yml", pid))
	if err != nil {
		log.Error().
			Err(err).
			Str("pid", pid).
			Msgf("Attach PID %v failed. Create temporary attach configuration failed.", pid)
		return err
	}
	defer os.Remove(tmpFile.Name())

	if _, err := tmpFile.Write(cfgData); err != nil {
		log.Error().
			Err(err).
			Str("pid", pid).
			Msgf("Attach PID %v failed. Write temporary attach configuration failed.", pid)
		return err
	}

	env := append(os.Environ(), "SCOPE_CONF_PATH="+tmpFile.Name())
	sL := loader.ScopeLoader{Path: run.LdscopePath()}
	stdoutStderr, err := sL.AttachSubProc([]string{pid}, env)
	if err != nil {
		log.Error().
			Err(err).
			Str("pid", pid).
			Str("loaderDetails", stdoutStderr).
			Msgf("Attach PID %v failed.", pid)
		return err
	} else {
		log.Info().
			Str("pid", pid).
			Msgf("Attach PID %v success.", pid)
	}

	os.Unsetenv("SCOPE_CONF_PATH")
	return nil
}

// for the host
// for all containers
// for all allowed proc
func startAttach(allowProcs []allowProcConfig) error {

	// Iterate over all allowed processses
	for _, process := range allowProcs {
		var pidsToAttach util.PidScopeMapState
		cfgSingleProc, err := yaml.Marshal(process.Config)
		if err != nil {
			log.Error().
				Err(err).
				Msg("Attach Failed. Serialize configuration failed.")
			return err
		}

		if process.Procname != "" {
			procsMap, err := util.PidScopeMapByProcessName(process.Procname)
			if err != nil {
				log.Error().
					Err(err).
					Str("process", process.Procname).
					Msgf("Attach Failed. Retrieve process name: %v failed.", process.Procname)
				return err
			}
			pidsToAttach = procsMap
		}
		if process.Arg != "" {
			procsMap, err := util.PidScopeMapByCmdLine(process.Arg)
			if err != nil {
				log.Error().
					Err(err).
					Str("process", process.Arg).
					Msgf("Attach Failed. Retrieve arg: %v failed.", process.Arg)
				return err
			}
			for k, v := range procsMap {
				pidsToAttach[k] = v
			}
		}

		for pid, scopeState := range pidsToAttach {
			if scopeState {
				log.Warn().
					Str("pid", strconv.Itoa(pid)).
					Msgf("Attach Failed. Process: %v is already scoped.", pid)
				continue
			}

			startAttachSingleProcess(strconv.Itoa(pid), cfgSingleProc)
		}
	}
	return nil
}

// for the host
func startConfigureHost(filename string) error {
	sL := loader.ScopeLoader{Path: run.LdscopePath()}
	stdoutStderr, err := sL.ConfigureHost(filename)
	if err != nil {
		log.Warn().
			Err(err).
			Str("loaderDetails", stdoutStderr).
			Msg("Setup host failed.")
		return err
	} else {
		log.Info().
			Msg("Setup host success.")
	}

	return nil
}

// for all containers
func startConfigureContainers(filename string) error {
	// Discover all containers
	cPids, err := util.GetDockerPids()
	if err != nil {
		switch {
		case errors.Is(err, util.ErrDockerNotAvailable):
			log.Warn().
				Msgf("Configure containers skipped. Docker is not available")
			return nil
		default:
			log.Error().
				Err(err).
				Msg("Configure containers failed.")
			return err
		}
	}

	ld := loader.ScopeLoader{Path: run.LdscopePath()}

	// Iterate over all containers
	for _, cPid := range cPids {
		stdoutStderr, err := ld.ConfigureContainer(filename, cPid)
		if err != nil {
			log.Warn().
				Err(err).
				Str("pid", strconv.Itoa(cPid)).
				Str("loaderDetails", stdoutStderr).
				Msgf("Configure containers failed. Configure container %v failed.", cPid)
		} else {
			log.Info().
				Str("pid", strconv.Itoa(cPid)).
				Msgf("Configure containers success. Configure container %v success.", cPid)
		}
	}

	return nil
}

// for the host
// for all allowed proc
func startServiceHost(allowProcs []allowProcConfig) error {
	sL := loader.ScopeLoader{Path: run.LdscopePath()}

	// Iterate over all allowed processses
	for _, process := range allowProcs {
		stdoutStderr, err := sL.ServiceHost(process.Procname)
		if err == nil {
			log.Info().
				Str("service", process.Procname).
				Msgf("Service %v host success.", process.Procname)
		} else if ee := (&exec.ExitError{}); errors.As(err, &ee) {
			if ee.ExitCode() == 1 {
				log.Warn().
					Err(err).
					Str("service", process.Procname).
					Str("loaderDetails", stdoutStderr).
					Msgf("Service %v host failed.", process.Procname)
			} else {
				log.Warn().
					Str("service", process.Procname).
					Str("loaderDetails", stdoutStderr).
					Msgf("Service %v host failed.", process.Procname)
			}
		}
	}
	return nil
}

// for all containers
// for all allowed proc
func startServiceContainers(allowProcs []allowProcConfig) error {
	// Discover all containers
	cPids, err := util.GetDockerPids()
	if err != nil {
		switch {
		case errors.Is(err, util.ErrDockerNotAvailable):
			log.Warn().
				Msgf("Setup container services skipped. Docker is not available")
			return nil
		default:
			log.Warn().
				Err(err).
				Msg("Setup container services failed.")
			return nil
		}
	}

	ld := loader.ScopeLoader{Path: run.LdscopePath()}

	// Iterate over all allowed processses
	for _, process := range allowProcs {
		// Iterate over all containers
		for _, cPid := range cPids {
			stdoutStderr, err := ld.ServiceContainer(process.Procname, cPid)
			if err == nil {
				log.Info().
					Str("service", process.Procname).
					Str("pid", strconv.Itoa(cPid)).
					Msgf("Setup containers. Service %v container %v success.", process.Procname, cPid)
			} else if ee := (&exec.ExitError{}); errors.As(err, &ee) {
				if ee.ExitCode() == 1 {
					log.Warn().
						Err(err).
						Str("service", process.Procname).
						Str("pid", strconv.Itoa(cPid)).
						Str("loaderDetails", stdoutStderr).
						Msgf("Setup containers failed. Service %v container %v failed.", process.Procname, cPid)
				} else {
					log.Warn().
						Str("service", process.Procname).
						Str("pid", strconv.Itoa(cPid)).
						Str("loaderDetails", stdoutStderr).
						Msgf("Setup containers failed. Service %v container %v failed.", process.Procname, cPid)
				}
			}
		}
	}
	return nil
}

// Start performs setup of scope in the host and containers
func Start(filename string) error {
	cfgData, err := ioutil.ReadFile(filename)
	if err != nil {
		log.Error().
			Err(err)
		return err
	}
	if len(cfgData) == 0 {
		log.Error().
			Err(err).
			Msg("Missing filter data.")
		return err
	}

	var startCfg startConfig
	if err = yaml.Unmarshal(cfgData, &startCfg); err != nil {
		log.Error().
			Err(err).
			Msg("Read of filter file failed.")
		return err
	}

	if err = run.CreateLdscope(); err != nil {
		log.Error().
			Err(err).
			Msg("Create ldscope failed.")
		return err
	}

	if err = startConfigureHost(filename); err != nil {
		return err
	}

	if err = startConfigureContainers(filename); err != nil {
		return err
	}

	if err = startServiceHost(startCfg.AllowProc); err != nil {
		return err
	}

	if err = startServiceContainers(startCfg.AllowProc); err != nil {
		return err
	}

	if err = startAttach(startCfg.AllowProc); err != nil {
		return err
	}

	// TODO: Deny list actions (Detach?/Deservice?)
	return nil
}