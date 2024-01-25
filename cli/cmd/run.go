package cmd

import (
	"os"

	"github.com/appview-team/appview/internal"
	"github.com/appview-team/appview/run"
	"github.com/spf13/cobra"
)

/* Args Matrix (X disallows)
 *                 cribldest metricformat metricdest eventdest nobreaker authtoken verbosity payloads loglevel librarypath userconfig coredump backtrace
 * cribldest       -                      X          X                                                                     X
 * metricformat              -                                                                                             X
 * metricdest      X                      -                                                                                X
 * eventdest       X                                 -                                                                     X
 * nobreaker                                                   -                                                           X
 * authtoken                                                             -                                                 X
 * verbosity                                                                       -                                       X
 * payloads                                                                                  -                             X
 * loglevel                                                                                           -                    X
 * librarypath                                                                                                 -
 * userconfig      X         X            X          X         X         X         X         X        X                    -          X         X
 * coredump     																										   X		  -
 * backtrace  																											   X				    -
 */

var rc *run.Config = &run.Config{}

// runCmd represents the run command
var runCmd = &cobra.Command{
	Use:   "run [flags] [command]",
	Short: "Executes a viewed command",
	Long: `Executes a viewed command. By default, calling appview with no subcommands will run the executables you pass as arguments to appview. However, appview allows for additional arguments to be passed to run to capture payloads or to increase metrics' verbosity. Must be called with the -- flag, e.g., 'appview run -- <command>', to prevent AppView from attempting to parse flags passed to the executed command.

The --*dest flags accept file names like /tmp/appview.log; URLs like file:///tmp/appview.log; or sockets specified with the pattern unix:///var/run/mysock, tcp://hostname:port, udp://hostname:port, or tls://hostname:port.`,
	Example: `  appview run -- /bin/echo "foo"
  appview run -- perl -e 'print "foo\n"'
  appview run --payloads -- nc -lp 10001
  appview run -- curl https://wttr.in/94105
  appview run -c tcp://127.0.0.1:10091 -- curl https://wttr.in/94105
  appview run -c edge -- top`,
	Args: cobra.MinimumNArgs(1),
	Run: func(cmd *cobra.Command, args []string) {
		internal.InitConfig()

		// Disallow bad argument combinations (see Arg Matrix at top of file)
		if rc.CriblDest != "" && rc.MetricsDest != "" {
			helpErrAndExit(cmd, "Cannot specify --cribldest and --metricdest")
		} else if rc.CriblDest != "" && rc.EventsDest != "" {
			helpErrAndExit(cmd, "Cannot specify --cribldest and --eventdest")
		} else if rc.CriblDest != "" && rc.UserConfig != "" {
			helpErrAndExit(cmd, "Cannot specify --cribldest and --userconfig")
		} else if cmd.Flags().Lookup("metricformat").Changed && rc.UserConfig != "" {
			helpErrAndExit(cmd, "Cannot specify --metricformat and --userconfig")
		} else if rc.EventsDest != "" && rc.UserConfig != "" {
			helpErrAndExit(cmd, "Cannot specify --eventdest and --userconfig")
		} else if rc.NoBreaker && rc.UserConfig != "" {
			helpErrAndExit(cmd, "Cannot specify --nobreaker and --userconfig")
		} else if rc.AuthToken != "" && rc.UserConfig != "" {
			helpErrAndExit(cmd, "Cannot specify --authtoken and --userconfig")
		} else if cmd.Flags().Lookup("verbosity").Changed && rc.UserConfig != "" {
			helpErrAndExit(cmd, "Cannot specify --verbosity and --userconfig")
		} else if rc.Payloads && rc.UserConfig != "" {
			helpErrAndExit(cmd, "Cannot specify --payloads and --userconfig")
		} else if rc.Loglevel != "" && rc.UserConfig != "" {
			helpErrAndExit(cmd, "Cannot specify --loglevel and --userconfig")
		} else if rc.Backtrace && rc.UserConfig != "" {
			helpErrAndExit(cmd, "Cannot specify --backtrace and --userconfig")
		} else if rc.Coredump && rc.UserConfig != "" {
			helpErrAndExit(cmd, "Cannot specify --coredump and --userconfig")
		}

		rc.Run(args)
	},
}

func init() {
	runCmdFlags(runCmd, rc)
	// This may be a bad assumption, if we have any args preceding this it might fail
	runCmd.SetFlagErrorFunc(func(cmd *cobra.Command, err error) error {
		internal.InitConfig()
		runCmd.Run(cmd, os.Args[2:])
		return nil
	})
	RootCmd.AddCommand(runCmd)
}
