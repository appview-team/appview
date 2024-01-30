package cmd

import (
	"errors"
	"fmt"
	"time"

	"github.com/appview-team/appview/internal"
	"github.com/appview-team/appview/util"
	"github.com/rs/zerolog/log"
	"github.com/spf13/cobra"
)

var (
	errNoViewedProcs     = errors.New("no viewed processes found")
	errDetachingMultiple = errors.New("at least one error found when detaching from more than 1 process. See logs")
)

/* Args Matrix (X disallows)
 *             all
 * all          -
 */

// detachCmd represents the detach command
var detachCmd = &cobra.Command{
	Use:   "detach [flags] PID | <process_name>",
	Short: "Unview a currently-running process",
	Long:  `Unviews a currently-running process identified by PID or <process_name>.`,
	Example: `  appview detach 1000
  appview detach firefox
  appview detach --all
  appview detach 1000 --rootdir /path/to/host/root
  appview detach --rootdir /path/to/host/root
  appview detach --all --rootdir /path/to/host/root/proc/<hostpid>/root`,
	Args: cobra.MaximumNArgs(1),
	Run: func(cmd *cobra.Command, args []string) {
		internal.InitConfig()
		all, _ := cmd.Flags().GetBool("all")
		wait, _ := cmd.Flags().GetBool("wait")
		rc.Rootdir, _ = cmd.Flags().GetString("rootdir")

		if all && len(args) > 0 {
			helpErrAndExit(cmd, "--all flag is mutually exclusive with PID or <process_name>")
		}

		id := ""
		if len(args) > 0 {
			id = args[0]
		}

		procs, err := util.HandleInputArg(id, "", rc.Rootdir, !all, true, false, false)
		if err != nil {
			util.ErrAndExit("Detach failure: %v", err)
		}

		if len(procs) == 0 {
			util.ErrAndExit("Detach failure: %v", errNoViewedProcs)
		}
		if len(procs) == 1 {
			if err = rc.Detach(procs[0].Pid); err != nil {
				util.ErrAndExit("Detach failure: %v", err)
			}
			// Simple approach for now
			// Later: check for detach then resume
			if wait {
				time.Sleep(11 * time.Second) // detach uses dynamic config (<10s)
			}
			return
		}
		// len(procs) is > 1
		if !util.Confirm(fmt.Sprintf("Are your sure you want to detach from all of these processes?")) {
			fmt.Println("info: canceled")
			return
		}

		errors := false
		for _, proc := range procs {
			if err = rc.Detach(proc.Pid); err != nil {
				log.Error().Err(err)
				errors = true
			}
		}
		if errors {
			util.ErrAndExit("Detach failure: %v", errDetachingMultiple)
		}

		// Simple approach for now
		// Later: check for detach then resume
		if wait {
			time.Sleep(11 * time.Second) // detach uses dynamic config (<10s)
		}
	},
}

func init() {
	detachCmd.Flags().BoolP("all", "a", false, "Detach from all processes")
	detachCmd.Flags().BoolP("wait", "w", false, "Wait for detach to complete")
	detachCmd.Flags().StringP("rootdir", "R", "", "Path to root filesystem of target namespace")
	RootCmd.AddCommand(detachCmd)
}
