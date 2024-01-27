---
title: CLI Reference
---

## CLI Reference
---

In the AppView CLI, the `appview` command takes a Linux command as an argument. That's called "scoping" the Linux command. For example, if you run `appview top`, we say you have "viewed" the `top` command.

The AppView CLI also has subcommands, which pair with `appview` to do many things. For example, if you run `appview dash`, AppView displays its dashboard.

This Reference explains how to use `appview` and its arsenal of subcommands.

### Subcommand Syntax

To execute CLI subcommands, the basic syntax is:

```
./appview <subcommand> [flags] [options]
```

### Subcommands Available

To see a list of available subcommands, enter `./appview` alone, or `./appview -h`, or `./appview --help`. This displays the basic help listing below.

```
Cribl AppView Command Line Interface

AppView is a general-purpose observable application telemetry system.

Running `appview` with no subcommands will execute the `appview run` command.

Usage:
  appview [command]

Available Commands:
  attach      View a currently-running process
  completion  Generates completion code for specified shell
  daemon      Run the appview daemon
  dash        Display appview dashboard for a previous or active session
  detach      Unview a currently-running process
  events      Outputs events for a session
  extract     Output instrumentary library files to <dir>
  rules       View or modify system-wide AppView rules
  flows       Observed flows from the session, potentially including payloads
  help        Help about any command
  history     List appview session history
  inspect     Returns information about viewed process
  k8s         Install appview in kubernetes
  logs        Display appview logs
  metrics     Outputs metrics for a session
  prom        Run the Prometheus Target
  prune       Prune deletes session history
  ps          List processes currently being viewed
  run         Executes a viewed command
  service     Configure a systemd/OpenRC service to be viewed
  snapshot    Create a snapshot for a process
  start       Install the AppView library
  stop        Stop scoping all viewed processes and services
  update      Updates the configuration of a viewed process
  version     Display appview version
  watch       Executes a viewed command on an interval

Flags:
  -h, --help          help for appview
  -z, --passthrough   AppView an application with current environment & no config.

Use "appview [command] --help" for more information about a command.
```

As noted just above, to see a specific subcommand's help or its required parameters, enter: 
`./appview <subcommand> -h` 

…or: 
`./appview help <subcommand>`.

---

### attach
---

Views a currently-running process identified by PID or ProcessName.

The `--*dest` flags accept file names like `/tmp/appview.log` or URLs like `file:///tmp/appview.log`. They may also
be set to sockets with `unix:///var/run/mysock`, `tcp://hostname:port`, `udp://hostname:port`, or `tls://hostname:port`.

#### Usage

`appview attach [flags] PID | <process_name>`

#### Examples

```
appview attach 1000
appview attach firefox 
appview attach top < appview.yml
appview attach --rootdir /path/to/host firefox 
appview attach --rootdir /path/to/host/mount/proc/<hostpid>/root 1000
appview attach --payloads 2000
```

#### Flags

```
  -a, --authtoken string      Set AuthToken for Cribl
  -b, --backtrace             Enable backtrace file generation when an application crashes.
  -d, --coredump              Enable core dump file generation when an application crashes.
  -c, --cribldest string      Set Cribl destination for metrics & events (host:port defaults to tls://)
  -e, --eventdest string      Set destination for events (host:port defaults to tls://)
  -h, --help                  help for attach
  -i, --inspect               Inspect the process after attach is complete
  -j, --json                  Output as newline delimited JSON
  -l, --librarypath string    Set path for dynamic libraries
      --loglevel string       Set appview library log level (debug, warning, info, error, none)
  -m, --metricdest string     Set destination for metrics (host:port defaults to tls://)
      --metricformat string   Set format of metrics output (statsd|ndjson) (default "ndjson")
      --metricprefix string   Set prefix for StatsD metrics, ignored if metric format isn't statsd
  -n, --nobreaker             Set Cribl to not break streams into events.
  -p, --payloads              Capture payloads of network transactions
  -R, --rootdir               Path to root filesystem of target namespace
  -u, --userconfig string     AppView an application with a user specified config file; overrides all other settings.
  -v, --verbosity int         Set appview metric verbosity (default 4)
```

