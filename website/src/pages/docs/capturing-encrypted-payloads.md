---
title: Capturing Encrypted Payloads
---

<span id="troubleshoot-all"></span>

# Reasons to Troubleshoot

You can use AppView to [troubleshoot an application](#crash-analysis) that's behaving problematically:

- Whenever a viewed app crashes, AppView can obtain a core dump, a backtrace (i.e., stack trace), or both, while capturing supplemental information in text files.

- AppView can generate a **snapshot** file containing debug information about processes that are running normally or crashing, unviewd or viewed.

On the other hand, you can [troubleshoot AppView itself](#troubleshoot-appview) if you're trying to appview an application but not getting the results you expect:

- AppView can retrieve the config currently in effect; you can then update (modify) it dynamically as desired.

- AppView can determine the status of the transport it's trying to use to convey events and metrics to a destination. This helps troubleshoot "Why am I getting no events or metrics?" scenarios.

<span id="troubleshoot-application"></span>

## Troubleshooting an Application

The AppView team considers it good practice to keep the **snapshot** feature turned on when scoping applications. That way, if a viewed app crashes, you'll always have either a stacktrace or a coredump or both (depending on how you configure AppView) as a starting point for your crash analysis.

When an application you're **not** scoping is crashing or otherwise behaving in unexpected ways, try scoping it with the **snapshot** feature turned on. Then you can explore the app's metrics and events while it's running, or analyze a coredump and/or stacktrace if it crashes. 

<span id="crash-analysis"></span>

### Crash Analysis

The `appview snapshot` command obtains debug information about a running or crashing process, regardless of whether or not the process is viewed.

Whenever the kernel sends the viewed app a fatal signal (i.e., illegal instruction, bus error, segmentation fault, or floating point exception), AppView will capture either a coredump, a stacktrace, or both, depending on what you have configured or the command-line options you choose. (You might want to keep coredump capture turned off if the relatively large size of coredump files would be problematic in your environment.) 

- See the `snapshot` section of the [config file](/docs/config-file).

- The `appview attach`, `appview run`, and `appview watch` commands can all take the `-b`/`--backtrace` and `-c`/`--coredump` options.  

The `snapshot` [field](/docs/schema-reference/#eventstartmsginfoconfigurationcurrentlibappviewsnapshot) in the process-start message records whether capturing coredumps and backtraces are enabled or not.

Apart from coredumps and stacktraces, AppView always writes two additional files to the snapshot directory:
- `info_<timestamp>` provides basic information about the process.
- `cfg_<timestamp>` records the AppView configuration in effect when the application crashed.

<span id="dynamic-configuration"></span>

### Dynamic Configuration Via the Command Directory

Separately from the `appview update` command, AppView offers another way to do dynamic configuration in real time, via the **command directory**.

To use this feature, you first need to know the PID (process ID) of the currently viewed process. You then create a `appview.<pid>` file, where:
- The `<pid>` part of the filename is the PID of the viewed process.
- Each line of the file consists of one configuration setting in the form `ENVIRONMENT_VARIABLE=value`.
  
Once per reporting period, AppView looks in its **command directory** for a `appview.<pid>` file. If it finds one where the `<pid>` part of the filename matches the PID of the current viewed process, AppView applies the configuration settings specified in the file, then deletes the file.

The command directory defaults to `/tmp` and the reporting period (a.k.a. metric summary interval) defaults to 10 seconds. You can configure them in the `libappview` section of the [config file](/docs/config-file)). See the `Command directory` and `Metric summary interval` subsections, respectively.
