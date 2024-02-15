---
title: Instrumenting an Application
---

<span id="instrumenting"></span>

# Instrumenting an Application

There are many ways to load AppView into your applications or services. Find below some examples to get you started.

<span id="ad-hoc-instrumentation"></span>

## Ad Hoc Instrumentation

<span id="new-process"></span>

### AppView a New Process
```
appview curl wttr.in  # Run a curl command, with AppView loaded 
appview events        # View events from the most recent session
appview metrics       # View metrics from the most recent session
appview flows         # View network flows from the most recent session
```

__Pro Tip__: Use the LD\_PRELOAD environment variable to load the libappview.so library into a new process without using the CLI.

<span id="existing-process"></span>

### AppView an Existing Process
```
top &               # Start top in the background (or in another terminal)
appview attach top  # Attach to the top process by name (or by pid: `pidof top`)
appview events -f   # View, and follow, events from the most recent session
appview metrics     # View metrics from the most recent session
appview flows       # View network flows from the most recent session
appview detach top  # Detach from the top process (if desired)
```

__Pro Tip__: It's also possible to View an application running inside a Docker container, from the host.

<span id="automating"></span>

## Automating AppView

<span id="system-service"></span>

### AppView a System Service
```
sudo systemctl stop nginx   # We will use nginx as an example. Stop nginx
appview service nginx       # Configure the nginx service to load AppView
sudo systemctl start nginx  # Restart the nginx service
```

<span id="group-of-processes"></span>

### AppView a Group of Processes
Use a global ruleset to automatically load AppView into processes that match the rule.
Existing processes will be attached to instantly, and new processes will load AppView.
```
appview rules --add nginx                  # Add nginx to the list of processes to load AppView
# appview rules --add nginx < appview.yml  # Optionally provide a config
appview rules --add java --arg myServer    # Add java myServer to the list of processes to load AppView
appview rules --add firefox                # Add firefox to the list of processes to load AppView
appview rules --remove firefox             # Remove firefox from the list of processes to load AppView
appview rules                              # View the current global rule set
```

