ARG GO_IMAGE_VER=golang:1.14
FROM $GO_IMAGE_VER

RUN apt-get -o Acquire::Check-Valid-Until=false update && \
    apt-get install -y vim curl file bsdmainutils gdb

COPY ./go/test_go.sh /go
COPY ./go/test_go_struct.sh /go

RUN mkdir /go/thread
COPY ./go/thread/fileThread.go /go/thread
RUN cd /go/thread && CGO_ENABLED=0 go build fileThread.go

RUN mkdir /go/syscalls
COPY ./go/syscalls/unlinkat.go /go/syscalls
RUN cd /go/syscalls && \
    go build -o unlinkat unlinkat.go
COPY ./go/syscalls/opfalse.go /go/syscalls
RUN cd /go/syscalls && \
    go build -o opfalse opfalse.go
COPY ./go/syscalls/readdir.go /go/syscalls
RUN cd /go/syscalls && \
    go build -o readdir readdir.go

RUN mkdir /go/net
COPY ./go/net/plainServer.go /go/net
RUN cd /go/net && \
    go build -buildmode=pie -o plainServerDynamicPie plainServer.go
RUN cd /go/net && \
    go build -o plainServerDynamic plainServer.go
RUN cd /go/net && \
    CGO_ENABLED=0 go build -o plainServerStatic plainServer.go
COPY ./go/net/tlsServer.go /go/net
RUN cd /go/net && \
    go build -o tlsServerDynamic tlsServer.go
RUN cd /go/net && \
    CGO_ENABLED=0 go build -o tlsServerStatic tlsServer.go
RUN cd /go/net && \
    openssl genrsa -out server.key 2048 && \
    openssl ecparam -genkey -name secp384r1 -out server.key && \
    openssl req -new -x509 -sha256 \
      -key server.key \
      -out server.crt \
      -days 3650 \
      -subj "/C=US/ST=California/L=San Francisco/O=Cribl/OU=Cribl/CN=localhost"
COPY ./go/net/plainClient.go /go/net
RUN cd /go/net &&\
    go build -o plainClientDynamic plainClient.go
RUN cd /go/net && \
    CGO_ENABLED=0 go build -o plainClientStatic plainClient.go
RUN cd /go/net && \
    CGO_ENABLED=0 go build -ldflags='-s' -o plainClientStaticStripped plainClient.go
COPY ./go/net/tlsClient.go /go/net
RUN cd /go/net && \
    go build -o tlsClientDynamic tlsClient.go
RUN cd /go/net && \
    CGO_ENABLED=0 go build -o tlsClientStatic tlsClient.go

RUN mkdir /go/signals
COPY ./go/signals/signalHandler.go /go/signals
RUN cd /go/signals && \
    go build -o signalHandlerDynamic signalHandler.go && \
    CGO_ENABLED=0 go build -o signalHandlerStatic signalHandler.go && \
    CGO_ENABLED=0 go build -ldflags="-s -w" -o signalHandlerStaticStripped signalHandler.go

RUN mkdir /go/cgo
COPY ./go/cgo/Makefile /go/cgo
COPY ./go/cgo/myc.c /go/cgo
COPY ./go/cgo/myc.h /go/cgo
COPY ./go/cgo/mygo.go /go/cgo
RUN cd /go/cgo && make all

RUN mkdir -p /go/influx
COPY ./go/influx/* /go/influx/
COPY ./go/influx/influxdb-selfsigned.key /etc/ssl/.
COPY ./go/influx/influxdb-selfsigned.crt /etc/ssl/.

ENV APPVIEW_CRIBL_ENABLE=false
ENV APPVIEW_LOG_LEVEL=warning
ENV APPVIEW_LOG_DEST=file:///tmp/appview.log
ENV APPVIEW_METRIC_VERBOSITY=4
ENV APPVIEW_EVENT_LOGFILE=true
ENV APPVIEW_EVENT_CONSOLE=true
ENV APPVIEW_EVENT_METRIC=true
ENV APPVIEW_EVENT_HTTP=true
ENV APPVIEW_EVENT_DEST=file:///go/events.log

RUN echo "export PATH=/usr/local/appview:/usr/local/appview/bin:${PATH}" >/etc/profile.d/path.sh
COPY appview-profile.sh /etc/profile.d/appview.sh
COPY gdbinit /root/.gdbinit

RUN  mkdir /usr/local/appview && \
     mkdir /usr/local/appview/bin && \
     mkdir /usr/local/appview/lib && \
     ln -s /opt/appview/bin/linux/$(uname -m)/appview /usr/local/appview/bin/appview && \
     ln -s /opt/appview/lib/linux/$(uname -m)/libappview.so /usr/local/appview/lib/libappview.so

COPY go/appview-test /usr/local/appview/appview-test

COPY docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["test"]