### completion
---

Generates completion code for specified shell.

#### Usage

`appview completion [flags] [bash|zsh]`

#### Examples

```
appview completion bash > /etc/bash_completion.d/appview # Generate and install appview autocompletion for bash
source <(appview completion bash)                      # Generate and load appview autocompletion for bash
```

#### Flags

```
  -h, --help   help for completion
```

### dash
---

Displays an interactive dashboard with an overview of what's happening with the selected session.

#### Usage

`appview dash [flags]`

#### Examples

`appview dash`

#### Flags

```
  -h, --help     help for dash
  -i, --id int   Display info from specific from session ID (default -1)
```

### detach
---

Unviews a currently-running process identified by PID or process name.

#### Usage

`appview detach [flags] PID | <process_name>`

#### Examples

```
appview detach 1000
appview detach firefox
appview detach --all
appview detach 1000 --rootdir /path/to/host/mount
appview detach --rootdir /path/to/host/mount
appview detach --all --rootdir /path/to/host/mount/proc/<hostpid>/root
```

#### Flags

```
  -a, --all                   Detach from all processes
  -h, --help                  Help for detach
  -R, --rootdir               Path to root filesystem of target namespace
  -w, --wait                  Wait for detach to complete
```

### events
---
Outputs events for a session. You can obtain detailed information about each event by inputting the Event ID as a positional parameter. (By default, the Event ID appears in blue, in `[]`'s at the left.) You can provide filters to narrow down by name (e.g., `http`, `net`, `fs`, or `console`), or by field (e.g., `fs.open`, `stdout`, or `net.open`). You can use JavaScript expressions to further refine the query, and to express logic.

#### Usage

`appview events [flags] ([eventId])`

#### Examples

```
Examples:
appview events
appview events m61
appview events --sourcetype http
appview events --source stderr
appview events --match file
appview events --fields net_bytes_sent,net_bytes_recv --match net_bytes
appview events --follow
appview events --all
appview events --allfields
appview events --id 4
appview events --sort _time --reverse
appview events --eval 'sourcetype!="net"'
appview events -n 1000 -e 'sourcetype!="console" && source.indexOf("cribl.log") == -1 && (data["file.name"] || "").indexOf("/proc") == -1'
```

#### Flags

```
  -a, --all                  Show all events
      --allfields            Displaying hidden fields
      --color                Force color on (if tty detection fails or piping)
  -e, --eval string          Evaluate JavaScript expression against event. Must return truthy to print event.
                             Note: Post-processes after matching, not guaranteed to return last <n> events.
      --fields strings       Display the names and values for specified fields only, for each event (look at JSON output for field names)
  -f, --follow               Follow a file, like tail -f
  -h, --help                 help for events
  -i, --id int               Display info from specific from session ID (default -1)
  -j, --json                 Output as newline delimited JSON
  -n, --last int             Show last <n> events (default 20)
  -m, --match string         Display events containing supplied string
  -r, --reverse              Reverse sort to ascending. Must be combined with --sort
      --sort string          Sort descending by field (look at JSON output for field names)
  -s, --source strings       Display events matching supplied sources
  -t, --sourcetype strings   Display events matching supplied sourcetypes
```

### extract
---
Outputs `libappview.so` and `appview.yml` to the provided directory. You can configure these files to instrument any application, and to output the data to any existing tool using simple TCP protocols.

The `--*dest` flags accept file names like `/tmp/appview.log` or URLs like `file:///tmp/appview.log`. They may also
be set to sockets with `unix:///var/run/mysock`, `tcp://hostname:port`, `udp://hostname:port`, or `tls://hostname:port`.

#### Usage

  `appview extract [flags] (<dir>)`

#### Aliases
  `extract`, `excrete`, `expunge`, `extricate`, `exorcise`

#### Examples

```
appview extract
appview extract /opt/libappview
appview extract --metricdest tcp://some.host:8125 --eventdest tcp://other.host:10070 .
```

#### Flags
 
```
  -a, --authtoken string      Set AuthToken for Cribl
  -c, --cribldest string      Set Cribl destination for metrics & events (host:port defaults to tls://)
  -e, --eventdest string      Set destination for events (host:port defaults to tls://)
  -h, --help                  Help for extract
  -m, --metricdest string     Set destination for metrics (host:port defaults to tls://)
      --metricformat string   Set format of metrics output (statsd|ndjson); default is "ndjson"
      --metricprefix string   Set prefix for StatsD metrics, ignored if metric format isn't statsd
  -n, --nobreaker             Set Cribl to not break streams into events
  -p, --parents               Create any missing intermediate pathname components in provided directory parameter
```

### rules
---

View or modify system-wide AppView rules to automatically view a set of processes. You can add or remove a single process at a time.

#### Usage

`appview rules [flags]`

#### Examples

```
appview rules
appview rules --rootdir /path/to/host/root --json
appview rules --add nginx
appview rules --add nginx < appview.yml
appview rules --add java --arg myServer 
appview rules --add firefox --rootdir /path/to/host/root
appview rules --remove chromium
```

#### Flags

```
      --add string            Add an entry to the global rules
      --arg string            Argument to the command to be added to the rules
  -a, --authtoken string      Set AuthToken for Cribl
  -b, --backtrace             Enable backtrace file generation when an application crashes.
  -d, --coredump              Enable core dump file generation when an application crashes.
  -c, --cribldest string      Set Cribl destination for metrics & events (host:port defaults to tls://)
  -e, --eventdest string      Set destination for events (host:port defaults to tls://)
  -h, --help                  help for rules
  -j, --json                  Output as newline delimited JSON
  -l, --librarypath string    Set path for dynamic libraries
      --loglevel string       Set appview library log level (debug, warning, info, error, none)
  -m, --metricdest string     Set destination for metrics (host:port defaults to tls://)
      --metricformat string   Set format of metrics output (statsd|ndjson|prometheus) (default "ndjson")
  -n, --nobreaker             Set Cribl to not break streams into events.
  -p, --payloads              Capture payloads of network transactions
      --remove string         Remove an entry from the global rules
  -R, --rootdir string        Path to root filesystem of target namespace
      --source string         Source identifier for a rules entry
      --unixpath string       Path to the unix socket
  -u, --userconfig string     AppView an application with a user specified config file; overrides all other settings.
  -v, --verbosity int         Set appview metric verbosity (default 4)
```

### flows
---

Displays observed flows from the given session. If run with payload capture on, outputs full payloads from the flow.

#### Usage

<!-- TO-DO How should we notate ID arguments? see also appview events -->

`appview flows [flags] <sessionId>`

#### Examples

```
appview flows                # Displays all flows
appview flows 124x3c         # Displays more info about the flow
appview flows --in 124x3c    # Displays the inbound payload of that flow
appview flows --out 124x3c   # Displays the outbound payload of that flow
appview flows -p 0.0.0.0/24  # Displays flows in that subnet range
appview flows --sort net_host_port --reverse  # Sort flows by ascending host port
```

#### Flags

```
  -a, --all           Show all flows
  -h, --help          Help for flows
  -i, --id int        Display flows from specific from session ID (default -1)
      --in            Output contents of the inbound payload. Requires flow ID specified.
  -j, --json          Output as newline-delimited JSON
  -n, --last int      Show last <n> flows (default 20)
      --out           Output contents of the outbound payload. Requires flow ID specified.
  -p, --peer ipNet    Filter to peers in the given network
  -r, --reverse       Reverse sort to ascending
  -s, --sort string   Sort descending by field (look at JSON output for field names)
```

### help
---

Displays help content for any AppView subcommand. Just type `appview help [subcommand]` for full details.

#### Usage

`appview help [subcommand] [flags]`

#### Examples

`appview help run`

### history
---

Prints information about sessions. Every time you view a command, that is called an AppView session. Each session has a directory which is referenced by a session ID. By default, the AppView CLI stores all the information it collects during a given session in that session's directory. When you run `history`, you see a listing of sessions, one session per viewed command, along with information about when the session started, how many events were output during the session, and so on.

#### Usage

`appview history [flags]`

#### Aliases

`history, hist`

#### Examples

```
appview history                    # Displays session history
appview hist                       # Shortcut for appview history
appview hist -r                    # Displays running sessions
appview hist --id 2                # Displays detailed information for session 2
appview hist -n 50                 # Displays last 50 sessions
appview hist -d                    # Displays directory for the last session
cat $(appview hist -d)/args.json   # Outputs contents of args.json in the appview history directory for the current session
```

#### Flags

```
  -a, --all        List all sessions
  -d, --dir        Output just directory (with -i)
  -h, --help       Help for history
  -i, --id int     Display info from specific from session ID (default -1)
  -n, --last int   Show last <n> sessions (default 20)
  -r, --running    List running sessions
```

### inspect
---

Returns information on viewed process identified by PID.

#### Usage

`appview inspect [flags]`

#### Examples

```
appview inspect
appview inspect 1000
appview inspect --all --json
appview inspect 1000 --rootdir /path/to/host/mount
appview inspect --all --rootdir /path/to/host/mount
appview inspect --all --rootdir /path/to/host/mount/proc/<hostpid>/root
```

#### Flags

```
  -a, --all             Inspect all processes
  -h, --help            Help for inspect
  -j, --json            Output as newline delimited JSON without pretty printing
  -R, --rootdir         Path to root filesystem of target namespace
```

### k8s
---

Prints configurations to pass to `kubectl`, which then automatically instruments newly-launched containers. This installs a mutating admission webhook, which adds an `initContainer` to each pod. The webhook also sets environment variables that install AppView for all processes in that container.

The `--*dest` flags accept file names like `/tmp/appview.log`; URLs like `file:///tmp/appview.log`; or sockets specified with the pattern `unix:///var/run/mysock`, `tcp://hostname:port`, `udp://hostname:port`, or `tls://hostname:port`.

#### Usage

`appview k8s [flags]`

#### Examples

```
appview k8s --metricdest tcp://some.host:8125 --eventdest tcp://other.host:10070 | kubectl apply -f -
kubectl label namespace default appview=enabled
```

#### Flags

```
      --app string            Name of the app in Kubernetes (default "appview")
  -a, --authtoken string      Set AuthToken for Cribl Stream
      --certfile string       Certificate file for TLS in the container (mounted secret) (default "/etc/certs/tls.crt")
  -c, --cribldest string      Set Cribl Stream destination for metrics & events (host:port defaults to tls://)
      --debug                 Turn on debug logging in the appview webhook container
  -e, --eventdest string      Set destination for events (host:port defaults to tls://)
  -h, --help                  Help for k8s
      --keyfile string        Private key file for TLS in the container (mounted secret) (default "/etc/certs/tls.key")
  -m, --metricdest string     Set destination for metrics (host:port defaults to tls://)
      --metricformat string   Set format of metrics output (statsd|ndjson); default is "ndjson"
      --metricprefix string   Set prefix for StatsD metrics, ignored if metric format isn't statsd
      --namespace string      Name of the namespace in which to install; default is "default"
  -n, --nobreaker             Set Cribl Stream to not break streams into events
      --noexporter            Disable StatsD to Prometheus Exporter deployment
      --port int              Port to listen on (default 4443)
      --promport int          Specify StatsD to Prometheus Exporter port for Prometheus HTTP metrics requests (default 9090)
      --server                Run Webhook server
      --signername string     Name of the signer used to sign the certificate request for the AppView Admission Webhook (default "kubernetes.io/kubelet-serving")
      --version string        Version of appview to deploy

```

### logs
---

Displays internal AppView logs for troubleshooting AppView itself.

#### Usage

`appview logs [flags]`

#### Examples

```
appview logs
```

#### Flags

```
  -h, --help             Help for logs
  -i, --id int           Display logs from specific from session ID (default -1)
  -n, --last int         Show last <n> lines (default 20)
  -s, --appview            Show appview.log (from CLI) instead of ldappview.log (from library)
  -S, --service string   Display logs from a systemd service instead of a session
 
```

### metrics
---

Outputs metrics for a session.

#### Usage

`appview metrics [flags]`

#### Examples

```
appview metrics
appview metrics -m net.error,fs.error
appview metrics -m net.tx -g
```

#### Flags

```
  -c, --cols             Display metrics as columns. Must be combined with -m
  -g, --graph            Graph this metric. Must be combined with -m
  -h, --help             Help for metrics
  -i, --id int           Display info from specific from session ID (default -1)
  -m, --metric strings   Display for specified metrics only (comma-separated)
  -u, --uniq             Display first instance of each unique metric
```

### prune
----

Prunes (deletes) one or more sessions from the history.

#### Usage

`appview prune [flags]`

#### Examples

```
appview prune -k 20
appview prune -a
appview prune -d 1
```

#### Flags

Negative arguments are not allowed.

```
  -a, --all          Delete all sessions
  -d, --delete int   Delete last <n> sessions
  -f, --force        Do not prompt for confirmation
  -h, --help         Help for prune
  -k, --keep int     Keep last <n> sessions, delete all others
```

### ps
---

Lists all viewed processes. This means processes whose functions AppView is interposing (which means that the AppView library was loaded, and the AppView reporting thread is running, in those processes, too).

#### Usage

`appview ps`

#### Examples

```
appview ps
appview ps --json
appview ps --rootdir /path/to/host/mount
appview ps --rootdir /path/to/host/mount/proc/<hostpid>/root`,
```

#### Flags

```
  -j, --json            Output as newline delimited JSON without pretty printing
  -R, --rootdir         Path to root filesystem of target namespace
```

### run
----

Executes a viewed command. By default, calling `appview` with no subcommands will run the executables you pass as arguments to 
`appview`. However, `appview` allows for additional arguments to be passed to `run`, to capture payloads or to increase metrics' 
verbosity. Must be called with the `--` flag, e.g., `appview run -- <command>`, to prevent AppView from attempting to parse flags passed to the executed command.

The `--*dest` flags accept file names like `/tmp/appview.log`; URLs like `file:///tmp/appview.log`; or sockets specified with the pattern `unix:///var/run/mysock`, `tcp://hostname:port`, `udp://hostname:port`, or `tls://hostname:port`.

#### Usage

`appview run [flags] [command]`

#### Examples

```
appview run -- /bin/echo "foo"
appview run -- perl -e 'print "foo\n"'
appview run --payloads -- nc -lp 10001
appview run -- curl https://wttr.in/94105
appview run -c tcp://127.0.0.1:10091 -- curl https://wttr.in/94105
appview run -c edge -- top
```

#### Flags

```
  -a, --authtoken string      Set AuthToken for Cribl
  -b, --backtrace             Enable backtrace file generation when an application crashes.
  -d, --coredump              Enable core dump file generation when an application crashes.
  -c, --cribldest string      Set Cribl destination for metrics & events (host:port defaults to tls://)
  -e, --eventdest string      Set destination for events (host:port defaults to tls://)
  -h, --help                  help for run
  -l, --librarypath string    Set path for dynamic libraries
      --loglevel string       Set appview library log level (debug, warning, info, error, none)
  -m, --metricdest string     Set destination for metrics (host:port defaults to tls://)
      --metricformat string   Set format of metrics output (statsd|ndjson) (default "ndjson")
      --metricprefix string   Set prefix for StatsD metrics, ignored if metric format isn't statsd
  -n, --nobreaker             Set Cribl to not break streams into events.
  -p, --payloads              Capture payloads of network transactions
  -u, --userconfig string     AppView an application with a user specified config file; overrides all other settings.
  -v, --verbosity int         Set appview metric verbosity (default 4)
```

### service
---

Configures the specified `systemd`/`OpenRC` service to be viewed upon starting.

#### Usage

`appview service SERVICE [flags]`

#### Examples

```
appview service cribl -c tls://in.my-instance.cribl.cloud:10090
```

#### Flags

```
  -a, --authtoken string      Set AuthToken for Cribl Stream
  -c, --cribldest string      Set Cribl Stream destination for metrics & events (host:port defaults to tls://)
  -e, --eventdest string      Set destination for events (host:port defaults to tls://)
      --force                 Bypass confirmation prompt
  -h, --help                  Help for service
  -m, --metricdest string     Set destination for metrics (host:port defaults to tls://)
      --metricformat string   Set format of metrics output (statsd|ndjson); default is "ndjson"
      --metricprefix string   Set prefix for StatsD metrics, ignored if metric format isn't statsd
  -n, --nobreaker             Set Cribl Stream to not break streams into events
  -u, --user string           Specify owner username
  
```

### snapshot
---

Create a snapshot for a process. Snapshot file/s will be created in `/tmp/appview/[PID]/`.

#### Usage

`appview snapshot [PID] [flags]`

#### Flags

`  -h, --help   help for snapshot`

### start
---

Install the AppView library to:
/usr/lib/appview/<version>/ with admin privileges, or 
/tmp/appview/<version>/ otherwise

#### Usage

`appview start [flags]`

#### Examples

```
appview start
appview start --rootdir /hostfs
```

#### Flags

```
  -h, --help             help for start
  -p, --rootdir string   Path to root filesystem of target namespace
```

### stop
---
Performs the following actions:
	- Removal of /etc/ld.so.preload contents
	- Removal of the rules file from /usr/lib/appview/appview_rules
	- Detach from all currently viewed processes

The command does not uninstall appview or libappview from /usr/lib/appview or /tmp/appview
or remove any service configurations.

#### Usage

`appview stop [flags]`

#### Examples

`appview stop`

#### Flags

```
  -f, --force      Use this flag when you're sure you want to run appview stop
  -R, --rootdir    Path to root filesystem of target namespace
  -h, --help       help for stop
```

### update
---

Updates configuration of viewed process identified by PID.

#### Usage

`appview update [flags]`

#### Examples

```
appview update 1000 --config appview_cfg.yml
appview update 1000 < appview_cfg.yml
appview update 1000 --json < appview_cfg.yml
appview update 1000 --rootdir /path/to/host/mount --config appview_cfg.yml
appview update 1000 --rootdir /path/to/host/mount/proc/<hostpid>/root < appview_cfg.yml
```

#### Flags

```
Flags:
  -i, --inspect         Inspect the process after the update is complete
  -c, --config string   Path to configuration file
  -h, --help            help for update
  -j, --json            Output as newline delimited JSON without pretty printing
  -R, --rootdir         Path to root filesystem of target namespace
```

### version
----

Outputs version info.

#### Usage

`appview version [flags]`

#### Examples

```
appview version
appview version --date
appview version --summary
appview version --tag
```

#### Flags

```
      --date      Output just the date
  -h, --help      Help for version
      --summary   Output just the summary
      --tag       Output just the tag
```

### watch
---

Executes a viewed command on an interval. Must be called with the `--` flag, e.g., `appview watch -- <command>`, to prevent AppView from attempting to parse flags passed to the executed command.

#### Usage

`appview watch [flags]`

#### Examples

```
appview watch -i 5s -- /bin/echo "foo"
appview watch --interval=1m-- perl -e 'print "foo\n"'
appview watch --interval=5s --payloads -- nc -lp 10001
appview watch -i 1h -- curl https://wttr.in/94105
appview watch --interval=10s -- curl https://wttr.in/94105
```

#### Flags

```
  -a, --authtoken string      Set AuthToken for Cribl
  -b, --backtrace             Enable backtrace file generation when an application crashes.
  -d, --coredump              Enable core dump file generation when an application crashes.
  -c, --cribldest string      Set Cribl destination for metrics & events (host:port defaults to tls://)
  -e, --eventdest string      Set destination for events (host:port defaults to tls://)
  -h, --help                  help for watch
  -i, --interval string       Run every <x>(s|m|h)
  -l, --librarypath string    Set path for dynamic libraries
      --loglevel string       Set appview library log level (debug, warning, info, error, none)
  -m, --metricdest string     Set destination for metrics (host:port defaults to tls://)
      --metricformat string   Set format of metrics output (statsd|ndjson) (default "ndjson")
      --metricprefix string   Set prefix for StatsD metrics, ignored if metric format isn't statsd
  -n, --nobreaker             Set Cribl to not break streams into events.
  -p, --payloads              Capture payloads of network transactions
  -u, --userconfig string     AppView an application with a user specified config file; overrides all other settings.
  -v, --verbosity int         Set appview metric verbosity (default 4)
```
