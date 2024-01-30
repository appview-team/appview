package cmd

import (
	"fmt"
	"io"
	"os"

	"github.com/appview-team/appview/util"
	"github.com/spf13/cobra"
)

// logsCmd represents the logs command
var logsCmd = &cobra.Command{
	Use:   "logs",
	Short: "Display appview logs",
	Long:  `Displays internal AppView logs for troubleshooting AppView itself.`,
	Example: `  appview logs
  appview logs -s`,
	Args: cobra.NoArgs,
	Run: func(cmd *cobra.Command, args []string) {
		id, _ := cmd.Flags().GetInt("id")
		lastN, _ := cmd.Flags().GetInt("last")
		appviewLog, _ := cmd.Flags().GetBool("appview")
		serviceLog, _ := cmd.Flags().GetString("service")

		var logPath string
		if len(serviceLog) > 0 {
			logPath = fmt.Sprintf("/var/log/appview/%s.log", serviceLog)
		} else {
			sessions := sessionByID(id)
			if appviewLog {
				logPath = sessions[0].AppViewLogPath
			} else {
				logPath = sessions[0].LibappviewLogPath
			}
		}

		// Open log file
		logFile, err := os.Open(logPath)
		if err != nil {
			if appviewLog {
				fmt.Println("No log file present for the CLI. If you want to see the library logs, try running without -s")
			} else {
				fmt.Println("No log file present for the library. If you want to see the CLI logs, try running with -s")
			}
			os.Exit(1)
		}

		offset, err := util.FindReverseLineMatchOffset(lastN, logFile, util.MatchAny())
		if offset < 0 {
			offset = 0
		}
		if err == io.EOF {
			util.ErrAndExit("No logs generated")
		}
		util.CheckErrSprintf(err, "error seeking last N offset: %v", err)
		_, err = logFile.Seek(offset, io.SeekStart)
		util.CheckErrSprintf(err, "error seeking log file: %v", err)
		util.NewlineReader(logFile, util.MatchAlways, func(idx int, Offset int64, b []byte) error {
			fmt.Printf("%s\n", b)
			return nil
		})
	},
}

func init() {
	logsCmd.Flags().IntP("id", "i", -1, "Display logs from specific from session ID")
	logsCmd.Flags().IntP("last", "n", 20, "Show last <n> lines")
	logsCmd.Flags().BoolP("appview", "s", false, "Show appview.log (from CLI) instead of libappview.log (from library)")
	logsCmd.Flags().StringP("service", "S", "", "Display logs from a Systemd service instead of a session")
	RootCmd.AddCommand(logsCmd)
}
