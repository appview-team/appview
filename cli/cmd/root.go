package cmd

import (
	"fmt"
	"os"
	"strings"

	"github.com/appview-team/appview/internal"
	"github.com/appview-team/appview/run"
	"github.com/spf13/cobra"
)

// RootCmd represents the base command when called without any subcommands
var RootCmd = &cobra.Command{
	Use:   "appview",
	Short: "AppView\n\nGain observability into any Linux command or application with no code modification.\n\nRunning `appview` with no subcommands will execute the `appview run` command.",
	Example: `  appview curl wttr.in
  appview events
  appview metrics
  appview flows
  appview run -- git pull
  appview attach top
  appview rules --add nginx`,
}

// Execute adds all child commands to the root command and sets flags appropriately.
// This is called by main.main(). It only needs to happen once to the rootCmd.
func Execute() {
	RootCmd.SilenceErrors = true
	if err := RootCmd.Execute(); err != nil {
		if strings.HasPrefix(err.Error(), "unknown command") {
			for _, cmd := range RootCmd.Commands() {
				if os.Args[1] == cmd.Name() || cmd.HasAlias(os.Args[1]) {
					cmd.PrintErrln("Error:", err.Error())
					cmd.PrintErrf("Run '%v --help' for usage.\n", cmd.CommandPath())
					os.Exit(1)
				}
			}

			// If not a known command, appview run by default
			internal.InitConfig()
			rc := run.Config{}
			rc.Run(os.Args[1:])
		}
		fmt.Println(err)
		os.Exit(1)
	}
}

func init() {
	// Constructor flags (for help only)
	RootCmd.Flags().BoolP("passthrough", "z", false, "AppView an application with current environment & no config.")
}
