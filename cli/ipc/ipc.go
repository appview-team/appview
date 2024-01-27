package ipc

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"math/rand"
	"os"
	"runtime"
	"syscall"
	"time"
)

// ipc structure representes Inter Process Communication object
type ipcObj struct {
	sender   *sendMessageQueue    // Message queue used to send messages
	receiver *receiveMessageQueue // Message queue used to receive messages
	ipcSame  bool                 // Indicator if IPC namespace switch occured
	mqUniqId int                  // Unique ID used in request/response
}

var (
	errCreateMsgQWriter          = errors.New("create writer message queue")
	errCreateMsgQReader          = errors.New("create reader message queue")
	errRequest                   = errors.New("error with sending request to PID")
	errFrameInconsistentUniqId   = errors.New("frame error inconsistent unique id during transmission")
	errFrameInconsistentDataSize = errors.New("frame error inconsistent data size during transmssion")
	ErrConsumerTimeout           = errors.New("timeout with consume the message")
	errReceiveTimeout            = errors.New("timeout with receive the message")
	errSendTimeout               = errors.New("timeout with sending the message")
	errMissingMandatoryField     = errors.New("missing mandatory field in response")
)

const ipcComTimeout time.Duration = 50 * time.Microsecond
const comRetryLimit int = 50

type IpcPidCtx struct {
	Pid        int
	PrefixPath string
}

// Ensure that IPC communication is done in same thread (see ipcNsSet/ipcNsRestore)
// In this way we ensure msgQOpen/msgQUnlink/msgQSend/msgQReceive/msgQCurrentMessages are
// always called in context of the thread which reassocatied IPC namespace.
func ipcStart() {
	runtime.LockOSThread()
}

// Ensure that IPC communication is done in same thread (see ipcNsSet/ipcNsRestore)
// In this way we ensure msgQOpen/msgQUnlink/msgQSend/msgQReceive/msgQCurrentMessages are
// always called in context of the thread which reassocatied IPC namespace.
func ipcEnd() {
	runtime.UnlockOSThread()
}

// ipcDispatcher dispatches IPC communication to the process specified by the pid.
// Returns the bytes array
func ipcDispatcher(appviewReq []byte, pidCtx IpcPidCtx) (*IpcResponseCtx, error) {
	ipcStart()
	defer ipcEnd()
	ipc, err := newIPC(pidCtx)
	if err != nil {
		return nil, err
	}
	defer ipc.destroyIPC()

	err = ipc.sendIpcRequest(appviewReq)
	if err != nil {
		return nil, fmt.Errorf("%w %v", errRequest, pidCtx.Pid)
	}
	// ensure that process consumed this request
	err = ipc.verifySendingMsg()
	if err != nil {
		return nil, err
	}

	return ipc.receiveIpcResponse()
}

// nsnewMsgQWriter creates an IPC writer structure with switching effective uid and gid
func nsnewMsgQWriter(name string, nsUid int, nsGid int, restoreUid int, restoreGid int) (*sendMessageQueue, error) {

	if err := syscall.Setegid(nsGid); err != nil {
		return nil, err
	}
	if err := syscall.Seteuid(nsUid); err != nil {
		return nil, err
	}

	sender, err := newMsgQWriter(name)
	if err != nil {
		return nil, err
	}

	if err := syscall.Seteuid(restoreUid); err != nil {
		return nil, err
	}

	if err := syscall.Setegid(restoreGid); err != nil {
		return nil, err
	}

	return sender, err
}

// nsnewMsgQReader creates an IPC reader structure with switching effective uid and gid
func nsnewMsgQReader(name string, nsUid int, nsGid int, restoreUid int, restoreGid int) (*receiveMessageQueue, error) {

	if err := syscall.Setegid(nsGid); err != nil {
		return nil, err
	}
	if err := syscall.Seteuid(nsUid); err != nil {
		return nil, err
	}

	receiver, err := newMsgQReader(name)
	if err != nil {
		return nil, err
	}

	if err := syscall.Seteuid(restoreUid); err != nil {
		return nil, err
	}

	if err := syscall.Setegid(restoreGid); err != nil {
		return nil, err
	}

	return receiver, err
}

