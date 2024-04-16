---
title: Downloading AppView
---

## Downloading AppView

Download AppView, then explore its CLI. Getting started is that easy.

You can download AppView as a binary in your Linux OS, or as a container.

### Download as Binary

Review the AppView [Requirements](/docs/requirements) to ensure that you’re completing these steps on a supported system. 

Then, you can use these CLI commands to directly download the binary and make it executable:

```
curl -Lo appview https://github.com/appview-team/appview/releases/download/v1.0.0/appview-x86_64
curl -Ls https://github.com/appview-team/appview/releases/download/v1.0.0/appview-x86_64.md5 | md5sum -c
mv appview-x86_64 appview
chmod +x appview
```

Congratulations! 

You're ready to start working with AppView.

Before you start, though, you may want to consider [where](#where-from) to run AppView from.

### Download as Container

Check out the [AppView repo on Docker Hub](https://hub.docker.com/r/appview-team/appview), which provides instructions and access to earlier builds.

Each container provides the AppView binary on Ubuntu 20.04.

### Verify AppView

Run some appview commands:

```
appview run ps -ef
appview events
appview metrics
```

That's it!

Now you are ready to explore the CLI:

- Run `appview --help` or `appview -h` to view CLI options.
- Try some basic CLI commands in [Using the CLI](/docs/cli-using).
- See the complete [CLI Reference](/docs/cli-reference).

If an application crashes when viewed, you can set the `APPVIEW_ERROR_SIGNAL_HANDLER` environment variable to `true` to turn on backtrace logging. This should provide more informative logs if crashes recur. Feel free to [contact](/docs/community) the AppView team and/or [open a new issue](https://github.com/criblio/appview/issues) if this happens.

<span id="where-from"> </span>

### What's the Best Location to Run AppView From?

Where AppView lives on your system is up to you. To decide wisely, first consider which of the [two ways of working](/docs/working-with) with AppView fit your situation: ad hoc or planned-out.

For ad hoc exploration and investigation, one choice that follows [standard practice](https://en.wikipedia.org/wiki/Filesystem_Hierarchy_Standard) for an "add-on software package" is to locate AppView in `/opt`. Since in the AppView context, `appview` is a command, it helps to call the directory where AppView lives something other than `appview`. For example, you can name the directory `/opt/appview`, to make it clear that `/opt/appview/appview` is the `appview` executable.

We do **not** recommend running AppView from any user's home directory, because that leads to file ownership and permissions problems if additional users try to run AppView.

By default, the AppView CLI will output data to the local filesystem where it's running. While this can work fine for "ad hoc" investigations, it can cause problems for longer-duration scenarios like application monitoring, where the local filesystem may not have room for the data being written. 

Also consider what level of logging you want from AppView, and where those logs should go. In planned-out scenarios, where AppView runs persistently (including as a service), configuring logging judiciously can prevent AppView from using up too much disk space. 

To customize AppView's data-writing and logging, edit the [`appview.yml`](/docs/config-file) config file.
