package cmd

// /*
import (
	"fmt"
	"time"

	"github.com/appview-team/appview/bpf"
	"github.com/appview-team/appview/daemon"
	"github.com/appview-team/appview/snapshot"
	"github.com/appview-team/appview/util"
	"github.com/rs/zerolog/log"
	"github.com/spf13/cobra"
)

/* Args Matrix (X disallows)
 *                 filedest 	sendcore
 * filedest        -
 * sendcore                     -
 */

// daemonCmd represents the daemon command
var daemonCmd = &cobra.Command{
	Use:   "daemon [flags]",
	Short: "Run the appview daemon",
	Long:  `Listen and respond to system events.`,
	Example: `appview daemon
	appview daemon --filedest localhost:10089`,
	Args: cobra.NoArgs,
	Run: func(cmd *cobra.Command, args []string) {
		filedest, _ := cmd.Flags().GetString("filedest")
		sendcore, _ := cmd.Flags().GetBool("sendcore")

		// Create a history directory for logs
		snapshot.CreateWorkDir("daemon")

		// Validate user has root permissions
		if err := util.UserVerifyRootPerm(); err != nil {
			log.Error().Err(err)
			util.ErrAndExit("appview daemon requires administrator privileges")
		}

		loader, err := bpf.NewLoader("appview-ebpf")
		if err != nil {
			log.Error().Err(err)
			util.ErrAndExit("appview-ebpf loader was not found. appview daemon requires that appview-ebpf loader need to run.")
		}

		sigdel, err := loader.NewSigdel()
		if err != nil {
			log.Error().Err(err)
			util.ErrAndExit("appview-ebpf loader cannot create sigdel object")
		}

		// Buffered Channel (non-blocking until full)
		sigDataChan := make(chan bpf.SigDelData, 128)

		go sigdel.ReadData(sigDataChan)

		// Terminate Loader
		if err := loader.Terminate(); err != nil {
			log.Error().Err(err)
			util.ErrAndExit("appview daemon was not able to terminate ebpf loader")
		}
		d := daemon.New(filedest)
		for {
			select {
			case sigEvent := <-sigDataChan:
				// Signal received
				log.Info().Msgf("Signal signal: %d errno: %d handler: 0x%x pid: %d nspid: %d uid: %d gid: %d app %s\n",
					sigEvent.Sig, sigEvent.Errno, sigEvent.Handler,
					sigEvent.Pid, sigEvent.NsPid, sigEvent.Uid, sigEvent.Gid, sigEvent.Comm)

				// TODO Filter out expected signals from libappview
				// get proc/pid/maps from ns
				// iterate through, find libappview and get range
				// is signal handler address in libappviewStart-libappviewEnd range

				// If the daemon is running on the host, use the pid(hostpid) to get the crash files
				// If the daemon is running in a container, use the nspid to get the crash files
				var pid uint32
				if util.InContainer() {
					pid = sigEvent.NsPid
				} else {
					pid = sigEvent.Pid
				}

				// Wait for libappview to generate crash files (if the app was viewed)
				time.Sleep(1 * time.Second) // TODO Is this long enough? Too long? Check for files instead?

				// Generate/retrieve snapshot files
				if err := snapshot.GenFiles(sigEvent.Sig, sigEvent.Errno, pid, sigEvent.Uid, sigEvent.Gid, sigEvent.Handler, string(sigEvent.Comm[:]), ""); err != nil {
					log.Error().Err(err).Msgf("error generating crash files")
				}

				// If a network destination is specified, send crash files
				if filedest != "" {
					if err := d.Connect(); err != nil {
						log.Error().Err(err).Msgf("error connecting to %s", filedest)
						continue
					}
					d.SendSnapshotFiles(fmt.Sprintf("/tmp/appview/%d/", pid), sendcore)
					d.Disconnect()
				}
			}
		}
	},
}

func init() {
	daemonCmd.Flags().StringP("filedest", "f", "", "Set destination for files (host:port defaults to tcp://)")
	daemonCmd.Flags().BoolP("sendcore", "s", false, "Include core file when sending files to network destination")
	RootCmd.AddCommand(daemonCmd)
}
