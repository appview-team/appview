---
title: Using the Library
---

## Using the Library (libappview.so)

To use the library for the first time in a given environment, complete this quick procedure:

1. [Download](/docs/downloading) AppView. 
2. Decide on a `APPVIEW_HOME` directory, i.e., the [directory from which AppView should run](/docs/downloading#where-from) in your environment.
3. Set `APPVIEW_HOME` to the desired directory.
   ```
   ubuntu@my_hostname:~/someuser/temp$ export APPVIEW_HOME=/opt/appview
   ```
4. Create the `APPVIEW_HOME` directory. (Here, `sudo` is required because `opt` is owned by root.)
   ```
   ubuntu@my_hostname:~/someuser/temp$ sudo mkdir $APPVIEW_HOME
   ```
5. Extract (`appview extract`) the contents of the AppView binary into the `APPVIEW_HOME` directory.
   ```
   ubuntu@my_hostname:~/someuser/temp$ sudo ./appview extract $APPVIEW_HOME
   Successfully extracted to /opt/appview.
   ```
6. Verify that `APPVIEW_HOME` contains the AppView library (`libappview.so`) and the config file (`appview.yml`). 
   ```
   ubuntu@my_hostname:~/someuser/temp$ ls -al $APPVIEW_HOME
   total 20528
   drwxr-xr-x 2 root root     4096 Jul 11 22:51 .
   drwxr-xr-x 5 root root     4096 Jul 11 22:51 ..
   -rwxr-xr-x 1 root root  9663240 Jul 11 22:51 libappview.so
   -rw-r--r-- 1 root root    35755 Jul 11 22:51 appview.yml
   ubuntu@my_hostname:~/someuser/temp$ 
   ```

Now you are ready to configure AppView to instrument any application and output data to any existing tool via simple TCP protocols.

Depending on your use case and preferred way of working, this usually entails editing `appview.yml`, and then setting environment variables while invoking the library.

How the library is loaded depends on the type of executable. A dynamic loader can preload the library (where supported), while AppView can load static executables. Regardless of how the library is loaded, you get full control of the data source, formats, and transports.

<span id="env-vars"> </span>

### The Config File

For the default settings in the sample `appview.yml` configuration file, see [Config File](/docs/config-file), or inspect the most-recent file on [GitHub](https://github.com/appview-team/appview/blob/master/conf/appview.yml).

To see the config file with comments omitted, run the following command:

```
egrep -v '^ *#.*$' appview.yml | sed '/^$/d' >appview-minimal.yml

```

This can help you get a clear idea of exactly how AppView is configured, assuming you have previously read and understood the comments.

### Using the Library Directly

To use the library directly, you rely on the `LD_PRELOAD` environment variable. 

The following examples provide an overview of this way of working with the library. All the examples call the system-level `ps` command, just to show how the syntax works.

For more, check out the [Further Examples](/docs/examples-use-cases), which include both CLI and library use cases.

#### `LD_PRELOAD` with a Single Command

Start with this basic example:

```
LD_PRELOAD=./libappview.so ps -ef
```

This executes the command `ps -ef`. But first, the OS's loader loads the AppView library, as part of loading and linking the `ps` executable.

Details of the `ps` application's execution are emitted to the configured transport, in the configured format. For configuration details, see [Env Vars and the Config File](#env-vars) above.

#### `LD_PRELOAD` with Verbosity Specified

```
LD_PRELOAD=./libappview.so APPVIEW_METRIC_VERBOSITY=5 ps -ef
```

This again executes the `ps` command using the AppView library. But it also defines the [verbosity](/docs/events-and-metrics#metrics) for metric extraction as level `5`. (This verbosity setting overrides any config-file setting, as well as the default value.)

#### `LD_PRELOAD` with a Config File

```
LD_PRELOAD=./libappview.so APPVIEW_HOME=/etc/appview ps -ef
```

This again executes the `ps` command using the AppView library. But it also directs the library to use the config file `/etc/appview/appview.yml`.

#### `LD_PRELOAD` with a TCP Connection

```
LD_PRELOAD=./libappview.so APPVIEW_EVENT_DEST=tcp://localhost:9999 APPVIEW_CRIBL_ENABLE=false ps -ef
```

This again executes the `ps` command using the AppView library. But here, we also specify that events (as opposed to metrics) will be sent over a TCP connection to localhost, using port `9999`. (This event destination setting overrides any config-file setting, as well as the default value.)

### Adding AppView to a `systemd` (boot-time) Service

In this example, we'll add AppView to the `httpd` service, described by an `httpd.service` file which contains an `EnvironmentFile=/etc/sysconfig/httpd` entry.

1. Extract the library to a new directory (`/opt/appview` in this example):

```
mkdir /opt/appview && cd /opt/appview
curl -Lo appview https://github.com/appview-team/appview/releases/download/v1.0.0/appview-x86_64
curl -Ls https://github.com/appview-team/appview/releases/download/v1.0.0/appview-x86_64.md5 | md5sum -c
chmod +x appview
./appview extract .
```

The result will be that the system uses `/opt/appview/appview.yml` to configure `libappview.so`.

2. Add an `LD_PRELOAD` environment variable to the `systemd` config file.

In the `httpd.service` file, edit the `/etc/sysconfig/httpd` entry to include the following environment variables:

```
APPVIEW_HOME=/opt/appview
LD_PRELOAD=/opt/appview/libappview.so
```

<span id="lambda"> </span>

### Deploying the Library in an AWS Lambda Function

You can interpose the `libappview.so` library into an AWS Lambda function as a [Lambda layer](https://docs.aws.amazon.com/lambda/latest/dg/configuration-layers.html). By default, Lambda functions use `lib` as their `LD_LIBRARY_PATH`, which makes loading AppView easy.

Assuming that you have [created](https://aws.amazon.com/lambda/getting-started/) one or more AWS Lambda functions, all you need to do is add the Lambda layer, then set environment variables for the Lambda function.

#### Adding an AppView AWS Lambda Layer

1. Start with one of the AWS Lambda Layers for AppView that we provide. You can obtain the AWS Lambda Layers and their MD5 checksums from the GitHub releases page.
    - [AWS Lambda Layer for x86](https://github.com/appview-team/appview/releases/download/v1.0.0/aws-lambda-layer-x86_64.zip)
    - [AWS Lambda Layer for ARM](https://github.com/appview-team/appview/releases/download/v1.0.0/aaws-lambda-layer-aarch64.zip)
    - To obtain the MD5 checksum for either file above, add `.md5` to the file path.
2. Complete the procedure for creating a layer described in the [AWS docs](https://docs.aws.amazon.com/lambda/latest/dg/configuration-layers.html#configuration-layers-create), uploading your AppView AWS Lambda Layer ZIP file in the **upload your layer code** step, and choosing `x86_64` or `ARM64`, as appropriate, for **Compatible architectures**.
3. After you click **Create**, note the **Version ARN** shown for your newly-created layer.
4. Navigate to **Lambda** > **Layers** > **Add layer**, and in the **Choose a layer** section, select **Specify an ARN**. 
5. Enter your layer's ARN, click **Verify**, and then click **Add**.  

#### Setting the Lambda Function's Environment Variables

The AWS docs [explain](https://docs.aws.amazon.com/lambda/latest/dg/configuration-envvars.html) how to set environmental variables for Lambda functions. You'll need to enter the following AppView environment variable settings in the AWS UI.

1. `LD_PRELOAD` gets your Lambda function working with AppView.

    - `LD_PRELOAD=libappview.so`

2. `APPVIEW_EXEC_PATH` is required for static executables (like the Go runtime).

    - `APPVIEW_EXEC_PATH=/opt/appview/appview`

3. To tell AppView where to deliver events, the required environment variable depends on your desired [Data Routing](/docs/data-routing).

    - For example, `APPVIEW_CRIBL_CLOUD` is required for an [AppView Source](https://docs.cribl.io/stream/sources-appview) in a Cribl.Cloud-managed instance of Cribl Stream. (Substitute your host and port values for the placeholders.)

    - `APPVIEW_CRIBL_CLOUD=tcp://<host>:<port>`

4. Optionally, set additional environment variables as desired. 

    - For example, `APPVIEW_CONF_PATH` ensures that your Lambda function uses AppView with the correct config file. (Edit the path if yours is different.)

    - `APPVIEW_CONF_PATH=/opt/appview/appview.yml`

