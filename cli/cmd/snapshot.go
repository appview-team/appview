package cmd

import (
	"errors"
	"strconv"

	"github.com/appview-team/appview/internal"
	"github.com/appview-team/appview/snapshot"
	"github.com/appview-team/appview/util"
	"github.com/spf13/cobra"
)

/* Args Matrix (X disallows)
 */

// snapshotCmd represents the ps command
var snapshotCmd = &cobra.Command{
	Use:   "snapshot [PID]",
	Short: "Create a snapshot for a process",
	Long:  `Create a snapshot for a process. Snapshot file/s will be created in /tmp/appview/[PID]/.`,
	Args:  cobra.ExactArgs(1),
	Run: func(cmd *cobra.Command, args []string) {
		internal.InitConfig()
		// Nice message for non-adminstrators
		err := util.UserVerifyRootPerm()
		if errors.Is(err, util.ErrGetCurrentUser) {
			util.ErrAndExit("Unable to get current user: %v", err)
		}
		if errors.Is(err, util.ErrMissingAdmPriv) {
			util.ErrAndExit(err.Error())
		}

		pid, err := strconv.Atoi(args[0])
		if err != nil {
			util.ErrAndExit("error parsing PID argument")
		}

		// Create a history directory for logs
		snapshot.CreateWorkDir("snapshot")

		err = snapshot.GenFiles(0, 0, uint32(pid), 0, 0, 0, "", "")
		if err != nil {
			util.ErrAndExit(err.Error())
		}
	},
}

func init() {
	RootCmd.AddCommand(snapshotCmd)
}
