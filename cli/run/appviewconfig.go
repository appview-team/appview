package run

import (
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"regexp"
	"strings"

	"github.com/appview-team/appview/libappview"
	"github.com/appview-team/appview/util"
	"gopkg.in/yaml.v2"
)

// SetDefault sets the default appview configuration as a struct
func (c *Config) SetDefault() error {
	if c.WorkDir == "" {
		return fmt.Errorf("workDir not set")
	}
	c.sc = &libappview.AppViewConfig{
		Cribl: libappview.AppViewCriblConfig{},
		Metric: libappview.AppViewMetricConfig{
			Enable: "true",
			Format: libappview.AppViewOutputFormat{
				FormatType: "ndjson",
				Verbosity:  4,
			},
			Transport: libappview.AppViewTransport{
				TransportType: "file",
				Path:          filepath.Join(c.WorkDir, "metrics.json"),
				Buffering:     "line",
			},
			Watch: []libappview.AppViewMetricWatchConfig{
				{
					WatchType: "fs",
				},
				{
					WatchType: "net",
				},
				{
					WatchType: "http",
				},
				{
					WatchType: "dns",
				},
				{
					WatchType: "process",
				},
				{
					WatchType: "statsd",
				},
			},
		},
		Event: libappview.AppViewEventConfig{
			Enable: "true",
			Format: libappview.AppViewOutputFormat{
				FormatType: "ndjson",
			},
			Transport: libappview.AppViewTransport{
				TransportType: "file",
				Path:          filepath.Join(c.WorkDir, "events.json"),
				Buffering:     "line",
			},
			Watch: []libappview.AppViewEventWatchConfig{
				{
					WatchType: "file",
					Name:      appviewLogRegex(),
					Value:     ".*",
				},
				{
					WatchType:   "console",
					Name:        "(stdout|stderr)",
					Value:       ".*",
					AllowBinary: "false",
				},
				{
					WatchType: "net",
					Name:      ".*",
					Field:     ".*",
					Value:     ".*",
				},
				{
					WatchType: "fs",
					Name:      ".*",
					Field:     ".*",
					Value:     ".*",
				},
				{
					WatchType: "dns",
					Name:      ".*",
					Field:     ".*",
					Value:     ".*",
				},
				{
					WatchType: "http",
					Name:      ".*",
					Field:     ".*",
					Value:     ".*",
				},
				// {
				// 	WatchType: "metric",
				// 	Name:      ".*",
				// 	Value:     ".*",
				// },
			},
		},
		Libappview: libappview.AppViewLibappViewConfig{
			SummaryPeriod: 10,
			CommandDir:    filepath.Join(c.WorkDir, "cmd"),
			ConfigEvent:   "true",
			Log: libappview.AppViewLogConfig{
				Level: "warning",
				Transport: libappview.AppViewTransport{
					TransportType: "file",
					Path:          filepath.Join(c.WorkDir, "libappview.log"),
					Buffering:     "line",
				},
			},
			Snapshot: libappview.AppViewSnapshotConfig{
				Coredump:  "false",
				Backtrace: "false",
			},
		},
	}

	return nil
}

// ConfigFromStdin loads a configuration from yml passed to stdin
func (c *Config) ConfigFromStdin(cfgData []byte) error {
	c.sc = &libappview.AppViewConfig{}

	if err := yaml.Unmarshal(cfgData, c.sc); err != nil {
		return err
	}

	// Update paths to absolute for file transports
	if c.sc.Metric.Transport.TransportType == "file" && c.sc.Metric.Transport.Path != "stdout" && c.sc.Metric.Transport.Path != "stderr" {
		newPath, err := filepath.Abs(c.sc.Metric.Transport.Path)
		util.CheckErrSprintf(err, "error getting absolute path for %s: %v", c.sc.Metric.Transport.Path, err)
		c.sc.Metric.Transport.Path = newPath
	}
	if c.sc.Event.Transport.TransportType == "file" && c.sc.Event.Transport.Path != "stdout" && c.sc.Event.Transport.Path != "stderr" {
		newPath, err := filepath.Abs(c.sc.Event.Transport.Path)
		util.CheckErrSprintf(err, "error getting absolute path for %s: %v", c.sc.Event.Transport.Path, err)
		c.sc.Event.Transport.Path = newPath
	}

	return nil
}

