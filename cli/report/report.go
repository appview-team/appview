package report

import (
	"errors"
	"fmt"
	"io"
	"strings"

	"github.com/criblio/scope/events"
	"github.com/criblio/scope/libscope"
	"github.com/criblio/scope/util"
	"github.com/mitchellh/mapstructure"
)

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

func GetEntries(eventsFile io.ReadSeeker) ([]netEntry, []fileEntry, error) {

	// Filter events to those of interest
	// Let's do net and fs events together so we only have to walk the event file once
	em := events.EventMatch{
		Sources: []string{"fs.open", "fs.close", "net.open", "net.close", "http.req", "http.resp"},
	}

	// Create an async event file reader
	in := make(chan libscope.EventBody)
	var readerr error
	go func() {
		err := em.Events(eventsFile, in)
		if err != nil {
			if strings.Contains(err.Error(), "Error searching for Offset: EOF") {
				err = errors.New("Empty event file.")
			}
			readerr = err
			close(in)
		}
	}()

	// Consume events from the reader and build a report
	// indexed by unique file path; or unique IP:Port
	fileReport := map[string]fileEntry{} // where index is the Filename
	netReport := map[string]netEntry{}   // where index is the IP:Port
	for evt := range in {
		if evt.SourceType == "fs" {
			fe := fileEntry{}
			err := mapstructure.Decode(evt.Data, &fe)
			if err != nil {
				util.ErrAndExit("error decoding fs event: %v", err)
			}
			// Clean the Data
			if fe.BytesWritten < 0 {
				fe.BytesWritten = 0
			}
			if fe.BytesRead < 0 {
				fe.BytesRead = 0
			}
			// Add to (or Update) Report
			if fe.File != "" {
				existing, exists := fileReport[fe.File]
				if !exists {
					fileReport[fe.File] = fe
				} else {
					fileReport[fe.File] = fileEntry{
						File:         fe.File,
						BytesWritten: existing.BytesWritten + fe.BytesWritten,
						BytesRead:    existing.BytesRead + fe.BytesRead,
					}
				}
			}
		} else if evt.SourceType == "net" || evt.SourceType == "http" { // Joins http and net events
			ne := netEntry{}
			err := mapstructure.Decode(evt.Data, &ne)
			if err != nil {
				util.ErrAndExit("error decoding net event: %v", err)
			}
			// Clean the Data
			if ne.URL == "" {
				ne.URL = "-"
			}
			if ne.BytesSent < 0 {
				ne.BytesSent = 0
			}
			if ne.BytesReceived < 0 {
				ne.BytesReceived = 0
			}
			if ne.Duration < 0 {
				ne.Duration = 0
			}
			// Add to (or Update) Report
			if ne.IP != "" {
				index := fmt.Sprintf("%s%d", ne.IP, ne.Port) // Because you could have multiple ports at one IP
				existing, exists := netReport[index]
				if !exists {
					netReport[index] = ne
				} else {
					url := ne.URL
					if existing.URL != "-" { // Don't overwrite a URL once present
						url = existing.URL
					}
					netReport[index] = netEntry{
						URL:           url,
						IP:            ne.IP,
						Port:          ne.Port,
						Duration:      existing.Duration + ne.Duration,
						BytesSent:     existing.BytesSent + ne.BytesSent,
						BytesReceived: existing.BytesReceived + ne.BytesReceived,
					}
				}
			}
		}
	}

	if readerr != nil {
		util.ErrAndExit("error: %v", readerr)
	}

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

	return netEntries, fileEntries, nil
}
