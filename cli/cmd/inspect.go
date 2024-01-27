package cmd

import (
	"encoding/json"
	"errors"
	"fmt"

	"github.com/appview-team/appview/inspect"
	"github.com/appview-team/appview/internal"
	"github.com/appview-team/appview/ipc"
	"github.com/appview-team/appview/util"
	"github.com/rs/zerolog/log"
	"github.com/spf13/cobra"
)

var (
	errInspectingMultiple = errors.New("at least one error found when inspecting more than 1 process. See logs")
)

// inspectCmd represents the inspect command
var inspectCmd = &cobra.Command{
	Use:   "inspect",
	Short: "Returns information about viewed process",
	Long:  `Returns information about viewed process identified by PID.`,
	Example: `  appview inspect
  appview inspect 1000
  appview inspect --all --json
  appview inspect 1000 --rootdir /path/to/host/root
  appview inspect --all --rootdir /path/to/host/root
  appview inspect --all --rootdir /path/to/host/root/proc/<hostpid>/root`,
	Args: cobra.MaximumNArgs(1),
	Run: func(cmd *cobra.Command, args []string) {
		internal.InitConfig()
		all, _ := cmd.Flags().GetBool("all")
		rootdir, _ := cmd.Flags().GetString("rootdir")
		jsonOut, _ := cmd.Flags().GetBool("json")

		// Nice message for non-adminstrators
		err := util.UserVerifyRootPerm()
		if errors.Is(err, util.ErrGetCurrentUser) {
			util.ErrAndExit("Unable to get current user: %v", err)
		}

		if all && len(args) > 0 {
			helpErrAndExit(cmd, "--all flag is mutually exclusive with PID or <process_name>")
		}

		pidCtx := &ipc.IpcPidCtx{
			PrefixPath: rootdir,
		}
		iouts := make([]inspect.InspectOutput, 0)

		id := ""
		if len(args) > 0 {
			id = args[0]
		}

		procs, err := util.HandleInputArg(id, "", rootdir, !all, false, false, false)
		if err != nil {
			util.ErrAndExit("Inspect failure: %v", err)
		}

		if len(procs) == 0 {
			util.ErrAndExit("Inspect failure: %v", errNoViewedProcs)
		}
		if len(procs) == 1 {
			pidCtx.Pid = procs[0].Pid
			iout, _, err := inspect.InspectProcess(*pidCtx)
			if err != nil {
				util.ErrAndExit("Inspect failure: %v", err)
			}
			iouts = append(iouts, iout)
		} else { // len(procs) > 1
			errors := false
			for _, proc := range procs {
				pidCtx.Pid = proc.Pid
				iout, _, err := inspect.InspectProcess(*pidCtx)
				if err != nil {
					log.Error().Err(err)
					errors = true
				}
				iouts = append(iouts, iout)
			}
			if errors {
				util.ErrAndExit("Inspect failure: %v", errInspectingMultiple)
			}
		}

		if jsonOut {
			// Print each json entry on a newline, without any pretty printing
			for _, iout := range iouts {
				cfg, err := json.Marshal(iout)
				if err != nil {
					util.ErrAndExit("Error creating json object: %v", err)
				}
				fmt.Println(string(cfg))
			}
		} else {
			var cfgs []byte
			// Print the array, in a pretty format
			if len(iouts) == 1 {
				// No need to create an array
				cfgs, err = json.MarshalIndent(iouts[0], "", "   ")
			} else {
				cfgs, err = json.MarshalIndent(iouts, "", "   ")
			}
			if err != nil {
				util.ErrAndExit("Error creating json array: %v", err)
			}
			fmt.Println(string(cfgs))
		}
	},
}

func init() {
	inspectCmd.Flags().StringP("rootdir", "R", "", "Path to root filesystem of target namespace")
	inspectCmd.Flags().BoolP("json", "j", false, "Output as newline delimited JSON")
	inspectCmd.Flags().BoolP("all", "a", false, "Inspect all processes")
	RootCmd.AddCommand(inspectCmd)
}
