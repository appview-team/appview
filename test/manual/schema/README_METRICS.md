# This defines how to create sample metrics in json format for use in schema validation

## Extract Samples

These were made by setting these and running the syscalls test

    export APPVIEW_METRIC_VERBOSITY=4 (or 9)

    export APPVIEW_METRIC_FORMAT=ndjson

    export APPVIEW_METRIC_DEST=file:///tmp/mtc.out

    /usr/local/appview/appview-test

    then running onemtc.sh to create a filtered version of the output

-rw-r--r--  1 ubuntu ubuntu     6635 Jan 31 22:39 mtc.filtered.4

-rw-r--r--  1 ubuntu ubuntu     7203 Jan 31 22:39 mtc.filtered.9

-rw-r--r--  1 ubuntu ubuntu  3012062 Jan 31 22:39 mtc.out.4

-rw-r--r--  1 ubuntu ubuntu 37283823 Jan 31 22:39 mtc.out.9

These were made by setting these and running the tls test

      export APPVIEW_METRIC_VERBOSITY=4 (or 9)

      export APPVIEW_METRIC_FORMAT=ndjson

      export APPVIEW_METRIC_DEST=file:///tmp/http.out

      /usr/local/appview/appview-test

      then running onemtc2.sh to create a filtered version of the output

-rw-r--r--  1 root   root       1282 Jan 31 23:20 http.filtered.4

-rw-r--r--  1 root   root       1310 Jan 31 23:28 http.filtered.9

-rw-r--r--  1 root   root     124879 Jan 31 23:21 http.out.4

-rw-r--r--  1 root   root   41129569 Jan 31 23:28 http.out.9

In order to capture the metric http.req.content_length the following command can be used:
curl -X POST https://reqbin.com/echo/post/json -H "Content-Type: application/json" -d '{"productId": 123456, "quantity": 100}'
Example:
      export APPVIEW_METRIC_VERBOSITY=4 (or 9)

      export APPVIEW_METRIC_FORMAT=ndjson

      export APPVIEW_METRIC_DEST=file:///tmp/http.clen

      ldappview -- curl -X POST https://reqbin.com/echo/post/json -H "Content-Type: application/json" -d '{"productId": 123456, "quantity": 100}'

      the metric for http.req.content_length will be found in http.clen
