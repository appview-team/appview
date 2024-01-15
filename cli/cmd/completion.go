package cmd

import (
	"os"

	"github.com/appview-team/appview/util"
	"github.com/spf13/cobra"
)

/* Args Matrix (X disallows)
 */

var completionCmd = &cobra.Command{
	Use:   "completion [flags] [bash|zsh]",
	Short: "Generates completion code for specified shell",
	Long:  "Generates completion code for specified shell.",
	Example: `  appview completion bash > /etc/bash_completion.d/appview # Generate and install appview autocompletion for bash
  source <(appview completion bash)                      # Generates and load appview autocompletion for bash
`,
	ValidArgs: []string{"bash", "zsh"},
	Args:      cobra.ExactValidArgs(1),
	Run: func(cmd *cobra.Command, args []string) {
		var err error

		switch args[0] {
		case "bash":
			err = cmd.Root().GenBashCompletion(os.Stdout)
		case "zsh":
			err = cmd.Root().GenZshCompletion(os.Stdout)
		default:
			util.ErrAndExit("Unsupported shell type %q", args[0])
		}
		if err != nil {
			util.ErrAndExit("Unable to generate completion script")
		}
	},
}

func init() {
	RootCmd.AddCommand(completionCmd)
}
