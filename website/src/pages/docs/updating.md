---
title: Updating
---

# Updating

You can update AppView in place. First, download a fresh version of the binary. (This example assumes that you are running AppView from `/opt/appview`, as described [here](/docs/downloading#where-from). If you're running AppView from a different location, adapt the command accordingly.) 

```
curl -Lo appview https://github.com/appview-team/appview/releases/download/v1.0.0/appview-x86_64
curl -Ls https://github.com/appview-team/appview/releases/download/v1.0.0/appview-x86_64.md5 | md5sum -c
mv appview-x86_64 appview
chmod +x appview
```

Then, to confirm the overwrite, verify AppView's version and build date:

```
appview version
```
