# This defines how to create sample json events for use in schema validation

## Extract Samples
The extract.sh script is used to grab 1 sample of each event type from 3 sources.
Create the sources, then run the extract script to create a single file with a sample of each event.

## Create the sources
edit appview/test/integration/syscalls/Dockerfile
to the list of ENV settings, add the line:
ENV APPVIEW_EVENT_DEST=file:///opt/appview/test/manual/schema/syscalls.evt

### Get the syscall events


$ cd appview/test/integration

$ make syscalls

### Get http client events


$ appview run -- curl wttr.in

$ cp ~/.appview/history/curl_XXX/events.json appview/test/manual/schema/http_client.evt

### Get http server events


(if needed install nginx; apt install -y nginx)

$ sudo nginx -s stop

$ sudo bash

$ appview run -- nginx

$ appview run -- curl localhost

$ cp ~/.appview/history/curl_XXX/events.json appview/test/manual/schema/http_server.evt

$ nginx -s stop

$ exit

### Extract the sources


$ appview/test/manual/schema/extract.sh

you have a sample.evt file with an example of each event.