// newIPC creates an IPC object designated for communication with specific PID
func newIPC(pidCtx IpcPidCtx) (*ipcObj, error) {
	restoreGid := os.Getegid()
	restoreUid := os.Geteuid()

	ipcSame, err := IpcNsIsSame(pidCtx)
	if err != nil {
		return nil, err
	}

	// Retrieve information about user and group id
	nsUid, err := ipcNsTranslateUidFromPid(restoreUid, pidCtx)
	if err != nil {
		return nil, err
	}
	nsGid, err := ipcNsTranslateGidFromPid(restoreGid, pidCtx)
	if err != nil {
		return nil, err
	}

	// Retrieve information about process namespace PID
	_, ipcPid, err := IpcNsLastPidFromPid(pidCtx)
	if err != nil {
		return nil, err
	}

	// Switch IPC namespace if needed
	if !ipcSame {
		if err := ipcNsSet(pidCtx); err != nil {
			return nil, err
		}
	}

	//  Create message queue to Write into it
	sender, err := nsnewMsgQWriter(fmt.Sprintf("AppViewIPCIn.%d", ipcPid), nsUid, nsGid, restoreUid, restoreGid)
	if err != nil {
		if !ipcSame {
			ipcNsRestore()
		}
		return nil, errCreateMsgQWriter
	}

	// Create message queue to Read from it
	receiver, err := nsnewMsgQReader(fmt.Sprintf("AppViewIPCOut.%d", ipcPid), nsUid, nsGid, restoreUid, restoreGid)
	if err != nil {
		sender.destroy()
		if !ipcSame {
			ipcNsRestore()
		}
		return nil, errCreateMsgQReader
	}

	return &ipcObj{sender: sender, receiver: receiver, ipcSame: ipcSame, mqUniqId: rand.Intn(9999)}, nil
}

// destroyIPC destroys an IPC object
func (ipc *ipcObj) destroyIPC() {
	ipc.sender.destroy()
	ipc.receiver.destroy()
	if !ipc.ipcSame {
		ipcNsRestore()
	}
}

// receive receive the message from the process endpoint
func (ipc *ipcObj) receive() ([]byte, error) {
	for i := 0; i < comRetryLimit; i++ {
		res, err := ipc.receiver.receive()
		if err != errMsgQEmpty {
			return res, err
		}
		time.Sleep(ipcComTimeout)
	}
	return nil, errReceiveTimeout
}

// send sends the message to the process endpoint
func (ipc *ipcObj) send(msg []byte) error {
	for i := 0; i < comRetryLimit; i++ {
		err := ipc.sender.send(msg)
		if err != errMsgQFull {
			return err
		}
		time.Sleep(ipcComTimeout)
	}
	return errSendTimeout
}

// verifies if message was consumed by the application
func (ipc *ipcObj) verifySendingMsg() error {
	// Ensure that message was consumed by the library
	for i := 0; i < comRetryLimit; i++ {
		curMsg, err := ipc.sender.size()
		if err != nil {
			return err
		}
		if curMsg != 0 {
			time.Sleep(ipcComTimeout)
		} else {
			return nil
		}
	}
	return ErrConsumerTimeout
}

// metadata in ipc frame
func (ipc *ipcObj) frameMeta(remainBytes int) ([]byte, error) {
	metadata, err := json.Marshal(metaRequest{Req: metaReqJsonPartial, Uniq: ipc.mqUniqId, Remain: remainBytes})
	if err != nil {
		return metadata, err
	}
	// Ensure that metadata is nul terminated
	metadata = append(metadata, 0)
	return metadata, nil
}

// prepareFrame preparet the appview message
func (ipc *ipcObj) prepareFrame(remain int, appviewData []byte, offset int) ([]byte, int, error) {
	frame, _ := ipc.frameMeta(remain)
	maxPossibleDataLen := ipc.sender.cap - len(frame) - 1
	if maxPossibleDataLen < 0 {
		return frame, -1, errRequest
	}

	appviewFrameDataSize := maxPossibleDataLen
	if remain < maxPossibleDataLen {
		appviewFrameDataSize = remain
	}

	frameAppViewData := appviewData[offset : offset+appviewFrameDataSize]

	frame = append(frame, frameAppViewData...)
	frame = append(frame, 0)

	return frame, appviewFrameDataSize, nil
}

