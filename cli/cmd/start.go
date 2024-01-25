package cmd

import (
	"github.com/appview-team/appview/internal"
	"github.com/appview-team/appview/start"
	"github.com/appview-team/appview/util"
	"github.com/rs/zerolog/log"
	"github.com/spf13/cobra"
)

// startCmd represents the start command
var startCmd = &cobra.Command{
	Use:   "start",
	Short: "Install the AppView library",
	Long: `Install the AppView library to:
/usr/lib/appview/<version>/ with admin privileges, or 
/tmp/appview/<version>/ otherwise`,
	Example: `  appview start
  appview start --rootdir /hostfs`,
	Args: cobra.NoArgs,
	Run: func(cmd *cobra.Command, args []string) {
		internal.InitConfig()
		rootdir, _ := cmd.Flags().GetString("rootdir")

		// Validate user has root permissions
		if rootdir != "" {
			if err := util.UserVerifyRootPerm(); err != nil {
				log.Error().Err(err)
				util.ErrAndExit("appview start with the --rootdir argument requires administrator privileges")
			}
		}

		if err := start.Start(rootdir); err != nil {
			util.ErrAndExit("Exiting due to start failure: %v", err)
		}
	},
}

func init() {
	startCmd.Flags().StringP("rootdir", "R", "", "Path to root filesystem of target namespace")
	RootCmd.AddCommand(startCmd)
}
