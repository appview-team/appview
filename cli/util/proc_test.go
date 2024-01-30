package util

import (
	"os"
	"os/user"
	"strings"
	"testing"

	"github.com/stretchr/testify/assert"
)

// TestProcessesByNameToAttach
// Assertions:
// - The expected process array is returned
// - No error is returned
func TestProcessesByNameToAttach(t *testing.T) {
	// Current process
	name := "util.test"
	result, err := ProcessesByNameToAttach("", name, "", true)
	user, _ := user.Current()
	exp := Processes{
		Process{
			ID:      1,
			Pid:     os.Getpid(),
			User:    user.Username,
			Command: strings.Join(os.Args[:], " "),
			Viewed:  false,
		},
	}
	assert.Equal(t, exp, result)
	assert.NoError(t, err)
}

// TestPidAppViewMapByProcessName
// Assertions:
// - The process map with single entry is returned
// - No error is returned
func TestPidAppViewMapByProcessName(t *testing.T) {
	currentProcessName, err := PidCommand("", os.Getpid())
	assert.NoError(t, err)
	pidMap, err := PidAppViewMapByProcessName("", currentProcessName)
	assert.NoError(t, err)
	assert.Len(t, pidMap, 1)
	assert.Contains(t, pidMap, os.Getpid())
	assert.Equal(t, pidMap[os.Getpid()], false)
}

// TestPidAppViewMapByProcessName
// Assertions:
// - Empty Map is returned
// - No error is returned
func TestPidAppViewMapByProcessNameCharShorter(t *testing.T) {
	currentProcName, err := PidCommand("", os.Getpid())
	assert.NoError(t, err)
	modProcName := currentProcName[:len(currentProcName)-1]

	pidMap, err := PidAppViewMapByProcessName("", modProcName)
	assert.NoError(t, err)
	assert.Len(t, pidMap, 0)
}

// TestPidAppViewMapByCmdLine
// Assertions:
// - The expected process map is returned
// - No error is returned
func TestPidAppViewMapByCmdLine(t *testing.T) {
	currentProcCmdLine, err := PidCmdline("", os.Getpid())
	assert.NoError(t, err)
	pidMap, err := PidAppViewMapByCmdLine("", currentProcCmdLine)
	assert.NoError(t, err)
	assert.Len(t, pidMap, 1)
	assert.Contains(t, pidMap, os.Getpid())
	assert.Equal(t, pidMap[os.Getpid()], false)
}

// TestPidAppViewMapByCmdLineCharShorter
// Assertions:
// - The expected process map is returned
// - No error is returned
func TestPidAppViewMapByCmdLineCharShorter(t *testing.T) {
	currentProcCmdLine, err := PidCmdline("", os.Getpid())
	assert.NoError(t, err)
	modProcCmdLine := currentProcCmdLine[:len(currentProcCmdLine)-1]

	pidMap, err := PidAppViewMapByCmdLine("", modProcCmdLine)
	assert.NoError(t, err)
	assert.Len(t, pidMap, 1)
	assert.Contains(t, pidMap, os.Getpid())
	assert.Equal(t, pidMap[os.Getpid()], false)
}

// TestPidUser
// Assertions:
// - The expected user value is returned
// - No error is returned
func TestPidUser(t *testing.T) {
	/*
		 * Disabled since we run as "builder" for `make docker-build`.
		 *
		// Process 1 owner
		pid := 1
		result := PidUser(pid)
		assert.Equal(t, "root", result)
	*/

	// Current process owner
	currentUser, err := user.Current()
	if err != nil {
		t.Fatal(err)
	}
	username := currentUser.Username
	pid := os.Getpid()
	result, err := PidUser("", pid)
	assert.Equal(t, username, result)
	assert.NoError(t, err)
}

// TestPidCommand
// Assertions:
// - The expected command value is returned
// - No error is returned
func TestPidCommand(t *testing.T) {
	// Current process command
	pid := os.Getpid()
	result, err := PidCommand("", pid)
	assert.Equal(t, "util.test", result)
	assert.NoError(t, err)
}

// TestPidCmdline
// Assertions:
// - The expected cmdline value is returned
// - No error is returned
func TestPidCmdline(t *testing.T) {
	// Current process command
	pid := os.Getpid()
	result, err := PidCmdline("", pid)
	assert.Equal(t, strings.Join(os.Args[:], " "), result)
	assert.NoError(t, err)
}

// TestPidThreadsPids
// Assertions:
// - Current process id is one of the element in thread
// - No error is returned
func TestPidThreadsPids(t *testing.T) {
	// Current process command
	pid := os.Getpid()
	threadPids, err := PidThreadsPids("", pid)
	assert.Contains(t, threadPids, pid)
	assert.NoError(t, err)
}

// TestPidExists
// Assertions:
// - The expected boolean value is returned
func TestPidExists(t *testing.T) {
	// Current process PID
	pid := os.Getpid()
	result := PidExists("", pid)
	assert.Equal(t, true, result)

	// PID 0
	pid = 0
	result = PidExists("", pid)
	assert.Equal(t, false, result)
}

// TestPidGetRefPidForMntNamespace
// Assertions:
// - The expected boolean value is returned
// - No error is returned
func TestPidGetRefPidForMntNamespace(t *testing.T) {
	pid := os.Getpid()
	result := PidGetRefPidForMntNamespace("", pid)
	assert.Equal(t, -1, result)
}
