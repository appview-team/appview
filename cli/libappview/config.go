package libappview

import (
	"errors"
	"fmt"
)

type BoolString string

func (b BoolString) MarshalYAML() (interface{}, error) {
	switch BoolString(b) {
	case "":
		return false, nil
	case "false":
		return false, nil
	case "true":
		return true, nil
	default:
		return nil, errors.New(fmt.Sprintf("Boolean Marshal error: invalid input %s", string(b)))
	}
}

func (bit *BoolString) UnmarshalJSON(data []byte) error {
	asString := string(data)
	if asString == "1" || asString == "true" || asString == `"true"` {
		*bit = "true"
	} else if asString == "0" || asString == "false" || asString == `"false"` {
		*bit = "false"
	} else {
		return errors.New(fmt.Sprintf("Boolean unmarshal error: invalid input %s", asString))
	}
	return nil
}

// AppViewConfig represents our current running configuration
type AppViewConfig struct {
	Cribl    AppViewCriblConfig    `mapstructure:"cribl,omitempty" json:"cribl,omitempty" yaml:"cribl"`
	Metric   AppViewMetricConfig   `mapstructure:"metric,omitempty" json:"metric,omitempty" yaml:"metric,omitempty"`
	Event    AppViewEventConfig    `mapstructure:"event,omitempty" json:"event,omitempty" yaml:"event,omitempty"`
	Payload  AppViewPayloadConfig  `mapstructure:"payload,omitempty" json:"payload,omitempty" yaml:"payload,omitempty"`
	Libappview AppViewLibappViewConfig `mapstructure:"libappview" json:"libappview" yaml:"libappview"`
}

// AppViewCriblConfig represents how to output metrics
type AppViewCriblConfig struct {
	Enable    BoolString     `mapstructure:"enable" json:"enable" yaml:"enable"`
	Transport AppViewTransport `mapstructure:"transport" json:"transport" yaml:"transport"`
	AuthToken string         `mapstructure:"authtoken" json:"authtoken" yaml:"authtoken"`
}

// AppViewMetricConfig represents how to output metrics
type AppViewMetricConfig struct {
	Enable    BoolString               `mapstructure:"enable" json:"enable" yaml:"enable"`
	Format    AppViewOutputFormat        `mapstructure:"format" json:"format" yaml:"format"`
	Transport AppViewTransport           `mapstructure:"transport,omitempty" json:"transport,omitempty" yaml:"transport,omitempty"`
	Watch     []AppViewMetricWatchConfig `mapstructure:"watch" json:"watch" yaml:"watch"`
}

// AppViewEventConfig represents how to output events
type AppViewEventConfig struct {
	Enable    BoolString              `mapstructure:"enable" json:"enable" yaml:"enable"`
	Format    AppViewOutputFormat       `mapstructure:"format" json:"format" yaml:"format"`
	Transport AppViewTransport          `mapstructure:"transport,omitempty" json:"transport,omitempty" yaml:"transport,omitempty"`
	Watch     []AppViewEventWatchConfig `mapstructure:"watch" json:"watch" yaml:"watch"`
}

// AppViewPayloadConfig represents how to capture payloads
type AppViewPayloadConfig struct {
	Enable BoolString `mapstructure:"enable" json:"enable" yaml:"enable"`
	Dir    string     `mapstructure:"dir" json:"dir" yaml:"dir"`
}

// AppViewMetricWatchConfig represents a metric watch configuration
type AppViewMetricWatchConfig struct {
	WatchType string `mapstructure:"type" json:"type" yaml:"type"`
}

// AppViewEventWatchConfig represents a event watch configuration
type AppViewEventWatchConfig struct {
	WatchType   string     `mapstructure:"type" json:"type" yaml:"type"`
	Name        string     `mapstructure:"name" json:"name" yaml:"name"`
	Field       string     `mapstructure:"field,omitempty" json:"field,omitempty" yaml:"field,omitempty"`
	Value       string     `mapstructure:"value" json:"value" yaml:"value"`
	AllowBinary BoolString `mapstructure:"allowbinary,omitempty" json:"allowbinary,omitempty" yaml:"allowbinary,omitempty"`
}

// AppViewLibappViewConfig represents how to configure libappview
type AppViewLibappViewConfig struct {
	ConfigEvent   BoolString          `mapstructure:"configevent" json:"configevent" yaml:"configevent"`
	SummaryPeriod int                 `mapstructure:"summaryperiod" json:"summaryperiod" yaml:"summaryperiod"`
	CommandDir    string              `mapstructure:"commanddir" json:"commanddir" yaml:"commanddir"`
	Log           AppViewLogConfig      `mapstructure:"log" json:"log" yaml:"log"`
	Snapshot      AppViewSnapshotConfig `mapstructure:"snapshot" json:"snapshot" yaml:"snapshot"`
}

// AppViewLogConfig represents how to configure libappview logs
type AppViewLogConfig struct {
	Level     string         `mapstructure:"level" json:"level" yaml:"level"`
	Transport AppViewTransport `mapstructure:"transport" json:"transport" yaml:"transport"`
}

// AppViewOutputFormat represents how to output a data type
type AppViewOutputFormat struct {
	FormatType   string            `mapstructure:"type" json:"type" yaml:"type"`
	Statsdmaxlen int               `mapstructure:"statsdmaxlen,omitempty" json:"statsdmaxlen,omitempty" yaml:"statsdmaxlen,omitempty"`
	StatsdPrefix string            `mapstructure:"statsdprefix,omitempty" json:"statsdprefix,omitempty" yaml:"statsdprefix,omitempty"`
	Verbosity    int               `mapstructure:"verbosity,omitempty" json:"verbosity,omitempty" yaml:"verbosity,omitempty"`
	Tags         map[string]string `mapstructure:"tags,omitempty" json:"tags,omitempty" yaml:"tags,omitempty"`
}

// AppViewTransport represents how we transport data
type AppViewTransport struct {
	TransportType string    `mapstructure:"type" json:"type" yaml:"type"`
	Host          string    `mapstructure:"host,omitempty" json:"host,omitempty" yaml:"host,omitempty"`
	Port          string    `mapstructure:"port,omitempty" json:"port,omitempty" yaml:"port,omitempty"`
	Path          string    `mapstructure:"path,omitempty" json:"path,omitempty" yaml:"path,omitempty"`
	Buffering     string    `mapstructure:"buffering,omitempty" json:"buffering,omitempty" yaml:"buffering,omitempty"`
	Tls           TlsConfig `mapstructure:"tls" json:"tls" yaml:"tls"`
}

// AppViewSnapshotConfig represents snapshot configuration in libappview
type AppViewSnapshotConfig struct {
	Coredump  BoolString `mapstructure:"coredump" json:"coredump" yaml:"coredump"`
	Backtrace BoolString `mapstructure:"backtrace" json:"backtrace" yaml:"backtrace"`
}

// TlsConfig is used when
type TlsConfig struct {
	Enable         BoolString `mapstructure:"enable" json:"enable" yaml:"enable"`
	ValidateServer BoolString `mapstructure:"validateserver" json:"validateserver" yaml:"validateserver"`
	CaCertPath     string     `mapstructure:"cacertpath" json:"cacertpath" yaml:"cacertpath"`
}
