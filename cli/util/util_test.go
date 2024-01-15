package util

import (
	"io/ioutil"
	"os"
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
)

func TestAppViewHome(t *testing.T) {
	homeBak := os.Getenv("HOME")
	appviewHomeBak := os.Getenv("APPVIEW_HOME")
	tmpDirBak := os.Getenv("TMPDIR")

	os.Setenv("APPVIEW_HOME", "/foo")
	ret := AppViewHome()
	assert.Equal(t, "/foo", ret)
	os.Unsetenv("APPVIEW_HOME")

	os.Setenv("HOME", "/foo2")
	ret = AppViewHome()
	assert.Equal(t, "/foo2/.appview", ret)
	os.Unsetenv("HOME")

	os.Setenv("TMPDIR", "/foo3")
	ret = AppViewHome()
	assert.Equal(t, "/foo3/.appview", ret)
	os.Unsetenv("TMPDIR")

	ret = AppViewHome()
	assert.Equal(t, "/tmp/.appview", ret)

	os.Setenv("HOME", homeBak)
	os.Setenv("APPVIEW_HOME", appviewHomeBak)
	os.Setenv("TMPDIR", tmpDirBak)
}

func TestGetHumanDuration(t *testing.T) {
	n := time.Now()
	ago := GetHumanDuration(n.Sub(n.Add(-1 * time.Second)))
	assert.Equal(t, "1s", ago)
	ago = GetHumanDuration(n.Sub(n.Add((-1 * time.Minute) + (-1 * time.Second))))
	assert.Equal(t, "1m1s", ago)
	ago = GetHumanDuration(n.Sub(n.Add((-1 * time.Hour) + (-1 * time.Minute))))
	assert.Equal(t, "1h1m", ago)
	ago = GetHumanDuration(n.Sub(n.Add((-24 * time.Hour) + (-1 * time.Hour))))
	assert.Equal(t, "1d1h", ago)
	ago = GetHumanDuration(n.Sub(n.Add((-24 * 7 * time.Hour) + (-1 * time.Hour))))
	assert.Equal(t, "7d", ago)
	ago = GetHumanDuration(n.Sub(n.Add(-100 * time.Millisecond)))
	assert.Equal(t, "100ms", ago)
}

func TestCountLines(t *testing.T) {
	rawEvent := `{"type":"evt","body":{"sourcetype":"console","id":"d55805e5c25e-echo-/bin/echo true","_time":1609191683.985,"source":"stdout","host":"d55805e5c25e","proc":"echo","cmd":"/bin/echo true","pid":10117,"_channel":"641503557208802","data":"true"}}
{"type":"evt","body":{"sourcetype":"console","id":"d55805e5c25e-echo-/bin/echo true","_time":1609191683.985,"source":"stdout","host":"d55805e5c25e","proc":"echo","cmd":"/bin/echo true","pid":10117,"_channel":"641503557208802","data":"true"}}
{"type":"evt","body":{"sourcetype":"console","id":"d55805e5c25e-echo-/bin/echo true","_time":1609191683.985,"source":"stdout","host":"d55805e5c25e","proc":"echo","cmd":"/bin/echo true","pid":10117,"_channel":"641503557208802","data":"true"}}
{"type":"evt","body":{"sourcetype":"console","id":"d55805e5c25e-echo-/bin/echo true","_time":1609191683.985,"source":"stdout","host":"d55805e5c25e","proc":"echo","cmd":"/bin/echo true","pid":10117,"_channel":"641503557208802","data":"true"}}
`

	ioutil.WriteFile("event.json", []byte(rawEvent), 0644)

	count, err := CountLines("event.json")
	assert.NoError(t, err)
	assert.Equal(t, 4, count)
	os.Remove("event.json")
}
