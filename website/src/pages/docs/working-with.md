---
title: Working With AppView
---

## Working With AppView 

These docs explain two ways to work with AppView: together with [Cribl Edge](https://docs.cribl.io/edge/), and on its own. Cribl Edge provides a means to manage AppView at scale. The concluding Reference topics generally apply for both approaches, although the [CLI Reference](/docs/cli-reference) is most relevant for using AppView on its own or with other open-source tools.

### Using AppView with Cribl Edge

Here we'll cover working on a Linux host or virtual machine, in a Docker or similar container, or on Kubernetes.

These topics are complementary to Cribl's documentation about the [AppView Source](https://docs.cribl.io/stream/sources-appview) and the [AppView Config Editor](https://docs.cribl.io/stream/4.0/appview-configs).

AppView comes bundled with Cribl Edge, so there's no need to discuss how to obtain AppView. 

### Using AppView On its Own

Here we'll cover obtaining, and then using AppView, starting with the CLI.

## Fundamental Concepts {#fundamentals}

To work effectively with AppView, start with these fundamental points:

* Your overall approach can be either spontaneous or more planned-out.
* You can [control](#config-file-etc) AppView by means of the config file, environment variables, or a combination of the two.
* The results you get from AppView will be in the form of [events and metrics](/docs/events-and-metrics). When you appview HTTP applications, you can get payloads, too.

## Scoping By PID and Scoping By Rule {#pid-vs-rule}

Another important distinction to understand when working with AppView is that between "appview by PID" and "appview by Rule."

* AppView by PID instruments one process on one host or container.
* AppView by Rule instruments one or more processes, not only on one host, but, when using AppView together with Cribl Edge, an entire Edge Fleet.
    * The principle is that AppView will instrument whatever processes match a given Rule.

## The Config File, Env Vars, and Flags {#config-file-etc}

AppView's ease of use stems from its flexible set of controls:

* AppView's configuration file, `appview.yml`, can be invoked from either the CLI or the library.

* The AppView library provides an extensive set of environment variables, which control settings like metric verbosity and event destinations. Environment variables override config file settings.

Check out the [CLI](/docs/cli-using) and [library](/docs/library-using) pages to see how it's done.