// ConfigFromFile loads a configuration from a yml file
func (c *Config) ConfigFromFile() error {
	c.sc = &libappview.AppViewConfig{}

	yamlFile, err := os.ReadFile(c.UserConfig)
	if err != nil {
		return err
	}
	if err = yaml.Unmarshal(yamlFile, c.sc); err != nil {
		return err
	}

	// Update paths to absolute for file transports
	if c.sc.Metric.Transport.TransportType == "file" && c.sc.Metric.Transport.Path != "stdout" && c.sc.Metric.Transport.Path != "stderr" {
		newPath, err := filepath.Abs(c.sc.Metric.Transport.Path)
		util.CheckErrSprintf(err, "error getting absolute path for %s: %v", c.sc.Metric.Transport.Path, err)
		c.sc.Metric.Transport.Path = newPath
	}
	if c.sc.Event.Transport.TransportType == "file" && c.sc.Event.Transport.Path != "stdout" && c.sc.Event.Transport.Path != "stderr" {
		newPath, err := filepath.Abs(c.sc.Event.Transport.Path)
		util.CheckErrSprintf(err, "error getting absolute path for %s: %v", c.sc.Event.Transport.Path, err)
		c.sc.Event.Transport.Path = newPath
	}

	return nil
}

// configFromRunOpts creates a configuration from run options
func (c *Config) configFromRunOpts() error {
	err := c.SetDefault()
	if err != nil {
		return err
	}

	if c.Verbosity != 0 {
		c.sc.Metric.Format.Verbosity = c.Verbosity
	}

	if c.Payloads {
		c.sc.Payload = libappview.AppViewPayloadConfig{
			Enable: "true",
			Dir:    filepath.Join(c.WorkDir, "payloads"),
		}
	}

	if c.MetricsFormat != "" {
		if c.MetricsFormat != "ndjson" && c.MetricsFormat != "statsd" {
			return fmt.Errorf("invalid metrics format %s", c.MetricsFormat)
		}
		c.sc.Metric.Format.FormatType = c.MetricsFormat
	}

	if c.MetricsPrefix != "" {
		c.sc.Metric.Format.StatsdPrefix = c.MetricsPrefix
	}

	parseDest := func(t *libappview.AppViewTransport, dest string) error {
		//
		// The regexp matches "proto://something:port" where the leading
		// "proto://" is optional. If given, "proto" must be "tcp", "udp",
		// "tls", "file" or "unix". The ":port" suffix is optional and
		// ignored for files and UNIX domain sockets but required for network sockets.
		//
		//     "relative/path"
		//     "edge"
		//     "file://another/relative/path"
		//     "/absolute/path"
		//     "file:///another/absolute/path"
		//     "unix:///socketpath"
		//     "unix://@abstractsocket"
		//     "tcp://host:port"
		//     "udp://host:port"
		//     "tls://host:port"
		//     "host:port" (assume tls)
		//
		// m[0][0] is the full match
		// m[0][1] is the "proto" string or an empty string
		// m[0][2] is the "something" string
		// m[0][3] is the "port" string or an empty string
		//
		m := regexp.MustCompile("^(?:(?i)(file|unix|tcp|udp|tls)://)?([^:]+)(?::(\\d+))?$").FindAllStringSubmatch(dest, -1)
		//fmt.Printf("debug: m=%+v\n", m)
		if len(m) <= 0 {
			// no match
			return fmt.Errorf("Invalid dest: %s", dest)
		} else if m[0][1] != "" {
			// got "proto://..."
			proto := strings.ToLower(m[0][1])
			if proto == "udp" || proto == "tcp" {
				// clear socket
				t.TransportType = proto
				t.Host = m[0][2]
				if m[0][3] == "" {
					return fmt.Errorf("Missing :port at the end of %s", dest)
				}
				t.Port = m[0][3]
				t.Path = ""
				t.Tls.Enable = "false"
				t.Tls.ValidateServer = "true"
			} else if proto == "tls" {
				// encrypted socket
				t.TransportType = "tcp"
				t.Host = m[0][2]
				if m[0][3] == "" {
					return fmt.Errorf("Missing :port at the end of %s", dest)
				}
				t.Port = m[0][3]
				t.Path = ""
				t.Tls.Enable = "true"
				t.Tls.ValidateServer = "true"
			} else if proto == "file" || proto == "unix" {
				t.TransportType = proto
				t.Path = m[0][2]
			} else {
				return fmt.Errorf("Invalid dest protocol: %s", proto)
			}
		} else if m[0][3] != "" {
			// got "something:port", assume tls://
			t.TransportType = "tcp"
			t.Host = m[0][2]
			t.Port = m[0][3]
			t.Path = ""
			t.Tls.Enable = "true"
			t.Tls.ValidateServer = "true"
		} else if m[0][0] == "edge" {
			t.TransportType = m[0][0]
		} else {
			// got "something", assume file://
			t.TransportType = "file"
			t.Path = m[0][2]
		}
		//fmt.Printf("debug: t=%+v\n", t)
		return nil // success
	}

	if c.MetricsDest != "" {
		err := parseDest(&c.sc.Metric.Transport, c.MetricsDest)
		if err != nil {
			return err
		}
	}

	if c.EventsDest != "" {
		err := parseDest(&c.sc.Event.Transport, c.EventsDest)
		if err != nil {
			return err
		}
	}

	if c.LogDest != "" {
		err := parseDest(&c.sc.Libappview.Log.Transport, c.LogDest)
		if err != nil {
			return err
		}
	}

	if c.CommandDir != "" {
		c.sc.Libappview.CommandDir = c.CommandDir
	}

	// Add AuthToken to config regardless of cribldest being set
	// To support mixing of config and environment variables
	c.sc.Cribl.AuthToken = c.AuthToken

	if c.Backtrace {
		c.sc.Libappview.Snapshot.Backtrace = "true"
	}
	if c.Coredump {
		c.sc.Libappview.Snapshot.Coredump = "true"
	}

	if c.CriblDest != "" {
		err := parseDest(&c.sc.Cribl.Transport, c.CriblDest)
		if err != nil {
			return err
		}
		c.sc.Cribl.Enable = "true"
		// If we're outputting to Cribl, disable metrics and event outputs
		c.sc.Metric.Transport = libappview.AppViewTransport{}
		c.sc.Event.Transport = libappview.AppViewTransport{}
		c.sc.Libappview.ConfigEvent = "true"
	}

	if c.Loglevel != "" {
		levels := map[string]bool{
			"error":   true,
			"warning": true,
			"info":    true,
			"debug":   true,
			"none":    true,
		}
		if _, ok := levels[c.Loglevel]; !ok {
			return fmt.Errorf("%s is an invalid log level. Log level must be one of debug, info, warning, error, or none", c.Loglevel)
		}
		c.sc.Libappview.Log.Level = c.Loglevel
	}

	// Update paths to absolute for file transports
	if c.sc.Metric.Transport.TransportType == "file" && c.sc.Metric.Transport.Path != "stdout" && c.sc.Metric.Transport.Path != "stderr" {
		newPath, err := filepath.Abs(c.sc.Metric.Transport.Path)
		util.CheckErrSprintf(err, "error getting absolute path for %s: %v", c.sc.Metric.Transport.Path, err)
		c.sc.Metric.Transport.Path = newPath
	}
	if c.sc.Event.Transport.TransportType == "file" && c.sc.Event.Transport.Path != "stdout" && c.sc.Event.Transport.Path != "stderr" {
		newPath, err := filepath.Abs(c.sc.Event.Transport.Path)
		util.CheckErrSprintf(err, "error getting absolute path for %s: %v", c.sc.Event.Transport.Path, err)
		c.sc.Event.Transport.Path = newPath
	}

	return nil
}

// AppViewConfigYaml serializes Config to YAML
func (c *Config) AppViewConfigYaml() ([]byte, error) {
	if c.sc == nil {
		err := c.configFromRunOpts()
		if err != nil {
			return []byte{}, err
		}
	}
	scb, err := yaml.Marshal(c.sc)
	if err != nil {
		return scb, fmt.Errorf("error marshaling's AppViewConfig to YAML: %v", err)
	}
	return scb, nil
}

// WriteAppViewConfig writes a appview config to a file
func (c *Config) WriteAppViewConfig(path string, filePerms os.FileMode) error {
	scb, err := c.AppViewConfigYaml()
	if err != nil {
		return err
	}
	err = ioutil.WriteFile(path, scb, filePerms)
	if err != nil {
		return fmt.Errorf("error writing AppViewConfig to file %s: %v", path, err)
	}
	return nil
}

// GetAppViewConfig returns a copy of the current configuration
func (c *Config) GetAppViewConfig() libappview.AppViewConfig {
	if c.sc == nil {
		return libappview.AppViewConfig{}
	}
	return *c.sc
}

func appviewLogRegex() string {
	// see appviewconfig_test.go for example paths that match
	return `(\/logs?\/)|(\.log$)|(\.log[.\d])`
}