---
title: Using TLS for Secure Connections
---

## Using TLS for Secure Connections

AppView supports TLS over TCP connections. Here's how that works:

- AppView can use TLS when connecting to Cribl Stream or another application (including its events and metrics destinations). 
- Once AppView establishes the connection, data can flow over that connection **in both directions**. 
- This means that when you tell AppView to connect using TLS, the connection is secured by TLS in both directions.

For security's sake, AppView never opens ports, nor does it listen for or allow incoming connections.

To enable TLS: In the `appview.yml` [config file](/docs/config-file), set the `transport : tls : enable` element to `true`.

## Using TLS in Cribl.Cloud

AppView uses TLS by default to communicate with Cribl Stream over TCP on Cribl.Cloud. Cribl Stream has an AppView Source ready to use out-of-the-box.

Within Cribl.Cloud, a front-end load balancer (reverse proxy) handles the encrypted TLS traffic and relays it to the AppView Source port in Cribl Stream. The connection from the load balancer to Cribl Stream does **not** use TLS, and you should not enable TLS on the [AppView Source](https://docs.cribl.io/docs/sources-appview) in Cribl Stream. No changes in Cribl Stream configuration are needed.

AppView connects to your Cribl.Cloud Ingest Endpoint on port 10090. The Ingest Endpoint URL is always the same except for the Cribl.Cloud Organization ID, which Cribl Stream uses in the hostname portion, in the following way:

```
https://in.main-default-<organization>.cribl.cloud:10090
```

If you **disable** TLS, the port is 10091.

### CLI usage

Use `appview run` with the `-c` option (in this example, we're scoping `ps -ef`):

```
appview run -c tls://host:10090 -- ps -ef
```

### Configuration for `LD_PRELOAD`

To connect AppView to a Cribl.Cloud-managed instance of Cribl Stream using TLS: 

1. Enable the `transport : tls : enable` element in `appview.yml`.
1. Connect to port 10090 on your Cribl.Cloud Ingest Endpoint.

To enable TLS in `appview.yml`, adapt the example below to your environment:

```
cribl:
  enable: true
  transport:
    type: tcp  # don't use tls here, use tcp and enable tls below
    host: in.main-default-<organization>.cribl.cloud
    port: 10090 # cribl.cloud's port for the TLS AppView Source
    tls:
      enable: true
      validateserver: true
      cacertpath: ''
```

## Scoping Without TLS

If you prefer to connect to Cribl Stream on Cribl.Cloud without encryption, connect to port 10091 instead of port 10090, and disable the `tls` element in `appview.yml`.

No changes in Cribl Stream configuration are needed.

## Backoff Algorithm

AppView uses a **backoff algorithm** for connections to avoid creating excessive network traffic and log entries. When a remote destination that AppView tries to connect to rejects the connection or is not available, AppView retries the connection at a progressively slower rate.
