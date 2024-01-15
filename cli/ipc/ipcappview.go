package ipc

import (
	"encoding/json"
	"fmt"

	"github.com/appview-team/appview/libappview"
	"gopkg.in/yaml.v2"
)

type IpcResponseCtx struct {
	ResponseAppViewMsgData []byte     // Contains appview message response (status + message response data)
	MetaMsgStatus        respStatus // Response status in msg metadata
}

// appviewReqCmd describes the command passed in the IPC request to library appview request
// Must be inline with server, see: ipc_appview_req_t
// IMPORTANT NOTE:
// NEW VALUES MUST BE PLACED AS A LAST
type appviewReqCmd int

const (
	reqCmdGetSupportedCmds appviewReqCmd = iota
	reqCmdGetAppViewStatus
	reqCmdGetAppViewCfg
	reqCmdSetAppViewCfg
	reqCmdGetTransportStatus
	reqCmdGetProcessDetails
)

// Request which contains only cmd without data
type appviewRequestOnly struct {
	// Unique request command
	Req appviewReqCmd `mapstructure:"req" json:"req" yaml:"req"`
}

// Set configuration request
// Must be inline with server, see: ipcProcessSetCfg
type appviewRequestSetCfg struct {
	// Unique request command
	Req appviewReqCmd `mapstructure:"req" json:"req" yaml:"req"`
	// Viewed Cfg
	Cfg libappview.AppViewConfig `mapstructure:"cfg" json:"cfg" yaml:"cfg"`
}

// Must be inline with server, see: ipcRespStatus
type appviewOnlyStatusResponse struct {
	// Response status
	Status *respStatus `mapstructure:"status" json:"status" yaml:"status"`
}

// CmdDesc describes the message
type CmdDesc []struct {
	// Id of the command
	Id int `mapstructure:"id" json:"id" yaml:"id"`
	// Name of the command
	Name string `mapstructure:"name" json:"name" yaml:"name"`
}

// Must be inline with server, see: ipcRespGetAppViewCmds
type appviewGetSupportedCmdsResponse struct {
	// Response status
	Status *respStatus `mapstructure:"status" json:"status" yaml:"status"`
	// Meta message commands description
	CommandsMeta CmdDesc `mapstructure:"commands_meta" json:"commands_meta" yaml:"commands_meta"`
	// AppView message commands description
	CommandsAppView CmdDesc `mapstructure:"commands_appview" json:"commands_appview" yaml:"commands_appview"`
}

// Must be inline with server, see: ipcRespGetAppViewStatus
type appviewGetStatusResponse struct {
	// Response status
	Status *respStatus `mapstructure:"status" json:"status" yaml:"status"`
	// Viewed status
	Viewed bool `mapstructure:"viewed" json:"viewed" yaml:"viewed"`
}

type AppViewGetCfgResponseCfg struct {
	Current libappview.AppViewConfig `mapstructure:"current" json:"current" yaml:"current"`
}

// Must be inline with server, see: ipcRespGetAppViewCfg
type appviewGetCfgResponse struct {
	// Response status
	Status *respStatus            `mapstructure:"status" json:"status" yaml:"status"`
	Cfg    AppViewGetCfgResponseCfg `mapstructure:"cfg" json:"cfg" yaml:"cfg"`
}

// CmdGetSupportedCmds describes Get Supported Commands command request and response
type CmdGetSupportedCmds struct {
	Response appviewGetSupportedCmdsResponse
}

func (cmd *CmdGetSupportedCmds) Request(pidCtx IpcPidCtx) (*IpcResponseCtx, error) {
	req, _ := json.Marshal(appviewRequestOnly{Req: reqCmdGetSupportedCmds})

	return ipcDispatcher(req, pidCtx)
}

func (cmd *CmdGetSupportedCmds) UnmarshalResp(respData []byte) error {
	err := yaml.Unmarshal(respData, &cmd.Response)
	if err != nil {
		return err
	}

	if cmd.Response.Status == nil {
		return fmt.Errorf("%w %v", errMissingMandatoryField, "status")
	}

	return nil
}

// CmdGetAppViewStatus describes Get AppView Status command request and response
type CmdGetAppViewStatus struct {
	Response appviewGetStatusResponse
}

// AppViewInterfaceDesc describes single appview interface (Possible interfaces are "logs"/"events"/"metrics"/"payload")
type AppViewInterfaceDesc []struct {
	// Name of channel
	Name string `mapstructure:"name" json:"name" yaml:"name"`
	// Status of connection
	Connected bool `mapstructure:"connected" json:"connected" yaml:"connected"`
	// Description of connection
	Details string `mapstructure:"config,omitempty" json:"config,omitempty" yaml:"config,omitempty"`
	// Attempts in case of failure
	Attempts int `mapstructure:"attempts,omitempty" json:"attempts,omitempty" yaml:"attempts,omitempty"`
	// Failure details
	FailureDetails string `mapstructure:"failure_details,omitempty" json:"failure_details,omitempty" yaml:"failure_details,omitempty"`
}

