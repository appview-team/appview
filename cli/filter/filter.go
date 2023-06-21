package filter

import (
	"errors"
	"io"
	"os"
	"path/filepath"

	"github.com/criblio/scope/libscope"
	"github.com/criblio/scope/loader"
	"github.com/criblio/scope/run"
	"github.com/criblio/scope/util"
	"github.com/rs/zerolog/log"
	"gopkg.in/yaml.v2"
)

var (
	errNoScopedProcs     = errors.New("no scoped processes found")
	errDetachingMultiple = errors.New("at least one error found when detaching from more than 1 process. See logs")
	errAttachingMultiple = errors.New("at least one error found when attaching to more than 1 process. See logs")
)

// Retrieve the filter file from the global location
func Retrieve(rootdir string) ([]byte, libscope.Filter, error) {
	filterFile := make(libscope.Filter, 0)

	filePath := "/usr/lib/appscope/scope_filter"
	if rootdir != "" {
		filePath = rootdir + filePath
	}

	file, err := os.Open(filePath)
	if err != nil {
		if os.IsNotExist(err) {
			// Return an empty filter file if it does not exist
			return []byte{}, filterFile, nil
		}
		return nil, nil, err
	}
	defer file.Close()

	data, err := io.ReadAll(file)
	if err != nil {
		return nil, nil, err
	}

	// Read yaml in
	err = yaml.Unmarshal(data, &filterFile)
	if err != nil {
		util.Warn("Error unmarshaling YAML:%v", err)
		return nil, nil, err
	}

	return data, filterFile, nil
}

// Add a process to the scope filter
func Add(filterFile libscope.Filter, addProc, procArg, sourceid, rootdir string, rc *run.Config) error {

	// Create a history directory for logs
	rc.CreateWorkDirBasic("filter")

	// Instantiate the loader
	ld := loader.New()

	////////////////////////////////////////////
	// Install libscope if not already installed
	////////////////////////////////////////////

	stdoutStderr, err := ld.Install(rootdir)
	if err != nil {
		log.Warn().
			Err(err).
			Str("loaderDetails", stdoutStderr).
			Msg("Install library failed.")
		return err
	} else {
		log.Info().
			Msg("Install library success.")
	}

	////////////////////////////////////////////
	// Update the global filter file
	////////////////////////////////////////////

	// Build the config
	rc.LoadConfig(true)

	// Add the entry to the filter
	newEntry := libscope.Entry{
		ProcName:      addProc,
		ProcArg:       procArg,
		SourceId:      sourceid,
		Configuration: rc.GetScopeConfig(),
	}
	filterFile["allow"] = append(filterFile["allow"], newEntry)

	// Write the filter contents to a temporary path
	filterFilePath := "/tmp/scope_filter"
	file, err := os.OpenFile(filterFilePath, os.O_WRONLY|os.O_CREATE|os.O_TRUNC, 0644)
	if err != nil {
		log.Warn().Err(err).Msgf("Error creating/opening %s", filterFilePath)
		return err
	}
	defer file.Close()

	data, err := yaml.Marshal(&filterFile)
	if err != nil {
		util.Warn("Error marshaling YAML:%v", err)
		return err
	}

	_, err = file.Write(data)
	if err != nil {
		util.Warn("Error writing to file:%v", err)
		return err
	}

	// Ask the loader to update the filter file
	if stdoutStderr, err := ld.Filter(filterFilePath, rc.Rootdir); err != nil {
		log.Warn().
			Err(err).
			Str("loaderDetails", stdoutStderr).
			Msgf("Install library in %s namespace failed.", rootdir)
		return err
	}

	////////////////////////////////////////////
	// Set ld.so.preload if it is not already set.
	////////////////////////////////////////////

	if stdoutStderr, err := ld.Preload("auto", rc.Rootdir); err != nil {
		log.Warn().
			Err(err).
			Str("loaderDetails", stdoutStderr).
			Msgf("Install library in %s namespace failed.", rootdir)
		return err
	}

	////////////////////////////////////////////
	// Attach to all matching processes that are not already scoped
	// (will re-attach to update existing procs)
	////////////////////////////////////////////

	// Create config file in the filter workdir for all processes to be attached
	scYamlPath := filepath.Join(rc.WorkDir, "scope.yml")
	if err := rc.WriteScopeConfig(scYamlPath, 0644); err != nil {
		return err
	}

	if rootdir != "" {
		util.Warn("It can take up to 1 minute to attach to processes in a parent namespace")
	}

	procs, err := util.HandleInputArg(addProc, procArg, rc.Rootdir, false, false, true, true)
	if err != nil {
		return err
	}
	// Note: if len(procs) == 0 do nothing
	if len(procs) == 1 {
		if err = rc.Attach(procs[0].Pid, false); err != nil {
			return err
		}
	} else if len(procs) > 1 {
		errors := false
		for _, proc := range procs {
			if err = rc.Attach(proc.Pid, false); err != nil {
				log.Error().Err(err)
				errors = true
			}
		}
		if errors {
			return errAttachingMultiple
		}
	}

	// TBC ? perform a scope detach to all processes that do not match anything in the filter file

	return nil
}

