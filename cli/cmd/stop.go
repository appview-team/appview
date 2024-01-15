package cmd

import (
	"fmt"
	"os"

	"github.com/appview-team/appview/internal"
	"github.com/appview-team/appview/stop"
	"github.com/appview-team/appview/util"
	"github.com/spf13/cobra"
)

/* Args Matrix (X disallows)
 *                 force
 * force           -
 */

func getStopUsage() string {
	return `The following actions will be performed:
	- Removal of /etc/ld.so.preload contents
	- Removal of the rules file from /usr/lib/appview/appview_rules
	- Detach from all currently viewed processes

The command does not uninstall appview or libappview from /usr/lib/appview or /tmp/appview
or remove any service configurations`
}

// stopCmd represents the stop command
var stopCmd = &cobra.Command{
	Use:   "stop",
	Short: "Stop scoping all viewed processes and services",
	Long: `Stop scoping all processes and services in the current or target namespace.

` + getStopUsage(),
	Example: `  appview stop`,
	Args:    cobra.NoArgs,
	Run: func(cmd *cobra.Command, args []string) {
		internal.InitConfig()
		rc.Rootdir, _ = cmd.Flags().GetString("rootdir")

		force, _ := cmd.Flags().GetBool("force")
		if !force {
			fmt.Println(getStopUsage())
			fmt.Println("\nIf you wish to proceed, run again with the -f flag.")
			os.Exit(0)
		}
		if err := stop.Stop(rc); err != nil {
			util.ErrAndExit("Exiting due to stop failure: %v", err)
		}
	},
}

func init() {
	stopCmd.Flags().StringP("rootdir", "R", "", "Path to root filesystem of target namespace")
	stopCmd.Flags().BoolP("force", "f", false, "Use this flag when you're sure you want to run appview stop")
	RootCmd.AddCommand(stopCmd)
}
