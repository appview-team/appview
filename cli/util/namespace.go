package util

import (
	"errors"
	"os"

	lxd "github.com/lxc/lxd/client"
)

var (
	ErrLXDSocketNotAvailable = errors.New("LXD socket is not available")
)

// Get the LXD server unix socket
func getLXDServerSocket(rootdir string) string {
	socketPaths := []string{"/var/lib/lxd/unix.socket", "/var/snap/lxd/common/lxd/unix.socket"}
	for _, path := range socketPaths {
		if _, err := os.Stat(path); err == nil {
			return path
		}
	}
	return ""
}

// Get the List of PID(s) related to LXC container
func GetLXCPids(rootdir string) ([]int, error) {
	lxdUnixPath := getLXDServerSocket(rootdir)
	if lxdUnixPath == "" {
		return nil, ErrLXDSocketNotAvailable
	}

	cli, err := lxd.ConnectLXDUnix(lxdUnixPath, nil)
	if err != nil {
		return nil, err
	}
	containers, err := cli.GetContainers()
	if err != nil {
		return nil, err
	}
	pids := make([]int, len(containers))

	for indx, container := range containers {
		state, _, err := cli.GetContainerState(container.Name)
		if err != nil {
			return nil, err
		}
		pids[indx] = int(state.Pid)
	}
	return pids, nil
}

// Get the List of PID(s) related to containerd container
func GetContainerDPids(rootdir string) ([]int, error) {
	return getContainerRuntimePids(rootdir, "containerd-shim")
}

// Get the List of PID(s) related to Podman container
func GetPodmanPids(rootdir string) ([]int, error) {
	return getContainerRuntimePids(rootdir, "conmon")
}

// Get the List of PID(s) related to specific container runtime
func getContainerRuntimePids(rootdir, runtimeProc string) ([]int, error) {
	runtimeContPids, err := PidAppViewMapByProcessName(rootdir, runtimeProc)
	if err != nil {
		return nil, err
	}
	pids := make([]int, 0, len(runtimeContPids))

	for runtimeContPid := range runtimeContPids {
		childrenPids, err := PidChildren(rootdir, runtimeContPid)
		if err != nil {
			return nil, err
		}

		// Iterate over all the children to found container init process
		for _, childPid := range childrenPids {
			status, _ := PidInitContainer(rootdir, childPid)
			if status {
				pids = append(pids, childPid)
			}
		}
	}

	return pids, nil
}

/*
 * Detect whether or not appview was executed inside a container
 * What is a reasonable algorithm for determining if the current process is in a container?
 * Checking ~/.dockerenv doesn't work for non-docker containers
 * Checking hierarchies in /proc/1/cgroup == / works in some case. However, some hierarchies
 * on non-containers are /init.appview (Ubuntu 20.04 cgroup 0 and 1). Not reliable.
 * Checking for container=lxc | docker in /proc/1/environ works for lxc but not docker, and it
 * requires root privs.
 * Checking /proc/self/cgroup for content after cpuset doesn't work in all cases
 * So, we check for the presence of /proc/2/comm
 */
func InContainer() bool {
	if _, err := os.Stat("/proc/2/comm"); err != nil {
		return true
	}
	return false
}
