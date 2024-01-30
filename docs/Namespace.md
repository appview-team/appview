# Switch namespaces

In the appview loader we currently switch two types of namespace:

- mount namespace - copy required files to the mount namespace of a container
- PID namespace - utilize the namespace of a PID in a given container

In the CLI we additionally switch IPC namespace to establish a communication channel between the CLI and libappview.

## Effective UID and effective GID switch

It is necessary to utilize the UID and GID values of a process created in a container when accessing files within a container from outside the container. A remap of the user/group ID between a namespace outside a container and namespace inside container is required for access.
Mapping information is accessed from in `/proc/PID/uid_map` and `/proc/PID/gid_map` files:

```
   ID-inside-ns   ID-outside-ns   length
```

This allows a user to be `root` in a container, but have no privilege on host.
On the other hand, as user root on the host, the user/group ID can be changed which will impact the ownership of newly created files.

The reinterpretation of the user/group is handled by AppView code in `nsFile.c`. The handling involves a dynamic change of the effective user/group ID during file creation. For example, copying libappview to a container mount point using the container namespace includes an adaptation of the UID/GID. This is necessary in order to ensure that processes using our libappview and the appview loader are performed with effective file permissions.

For example, during a first attach of a process running in a container we do following:

- switch the mount namespace and PID namespace
- copy libappview and the appview loader into a new mount namespace (this involves creation of the new files)
- start the appview loader in the new PID and mount namespaces

In the scenario described above we must ensure that we have proper permissions in order to execute the appview loader in the context of the new namespace.

For example, performing detach or reattach in a process that is running in a container context we do the following:

- switch the mount namespace and PID namespace
- create a dynamic configuration file which is read by the process being viewed

In scenario above we must ensure that the process can read and unlink the dynamic configuration file created from the host.

## Current Status

| Container            |                    |
|----------------------|--------------------|
| docker               | :heavy_check_mark: |
| podman               | :heavy_check_mark: |
| lxc                  | :heavy_check_mark: |
| containerd - ctr     | :heavy_check_mark: |
| containerd - nerdctl | :heavy_check_mark: |

## Start an Edge container

```console
docker run --privileged -d -e CRIBL_EDGE=1 -p 9420:9420 -v /var/run/appview:/var/run/appview -v /var/run/docker.sock:/var/run/docker.sock  -v /:/hostfs:ro --restart unless-stopped --name cribl-edge cribl/cribl:next

# Optionally replace the AppView CLI in the Edge container - useful for local development
docker cp ./bin/linux/x86_64/appview cribl-edge:/opt/cribl/bin
docker exec --user root cribl-edge chown root:root /opt/cribl/bin/appview
```

The command above enables:

- [access Edge UI](http://localhost:9420/)
- Share the AppView source listening socket: `/var/run/appview/appview.sock` between host and Edge container

## [Docker](https://github.com/docker) container

```console
docker run --privileged --rm -p 6379:6379 --name redisAlpine -v /var/run/appview:/var/run/appview redis:alpine
```

## [Podman](https://github.com/containers/podman) container

```console
podman pull docker.io/redis
podman run -d --name redisPodman -p 6379:6379 -v /var/run/appview:/var/run/appview redis:latest
```

## [LXC](https://github.com/lxc/lxc)

```console
lxc launch ubuntu:20.04 lxc-example
<!-- Share the AppView Source listening socket-->
lxc config device add lxc-example appviewSocket disk source=/run/appview path=/var/run/appview/
<!-- Login to lxc container-->
lxc exec lxc-example bash
```

#### Containerd - [ctr](https://github.com/containerd/containerd)

```console
sudo ctr image pull docker.io/library/redis:latest
sudo ctr run -d --mount type=bind,src=/var/run/appview/,dst=/var/run/appview/,options=rbind:rw docker.io/library/redis:alpine ctr_redis
```

#### Containerd - [nerdctl](https://github.com/containerd/nerdctl.git) as root

```console
sudo nerdctl run -d -v /var/run/appview/:/var/run/appview/ --name nerdctl_root_redis redis:7.0.5
```

## Useful links

[Namespaces Overview](https://lwn.net/Articles/531114/)

[User namespace](https://lwn.net/Articles/532593/)
