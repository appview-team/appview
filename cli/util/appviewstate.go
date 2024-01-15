package util

import (
	"errors"

	"github.com/appview-team/appview/ipc"
)

// AppViewStatus represents the process status in context of libappview.so
type AppViewStatus int

// AppView Status description
//
// When process started it will be in one of the following states:
// Disable
// Active
// Setup
//
// Possible transfer between states:
// Disable -> Active (attach required sudo - ptrace)
// Setup -> Active   (attach required sudo - ptrace)
// Latent -> Active
// Active -> Latent
// Setup -> Latent
//
// If the process is in Active status it can be changed only to Latent status

const (
	Disable AppViewStatus = iota // libappview.so is not loaded
	Setup                      // libappview.so is loaded, reporting thread is not present
	Active                     // libappview.so is loaded, reporting thread is present we are sending data
	Latent                     // libappview.so is loaded, reporting thread is present we are not emiting any data
)

func (state AppViewStatus) String() string {
	return []string{"Disable", "Setup", "Active", "Latent"}[state]
}

var errAppViewStatus = errors.New("error appview status")

// getAppViewStatus retreives information about AppView status using IPC
func getAppViewStatus(rootdir string, pid int) (AppViewStatus, error) {
	cmd := ipc.CmdGetAppViewStatus{}
	resp, err := cmd.Request(ipc.IpcPidCtx{
		PrefixPath: rootdir,
		Pid:        pid,
	})
	if err != nil {
		return Disable, err
	}
	err = cmd.UnmarshalResp(resp.ResponseAppViewMsgData)
	if err != nil {
		return Disable, err
	}

	if resp.MetaMsgStatus == ipc.ResponseOK && *cmd.Response.Status == ipc.ResponseOK {
		if cmd.Response.Viewed {
			return Active, nil
		}
		return Latent, nil
	}
	return Disable, errAppViewStatus
}
