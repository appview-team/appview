package cmd

import (
	"errors"

	"github.com/appview-team/appview/internal"
	"github.com/appview-team/appview/util"
	"github.com/spf13/cobra"
)

/* Args Matrix (X disallows)
 */

// psCmd represents the ps command
var psCmd = &cobra.Command{
	Use:   "ps",
	Short: "List processes currently being viewed",
	Long:  `List processes currently being viewed.`,
	Example: `  appview ps
  appview ps --json
  appview ps --rootdir /path/to/host/root
  appview ps --rootdir /path/to/host/root/proc/<hostpid>/root`,
	Args: cobra.NoArgs,
	Run: func(cmd *cobra.Command, args []string) {
		internal.InitConfig()
		rootdir, _ := cmd.Flags().GetString("rootdir")

		// Nice message for non-adminstrators
		err := util.UserVerifyRootPerm()
		if errors.Is(err, util.ErrGetCurrentUser) {
			util.ErrAndExit("Unable to get current user: %v", err)
		}
		if errors.Is(err, util.ErrMissingAdmPriv) {
			util.Warn("INFO: Run as root (or via sudo) to see all viewed processes")
		}

		procs, err := util.ProcessesViewed(rootdir)
		if err != nil {
			util.ErrAndExit("Unable to retrieve viewed processes: %v", err)
		}
		util.PrintObj([]util.ObjField{
			{Name: "ID", Field: "id"},
			{Name: "Pid", Field: "pid"},
			{Name: "User", Field: "user"},
			{Name: "Command", Field: "command"},
		}, procs)
	},
}

func init() {
	psCmd.Flags().StringP("rootdir", "R", "", "Path to root filesystem of target namespace")
	psCmd.Flags().BoolP("json", "j", false, "Output as newline delimited JSON")
	RootCmd.AddCommand(psCmd)
}
