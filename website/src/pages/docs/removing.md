---
title: Removing
---

## Removing

You can remove AppView by simply deleting the binary, along with the rest of the contents of the `APPVIEW_HOME` directory. For example, if your `APPVIEW_HOME` directory is `/opt/appview/`:

```
rm -rf /opt/appview/
```

Then delete the associated history directory:

```
cd ~
rm -rf .appview/
```
</br>

Currently appview’d applications will continue to run. To remove the AppView library from a running process, use `appview detach <process_ID>`.
