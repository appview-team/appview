---
title: Overview
---

## What Is AppView?

AppView is a tool that unlocks 100% application observability with near-zero overhead, including for applications and processes whose data is otherwise hard to obtain.

AppView is an open source, runtime-agnostic instrumentation utility for any Linux command or application. It helps users explore, understand, and gain visibility into any process running in any Linux host or container with **no code modification**. 

AppView provides the fine-grained observability of a proxy/service mesh, without the latency of a sidecar. It emits APM-like metric and event data, in open formats, to existing log and metric tools.

It’s like [strace](https://strace.io/) meets [tcpdump](https://www.tcpdump.org/) – but with consumable output for events like file access, DNS, and network activity, and StatsD-style metrics for applications. AppView can also look inside encrypted payloads, offering [WAF](https://en.wikipedia.org/wiki/Web_application_firewall)-like visibility without proxying traffic. 
</br>
</br>

![AppView in-terminal monitoring](./images/AppView-GUI-screenshot.png)

## Instrument, Collect, and Observe with AppView

AppView is runtime-agnostic, has no dependencies, and requires no code development. You can use AppView to:

- Instrument both static and dynamic executables.
- Attach to processes *while they are running* or start when the process does.
- Run on Alpine Linux, another Linux distribution based on musl libc, or on a glibc-based distro.
- Communicate safely and securely with TLS over TCP connections.
- Capture application metrics: **File, Network, Memory, CPU**.
- Capture application events: console content, stdin/out, logs, errors.
- Capture any and all payloads: DNS, HTTP, HTTPS.
- Summarize metrics and detect protocols.
- Normalize and forward metrics and events, in real time, to remote systems.