// sendIpcRequest puts the request in message queue (the message can be splited by the frames)
func (ipc *ipcObj) sendIpcRequest(appviewMsg []byte) error {
	appviewMsgDataRemain := len(appviewMsg)
	var appviewMsgOffset int
	for appviewMsgDataRemain != 0 {

		frame, dataSize, err := ipc.prepareFrame(appviewMsgDataRemain, appviewMsg, appviewMsgOffset)
		if err != nil {
			return err
		}

		appviewMsgOffset += dataSize
		appviewMsgDataRemain -= dataSize

		// Handling the last frame
		//  - change the request type metaReqJsonPartial -> metaReqJson
		if appviewMsgDataRemain == 0 {
			frame = bytes.Replace(frame, []byte("req\":1"), []byte("req\":0"), 1)
		}

		err = ipc.send(frame)
		if err != nil {
			return err
		}

	}
	return nil
}

// ipcGetMsgSeparatorIndex returns index of Meta and AppView message separator
func ipcGetMsgSeparatorIndex(msg []byte) int {
	return bytes.Index(msg, []byte("\x00"))
}

// unmarshall meta response check presence of mandatory field
func UnmarshalMeta(msg []byte, metaResp *metaResponse) error {
	err := json.Unmarshal(msg, &metaResp)
	if err != nil {
		return err
	}

	if metaResp.Status == nil {
		return fmt.Errorf("%w %v", errMissingMandatoryField, "status")
	}

	if metaResp.Uniq == nil {
		return fmt.Errorf("%w %v", errMissingMandatoryField, "uniq")
	}

	if metaResp.Remain == nil {
		return fmt.Errorf("%w %v", errMissingMandatoryField, "remain")
	}

	return nil
}

// parseIpcFrame parses single frame in IPC communication
// returns frame metadata, appview message and error
func parseIpcFrame(msg []byte) (metaResponse, []byte, error) {
	var metaResp metaResponse
	var err error

	// Only metadata case
	sepIndx := ipcGetMsgSeparatorIndex(msg)
	if sepIndx == -1 {
		err = UnmarshalMeta(msg, &metaResp)
		return metaResp, nil, err
	}

	metadata := msg[0:sepIndx]
	err = UnmarshalMeta(metadata, &metaResp)
	if err != nil {
		return metaResp, nil, err
	}

	// Skip the separator
	appviewData := msg[sepIndx+1:]

	return metaResp, appviewData, nil
}

// receiveIpcResponse receives the whole message from process endpoint
func (ipc *ipcObj) receiveIpcResponse() (*IpcResponseCtx, error) {
	listenForResponseTransmission := true
	var appviewMsg []byte
	var lastRespStatus respStatus
	firstFrame := true
	var previousFrameRemainBytes int

	for listenForResponseTransmission {

		msg, err := ipc.receive()
		if err != nil {
			return nil, err
		}

		ipcFrameMeta, appviewFrame, err := parseIpcFrame(msg)
		if err != nil {
			return nil, err
		}

		// Validate unique identifier for transmission
		if *ipcFrameMeta.Uniq != ipc.mqUniqId {
			return nil, errFrameInconsistentUniqId
		}
		// Validate data remaining bytes in transmission
		if (!firstFrame) && *ipcFrameMeta.Remain > previousFrameRemainBytes {
			return nil, errFrameInconsistentDataSize
		}

		previousFrameRemainBytes = *ipcFrameMeta.Remain
		lastRespStatus = *ipcFrameMeta.Status

		// Continue to listen if we still expect data
		if lastRespStatus != ResponseOKwithContent {
			listenForResponseTransmission = false
		}
		appviewMsg = append(appviewMsg, appviewFrame...)
		firstFrame = false
	}

	return &IpcResponseCtx{appviewMsg, lastRespStatus}, nil
}
