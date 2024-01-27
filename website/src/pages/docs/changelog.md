---
title: Changelog
---
# Changelog

See the AppView repo to view [all issues](https://github.com/criblio/appview/issues).

## AppView 1.4.1

2023-07-19 - Update to Feature Release

Assets are available via Docker and the Cribl CDN at the links below.

- `Docker`: `cribl/appview:1.4.1`
- `x86`: [https://cdn.cribl.io/dl/appview/1.4.1/linux/x86_64/appview](https://cdn.cribl.io/dl/appview/1.4.1/linux/x86_64/appview)
- `ARM`: [https://cdn.cribl.io/dl/appview/1.4.1/linux/aarch64/appview](https://cdn.cribl.io/dl/appview/1.4.1/linux/aarch64/appview)
- `AWS Lambda Layer for x86`: [https://cdn.cribl.io/dl/appview/1.4.1/linux/x86_64/aws-lambda-layer.zip](https://cdn.cribl.io/dl/appview/1.4.1/linux/x86_64/aws-lambda-layer.zip)
- `AWS Lambda Layer for ARM`: [https://cdn.cribl.io/dl/appview/1.4.1/linux/aarch64/aws-lambda-layer.zip](https://cdn.cribl.io/dl/appview/1.4.1/linux/aarch64/aws-lambda-layer.zip)

To obtain the MD5 checksum for any file above, add `.md5` to the file path.

Assets other than AWS Lambda Layers are available in the [Docker container](https://hub.docker.com/r/cribl/appview/tags) tagged `cribl/appview:1.4.1`.

Cribl's AppView Helm Chart is available here: [https://github.com/criblio/helm-charts/tree/master/helm-chart-sources/appview](https://github.com/criblio/helm-charts/tree/master/helm-chart-sources/appview)

### New Features and Improvements

Beginning in version 1.4.1, AppView applies Rules to new processes in existing containers. That is, within any container that exists when you start scoping by Rule, AppView will automatically begin monitoring any **new** process that matches the Rule. 

This holds true whether you are using the `appview rules` CLI command, or the **AppView Rules** settings in the AppView Source UI in Cribl Edge or Cribl Stream. Related issue: [1523](https://github.com/criblio/appview/issues/1523).

### Fixes

- [1584](https://github.com/criblio/appview/issues/1584) AppView 1.4.1 fixes a problem introduced in AppView 1.4.0 where, while upgrading, multiple AppView libraries were loaded into one process, causing a crash.

## AppView 1.4.0

2023-07-19 - Feature Release

**This release has been replaced by AppView 1.4.1.** Therefore, the AppView recommends against downloading AppView 1.4.0 assets from Docker or the Cribl CDN.

### New Features and Improvements

AppView 1.4.0 integrates more deeply with Cribl Edge:

- Using [Cribl Edge](https://docs.cribl.io/edge/) to "drive," you can now appview individual processes by PID on an Edge Node, and/or appview multiple processes by Rule, on an entire Edge Fleet.
- Using Cribl Edge's [Prometheus Scraper Source](https://docs.cribl.io/edge/sources-prometheus/), you can now obtain metrics in [OpenMetrics](https://github.com/OpenObservability/OpenMetrics/blob/main/specification/OpenMetrics.md) format from processes running in Kubernetes containers.
- The [AppView CLI](/docs/cli-reference) has new functionality to support the Cribl Edge integration, including: 
    - A new `rules` command that specifies processes to appview, along with configs to apply to those processes.
    - Improvements to the `start`, `stop`, `attach`, `detach`, `update`, and `inspect` commands.

In general, AppView 1.4.0 expands support for Kubernetes.

### Fixes

- [1567](https://github.com/criblio/appview/issues/1567) Scoping top on Fedora no longer produces an `unknown terminal type` error.
- [1521](https://github.com/criblio/appview/issues/1521), [1557](https://github.com/criblio/appview/issues/1557) Memory leaks are fixed.
- [1547](https://github.com/criblio/appview/issues/1547) `appview ps` no longer fails to shows child processes.
- [1529](https://github.com/criblio/appview/issues/1529) It is now possible to appview postgres using the `LD_PRELOAD` mechanism.
- [1515](https://github.com/criblio/appview/issues/1515) When AppView is in the user's `$PATH`` and the viewed application performs a fork and exec, AppView no longer crashes the application.
- [1502](https://github.com/criblio/appview/issues/1502) Scoping bash no longer uses an (incorrect) configuration that is other than the one specified.

## AppView 1.3.4

2023-06-14 - Maintenance Release

Assets are available via Docker and the Cribl CDN at the links below.

- `Docker`: `cribl/appview:1.3.4`
- `x86`: [https://cdn.cribl.io/dl/appview/1.3.4/linux/x86_64/appview](https://cdn.cribl.io/dl/appview/1.3.4/linux/x86_64/appview)
- `ARM`: [https://cdn.cribl.io/dl/appview/1.3.4/linux/aarch64/appview](https://cdn.cribl.io/dl/appview/1.3.4/linux/aarch64/appview)
- `AWS Lambda Layer for x86`: [https://cdn.cribl.io/dl/appview/1.3.4/linux/x86_64/aws-lambda-layer.zip](https://cdn.cribl.io/dl/appview/1.3.4/linux/x86_64/aws-lambda-layer.zip)
- `AWS Lambda Layer for ARM`: [https://cdn.cribl.io/dl/appview/1.3.4/linux/aarch64/aws-lambda-layer.zip](https://cdn.cribl.io/dl/appview/1.3.4/linux/aarch64/aws-lambda-layer.zip)

To obtain the MD5 checksum for any file above, add `.md5` to the file path.

Assets other than AWS Lambda Layers are available in the [Docker container](https://hub.docker.com/r/cribl/appview/tags) tagged `cribl/appview:1.3.4`.

### Fixes

- [1499](https://github.com/criblio/appview/issues/1499) AppView no longer exposes the `backtrace` symbol. Before this fix, certain viewed applications would segfault when they called `backtrace`.
- [1491](https://github.com/criblio/appview/issues/1491) When you run AppView in a Kubernetes pod, the pod now starts normally even when the pod definition is missing the `label` section.
- [1481](https://github.com/criblio/appview/issues/1481) When scoping Java apps that use SSL and that are run on certain JREs, AppView no longer causes the viewed app to segfault.

## AppView 1.3.3

2023-05-17 - Maintenance Release

Assets are available via Docker and the Cribl CDN at the links below.

- `Docker`: `cribl/appview:1.3.3`
- `x86`: [https://cdn.cribl.io/dl/appview/1.3.3/linux/x86_64/appview](https://cdn.cribl.io/dl/appview/1.3.3/linux/x86_64/appview)
- `ARM`: [https://cdn.cribl.io/dl/appview/1.3.3/linux/aarch64/appview](https://cdn.cribl.io/dl/appview/1.3.3/linux/aarch64/appview)
- `AWS Lambda Layer for x86`: [https://cdn.cribl.io/dl/appview/1.3.3/linux/x86_64/aws-lambda-layer.zip](https://cdn.cribl.io/dl/appview/1.3.3/linux/x86_64/aws-lambda-layer.zip)
- `AWS Lambda Layer for ARM`: [https://cdn.cribl.io/dl/appview/1.3.3/linux/aarch64/aws-lambda-layer.zip](https://cdn.cribl.io/dl/appview/1.3.3/linux/aarch64/aws-lambda-layer.zip)

To obtain the MD5 checksum for any file above, add `.md5` to the file path.

Assets other than AWS Lambda Layers are available in the [Docker container](https://hub.docker.com/r/cribl/appview/tags) tagged `cribl/appview:1.3.3`.

### Improvements

AppView 1.3.3 removes our support for two Go versions that the Go project no longer supports: [1.9](https://go.dev/doc/go1.9) and [1.10](https://go.dev/doc/go1.10). This makes the AppView code less complicated and more robust. Related issue: [#1452](https://github.com/criblio/appview/issues/1452).

### Fixes

- [1461](https://github.com/criblio/appview/issues/1461) In libc musl environments, attaching to an already-running but idle process now works normally. (Idle means not producing events at that moment.) 

  - Before this fix, commands that rely on the IPC mechanism and/or the presence of a running periodic thread (e.g., `appview ps`, `appview inspect`, `appview update`) would not work, because AppView's periodic thread would not start. For example, attaching AppView to the Nginx master process running in an Alpine container, you would be unable to use the CLI to interact with the process. (This problem existed only in libc musl environments.)

- [1412](https://github.com/criblio/appview/issues/1412) Node.js apps compiled as [Position Independent Executables](https://www.redhat.com/en/blog/position-independent-executables-pie) (PIE) no longer seg fault when viewed.

## AppView 1.3.2

2023-04-19 - Maintenance Release

Assets are available via Docker and the Cribl CDN at the links below.

- `Docker`: `cribl/appview:1.3.2`
- `x86`: [https://cdn.cribl.io/dl/appview/1.3.2/linux/x86_64/appview](https://cdn.cribl.io/dl/appview/1.3.2/linux/x86_64/appview)
- `ARM`: [https://cdn.cribl.io/dl/appview/1.3.2/linux/aarch64/appview](https://cdn.cribl.io/dl/appview/1.3.2/linux/aarch64/appview)
- `AWS Lambda Layer for x86`: [https://cdn.cribl.io/dl/appview/1.3.2/linux/x86_64/aws-lambda-layer.zip](https://cdn.cribl.io/dl/appview/1.3.2/linux/x86_64/aws-lambda-layer.zip)
- `AWS Lambda Layer for ARM`: [https://cdn.cribl.io/dl/appview/1.3.2/linux/aarch64/aws-lambda-layer.zip](https://cdn.cribl.io/dl/appview/1.3.2/linux/aarch64/aws-lambda-layer.zip)

To obtain the MD5 checksum for any file above, add `.md5` to the file path.

Assets other than AWS Lambda Layers are available in the [Docker container](https://hub.docker.com/r/cribl/appview/tags) tagged `cribl/appview:1.3.2`.

### New Features and Improvements

AppView 1.3.2 introduces support for [Go version 1.20](https://go.dev/doc/go1.20).

### Fixes

- [#1409](https://github.com/criblio/appview/issues/1409) The `appview k8s` command now preloads the application image into the k8s cluster, avoiding `failed to call webhook` errors.
- [#1365](https://github.com/criblio/appview/issues/1365) The `appview k8s` command now calls an up-to-date version of a k8s library needed for obtaining signed certificates from the k8s Certificate Authority. This fixes a problem where running in newer versions of k8s produced `the server doesn't have a resource type "certificatesigningrequests"` errors. 
- [#1408](https://github.com/criblio/appview/issues/1408) On older (pre-1.1.24) versions of Alpine and other distributions based on musl libc, the `appview run` command now works as expected, and no longer encounters `secure_getenv: symbol not found` errors.
- [#1170](https://github.com/criblio/appview/issues/1170) The way AppView runs `pcre2` functions internally is improved, fixing various problems including Go crashes under certain conditions.
- [#1147](https://github.com/criblio/appview/issues/1147) Docker no longer crashes when viewed. (This bug appears only in pre-1.3 versions of AppView.)

## AppView 1.3.1

2023-03-21 - Maintenance Release

Assets are available via Docker and the Cribl CDN at the links below.

- `Docker`: `cribl/appview:1.3.1`
- `x86`: [https://cdn.cribl.io/dl/appview/1.3.1/linux/x86_64/appview](https://cdn.cribl.io/dl/appview/1.3.1/linux/x86_64/appview)
- `ARM`: [https://cdn.cribl.io/dl/appview/1.3.1/linux/aarch64/appview](https://cdn.cribl.io/dl/appview/1.3.1/linux/aarch64/appview)
- `AWS Lambda Layer for x86`: [https://cdn.cribl.io/dl/appview/1.3.1/linux/x86_64/aws-lambda-layer.zip](https://cdn.cribl.io/dl/appview/1.3.1/linux/x86_64/aws-lambda-layer.zip)
- `AWS Lambda Layer for ARM`: [https://cdn.cribl.io/dl/appview/1.3.1/linux/aarch64/aws-lambda-layer.zip](https://cdn.cribl.io/dl/appview/1.3.1/linux/aarch64/aws-lambda-layer.zip)

To obtain the MD5 checksum for any file above, add `.md5` to the file path.

Assets other than AWS Lambda Layers are available in the [Docker container](https://hub.docker.com/r/cribl/appview/tags) tagged `cribl/appview:1.3.1`.

### Security Fixes

- AppView 1.3.1 updates dependencies to address concerns about vulnerabilities recently resolved in Go libraries. The relevant vulnerabilities are [CVE-2022-41721](https://nvd.nist.gov/vuln/detail/CVE-2022-41721), [CVE-2022-41723](https://nvd.nist.gov/vuln/detail/CVE-2022-41723), and [CVE-2022-41717](https://nvd.nist.gov/vuln/detail/CVE-2022-41717).

## AppView 1.3.0

2023-03-21 - Feature Release

Assets are available via Docker and the Cribl CDN at the links below.

- `Docker`: `cribl/appview:1.3.0`
- `x86`: [https://cdn.cribl.io/dl/appview/1.3.0/linux/x86_64/appview](https://cdn.cribl.io/dl/appview/1.3.0/linux/x86_64/appview)
- `ARM`: [https://cdn.cribl.io/dl/appview/1.3.0/linux/aarch64/appview](https://cdn.cribl.io/dl/appview/1.3.0/linux/aarch64/appview)
- `AWS Lambda Layer for x86`: [https://cdn.cribl.io/dl/appview/1.3.0/linux/x86_64/aws-lambda-layer.zip](https://cdn.cribl.io/dl/appview/1.3.0/linux/x86_64/aws-lambda-layer.zip)
- `AWS Lambda Layer for ARM`: [https://cdn.cribl.io/dl/appview/1.3.0/linux/aarch64/aws-lambda-layer.zip](https://cdn.cribl.io/dl/appview/1.3.0/linux/aarch64/aws-lambda-layer.zip)

To obtain the MD5 checksum for any file above, add `.md5` to the file path.

Assets other than AWS Lambda Layers are available in the [Docker container](https://hub.docker.com/r/cribl/appview/tags) tagged `cribl/appview:1.3.0`.

### New Features and Improvements

AppView 1.3.0 introduces features that support analyzing crashes, obtaining snapshots of processes, troubleshooting transports, and dynamically managing configs. Meanwhile, AppView's architecture, connection and payload handling, container-awareness, and CLI are all improved.

AppView 1.3.0 introduces support for: 

- Instrumenting Go executables on ARM.
- Scoping apps running in [LXC](https://github.com/lxc/lxc) and [LXD](https://linuxcontainers.org/lxd/introduction/) containers.

#### The Crash Analysis and Snapshot Features

Whenever a viewed app crashes, AppView can obtain a core dump, a backtrace (i.e., stack trace), or both, while capturing supplemental information in text files. 

AppView can generate a **snapshot** file containing debug information about processes that are running normally or crashing, unviewd or viewed.

#### Troubleshooting Transports and Dynamically Managing Configs  

AppView now uses **inter-process communication** ([IPC](https://tldp.org/LDP/tlk/ipc/ipc.html)) to interact with processes in new ways. These include determining whether the process is viewed or not. If the process is viewed, AppView can:
- Determine the status of the transport AppView is trying to use to convey events and metrics to a destination. This helps troubleshoot "Why am I getting no events or metrics?" scenarios.
- Retrieve or dynamically update (modify) the AppView config currently in effect.

#### Improved Handling of Connections

AppView now uses a **backoff algorithm** for connections to avoid creating excessive network traffic and log entries. When a remote destination that AppView tries to connect to rejects the connection or is not available, AppView retries the connection at a progressively slower rate.

#### Improved Handling of Payloads

When the **payloads** feature is enabled, setting `APPVIEW_PAYLOAD_TO_DISK` to `true` now guarantees that AppView will write payloads to the local directory specified in `APPVIEW_PAYLOAD_DIR`.

#### Simplified Architecture

The `ldappview` utility no longer exists, and you can use CLI commands instead; `ldappview.log` has been renamed as `libappview.log`.

#### CLI Improvements

The AppView CLI is enhanced in the following ways:
- `appview start` can now attach to processes running in rootless and nested containers.
- `appview detach`, when run with the new `--all` flag, detaches from all processes at once.
- `appview stop`, a new command, runs `appview detach --all`, removes the filter file from the system, and removes `appview` from service configurations. This undoes the effects of the `appview attach`, `appview start`, and/or `appview service` commands.
- `appview snapshot` obtains debug information about a running or crashing process, regardless of whether or not the process is viewed.
- `appview --passthrough` replaces `appview run --passthrough`.

Three commands use IPC, which is new in AppView 1.3.0. `appview inspect` and `appview update` are completely new, while `appview ps` has new capabilities thanks to IPC.
- `appview inspect` retrieves the AppView config currently in effect and determines the status of the transport AppView is trying to use.
- `appview update` modifies the current AppView config.
- `appview ps` now determines whether the processes it lists are viewed or not.

### Fixes

- [#1328](https://github.com/criblio/appview/issues/1328) Scoping Terraform – e.g., `appview terraform plan` – no longer causes Terraform to crash. 
- [#1310](https://github.com/criblio/appview/issues/1310) Follow mode – i.e., running `appview events -f` to see the viewed app's events scrolling – works correctly again, fixing a regression in recent versions of AppView.
- [#1293](https://github.com/criblio/appview/issues/1293) AppView no longer causes Redis to crash when Redis (running as a service, and viewed) receives a `GET` or `SET` command.
- [#1252](https://github.com/criblio/appview/issues/1252) AppView no longer uses the [UPX](https://upx.github.io/) executable file compressor, avoiding scenarios where some Java applications crashed when viewed.
- [#1153](https://github.com/criblio/appview/issues/1153) The `appview ps` command no longer erroneously reports that a process is viewed even after `appview detach` has been run for that process.

## AppView 1.2.2

2023-01-18 - Maintenance Release

<strong>AppView 1.2.2 fixes a critical security vulnerability in OpenSSL: [CVE-2022-3602](https://nvd.nist.gov/vuln/detail/CVE-2022-3602). Cribl strongly recommends upgrading to AppView 1.2.2 as soon as possible. See [this](https://github.com/criblio/appview/security/advisories/GHSA-j3cg-7mpq-v62p) AppView security advisory</strong>.

Assets are available via Docker and the Cribl CDN at the links below.

- `Docker`: `cribl/appview:1.2.2`
- `x86`: [https://cdn.cribl.io/dl/appview/1.2.2/linux/x86_64/appview](https://cdn.cribl.io/dl/appview/1.2.2/linux/x86_64/appview)
- `ARM`: [https://cdn.cribl.io/dl/appview/1.2.2/linux/aarch64/appview](https://cdn.cribl.io/dl/appview/1.2.2/linux/aarch64/appview)
- `AWS Lambda Layer for x86`: [https://cdn.cribl.io/dl/appview/1.2.2/linux/x86_64/aws-lambda-layer.zip](https://cdn.cribl.io/dl/appview/1.2.2/linux/x86_64/aws-lambda-layer.zip)
- `AWS Lambda Layer for ARM`: [https://cdn.cribl.io/dl/appview/1.2.2/linux/aarch64/aws-lambda-layer.zip](https://cdn.cribl.io/dl/appview/1.2.2/linux/aarch64/aws-lambda-layer.zip)

To obtain the MD5 checksum for any file above, add `.md5` to the file path.

Assets other than AWS Lambda Layers are available in the [Docker container](https://hub.docker.com/r/cribl/appview/tags) tagged `cribl/appview:1.2.2`.

### New Features and Improvements

AppView 1.2.2 introduces: 

- Support for [OCI containers](https://opencontainers.org/) run by the [Podman](https://podman.io/) container engine. See issue [#1216](https://github.com/criblio/appview/issues/1216).

- Support for writing payloads to files on disk while simultaneously sending events and metrics to Cribl Stream or Cribl Edge. To do this, use the new environment variable `APPVIEW_PAYLOAD_TO_DISK` together with existing env vars `APPVIEW_CRIBL_ENABLE` and `APPVIEW_PAYLOAD_ENABLE`, as described [here](/docs/data-routing). See issue [#1158](https://github.com/criblio/appview/issues/1158).

AppView 1.2.2 also updates the [UPX](https://upx.github.io/) executable packer from version 4.0.0 to version 4.0.1. See issue [#1214](https://github.com/criblio/appview/issues/1214).

### Fixes

- [#1258](https://github.com/criblio/appview/issues/1258) A viewed application no longer crashes when one of its network connections is [closed](/docs/schema-reference#eventnetclose) and then an attempt is made to [send](/docs/schema-reference#eventnettx) or [receive](/docs/schema-reference#eventnetrx) data on the same connection.
- [#1251](https://github.com/criblio/appview/issues/1251) A viewed, running process no longer crashes when you [change](/docs/troubleshooting#dynamic-configuration) configurations and the new configuration defines a protocol that the old configuration also defines.
- [#1197](https://github.com/criblio/appview/issues/1197) When run in a container that is itself inside a container (i.e., Docker in Docker), AppView now successfully locates the host namespace.

### Security Fixes

- [#1182](https://github.com/criblio/appview/issues/1182) AppView 1.2.2 updates OpenSSL from version 3.0.0 to version 3.0.7, fixing an OpenSSL security vulnerability, as described in [this](https://github.com/criblio/appview/security/advisories/GHSA-j3cg-7mpq-v62p) AppView security advisory.

## AppView 1.2.1

2022-12-07 - Maintenance Release

Assets are available via Docker and the Cribl CDN at the links below.

- `Docker`: `cribl/appview:1.2.1`
- `x86`: [https://cdn.cribl.io/dl/appview/1.2.1/linux/x86_64/appview](https://cdn.cribl.io/dl/appview/1.2.1/linux/x86_64/appview)
- `ARM`: [https://cdn.cribl.io/dl/appview/1.2.1/linux/aarch64/appview](https://cdn.cribl.io/dl/appview/1.2.1/linux/aarch64/appview)
- `AWS Lambda Layer for x86`: [https://cdn.cribl.io/dl/appview/1.2.1/linux/x86_64/aws-lambda-layer.zip](https://cdn.cribl.io/dl/appview/1.2.1/linux/x86_64/aws-lambda-layer.zip)
- `AWS Lambda Layer for ARM`: [https://cdn.cribl.io/dl/appview/1.2.1/linux/aarch64/aws-lambda-layer.zip](https://cdn.cribl.io/dl/appview/1.2.1/linux/aarch64/aws-lambda-layer.zip)

To obtain the MD5 checksum for any file above, add `.md5` to the file path.

Assets other than AWS Lambda Layers are available in the [Docker container](https://hub.docker.com/r/cribl/appview/tags) tagged `cribl/appview:1.2.1`.

### New Features and Improvements

AppView 1.2.1 introduces: 

- Support for attaching to processes running in [LXD and LXC containers](https://www.sumologic.com/blog/lxc-lxd-linux-containers/).
- A new `--all` or `-a` flag for the `appview detach` [command](/docs/cli-reference#detach). Running `appview detach --all` detaches AppView from all processes.

### Fixes

- [#1055](https://github.com/criblio/appview/issues/1055) AppView now successfully attaches to processes given [certain combinations of mount and PID namespaces](/docs/known-issues#appview-120) that previously caused attach to fail.
- [#1192](https://github.com/criblio/appview/issues/1192) Attempting to appview applications that use their own custom-built versions of system libraries no longer causes those applications to crash. However, scoping such applications is not supported, and AppView will return a `failed to find libc in target process` error.
- [#1186](https://github.com/criblio/appview/issues/1186) Scoping a MySQL process no longer causes `mysqld` to crash.

## AppView 1.2.0

2022-11-09 - Feature Release

Assets are available via Docker and the Cribl CDN at the links below.

- `Docker`: `cribl/appview:1.2.0`
- `x86`: [https://cdn.cribl.io/dl/appview/1.2.0/linux/x86_64/appview](https://cdn.cribl.io/dl/appview/1.2.0/linux/x86_64/appview)
- `ARM`: [https://cdn.cribl.io/dl/appview/1.2.0/linux/aarch64/appview](https://cdn.cribl.io/dl/appview/1.2.0/linux/aarch64/appview)
- `AWS Lambda Layer for x86`: [https://cdn.cribl.io/dl/appview/1.2.0/linux/x86_64/aws-lambda-layer.zip](https://cdn.cribl.io/dl/appview/1.2.0/linux/x86_64/aws-lambda-layer.zip)
- `AWS Lambda Layer for ARM`: [https://cdn.cribl.io/dl/appview/1.2.0/linux/aarch64/aws-lambda-layer.zip](https://cdn.cribl.io/dl/appview/1.2.0/linux/aarch64/aws-lambda-layer.zip)

To obtain the MD5 checksum for any file above, add `.md5` to the file path.

Assets other than AWS Lambda Layers are available in the [Docker container](https://hub.docker.com/r/cribl/appview/tags) tagged `cribl/appview:1.2.0`.

### New Features and Improvements

AppView 1.2.0 introduces substantial new functionality:

- The new `appview start` command, which takes a **filter list** as an argument. The filter list has an allowlist section for specifying which processes to appview (along with desired configs), and an optional denylist section for specifying which processes **not** to appview. Related issues: [#987](https://github.com/criblio/appview/issues/987),[#1027](https://github.com/criblio/appview/issues/1027),[#1038](https://github.com/criblio/appview/issues/1038),[#1069](https://github.com/criblio/appview/issues/1069).

- Integration with Cribl Edge and Cribl Stream version 4.0, whose built-in AppView Source now supports creating filters (equivalent to editing the filter list described above). Cribl Edge and Cribl Stream also now provide an AppView Config Editor for creating AppView config files (variants of `appview.yml`) and storing them in an AppView Config Library.  

- The new `appview detach` command, which stops scoping a process without terminating that process. Related issues: [#370](https://github.com/criblio/appview/issues/370),[#1039](https://github.com/criblio/appview/issues/1039),[#1085](https://github.com/criblio/appview/issues/1085),[#1087](https://github.com/criblio/appview/issues/1087),[#1088](https://github.com/criblio/appview/issues/1088),[#1089](https://github.com/criblio/appview/issues/1089),[#1094](https://github.com/criblio/appview/issues/1094),[#1107](https://github.com/criblio/appview/issues/1107),[#1129](https://github.com/criblio/appview/issues/1129).

- To attach to or detach from a process running in a container, it is no longer necessary to run AppView within the container. You can just appview the container processes from the host. Related issues: [#1061](https://github.com/criblio/appview/issues/1061),
[#1119](https://github.com/criblio/appview/issues/1119).

- Support for executables built with Go 1.19. (This release also removes support for executables built with Go 1.8.) Related issue: [#1073](https://github.com/criblio/appview/issues/1073).

This release also improves the `appview ps` command, which now lists all viewed processes rather than listing all processes into which the library was loaded. Related issue: [#1097](https://github.com/criblio/appview/issues/1097).

## AppView 1.1.3

2022-09-06 - Maintenance Release

Assets are available from the Cribl CDN at the links below.

- `AppView for x86`: [https://cdn.cribl.io/dl/appview/1.1.3/linux/x86_64/appview](https://cdn.cribl.io/dl/appview/1.1.3/linux/x86_64/appview)
- `AppView for ARM`: [https://cdn.cribl.io/dl/appview/1.1.3/linux/aarch64/appview](https://cdn.cribl.io/dl/appview/1.1.3/linux/aarch64/appview)
- `AWS Lambda Layer for x86`: [https://cdn.cribl.io/dl/appview/1.1.3/linux/x86_64/aws-lambda-layer.zip](https://cdn.cribl.io/dl/appview/1.1.3/linux/x86_64/aws-lambda-layer.zip)
- `AWS Lambda Layer for ARM`: [https://cdn.cribl.io/dl/appview/1.1.3/linux/aarch64/aws-lambda-layer.zip](https://cdn.cribl.io/dl/appview/1.1.3/linux/aarch64/aws-lambda-layer.zip)

To obtain the MD5 checksum for any file above, add `.md5` to the file path.

Assets other than AWS Lambda Layers are available in the [Docker container](https://hub.docker.com/r/cribl/appview/tags) tagged `cribl/appview:1.1.3`.

### Fixes

- [#1067](https://github.com/criblio/appview/issues/1067) When you run `appview attach` and then select an ID that's beyond the range of the resulting list, AppView now handles this gracefully, and the AppView CLI no longer crashes.
- [#1080](https://github.com/criblio/appview/issues/1080) On an M1 Mac, trying to view a Go static executable in a Docker container emulating x86_64 Linux results in an address mapping conflict between the Go executable and the emulator. AppView now handles this limitation by allowing the Go executable to run un-viewed. With this fix, the Go executable no longer segfaults.

## AppView 1.1.2

2022-08-09 - Maintenance Release

Assets are available from the Cribl CDN at the links below.

- `AppView for x86`: [https://cdn.cribl.io/dl/appview/1.1.2/linux/x86_64/appview](https://cdn.cribl.io/dl/appview/1.1.2/linux/x86_64/appview)
- `AppView for ARM`: [https://cdn.cribl.io/dl/appview/1.1.2/linux/aarch64/appview](https://cdn.cribl.io/dl/appview/1.1.2/linux/aarch64/appview)
- `AWS Lambda Layer for x86`: [https://cdn.cribl.io/dl/appview/1.1.2/linux/x86_64/aws-lambda-layer.zip](https://cdn.cribl.io/dl/appview/1.1.2/linux/x86_64/aws-lambda-layer.zip)
- `AWS Lambda Layer for ARM`: [https://cdn.cribl.io/dl/appview/1.1.2/linux/aarch64/aws-lambda-layer.zip](https://cdn.cribl.io/dl/appview/1.1.2/linux/aarch64/aws-lambda-layer.zip)

To obtain the MD5 checksum for any file above, add `.md5` to the file path. 

Assets other than AWS Lambda Layers are available in the [Docker container](https://hub.docker.com/r/cribl/appview/tags) tagged `cribl/appview:1.1.2`.

### New Features and Improvements

- [#1025](https://github.com/criblio/appview/issues/1025) The process‑start message (the [start.msg](schema-reference#eventstartmsg) event) now includes a total of four identifiers. By itself, the new **UUID** process ID is unique for a given machine. In principle, **UUID** together with the new **Machine ID** constitutes a tuple ID that is unique across all machine namespaces. Here's a summary of the IDs available in AppView 1.1.2:
    - **UUID** (new in AppView 1.1.2), with key `uuid` and a value in [canonical UUID form](https://en.wikipedia.org/wiki/Universally_unique_identifier). UUID is a universally-unique process identifier, meaning that no two processes will ever have the same UUID value on the same machine.
    - **Machine ID** (new in AppView 1.1.2), with key `machine_id` and a value that AppView obtains from `/etc/machine-id`. The machine ID uniquely identifies the host, as described in the [man page](https://man7.org/linux/man-pages/man5/machine-id.5.html). When `/etc/machine-id` is not available (e.g., in Alpine, or in a container), AppView generates the machine ID using a repeatable MD5 hash of the host's first MAC address. Two containers on the same host can have the same machine ID.
    - **Process ID**, with key `pid` and a value that is always unique at any given time, but that the machine can reuse for different processes at different times.
    - **ID**, with key `id` and a value that concatenates (and may truncate) the viewed app's hostname, procname, and command. This value is not guaranteed to be unique.
- [#1037](https://github.com/criblio/appview/issues/1037) You can now run `appview service` on Linux distros that use the OpenRC init manager (e.g., Alpine), as well as on distros that use systemd (which was already supported). See the [CLI reference](cli-reference#service).

### Fixes

- [#1035](https://github.com/criblio/appview/issues/1035) The `appview service` command no longer ignores the `-c` (`--cribldest`), `-e` (`--eventdest`), and `-m` (`--metricdest`) options.

## AppView 1.1.1

2022-07-12 - Maintenance Release

Assets are available from the Cribl CDN at the links below.

- `AppView for x86`: [https://cdn.cribl.io/dl/appview/1.1.1/linux/x86_64/appview](https://cdn.cribl.io/dl/appview/1.1.1/linux/x86_64/appview)
- `AppView for ARM`: [https://cdn.cribl.io/dl/appview/1.1.1/linux/aarch64/appview](https://cdn.cribl.io/dl/appview/1.1.1/linux/aarch64/appview)
- `AWS Lambda Layer for x86`: [https://cdn.cribl.io/dl/appview/1.1.1/linux/x86_64/aws-lambda-layer.zip](https://cdn.cribl.io/dl/appview/1.1.1/linux/x86_64/aws-lambda-layer.zip)
- `AWS Lambda Layer for ARM`: [https://cdn.cribl.io/dl/appview/1.1.1/linux/aarch64/aws-lambda-layer.zip](https://cdn.cribl.io/dl/appview/1.1.1/linux/aarch64/aws-lambda-layer.zip)

To obtain the MD5 checksum for any file above, add `.md5` to the file path. 

Assets other than AWS Lambda Layers are available in the [Docker container](https://hub.docker.com/r/cribl/appview/tags) tagged `cribl/appview:1.1.1`.

### New Features and Improvements

- [#964](https://github.com/criblio/appview/issues/964) AppView downloadable assets now include [AWS Lambda Layers](https://docs.aws.amazon.com/lambda/latest/dg/gettingstarted-concepts.html#gettingstarted-concepts-layer) for x86 and for ARM, along with their respective MD5 checksums.

### Fixes

- [#1017](https://github.com/criblio/appview/issues/1017) AppView now correctly handles the "peek" flag in interposed functions that receive network data. Before this change, when a server "peeked" at the first byte of data, AppView counted that byte twice, which broke protocol detection. This fix enables AppView to correctly produce HTTP events when scoping a server that uses "peek" flags.

- [#1006](https://github.com/criblio/appview/issues/1006) AppView now correctly instruments the child processes of an sshd process started by a server. To do this, AppView interposes both the `execve` and `execv` system calls, and overrides some of the sandboxing that sshd normally imposes using `setrlimit`. Interposing `execv` is new, and gives AppView visibility into sshd child processes. Changing `setrlimit` settings enables AppView to perform actions required by AppView's configured backend and transport, such as establishing connections, creating threads, and creating files.

## AppView 1.1.0

2022-06-28 - Minor Release

Assets are available via Docker and the Cribl CDN at the links below.

- `Docker`: `cribl/appview:1.1.0`
- `x86`: [https://cdn.cribl.io/dl/appview/1.1.0/linux/x86_64/appview](https://cdn.cribl.io/dl/appview/1.1.0/linux/x86_64/appview)
- `ARM`: [https://cdn.cribl.io/dl/appview/1.1.0/linux/aarch64/appview](https://cdn.cribl.io/dl/appview/1.1.0/linux/aarch64/appview)

### New Features and Improvements

AppView 1.1.0 introduces improved capabilities for scoping Go applications:

- [#637](https://github.com/criblio/appview/issues/637) Support for Go versions 17 and 18.
- [#904](https://github.com/criblio/appview/issues/904) Support for HTTP2 when scoping Go executables. 
- [#667](https://github.com/criblio/appview/issues/667) AppView's own internal libc, which resolves problems we'd seen when scoping Go apps that handle signals.

Usability improvements include:

- [#917](https://github.com/criblio/appview/issues/917) Individual on/off control for all classes of metric data using metric watch types, via the config file or environment variables. Since event watch types already existed, this means that you can now turn all classes of events and metrics on or off individually.
- [#812](https://github.com/criblio/appview/issues/812) Metric and event [schemas](schema-reference) now include more informative definitions.
- [#969](https://github.com/criblio/appview/issues/969) The start message (`start.msg`) event which AppView sends when log level is `info` or `debug` is now [documented](schema-reference#eventstartmsg) in the schema.
- [#938](https://github.com/criblio/appview/issues/938) A new `APPVIEW_ERROR_SIGNAL_HANDLER` environment variable, provided for situations where a viewed app is crashing. Setting this variable to `true` sends backtrace information to the log, which can help you diagnose problems. 
- [#498](https://github.com/criblio/appview/issues/498) AppView can now accept remote commands over UNIX sockets or non-TLS TCP connections.
- [#970](https://github.com/criblio/appview/issues/970), [#988](https://github.com/criblio/appview/issues/988) Better controls for redacting binary data that would otherwise show up in AppView output. In the AppView library, `APPVIEW_ALLOW_BINARY_CONSOLE` can now be set using the environment variable (prior to 1.1.0, the only available method) or in the config file. For the library, its default value has been changed from `false` to `true`. Meanwhile, the CLI still defaults to `allowbinary=false`, because allowing binary data in CLI output makes sense only in rare cases.
 
### Fixes

- [#781](https://github.com/criblio/appview/issues/781) Delivery of data as processes exit is now more consistent.
- [#918](https://github.com/criblio/appview/issues/918) When AppView reports its configuration (upon start of a new viewed process), it now includes the formerly missing `cribl` section describing [Cribl backend](cribl-integration#integrating-with-cribl-edge) configuration.
- [#994](https://github.com/criblio/appview/issues/994) When you try to view a Go application on ARM, which is [not supported](requirements#known-limitations), AppView now leaves the Go app un-instrumented, allowing the app to run normally. Before this bug was discovered and fixed, a viewed Go app on ARM would not run at all.

## AppView 1.0.4

2022-05-10 - Maintenance Release

Assets are available via Docker and the Cribl CDN at the links below.

- `Docker`: `cribl/appview:1.0.4`
- `x86`: [https://cdn.cribl.io/dl/appview/1.0.4/linux/x86_64/appview](https://cdn.cribl.io/dl/appview/1.0.4/linux/x86_64/appview)
- `ARM`: [https://cdn.cribl.io/dl/appview/1.0.4/linux/aarch64/appview](https://cdn.cribl.io/dl/appview/1.0.4/linux/aarch64/appview)

### Fixes

AppView 1.0.4 aims to resolve reported connectivity issues.

- [#896](https://github.com/criblio/appview/issues/896) AppView now creates a connection dedicated to transmitting payload data only if payloads are enabled.
- [#665](https://github.com/criblio/appview/issues/665) AppView now has safeguards to ensure that AppView does not interfere with viewed processes exiting.
- [#869](https://github.com/criblio/appview/issues/869) AppView now uses an improved algorithm for omitting raw binary data from console events. The improved algorithm is better able to handle commands that switch back and forth between outputting text and outputting raw binary data to the console.

## AppView 1.0.3

2022-04-12 - Maintenance Release

Assets are available via Docker and the Cribl CDN at the links below.

- `Docker`: `cribl/appview:1.0.3`
- `x86`: [https://cdn.cribl.io/dl/appview/1.0.3/linux/x86_64/appview](https://cdn.cribl.io/dl/appview/1.0.3/linux/x86_64/appview)
- `ARM`: [https://cdn.cribl.io/dl/appview/1.0.3/linux/aarch64/appview](https://cdn.cribl.io/dl/appview/1.0.3/linux/aarch64/appview)

### Fixes

AppView 1.0.3 improves the quality of results when scoping Go executables:

- [#738](https://github.com/criblio/appview/issues/738) When scoping Go executables, AppView is now able to produce events from two more Go functions: `unlinkat` and `getdents`. AppView already had the ability to interpose the equivalent system calls in C (`unlink`, `unlinkat`, `readdir`).
- [#864](https://github.com/criblio/appview/issues/864) When scoping Go executables that call the `openat` function, AppView no longer fails to collect `fs.open` events. This brings AppView up to date with changes to `openat` in recent versions of Go.
- [#862](https://github.com/criblio/appview/issues/862) When scoping dynamic Go executables, AppView no longer falsely logs nonexistent errors.

## AppView 1.0.2

2022-03-15 - Maintenance Release

Assets are available via Docker and the Cribl CDN at the links below.

- `Docker`: `cribl/appview:1.0.2`
- `x86`: [https://cdn.cribl.io/dl/appview/1.0.2/linux/x86_64/appview](https://cdn.cribl.io/dl/appview/1.0.2/linux/x86_64/appview)
- `ARM`: [https://cdn.cribl.io/dl/appview/1.0.2/linux/aarch64/appview](https://cdn.cribl.io/dl/appview/1.0.2/linux/aarch64/appview)

### New Features and Improvements

AppView 1.0.2 introduces fine-grained control of `appview events` output:

- [#826](https://github.com/criblio/appview/issues/826) When you do filtering or matching with `appview events`, AppView now shows **all** results by default. (The filtering/matching options are `--sort`, `--match`, `--source`, and `--sourcetype`.) Alternatively, you can limit the number of events with the `--last` option, e.g., `appview events --last 20`.
  - When you are **not** filtering or matching, `appview events` shows the last 20 results by default. Use `--last` to specify a different limit, or `--all` to see all results.
- [#20](https://github.com/criblio/appview/issues/20) You can now specify one or more field names, with `appview events --fields`, and the output will show only those field names and their values alongside each event. (If you want to restrict output to only those events which **contain** the selected fields, add the `--match` option.)
- [#813](https://github.com/criblio/appview/issues/813) You can use the `--sort` option to sort by a top-level field (i.e., a top-level element of the event body). By default, this returns events sorted in descending order, both for numeric values (such as timestamps) and string values (such as the values for the `proc` field). To sort in ascending order, add the `--reverse` flag.

Another usability improvement applies to the CLI as a whole:

- [#611](https://github.com/criblio/appview/issues/611) Given combinations of arguments that don't make sense, `appview` now responds with an error.

### Fixes

- [#825](https://github.com/criblio/appview/issues/825) Output of `appview events` is now rendered on one line.
- [#728](https://github.com/criblio/appview/issues/728) Added more check (`_chk`) functions to `libappview.so`, without which a viewed command could fail when that command had been compiled with a particular GCC toolchain.
- [#787](https://github.com/criblio/appview/issues/787) Through improved HTTP pipelining support, AppView now avoids some cases where the `http_method` field in HTTP events contained junk. This work continues in [#829](https://github.com/criblio/appview/issues/829).
- [#800](https://github.com/criblio/appview/issues/800) AppView now checks for conditions which caused Java applications to crash when viewed, and avoids the crashes by applying fallback behaviors where necessary.

## AppView 1.0.1

2022-02-28 - Update to GA Release 

Assets are available via Docker and the Cribl CDN at the links below.

- `Docker`: `cribl/appview:1.0.1`
- `x86`: [https://cdn.cribl.io/dl/appview/1.0.1/linux/x86_64/appview](https://cdn.cribl.io/dl/appview/1.0.1/linux/x86_64/appview)
- `ARM`: [https://cdn.cribl.io/dl/appview/1.0.1/linux/aarch64/appview](https://cdn.cribl.io/dl/appview/1.0.1/linux/aarch64/appview)

### Fixes

AppView 1.0.1 updates event and metric names. The AppView team strongly recommends using AppView 1.0.1 and not 1.0.0.

#### Schema

- [#815](https://github.com/criblio/appview/issues/815) Names of events and metrics are now complete, correct, and consistent with the original design intent.

## AppView 1.0.0

2022-02-15 - GA Release <!-- Official v1 Release -->

This release has been replaced by AppView 1.0.1.

### New Features and Improvements

AppView's official v1 release includes advances in metric capture and forwarding, stabilization, formal schema definition and associated standardization, and enhanced filesystem events.

#### Instrumentation

- [#426](https://github.com/criblio/appview/issues/426) You can now appview (attach to) a running process on ARM64.

#### Logging

- [#678](https://github.com/criblio/appview/issues/678) Time zone is present in all logs.
- [#636](https://github.com/criblio/appview/issues/636) The log level of the `missing uv_fileno` error is reduced to debug level.

#### Events and Metrics

- [#644](https://github.com/criblio/appview/issues/644) Application-specific metrics can be captured.
- [#725](https://github.com/criblio/appview/issues/725) Metrics and events are now [defined](https://github.com/criblio/appview/tree/master/docs/schemas) in JSON Schema.
- [#723](https://github.com/criblio/appview/issues/723) Metrics and events are now standardized. (Summary [here](https://github.com/criblio/appview/issues/712#issuecomment-1030234850).)
- [#699](https://github.com/criblio/appview/issues/699) Added an option to enable events via configuration file entries.
- [#402](https://github.com/criblio/appview/issues/402) Added support for readdir to `fs` events.
- [#709](https://github.com/criblio/appview/issues/709) Added support for unlinks / file removals to `fs` events.
- [#162](https://github.com/criblio/appview/issues/162) Added `net.open` and `net.close` metrics.

#### Data Routing

- [#697](https://github.com/criblio/appview/issues/697) Transport defaults and priorities have changed. (Summary [here](https://github.com/criblio/appview/issues/712#issuecomment-1030234850).) 
- [#661](https://github.com/criblio/appview/issues/661) The process uid and gid are now present in the `cribl` connection header.
- [#670](https://github.com/criblio/appview/issues/670), [#700](https://github.com/criblio/appview/issues/700), [#707](https://github.com/criblio/appview/issues/707)  Added an `edge` transport type, with its own default search path and CLI support.
- [#571](https://github.com/criblio/appview/issues/571) Added support for a `unix://` destination in the CLI.

### Fixes

- [#640](https://github.com/criblio/appview/issues/640) An error message is now generated when AppView can't attach to a static executable.
- [#677](https://github.com/criblio/appview/issues/677), [#687](https://github.com/criblio/appview/issues/687) Fixed HTTP header extraction.
- [#737](https://github.com/criblio/appview/issues/737) Fixed a seg fault on a busy Jenkins.
- [#232](https://github.com/criblio/appview/issues/232) Fixed HTTP metric aggregation.
- [#657](https://github.com/criblio/appview/issues/657) `apt-get` no longer hangs.

## AppView 0.8.1

2021-12-21 - Maintenance Pre-Release

- **Improvement**: [#533](https://github.com/criblio/appview/issues/533) AppView logs now include dropped message and connection failure indicators, which make it easier to diagnose TCP/TLS connection issues, or to see when no data is flowing from a viewed process.

- **Improvement**: [#534](https://github.com/criblio/appview/issues/534) AppView log entries now include timestamps, e.g., `[2021-10-20 17:48:36.974]`. This timestamp format is ambiguous, and a fix is planned for AppView 1.0.0. See [#678](https://github.com/criblio/appview/issues/678). 

- **Improvement**: [#229](https://github.com/criblio/appview/issues/229) When a process starts, the User Identifier (UID) and Group Identifier (GID) now appear in AppView's `proc.start` metric and process start event (also known as TCP JSON connection header).

- **Fix**: [#650](https://github.com/criblio/appview/issues/650) When scoping a process with TLS, and that process does not last long enough for AppView to complete the TLS connection, AppView no longer causes the viewed process to crash.

## AppView 0.8.0

2021-10-26 - Maintenance Pre-Release

- **Improvement**: [#543](https://github.com/criblio/appview/issues/543) HTTP events are now all processed by the `libappview.so` library, not LogStream as was the case [previously](https://github.com/criblio/appview/issues/311).

- **Improvement**: [#572](https://github.com/criblio/appview/issues/572) The CLI now supports invoking a configuration file, using the syntax `appview run -u appview.yml -- foo` where `appview.yml` is the configuration file and `foo` is the command being viewed.

- **Improvement**: [#241](https://github.com/criblio/appview/issues/241),[#271](https://github.com/criblio/appview/issues/271),[#379](https://github.com/criblio/appview/issues/379) The ARM64 limitations described in [#241](https://github.com/criblio/appview/issues/241) are resolved and no longer apply, except that Go executables remain unsupported on ARM64.


- **Fix**: [#598](https://github.com/criblio/appview/issues/598) Attempting to appview an executable that is Go version 1.17 or newer no longer causes AppView to crash. AppView does not support Go 1.17 yet, but now recognizes these executables and displays an informative message. 

- **Fix**: [#481](https://github.com/criblio/appview/issues/481),[#575](https://github.com/criblio/appview/issues/575) With musl libc-based distros, previous limitations on filesystem write events are now resolved, and no longer apply.

- **Fix**: [#397](https://github.com/criblio/appview/issues/397),[#403](https://github.com/criblio/appview/issues/403),[#567](https://github.com/criblio/appview/issues/567),[#586](https://github.com/criblio/appview/issues/586) With musl libc-based distros and Java, previous limitations on `appview attach` are now resolved and no longer apply.


## AppView 0.7.5

2021-10-05 - Maintenance Pre-Release

- **Improvement**: [#462](https://github.com/criblio/appview/issues/462), [#540](https://github.com/criblio/appview/issues/540) Add support for sending event/metric output to a Unix domain socket.

- **Improvement**: [#492](https://github.com/criblio/appview/issues/492) Add support for custom configuration with new `custom` section in `appview.yml`.  

- **Improvement**: [#489](https://github.com/criblio/appview/issues/489) Streamline configuration file structure by eliminating `appview_protocol.yml` and merging its content into `appview.yml`.

- **Improvement**: [#503](https://github.com/criblio/appview/issues/503) Promote the `tags` section of `appview.yml` to top level, removing it from the `metric` > `format` section. This change is **not** backwards-compatible. 

## AppView 0.7.4

2021-09-14 - Maintenance Pre-Release

- **Fix**: [#264](https://github.com/criblio/appview/issues/264),[#525](https://github.com/criblio/appview/issues/525) `appview k8s` now (1) supports specifying the namespace to install into, and (2) when the [ConfigMap](https://kubernetes.io/docs/concepts/configuration/configmap/) in that namespace is edited, automatically propagates the changes to all namespaces instrumented with the label `appview=enabled`.

## AppView 0.7.3

2021-09-01 - Maintenance/Hotfix Pre-Release

- **Fix**: [#516](https://github.com/criblio/appview/issues/516) Where appview'd processes use TLS, child processes no longer get stuck in fork calls.

- **Fix**: [#442](https://github.com/criblio/appview/issues/442)
 Remove TLS exit handling code that is no longer needed.

- **Fix**: [#518](https://github.com/criblio/appview/issues/518)
 Fix a build caching issue.

## AppView 0.7.2

2021-08-17 - Maintenance Pre-Release

- **Improvement**: [#241](https://github.com/criblio/appview/issues/241) Add experimental support for ARM64 architecture, with [limitations](https://github.com/criblio/appview/issues/241#issuecomment-824428842).

- **Improvement**: [#401](https://github.com/criblio/appview/issues/401) When `appview` attaches to a bash process, it now emits events for any child processes.

- **Improvement**: [#395](https://github.com/criblio/appview/issues/395) Add a new environment variable, `APPVIEW_CONNECT_TIMEOUT_SECS`, to configure wait behavior. This prevents events and metrics from being dropped when a viewed command exits quickly while using a connection with network latency.

- **Improvement**: [#311](https://github.com/criblio/appview/issues/311) AppView can now stream HTTP payloads to LogStream, whether the payloads originate as HTTP/1.1 or HTTP/2 traffic. LogStream then converts the payloads to HTTP events.

- **Fix**: [#407](https://github.com/criblio/appview/issues/407) To prevent errors, `appview prune` now rejects negative numbers as arguments. Positive arguments implicitly mean "how many sessions back" to delete or keep.

- **Fix**: [#416](https://github.com/criblio/appview/issues/416) Ensure that file descriptors opened by libappview are not closed by SSH, to avoid scenarios where processes operating over network connections can get stuck at 100% CPU utilization.

- **Fix**: [#417](https://github.com/criblio/appview/issues/417) Ensure that AppView performs exit handling only after SSL is finished. This avoids double free errors where the OpenSSL exit handler, being called while TLS is not enabled, frees memory that was not allocated.

- **Fix**: [#327](https://github.com/criblio/appview/issues/327) Protocol-detect events now use the `net` event type, which prevents them from being dropped by periodic threads.

This pre-release addresses the following issues:

## AppView 0.7.1

2021-07-20 - Maintenance Pre-Release

This pre-release addresses the following issues:

- **Improvement**: [#363](https://github.com/criblio/appview/pull/363) Add a new `appview ps` command to report on all processes into which the libappview library is loaded.
- **Improvement**: [#363](https://github.com/criblio/appview/pull/363) Add support for attaching to a process by name (preserving existing attach by PID functionality), for example: `appview attach NAME`.
- **Improvement**: [#304](https://github.com/criblio/appview/issues/304) Add support for configuring an `authToken` to pass to LogStream as a header, using `appview run -a ${authToken}` in the CLI, or the `cribl` section of `appview.yml`.
- **Improvement**: [#368](https://github.com/criblio/appview/issues/368) Add the new `-n` or `--nobreaker` option for configuring LogStream breaker behavior from AppView. This option prevents LogStream from running Event Breakers on an AppView Source. The `--nobreaker` option can only be used with LogStream starting with version 3.0.4.
- **Improvement**: [#359](https://github.com/criblio/appview/pull/359) Users who run appview as root now see the message `WARNING: Session history will be stored in $HOME/.appview/history and owned by root`.
- **Improvement**: [#278](https://github.com/criblio/appview/pull/278) Integration tests are now enabled for PRs to the `master` and `release/*` branches.
- **Improvement**: [#381](https://github.com/criblio/appview/pull/381) Commits to master now redeploy the website only when there are changes in the `website/` content.
- **Improvement**: [#350](https://github.com/criblio/appview/issues/350) In `docker-build`, add support for:
  - Overriding the `build` command, for example: `make docker-build CMD="make coreall"`.
  - Passing `-u $(id -u):$(id -g)` to Docker so that the current user owns build results.
  - Using `.dockerignore` to omit unnecessary and potentially large items like `**/core`, `website/`, `test/testContainers`.

- **Fix**: [#364](https://github.com/criblio/appview/issues/364) Improved AppView's delivery of pending events to a TLS server, upon process exit.
- **Fix**: [#394](https://github.com/criblio/appview/pull/394) Improved AppView's regex for determining whether a given filename represents a log file.
- **Fix**: [#309](https://github.com/criblio/appview/issues/309) The `appview flows` command now works when `stdin` is a pipe.
- **Fix**: [#388](https://github.com/criblio/appview/issues/388) Ensure that all dimension names contain underscores (`_`) rather than periods (`.`). The `http.status` and `http.target` dimensions have been corrected. This change is not backwards-compatible.
- **Fix**: [#393](https://github.com/criblio/appview/pull/393) The metrics `http.client.duration` and `http.server.duration` are now the correct type, `timer`. This change is not backwards-compatible.
- **Fix**: [#358](https://github.com/criblio/appview/pull/358) Cleaned up help for `-d` and `-k` options to `appview prune`.

# Changelog

## AppView 0.7

2021-07-02 - Maintenance Pre-Release

This pre-release addresses the following issues:

- **Improvement**: [#267](https://github.com/criblio/appview/issues/267),[#256](https://github.com/criblio/appview/issues/256) Add support for attaching AppView to a running process.
- **Improvement**: [#292](https://github.com/criblio/appview/issues/292) Add support for attaching AppView to a running process from the CLI.
- **Improvement**: [#143](https://github.com/criblio/appview/issues/143) Add support for the Alpine Linux distribution, including support for musl libc (the compact glibc alternative on which Alpine and some other Linux distributions are based).
- **Improvement**: [#164](https://github.com/criblio/appview/issues/164), [#286](https://github.com/criblio/appview/issues/286) Add support for TLS over TCP connections, with new TLS-related environment variables shown by the command `ldappview --help configuration | grep TLS`.
- **Deprecation**: [#286](https://github.com/criblio/appview/issues/286) Drop support for CentOS 6 (because TLS support requires glibc 2.17 or newer).
- **Improvement**: [#132](https://github.com/criblio/appview/issues/132) Add new `appview logs` command to view logs from the CLI.
- **Improvement**: Add new `appview watch` command to run AppView at intervals of seconds, minutes, or hours, from the CLI.
- **Improvement**: Make developer docs available at [https://github.com/criblio/appview/tree/master/docs](https://github.com/criblio/appview/tree/master/docs).
- **Fix**: [#265](https://github.com/criblio/appview/issues/265) Re-direct bash malloc functions to glibc, to avoid segfaults and memory allocation issues.
- **Fix**: [#279](https://github.com/criblio/appview/issues/279) Correct handling of pointers returned by gnutls, which otherwise could cause `git clone` to crash.

## AppView 0.6.1

2021-04-26 - Maintenance Pre-Release

This pre-release addresses the following issues:

- **Improvement**: [#238](https://github.com/criblio/appview/issues/238) Resolve multiple issues with payloads from Node.js processes.
- **Improvement**: [#257](https://github.com/criblio/appview/issues/257) Add missing console/log events, first observed in Python 3.
- **Improvement**: [#249](https://github.com/criblio/appview/issues/249) Add support for Go apps built with `‑buildmode=pie`.
 
## AppView 0.6

2021-04-01 - Maintenance Pre-Release

This pre-release addresses the following issues:

- **Improvement**: [#99](https://github.com/criblio/appview/issues/99) Enable augmenting HTTP events with HTTP header and payload information.
- **Improvement**: [#38](https://github.com/criblio/appview/issues/38) Disable (by default) TLS handshake info in payload capture.
- **Improvement**: [#36](https://github.com/criblio/appview/issues/36) Enable sending running appview instances a signal to reload fresh configuration.
- **Improvement**: [#126](https://github.com/criblio/appview/issues/126) Add Resolved IP Addresses list to a DNS response event.
- **Improvement**: [#100](https://github.com/criblio/appview/issues/100) 
Add `appview run` flags to facilitate sending appview data to third-party systems; also add a `appview k8s` command to facilitate installing a mutating admission webhook in a Kubernetes environment.
- **Improvement**: [#149](https://github.com/criblio/appview/issues/149) 
Apply tags set via environment variables to events, to match their application to metrics.
- **Fix**: [#37](https://github.com/criblio/appview/issues/37) Capture DNS requests' payloads.
- **Fix**: [#2](https://github.com/criblio/appview/issues/2) Enable scoping of appview.
- **Fix**: [#35](https://github.com/criblio/appview/issues/35) Improve error messaging and logging when symbols are stripped from Go executables.

## AppView 0.5.1

2021-02-05 - Maintenance Pre-Release

This pre-release addresses the following issues:

- **Fix**: [#96](https://github.com/criblio/appview/issues/96) Corrects JSON corruption observed in non-US locales.
- **Fix**: [#101](https://github.com/criblio/appview/issues/101) Removes 10-second processing delay observed in short-lived processes.
- **Fix**: [#25](https://github.com/criblio/appview/issues/25) Manages contention of multiple processes writing to same data files.

## AppView 0.5

2021-02-05 - Initial Pre-Release

AppView is now a thing. Its public repo is at https://github.com/criblio/appview.
