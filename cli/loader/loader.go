package loader

import (
	"fmt"
	"os"
	"os/exec"
	"strconv"
	"syscall"

	"github.com/appview-team/appview/util"
)

// Loader represents appview loader object
type AppViewLoader struct {
	Path string
}

// Initialize the loader
func New() AppViewLoader {
	// Get path to this appview executable
	exPath, err := os.Executable()
	if err != nil {
		util.ErrAndExit("error getting executable path, %v", err)
	}

	return AppViewLoader{
		Path: exPath,
	}
}

// Rules Command
func (sL *AppViewLoader) Rules(tmpPath, rootdir string) (string, error) {
	args := make([]string, 0)
	args = append(args, "--rules")
	args = append(args, tmpPath)
	if rootdir != "" {
		args = append(args, "--rootdir")
		args = append(args, rootdir)
	}
	return sL.RunSubProc(args, os.Environ())
}

// Preload Command
func (sL *AppViewLoader) Preload(path, rootdir string) (string, error) {
	args := make([]string, 0)
	args = append(args, "--preload")
	args = append(args, path)
	if rootdir != "" {
		args = append(args, "--rootdir")
		args = append(args, rootdir)
	}
	return sL.RunSubProc(args, os.Environ())
}

// Mount Command
func (sL *AppViewLoader) Mount(path string, cPid int, rootdir string) (string, error) {
	args := make([]string, 0)
	args = append(args, "--mount")
	args = append(args, path)
	args = append(args, "--namespace")
	args = append(args, fmt.Sprint(cPid))
	if rootdir != "" {
		args = append(args, "--rootdir")
		args = append(args, rootdir)
	}
	return sL.RunSubProc(args, os.Environ())
}

// Install Command
// - Extract libappview.so to /usr/lib/appview/<version>/libappview.so /tmp/appview/<version>/libappview.so locally
func (sL *AppViewLoader) Install(rootdir string) (string, error) {
	args := make([]string, 0)
	args = append(args, "--install")
	if rootdir != "" {
		args = append(args, "--rootdir")
		args = append(args, rootdir)
	}
	return sL.RunSubProc(args, os.Environ())
}

// - Modify the relevant service configurations to preload /usr/lib/appview/<version>/libappview.so or /tmp/appview/<version>/libappview.so on the host
func (sL *AppViewLoader) ServiceHost(serviceName string) (string, error) {
	return sL.RunSubProc([]string{"--service", serviceName}, os.Environ())
}

// - Modify the relevant service configurations to preload /usr/lib/appview/<version>/libappview.so or /tmp/appview/<version>/libappview.so in containers
func (sL *AppViewLoader) ServiceContainer(serviceName string, cpid int) (string, error) {
	return sL.RunSubProc([]string{"--service", serviceName, "--namespace", strconv.Itoa(cpid)}, os.Environ())
}

// - Modify the relevant service configurations to NOT preload /usr/lib/appview/<version>/libappview.so or /tmp/appview/<version>/libappview.so on the host
func (sL *AppViewLoader) UnserviceHost() (string, error) {
	return sL.RunSubProc([]string{"--unservice"}, os.Environ())
}

// - Modify the relevant service configurations to NOT preload /usr/lib/appview/<version>/libappview.so or /tmp/appview/<version>/libappview.so in containers
func (sL *AppViewLoader) UnserviceContainer(cpid int) (string, error) {
	return sL.RunSubProc([]string{"--unservice", "--namespace", strconv.Itoa(cpid)}, os.Environ())
}

// - Attach to a process on the host or in containers
func (sL *AppViewLoader) Attach(args []string, env []string) error {
	args = append([]string{"--ldattach"}, args...)
	return sL.ForkAndRun(args, env)
}

// - Attach to a process on the host or in containers
func (sL *AppViewLoader) AttachSubProc(args []string, env []string) (string, error) {
	args = append([]string{"--ldattach"}, args...)
	return sL.RunSubProc(args, env)
}

func (sL *AppViewLoader) Patch(libraryPath string) (string, error) {
	return sL.RunSubProc([]string{"--patch", libraryPath}, os.Environ())
}

// Detach transforms the calling process into a appview detach operation
func (sL *AppViewLoader) Detach(args []string, env []string) error {
	args = append([]string{"--lddetach"}, args...)
	return sL.Run(args, env)
}

// DetachSubProc runs a appview detach as a seperate process
func (sL *AppViewLoader) DetachSubProc(args []string, env []string) (string, error) {
	args = append([]string{"--lddetach"}, args...)
	return sL.RunSubProc(args, env)
}

// Passthrough views a process
func (sL *AppViewLoader) Passthrough(args []string, env []string) error {
	args = append([]string{"--passthrough"}, args...)
	return sL.Run(args, env)
}

// PassthroughSubProc views a process as a seperate process
func (sL *AppViewLoader) PassthroughSubProc(args []string, env []string) (string, error) {
	args = append([]string{"--passthrough"}, args...)
	return sL.RunSubProc(args, env)
}

func (sL *AppViewLoader) Run(args []string, env []string) error {
	args = append([]string{"appview"}, args...)
	return syscall.Exec(sL.Path, args, env)
}

func (sL *AppViewLoader) ForkAndRun(args []string, env []string) error {
	args = append([]string{"appview"}, args...)

	pid, err := syscall.ForkExec(sL.Path, args, &syscall.ProcAttr{
		Env: env,
		Files: []uintptr{uintptr(syscall.Stdin),
			uintptr(syscall.Stdout),
			uintptr(syscall.Stderr)}})

	// Child has exec'ed. Below is the Parent path
	if err != nil {
		return err
	}
	proc, suberr := os.FindProcess(pid)
	if suberr != nil {
		return err
	}
	_, suberr = proc.Wait()
	if suberr != nil {
		return err
	}

	return err
}

func (sL *AppViewLoader) RunSubProc(args []string, env []string) (string, error) {
	cmd := exec.Command(sL.Path, args...)
	cmd.Env = env
	stdoutStderr, err := cmd.CombinedOutput()
	return string(stdoutStderr[:]), err
}
