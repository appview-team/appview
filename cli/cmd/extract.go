package cmd

import (
	"fmt"
	"os"
	"path"
	"path/filepath"

	"github.com/appview-team/appview/loader"
	"github.com/appview-team/appview/run"
	"github.com/appview-team/appview/util"
	"github.com/spf13/cobra"
)

// excreteCmd represents the excrete command
var excreteCmd = &cobra.Command{
	Use:     "extract [flags] <dir>",
	Aliases: []string{"excrete", "expunge", "extricate", "exorcise"},
	Short:   "Output instrumentary library files to <dir>",
	Long: `Outputs libappview.so, and appview.yml to the provided directory. You can configure these files to instrument any application, and to output the data to any existing tool using simple TCP protocols.

The --*dest flags accept file names like /tmp/appview.log or URLs like file:///tmp/appview.log. They may also
be set to sockets with unix:///var/run/mysock, tcp://hostname:port, udp://hostname:port, or tls://hostname:port.`,
	Example: `  appview extract /opt/libappview
  appview extract --metricdest tcp://some.host:8125 --eventdest tcp://other.host:10070 .
`,
	Args: cobra.ExactArgs(1),
	Run: func(cmd *cobra.Command, args []string) {
		createDirFlag, _ := cmd.Flags().GetBool("parents")
		outPath := "./"
		if len(args) > 0 {
			if createDirFlag {
				os.MkdirAll(args[0], 0755)
			}
			if !util.CheckDirExists(args[0]) {
				util.ErrAndExit("%s does not exist or is not a directory", args[0])
			}
			outPath = args[0]
		}
		newPath, err := filepath.Abs(outPath)
		util.CheckErrSprintf(err, "cannot resolve absolute path of %s: %v", outPath, err)
		outPath = newPath

		err = run.CreateAll(outPath)
		util.CheckErrSprintf(err, "error excreting files: %v", err)
		if rc.MetricsDest != "" || rc.EventsDest != "" || rc.CriblDest != "" {
			err = os.Rename(path.Join(outPath, "appview.yml"), path.Join(outPath, "appview_example.yml"))
			util.CheckErrSprintf(err, "error renaming appview.yml: %v", err)
			rc.WorkDir = outPath
			err = rc.WriteAppViewConfig(path.Join(outPath, "appview.yml"), 0644)
			util.CheckErrSprintf(err, "error writing appview.yml: %v", err)
		}
		ld := loader.New()
		ld.Patch(path.Join(outPath, "libappview.so"))
		fmt.Printf("Successfully extracted to %s.\n", outPath)
	},
}

func init() {
	metricAndEventDestFlags(excreteCmd, rc)
	excreteCmd.Flags().BoolP("parents", "p", false, "Create any missing intermediate pathname components in provided directory parameter")
	RootCmd.AddCommand(excreteCmd)
}