// Must be inline with server, see: ipcRespGetTransportStatus
type appviewGetTransportStatusResponse struct {
	// Response status
	Status *respStatus `mapstructure:"status" json:"status" yaml:"status"`
	// Interface description
	Interfaces AppViewInterfaceDesc `mapstructure:"interfaces" json:"interfaces" yaml:"interfaces"`
}

// Must be inline with server, see: ipcRespGetProcessDetails
type appviewGetProcessDetailsResponse struct {
	// Response status
	Status *respStatus `mapstructure:"status" json:"status" yaml:"status"`
	// Pid
	Pid int `mapstructure:"pid" json:"pid" yaml:"pid"`
	// UUID
	Uuid string `mapstructure:"uuid" json:"uuid" yaml:"uuid"`
	// Id
	Id string `mapstructure:"id" json:"id" yaml:"id"`
	// Machine id
	MachineId string `mapstructure:"machine_id" json:"machine_id" yaml:"machine_id"`
}

func (cmd *CmdGetAppViewStatus) Request(pidCtx IpcPidCtx) (*IpcResponseCtx, error) {
	req, _ := json.Marshal(appviewRequestOnly{Req: reqCmdGetAppViewStatus})

	return ipcDispatcher(req, pidCtx)
}

func (cmd *CmdGetAppViewStatus) UnmarshalResp(respData []byte) error {
	err := yaml.Unmarshal(respData, &cmd.Response)
	if err != nil {
		return err
	}

	if cmd.Response.Status == nil {
		return fmt.Errorf("%w %v", errMissingMandatoryField, "status")
	}

	return nil
}

// CmdGetAppViewCfg describes Get AppView Configuration command request and response
type CmdGetAppViewCfg struct {
	Response appviewGetCfgResponse
}

func (cmd *CmdGetAppViewCfg) Request(pidCtx IpcPidCtx) (*IpcResponseCtx, error) {
	req, _ := json.Marshal(appviewRequestOnly{Req: reqCmdGetAppViewCfg})

	return ipcDispatcher(req, pidCtx)
}

func (cmd *CmdGetAppViewCfg) UnmarshalResp(respData []byte) error {
	err := yaml.Unmarshal(respData, &cmd.Response)
	if err != nil {
		return err
	}

	if cmd.Response.Status == nil {
		return fmt.Errorf("%w %v", errMissingMandatoryField, "status")
	}

	return nil
}

// CmdSetAppViewCfg describes Set AppView Configuration command request and response
type CmdSetAppViewCfg struct {
	Response appviewOnlyStatusResponse
	CfgData  []byte
}

func (cmd *CmdSetAppViewCfg) Request(pidCtx IpcPidCtx) (*IpcResponseCtx, error) {
	appviewReq := appviewRequestSetCfg{Req: reqCmdSetAppViewCfg}

	err := yaml.Unmarshal(cmd.CfgData, &appviewReq.Cfg)
	if err != nil {
		return nil, err
	}
	req, _ := json.Marshal(appviewReq)

	return ipcDispatcher(req, pidCtx)
}

func (cmd *CmdSetAppViewCfg) UnmarshalResp(respData []byte) error {
	err := yaml.Unmarshal(respData, &cmd.Response)
	if err != nil {
		return err
	}

	if cmd.Response.Status == nil {
		return fmt.Errorf("%w %v", errMissingMandatoryField, "status")
	}

	return nil
}

// CmdGetTransportStatus describes Get AppView Status command request and response
type CmdGetTransportStatus struct {
	Response appviewGetTransportStatusResponse
}

func (cmd *CmdGetTransportStatus) Request(pidCtx IpcPidCtx) (*IpcResponseCtx, error) {
	req, _ := json.Marshal(appviewRequestOnly{Req: reqCmdGetTransportStatus})

	return ipcDispatcher(req, pidCtx)
}

func (cmd *CmdGetTransportStatus) UnmarshalResp(respData []byte) error {
	err := yaml.Unmarshal(respData, &cmd.Response)
	if err != nil {
		return err
	}

	if cmd.Response.Status == nil {
		return fmt.Errorf("%w %v", errMissingMandatoryField, "status")
	}

	return nil
}

// CmdGetProcessDetails describes Get Process Details command request and response
type CmdGetProcessDetails struct {
	Response appviewGetProcessDetailsResponse
}

func (cmd *CmdGetProcessDetails) Request(pidCtx IpcPidCtx) (*IpcResponseCtx, error) {
	req, _ := json.Marshal(appviewRequestOnly{Req: reqCmdGetProcessDetails})

	return ipcDispatcher(req, pidCtx)
}

func (cmd *CmdGetProcessDetails) UnmarshalResp(respData []byte) error {
	err := yaml.Unmarshal(respData, &cmd.Response)
	if err != nil {
		return err
	}

	if cmd.Response.Status == nil {
		return fmt.Errorf("%w %v", errMissingMandatoryField, "status")
	}

	return nil
}
