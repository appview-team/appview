package cmd

import (
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"os"
	"strings"

	"github.com/criblio/scope/events"
	"github.com/criblio/scope/libscope"
	"github.com/criblio/scope/util"
	"github.com/mitchellh/mapstructure"
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
	Short: "Create a report from the scope session",
	Long:  `Using event and metric data from the specified session, this command will create a report on Network and File events.`,
	Example: `  scope report         # Create and display a report for the last session 
  scope report --id 2                # Report on a specific session ID
//  scope report --pdf                # Generate the report in PDF format
  scope report --json                # Generate the report in JSON format`,
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

		type netEntry struct {
			URL           string `json:"url"            mapstructure:"http_host"`
			IP            string `json:"ip"             mapstructure:"net_peer_ip"`
			Port          int    `json:"port"           mapstructure:"net_peer_port"`
			Duration      int    `json:"duration"       mapstructure:"duration"`
			BytesSent     int    `json:"bytes_sent"     mapstructure:"net_bytes_sent"`
			BytesReceived int    `json:"bytes_received" mapstructure:"net_bytes_recv"`
		}
		type fileEntry struct {
			File         string `json:"file"           mapstructure:"file"`
			BytesRead    int    `json:"bytes_read"     mapstructure:"file_read_bytes"`
			BytesWritten int    `json:"bytes_written"  mapstructure:"file_write_bytes"`
		}

		/*
			fm, err := report.GetNetEntries(sessions[0].PayloadsPath, file)
			util.CheckErrSprintf(err, "error getting net entries: %v", err)
			flowsList := fm.List()
		*/

		// loop through all events

		// Filter events to those of interest
		// Let's do net and fs events together so we only have to walk the event file once
		em := events.EventMatch{
			Sources: []string{"fs.open", "fs.close", "net.open", "net.close", "http.req", "http.resp"},
		}
		// TODO add http events http.req http.resp

		// Create an async event file reader
		in := make(chan libscope.EventBody)
		var readerr error
		go func() {
			err := em.Events(file, in)
			if err != nil {
				if strings.Contains(err.Error(), "Error searching for Offset: EOF") {
					err = errors.New("Empty event file.")
				}
				readerr = err
				close(in)
			}
		}()

		// TODO add values instead of replacing

		// Consume events from the reader and build a report
		// indexed by unique file path
		fileReport := map[string]fileEntry{} // where index is the Filename
		netReport := map[string]netEntry{}   // where index is the IP
		for evt := range in {
			fmt.Println(evt.SourceType)
			if evt.SourceType == "file" {
				fe := fileEntry{}
				err := mapstructure.Decode(evt.Data, &fe)
				if err != nil {
					util.ErrAndExit("error decoding fs event: %v", err)
				}
				if fe.File != "" {
					fileReport[fe.File] = fe
				}
			} else if evt.SourceType == "net" || evt.SourceType == "http" { // TODO or http Join http and net events
				ne := netEntry{}
				err := mapstructure.Decode(evt.Data, &ne)
				if err != nil {
					util.ErrAndExit("error decoding net event: %v", err)
				}
				if ne.IP != "" {
					netReport[ne.IP] = ne
				}
			}
		}

		if readerr != nil {
			util.ErrAndExit("error: %v", err)
			//	return ret, readerr
		}
		/*
			return ret, nil
		*/

		// gather unique connections

		// gather unique filenames

		// Convert netReport map to netEntries slice
		netEntries := make([]netEntry, 0, len(netReport))
		for _, value := range netReport {
			netEntries = append(netEntries, value)
		}

		// Convert fileReport map to fileEntries slice
		fileEntries := make([]fileEntry, 0, len(fileReport))
		for _, value := range fileReport {
			fileEntries = append(fileEntries, value)
		}

		// Print Network Activity
		netFields := []util.ObjField{
			{Name: "URL", Field: "url"},
			{Name: "IP", Field: "ip"},
			{Name: "Port", Field: "port"},
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

		// TODO Save the report on disk for diffing
	},
}

func init() {
	reportCmd.Flags().IntP("id", "i", -1, "Report on a specific session ID")
	reportCmd.Flags().BoolP("json", "j", false, "Output as newline delimited JSON")
	RootCmd.AddCommand(reportCmd)
}
