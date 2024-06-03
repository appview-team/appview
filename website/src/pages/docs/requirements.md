---
title: Requirements
---

## Requirements

Requirements for AppView are as follows:

### Operating Systems (Linux Only)

AppView runs on:

- Ubuntu 16 and later
- Alpine and other distributions based on musl libc
- RedHat Enterprise Linux or CentOS 7 and later
- Amazon Linux 1 and 2
- Debian

When building AppView from source, use:

- Ubuntu 18.04

This restriction is imposed to make the resulting executable more portable.

### System

- CPU: x86-64 and ARM64 architectures
- Memory: 1GB
- Disk: 20MB (library + CLI)

### Filesystem Configuration

The distros that AppView supports all require the use of `/tmp`, `/dev/shm`, and `/proc`. You should avoid custom filesystem configuration that interferes with AppView's ability to use these directories.

### Cross-Compatibility with Cribl Suite

AppView 1.4.1, Cribl Stream 4.2.1, Cribl Edge 4.2.1, and Cribl Search 4.2.1 are mutually compatible. If you integrate any of these products with earlier versions of peer products, some or all features will be unavailable.

### Known Limitations

AppView can instrument static executables only when they are written in Go.

AppView cannot:

- Instrument Go executables built with Go 1.10 or earlier.
- Instrument static stripped Go executables built with Go 1.12 or earlier.
- Instrument Java executables that use Open JVM 6 or earlier, or Oracle JVM 6 or earlier.
- Obtain a core dump either (a) for a Go executable, or (b) in a musl libc environment.
- Unload the libappview library, once loaded (but a `detach` removes all AppView interpositions and terminates the AppView thread).

When an executable that's being viewed has been [stripped](https://en.wikipedia.org/wiki/Strip_(Unix)), it is not possible for `libappview.so` to obtain a file descriptor for an SSL session, and in turn, AppView cannot include IP and port number fields in HTTP events.

If you run AppView on a system where [AppArmor](https://apparmor.net/) or [SELinux](https://github.com/SELinuxProject/selinux) are in an enforcing mode, it can be necessary to modify your AppArmor or SELinux profiles to allow AppView (and the system as whole) to work normally.
