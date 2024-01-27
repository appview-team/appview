package update

import (
	"errors"
	"io"
	"os"

	"github.com/appview-team/appview/ipc"
	"github.com/appview-team/appview/libappview"
	"github.com/rs/zerolog/log"
	"gopkg.in/yaml.v2"
)

var errSettingCfg = errors.New("error setting cfg")
var errWaitingInput = errors.New("error waiting for input")
var errReadingStdIn = errors.New("error reading standard input")
var errReadingData = errors.New("error reading data")

// GetCfgStdIn reads a AppView Config file from StdIn
func GetCfgStdIn() (libappview.AppViewConfig, error) {
	var cfg libappview.AppViewConfig

	stdinFs, err := os.Stdin.Stat()
	if err != nil {
		return cfg, errReadingStdIn
	}

	// Avoid waiting for input from terminal when no data was provided
	if stdinFs.Mode()&os.ModeCharDevice != 0 && stdinFs.Size() == 0 {
		return cfg, errWaitingInput
	}

	var cfgData []byte
	if cfgData, err = io.ReadAll(os.Stdin); err != nil {
		return cfg, errReadingData
	}

	if err := yaml.Unmarshal(cfgData, &cfg); err != nil {
		log.Error().Err(err).Msg("Bad config file format.")
		return cfg, err
	}

	return cfg, nil
}

// UpdateAppViewCfg updates the configuration of a viewed process
func UpdateAppViewCfg(pidCtx ipc.IpcPidCtx, confFile []byte) error {
	cmd := ipc.CmdSetAppViewCfg{CfgData: confFile}
	resp, err := cmd.Request(pidCtx)
	if err != nil {
		return err
	}
	err = cmd.UnmarshalResp(resp.ResponseAppViewMsgData)
	if err != nil {
		return err
	}

	if resp.MetaMsgStatus != ipc.ResponseOK || *cmd.Response.Status != ipc.ResponseOK {
		return errSettingCfg
	}

	return nil
}