// Remove a process from the scope filter
// Note: No matching of the 'arg' field intended for removal.
func Remove(filterFile libscope.Filter, remProc, sourceid, rootdir string, rc *run.Config) error {

	// Create a history directory for logs
	rc.CreateWorkDirBasic("filter")

	// Instantiate the loader
	ld := loader.New()

	////////////////////////////////////////////
	// Update the global filter file
	////////////////////////////////////////////

	// Remove the entry from the filter
	// (only the last instance matching the name)
	// (only if there are changes)
	if allowList, ok := filterFile["allow"]; ok {
		remove := -1
		for i, entry := range allowList {
			if entry.ProcName == remProc && entry.SourceId == sourceid {
				remove = i
			}
		}
		if remove > -1 {
			allowList = append(allowList[:remove], allowList[remove+1:]...)
			filterFile["allow"] = allowList // copy back the allowList

			// Write the filter contents to a temporary path
			filterFilePath := "/tmp/scope_filter"
			file, err := os.OpenFile(filterFilePath, os.O_WRONLY|os.O_CREATE|os.O_TRUNC, 0644)
			if err != nil {
				log.Warn().Err(err).Msgf("Error creating/opening %s", filterFilePath)
				return err
			}
			defer file.Close()

			data, err := yaml.Marshal(&filterFile)
			if err != nil {
				util.Warn("Error marshaling YAML:%v", err)
				return err
			}

			_, err = file.Write(data)
			if err != nil {
				util.Warn("Error writing to file:%v", err)
				return err
			}

			// Ask the loader to update the filter file
			if stdoutStderr, err := ld.Filter(filterFilePath, rc.Rootdir); err != nil {
				log.Warn().
					Err(err).
					Str("loaderDetails", stdoutStderr).
					Msgf("Install library in %s namespace failed.", rootdir)
				return err
			}
		}
	}

	////////////////////////////////////////////
	// Unset ld.so.preload if it is already set.
	// Only if this is the last entry in the file
	////////////////////////////////////////////

	if len(filterFile["allow"]) == 0 {
		if stdoutStderr, err := ld.Preload("off", rc.Rootdir); err != nil {
			log.Warn().
				Err(err).
				Str("loaderDetails", stdoutStderr).
				Msgf("Install library in %s namespace failed.", rootdir)
			return err
		}
	}

	////////////////////////////////////////////
	// Detach from all matching, scoped processes
	////////////////////////////////////////////

	procs, err := util.HandleInputArg(remProc, "", rc.Rootdir, false, false, false, true)
	if err != nil {
		return err
	}
	// Note: if len(procs) == 0 do nothing
	if len(procs) == 1 {
		if err = rc.Detach(procs[0].Pid); err != nil {
			return err
		}
	} else if len(procs) > 1 {
		errors := false
		for _, proc := range procs {
			if err = rc.Detach(proc.Pid); err != nil {
				log.Error().Err(err)
				errors = true
			}
		}
		if errors {
			return errDetachingMultiple
		}
	}

	return nil
}