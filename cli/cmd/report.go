package cmd

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"strings"

	"github.com/appview-team/appview/report"
	"github.com/appview-team/appview/util"
	"github.com/spf13/cobra"
	"golang.org/x/crypto/ssh/terminal"
)

/* Args Matrix (X disallows)
 *         id
 * id      X
 */

// reportCmd represents the report command
var reportCmd = &cobra.Command{
	Use:   "report [flags]",
	Short: "Create a report from the appview session",
	Long:  `Using event and metric data from the specified session, this command will create a report on Network and File events.`,
	Example: `  appview report         # Create and display a report for the last session 
  appview report --id 2                # Report on a specific session ID
  appview report --json | jq           # Generate the report in JSON format and render with jq`,
	Args: cobra.NoArgs,
	Run: func(cmd *cobra.Command, args []string) {
		id, _ := cmd.Flags().GetInt("id")
		jsonOut, _ := cmd.Flags().GetBool("json")

		termWidth, _, err := terminal.GetSize(0)
		if err != nil {
			// If we cannot get the terminal size, we are dealing with redirected stdin
			// as opposed to an actual terminal, so we will assume terminal width is
			// 160, to show all columns.
			termWidth = 160
		}
		enc := json.NewEncoder(os.Stdout)

		sessions := sessionByID(id)

		// Open events.json file
		file, err := os.Open(sessions[0].EventsPath)
		if err != nil && strings.Contains(err.Error(), "events.json: no such file or directory") {
			if util.CheckFileExists(sessions[0].EventsDestPath) {
				dest, _ := ioutil.ReadFile(sessions[0].EventsDestPath)
				fmt.Printf("Cannot run report: Events were output to %s\n", dest)
				os.Exit(0)
			}
			promptClean(sessions[0:1])
		} else {
			util.CheckErrSprintf(err, "error opening events file: %v", err)
		}

		// Get unique file and network activity from events file
		netEntries, fileEntries, err := report.GetEntries(file)
		if err != nil {
			util.ErrAndExit("error getting file/network entries: %v", err)
		}

		// Print Network Activity
		netFields := []util.ObjField{
			{Name: "IP", Field: "ip"},
			{Name: "Port", Field: "port"},
			{Name: "URL", Field: "url"},
			{Name: "Duration", Field: "duration"},
			{Name: "Bytes Sent", Field: "bytes_sent"},
			{Name: "Bytes Received", Field: "bytes_received"},
		}
		if termWidth > 145 {
		}
		if jsonOut {
			for _, f := range netEntries {
				enc.Encode(f)
			}
		} else {
			fmt.Println("================\nNetwork Activity\n================")
			util.PrintObj(netFields, netEntries)
		}

		// Print File Activity
		fileFields := []util.ObjField{
			{Name: "File", Field: "file"},
			{Name: "Bytes Read", Field: "bytes_read"},
			{Name: "Bytes Written", Field: "bytes_written"},
		}
		if termWidth > 145 {
		}
		if jsonOut {
			for _, f := range fileEntries {
				enc.Encode(f)
			}
		} else {
			fmt.Println("================\nFile Activity\n================")
			util.PrintObj(fileFields, fileEntries)
		}

		// TODO Save the report on disk in session directory for diffing
		// For now, users can just redirect stdout like >> filename.txt
	},
}

func init() {
	reportCmd.Flags().IntP("id", "i", -1, "Report on a specific session ID")
	reportCmd.Flags().BoolP("json", "j", false, "Output as newline delimited JSON")
	RootCmd.AddCommand(reportCmd)
}
