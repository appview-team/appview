---
title: Updating
---

# Updating

You can update AppView in place. First, download a fresh version of the binary. (This example assumes that you are running AppView from `/opt/appview`, as described [here](/docs/downloading#where-from). If you're running AppView from a different location, adapt the command accordingly.) 

```
curl -Lo /opt/appview https://s3-us-west-2.amazonaws.com/io.cribl.cdn/dl/appview/cli/linux/appview && chmod 755 /opt/appview
```

Then, to confirm the overwrite, verify AppView's version and build date:

```
appview version
```
