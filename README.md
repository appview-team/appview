[![Build & Test](https://github.com/appview-team/appview/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/appview-team/appview/actions/workflows/build-and-test.yml)
![GitHub Release](https://img.shields.io/github/v/release/appview-team/appview?label=Release&link=https%3A%2F%2Fgithub.com%2Fappview-team%2Fappview%2Freleases)

# AppView

AppView is an open source, runtime-agnostic instrumentation utility for any Linux command or application. It helps users to explore, understand, and gain visibility with **no code modification**.

AppView provides the fine-grained observability of a proxy/service mesh, without the latency of a sidecar. It emits APM-like metric and event data, in open formats, to existing log and metric tools.

It’s like [strace](https://github.com/strace/strace) meets [tcpdump](https://www.tcpdump.org/) – but with consumable output for events like file access, DNS, and network activity, and StatsD-style metrics for applications. AppView can also look inside encrypted payloads, offering WAF-like visibility without proxying traffic.

<br />
<br />

```mermaid
graph LR
    A[Application] --> B[libappview]
    A[Application]--> C[libgnutls]
    A[Application]--> D[libc]
    C --> D
    B --> D
    B --> C
    D --> I[Kernel]
    B --> E[In-memory Queue]
    E -.-> F[Reporting Thread]
    F --> G[Network Destination]
    F --> H[File System Destination]
    style B fill:#f3ffec,stroke:#89db70
    style E fill:#fafafa,stroke:#a6a6a6
    style F fill:#fafafa,stroke:#a6a6a6
    style G fill:#fafafa,stroke:#a6a6a6
    style H fill:#fafafa,stroke:#a6a6a6
```

## 💎 Features

- Generate metrics on process and application performance.
- Generate events, reporting on network, file, logs, console messages and http/s activity.
- Capture (decrypted) payload data without the need for keys.
- Generate a stack trace, and a core dump when an application crashes.
- Generate network flow information.
- Create a report on unique file and network activity.
- Install AppView in a Kubernetes cluster.
- Secure file and network access in an application.
- Instrument both static and dynamic executables.
- Attach to processes *while they are running* or start when the process does.
- Normalize and forward metrics and events, in real time, to remote systems.
- Summarize metrics and detect protocols.

## 💡 Use Cases

You could do any of the following with AppView:

- Generate a privacy report for an application like VS Code, to discover what files and remote networks the extensions are accessing.
- Power a Grafana or Cribl Search dashboard with an application's metric and event data.
- Generate a report on `npm ci` to capture a list of dependency sources.
- Send live metrics from nginx to a Datadog server.
- Run Firefox from the AppView CLI, and view results on a terminal-based dashboard.
- Configure Slack security alerts when certain resources are accessed, or functions are GOT hooked in your application.
- Send HTTP events from Slack to a specified Splunk server.
- Run `appview service sshd` in the AppView CLI, so that the next time the `sshd` service starts, it will be viewed.
- Generate a footprint of an application in a long running test that can be used to compare future behavior, i.e. to guard against dependency chain attacks.

## ✨ Example

```
appview curl wttr.in  # Run a curl command, with AppView loaded 
appview events        # View events from the most recent session
appview metrics       # View metrics from the most recent session
appview flows         # View network flows from the most recent session
```
Events output:
```
[Bu] Jan 15 15:53:19 nginx worker process net net.open net_peer_ip:127.0.0.1 net_peer_port:60240 net_host_ip:0.0.0.0 net_host_port:80 net_protocol:http net_transport:IP.TCP
[BB] Jan 15 15:53:19 nginx worker process net net.app fd:3 host:precision pid:1504 proc:"ker process" protocol:HTTP
[CH] Jan 15 15:53:19 nginx worker process http http.req http_host:localhost http_method:GET http_scheme:http http_target:/
[uQ] Jan 15 15:53:19 nginx worker process fs fs.open file:/var/www/html/index.nginx-debian.html
[1Z] Jan 15 15:53:19 nginx worker process http http.resp http_host:localhost http_method:GET http_scheme:http http_target:/ http_response_content_length:612
...
```
Metrics output:
```
NAME             VALUE           TYPE     UNIT           PID        TAGS
proc.start       1               Count    process        3793875    args: curl wttr.in,gid: 1000,groupname: sean,host: precision,proc: curl,uid:…
proc.cpu         1.356123e+06    Count    microsecond    3793875    host: precision,proc: curl
proc.cpu_perc    13.56123        Gauge    percent        3793875    host: precision,proc: curl
proc.mem         125732          Gauge    kibibyte       3793875    host: precision,proc: curl
proc.thread      2               Gauge    thread         3793875    host: precision,proc: curl
...
```
Flows output:
```
ID   	HOST IP      	HOST PORT	PEER IP    	PEER PORT	LAST SENT	DURATION
6NFr9	192.168.1.44	59318    	5.9.243.187	80       	3s       	614ms
```

See [the docs](http://appview.org) for many more examples.

## 🛟 Support

AppView runs on most Linux distributions and is able to instrument **most applications**. You might be surprised to learn that AppView is even able to instrument static applications, and applications running in other containers. 

We regularly test against applications like `nginx`, `redis`, `ssh`, `curl`, `bash`, `git`, `python`, `kafka`, and `node`. We have an extensive set of [integration tests](./test/integration), validating support for these applications on both ARM and x86 architectures, even in musl-based distributions like alpine.

However, AppView *cannot*:

- Instrument Go executables built with Go 1.10 or earlier.
- Instrument static stripped Go executables built with Go 1.12 or earlier.
- Instrument Java executables that use Open JVM 6 or earlier, or Oracle JVM 6 or earlier.
- Obtain a core dump either (a) for a Go executable, or (b) in a musl libc environment.

## 🚀 Try It Out

Before you begin, ensure that your environment meets the AppView [requirements](https://appview.org/docs/requirements).

**With the Download**
```
curl -Lo appview https://github.com/appview-team/appview/releases/download/v1.0.0/appview-x86_64
curl -Ls https://github.com/appview-team/appview/releases/download/v1.0.0/appview-x86_64.md5 | md5sum -c
chmod +x appview
appview <some app>
appview metrics
sudo appview attach <already running process>
appview events -f
appview detach --all
```

**With Docker**
```
docker run --rm -it -v/:/hostfs:ro --privileged appview/appview
appview <some app> # AppView an app in the container
appview metrics
appview events
appview attach --rootdir /hostfs <process running on host> # AppView an app in the host
appview events -f
appview detach --all --rootdir /hostfs
```

**From Source**

AppView is not built or distributed like most traditional Linux software.

- Insofar as possible, we want AppView binaries to be  **Build Once, Run Anywhere**. To approach this goal, we build with a version of glibc that is (1) recent enough that the resulting binary contains references to versions of functions in the glibc library *that are still supported in the latest glibc*, yet (2) old enough that the binaries can run on a wide range of Linux platforms without having to rebuild locally.
.
- We don't build OS installation packages like DEBs or RPMs. This way, when you want to investigate a running system or build a custom container image, you can simply drop AppView in and use it.

Build from source:

```text
git clone https://github.com/appview-team/appview.git
cd appview
./install_build_tools.sh # Install dependencies (ubuntu)
make all test # Build and test
```

If you aren't on Ubuntu, or would prefer not to install the dependencies, ensure that [Docker], [BuildX], and `make` are installed, then build in a container with:

```text
make build CMD="make all"
```

Either way, the resulting binaries will be in `lib/linux/$(uname -m)/libappview.so` and `bin/linux/$(uname -m)/appview`.

We support building `x86_64` (amd64) or `aarch64` (arm64/v8) binaries by adding `ARCH=x86_64` or `ARCH=aarch64` to the `make build` command. See the [BUILD](docs/BUILD.md) doc for details.

## ℹ️  Resources

On the [AppView Website](https://appview.org/) you can:

- Get an [overview](https://appview.org/docs/how-works/) of AppView beyond the CLI.
- See [examples](https://appview.org/docs/instrumenting-an-application) of AppView usage.
- Learn about all of the CLI commands [in more depth](https://appview.org/docs/cli-reference).
- See what happens when you [connect AppView to Cribl Stream or Cribl Edge](https://appview.org/docs/cribl-integration).

_The content on that site is built from the [website/](website/) directory in this project._

## ✏️  Contributing

If you're interested in contributing to the project, you can:

- View and add to GitHub [discussions](https://github.com/appview-team/appview/discussions) discussions about future work.
- View and add GitHub [issues](https://github.com/appview-team/appview/issues) that need to be resolved.
- See our developer guides in the [docs/](./docs/) directory in this repository.

## 📄 License

AppView is licensed under the Apache License, Version 2.0. 

[Docker]: https://docs.docker.com/engine/install/
[BuildX]: https://docs.docker.com/buildx/working-with-buildx/
