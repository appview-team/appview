# Snapshot information

Snapshot feature is enabled by one of the following

- enabling the coredump, see `snapshot` -> `coredump` option in `appview.yml`
- enabling the backtrace, see `snapshot` -> `backtrace` option in `appview.yml`

Snapshot file details
The files will be placed in folllowing locations:
`/tmp/appview/<PID>/`

`/tmp/appview/<PID>/info_<timestamp>`
This file is generated when the snapshot feature is enabled.

Example content:
```
AppView Version: v1.2.2-328-g73a7ded1e726
Unix Time: 1675350952 sec
PID: 1370399
Process name: htop
```

`/tmp/appview/<PID>/cfg_<timestamp>`
This file is generated when the snapshot feature is enabled.

Example content:
```
{"metric":{"enable":"true","transport":{"type":"file","path":"/home/testuser/.appview/history/htop_4_1370399_1675350868661357625/metrics.json","buffering":"line"},"format":{"type":"ndjson","statsdprefix":"","statsdmaxlen":512,"verbosity":4},"watch":[{"type":"fs"},{"type":"net"},{"type":"http"},{"type":"dns"},{"type":"process"},{"type":"statsd"}]},"libappview":{"log":{"level":"warning","transport":{"type":"file","path":"/home/testuser/.appview/history/htop_4_1370399_1675350868661357625/libappview.log","buffering":"line"}},"snapshot":{"backtrace":"false"},"configevent":"false","summaryperiod":10,"commanddir":"/home/testuser/.appview/history/htop_4_1370399_1675350868661357625/cmd"},"event":{"enable":"true","transport":{"type":"file","path":"/home/testuser/.appview/history/htop_4_1370399_1675350868661357625/events.json","buffering":"line"},"format":{"type":"ndjson","maxeventpersec":10000,"enhancefs":"true"},"watch":[{"type":"file","name":"(\\/logs?\\/)|(\\.log$)|(\\.log[.\\d])","field":".*","value":".*"},{"type":"console","name":"(stdout|stderr)","field":".*","value":".*","allowbinary":"false"},{"type":"http","name":".*","field":".*","value":".*","headers":[]},{"type":"net","name":".*","field":".*","value":".*"},{"type":"fs","name":".*","field":".*","value":".*"},{"type":"dns","name":".*","field":".*","value":".*"}]},"payload":{"enable":"false","dir":"/tmp"},"tags":{},"protocol":[],"cribl":{"enable":"false","transport":{"type":"edge"},"authtoken":""}}
```

`/tmp/appview/<PID>/coredump_<timestamp>`
This file is generated when the coredump feature is enabled.
Contains the coredump generated during crash.
For further detail: [gdb](https://sourceware.org/gdb/onlinedocs/gdb/Core-File-Generation.html)

`/tmp/appview/<PID>/backtrace_<timestamp>`
This file is generated when the backtrace feature is enabled.
Contains the backtrace captured during crash.
