package cmd

import (
	"time"

	"github.com/appview-team/appview/internal"
	"github.com/appview-team/appview/util"
	"github.com/spf13/cobra"
)

// watchCmd represents the watch command
var watchCmd = &cobra.Command{
	Use:   "watch [flags]",
	Short: "Executes a viewed command on an interval",
	Long:  `Executes a viewed command on an interval. Must be called with the -- flag, e.g., 'appview watch -- <command>', to prevent AppView from attempting to parse flags passed to the executed command.`,
	Example: `  appview watch -i 5s -- /bin/echo "foo"
  appview watch --interval=1m-- perl -e 'print "foo\n"'
  appview watch --interval=5s --payloads -- nc -lp 10001
  appview watch -i 1h -- curl https://wttr.in/94105
  appview watch --interval=10s -- curl https://wttr.in/94105`,
	Args: cobra.MinimumNArgs(1),
	Run: func(cmd *cobra.Command, args []string) {
		interval, _ := cmd.Flags().GetString("interval")
		internal.InitConfig()
		rc.Subprocess = true
		dur, err := time.ParseDuration(interval)
		util.CheckErrSprintf(err, "error parsing time duration string \"%s\": %v", interval, err)
		timer := time.Tick(dur)
		rc.Run(args)
		for range timer {
			rc.Run(args)
		}
	},
}

func init() {
	runCmdFlags(watchCmd, rc)
	watchCmd.Flags().StringP("interval", "i", "", "Run every <x>(s|m|h)")
	watchCmd.MarkFlagRequired("interval")
	RootCmd.AddCommand(watchCmd)
}
