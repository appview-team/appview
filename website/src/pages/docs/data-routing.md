---
title: Data Routing
---

## Data Routing

AppView gives you multiple ways to route data. The basic operations are:

- Routing [both events and metrics](#routing-to-edge) to [Cribl Edge](https://docs.cribl.io/edge/).
- Routing [both events and metrics](#routing-to-cloud) to [Cribl Stream](https://docs.cribl.io/stream).
- Routing [events](#routing-events) to a file, local unix socket, network destination, or Cribl Edge.
- Routing [metrics](#routing-metrics) to a file, local unix socket, network destination, or Cribl Edge.

For each of these operations, the CLI has command-line options, the config file has settings, and the AppView library has environment variables.

If you plan to use the config file, do take time to [read it all the way through](/docs/config-file) - then this page will make more sense!

If you want to run Cribl Edge and AppView in a container, and/or appview apps that are running in containers, see [these](#container-with-edge) instructions.

### Routing to Cribl Edge {#routing-to-edge}

In a single operation, you can route both events and metrics to Cribl Edge. 

#### Routing to Edge with the CLI

Use the `-c` or `--cribldest` option. You need this option because, by default, the CLI routes data to the local filesystem. For example:

```
appview run -c edge -- curl https://wttr.in/94105
```

This usage works with `appview run`, `appview attach`, `appview watch`, `appview k8s`, and `appview service`. It does **not** work with `appview metrics`, where the `-c` option stands for `--cols`, telling the CLI to output data in columns instead of rows.

#### Routing to Edge with the AppView Library 

Use the `APPVIEW_CRIBL` environment variable to define a unix socket connection to Cribl Edge. For example:

```
LD_PRELOAD=./libappview.so APPVIEW_CRIBL=edge ls -al
```

#### Routing to Edge with the Config File

Complete these steps, paying particular attention to the sub-elements of `cribl > transport`, which is where you specify routing:

* Verify that `cribl > enable` is set to `true`, which enables the `cribl` backend. (This is the default setting.)
* To route data to Cribl Edge, set `cribl > transport > type` to `edge`. (This is the default value.)

### Routing to Cribl Stream in the Cloud {#routing-to-cloud}

In a single operation, you can route both events and metrics to Cribl Stream in the Cloud, or Cribl.Cloud for short.

#### Routing to Cribl.Cloud with the CLI

Use the `-c` or `--cribldest` option. You need this option because, by default, the CLI routes data to the local filesystem. For example:

```
appview run -c tls://127.0.0.1:10090 -- curl https://wttr.in/94105
```

This usage works with `appview run`, `appview attach`, `appview watch`, `appview k8s`, and `appview service`. It does **not** work with `appview metrics`, where the `-c` option stands for `--cols`, telling the CLI to output data in columns instead of rows.

#### Routing to Cribl.Cloud with the AppView Library 

Use the `APPVIEW_CRIBL_CLOUD` environment variable to define a TLS-encrypted connection to Cribl.Cloud. Specify a transport type, a host name or IPv4 address, and a port number, for example:

```
LD_PRELOAD=./libappview.so APPVIEW_CRIBL_CLOUD=tcp://in.main-default-<organization>.cribl.cloud:10090 ls -al
```

As a convenience, when you set `APPVIEW_CRIBL_CLOUD`, AppView automatically overrides the defaults of three other environment variables, setting them to the values that Cribl.Cloud via TLS requires. That produces these (and a few [other](/docs/cribl-integration#parameter-overrides)) settings:

* `APPVIEW_CRIBL_TLS_ENABLE` is set to `true`.
* `APPVIEW_CRIBL_TLS_VALIDATE_SERVER` is set to `true`.
* `APPVIEW_CRIBL_TLS_CA_CERT_PATH` is set to the empty string.

If you prefer an **unencrypted** connection to Cribl.Cloud, use `APPVIEW_CRIBL`, as described [here](/docs/cribl-integration#cloud-unencrypted).

#### Routing to Cribl.Cloud with the Config File

Complete these steps, paying particular attention to the sub-elements of `cribl > transport`, which is where you specify routing:

* Verify that `cribl > enable` is set to `true`, which enables the `cribl` backend. (This is the default setting.)
* To route data to Cribl.Cloud, set `cribl > transport > type` to `tcp`. (The default value is `edge`.)
* Specify desired values for the rest of the `cribl > transport` sub-elements, namely `host`, `port`, and `tls`.

### Routing Events {#routing-events}

You can route events independently of metrics.

#### Routing Events with the CLI

Use the `-e` or `--eventdest` option. For example:

```
appview run -e tcp://localhost:9109 -- curl https://wttr.in/94105
```

The above example sends events in [ndjson](http://ndjson.org/), the CLI's default output format. To send events in StatsD, you would include `--metricformat statsd` in the command.

#### Routing Events with the AppView Library 

Use the `APPVIEW_EVENT_DEST` environment variable, and set the `APPVIEW_CRIBL_ENABLE` to `false`.

For example:

```
LD_PRELOAD=./libappview.so APPVIEW_CRIBL_ENABLE=false APPVIEW_EVENT_DEST=tcp://localhost:9109 curl https://wttr.in/94105
```

The above example sends events in StatsD, the AppView library's default output format.

#### Routing Events with the Config File

Complete these steps, paying particular attention to the sub-elements of `event > transport`, which is where you specify routing:

* Set `cribl > enable` to `false` to disable the `cribl` backend.
* Set `event > enable` to `true` to enable the events backend.
* Specify desired values for the rest of the `event` elements, namely `format`, `watch`, and `transport`.

### Routing Metrics {#routing-metrics}

You can route metrics independently of events.

#### Routing Metrics with the CLI

Use the `-m` or `--metricdest` option. For example:

```
appview run -m udp://localhost:8125 -- curl https://wttr.in/94105
```

The above example sends events in [ndjson](http://ndjson.org/), the CLI's default output format. To send events in StatsD, you would include `--metricformat statsd` in the command.

#### Routing Events with the AppView Library 

Use the `APPVIEW_METRIC_DEST` environment variable, and set the `APPVIEW_CRIBL_ENABLE` to `false`.

For example:

```
LD_PRELOAD=./libappview.so APPVIEW_CRIBL_ENABLE=false APPVIEW_METRIC_DEST=udp://localhost:8125 curl https://wttr.in/94105
```

The above example sends events in StatsD, the AppView library's default output format.

This sends metrics in StatsD format. Adding `APPVIEW_METRIC_FORMAT=ndjson` would change the format to ndjson.

#### Routing Metrics with the Config File

Complete these steps, paying particular attention to the sub-elements of `metric > transport`, which is where you specify routing:

* Set `cribl > enable` to `false` to disable the `cribl` backend.
* Set `metric > enable` to `true` to enable the metrics backend.
* Specify desired values for the rest of the `metric` elements, namely `format`, `transport`, and optionally, `watch`.

### Running AppView and Cribl Edge in a Container {#container-with-edge}

This section describes one of many possible scenarios involving AppView, Cribl Edge, and containers. If you are interested in doing something different let us know via the `#appview` channel of Cribl's [Community Slack](https://cribl-community.slack.com/).

You can start Cribl Edge and AppView together in a container, then use Cribl Edge's [AppView Source](https://docs.cribl.io/edge/sources-appview/) to "drive" AppView. You'll decide what apps to appview, and work with the resulting events and metrics in Cribl Edge.

To do this, you can use the `docker run` command, choosing options based on considerations including whether to mount the host filesystem in read-only or read-write mode. By default, the `-v` or `--volume` mounts in read-write mode, for example `-v /:/hostfs`. For read-only mode, add `:ro`, for example `-v /:/hostfs:ro`.

In the examples below, we use `/hostfs` to specify the root filesystem mount point; alternatively, you could use a path defined by the environment variable `CRIBL_EDGE_FS_ROOT`.

The examples progress from most to least "locked down."

#### Example 1: Mount the Host Filesystem Read-only

The command below mounts the overall host filesystem read-only. It then mounts the `appview start` command's three mount points in read-write mode, which is required for `appview start` to work, even when the overall filesystem is read-only.

```
docker run -d -e CRIBL_EDGE=1 -p 9420:9420 -v /var/run/appview:/var/run/appview -v /var/run/docker.sock:/var/run/docker.sock -v /:/hostfs:ro -v /etc/cron.d/:/hostfs/etc/cron.d/ -v /tmp/:/hostfs/tmp/ -v /usr/lib/:/hostfs/usr/lib/ --restart unless-stopped --name cribl-edge cribl/cribl:4.0.4
```

#### Example 2: Mount the Host Filesystem Read-Only With the `--privileged` flag

The command below mounts the overall host filesystem read-only, but by adding the `--privileged` flag, [gives the container access](https://docs.docker.com/engine/reference/run/#runtime-privilege-and-linux-capabilities) to processes running outside containers on the host. With this usage, there's no need to specify the `appview start` mount points.

```
docker run -d -e CRIBL_EDGE=1 -p 9420:9420 -v /var/run/appview:/var/run/appview -v /var/run/docker.sock:/var/run/docker.sock -v /:/hostfs:ro  --privileged --restart unless-stopped --name cribl-edge cribl/cribl:4.0.4
```

The `--privileged` flag should be used with care, because it bestows Linux capabilities – including ptrace, the ability to read the `/proc` filesystem, and more – on whatever apps run in the container.

#### Example 3: Mount the Host Filesystem Read-Write

The command below mounts the overall host filesystem read-write. With this usage, there's no need to specify the `appview start` mount points or to add the `--privileged` flag.

```
docker run -d -e CRIBL_EDGE=1 -p 9420:9420 -v /var/run/appview:/var/run/appview -v /var/run/docker.sock:/var/run/docker.sock -v /:/hostfs  --restart unless-stopped --name cribl-edge cribl/cribl:4.0.4
```
