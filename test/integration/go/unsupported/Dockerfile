ARG GO_IMAGE_VER=golang:1.14
FROM $GO_IMAGE_VER

COPY ./go/unsupported/test_go.sh /go

RUN mkdir /go/thread
COPY ./go/thread/fileThread.go /go/thread
RUN cd /go/thread && \
    CGO_ENABLED=0 go build fileThread.go

RUN mkdir /go/net
COPY ./go/net/plainServer.go /go/net
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
    openssl req -new -x509 -sha256 -key server.key -out server.crt \
     -days 3650 \
     -subj "/C=US/ST=California/L=San Francisco/O=Cribl/OU=Cribl/CN=localhost"
COPY ./go/net/plainClient.go /go/net
RUN cd /go/net && \
    go build -o plainClientDynamic plainClient.go
RUN cd /go/net && \
    CGO_ENABLED=0 go build -o plainClientStatic plainClient.go
COPY ./go/net/tlsClient.go /go/net
RUN cd /go/net && \
    go build -o tlsClientDynamic tlsClient.go
RUN cd /go/net && \
    CGO_ENABLED=0 go build -o tlsClientStatic tlsClient.go

RUN mkdir /go/cgo
COPY ./go/cgo/Makefile /go/cgo
COPY ./go/cgo/myc.c /go/cgo
COPY ./go/cgo/myc.h /go/cgo
COPY ./go/cgo/mygo.go /go/cgo
RUN cd /go/cgo && \
    make all

RUN mkdir -p /go/influx
COPY ./go/influx/* /go/influx/
COPY ./go/influx/influxdb-selfsigned.key /etc/ssl/.
COPY ./go/influx/influxdb-selfsigned.crt /etc/ssl/.

ENV APPVIEW_METRIC_VERBOSITY=4
ENV APPVIEW_EVENT_LOGFILE=true
ENV APPVIEW_EVENT_CONSOLE=true
ENV APPVIEW_EVENT_METRIC=true
ENV APPVIEW_EVENT_HTTP=true
ENV APPVIEW_EVENT_DEST=file:///go/events.log
ENV APPVIEW_LOG_LEVEL=warning
ENV APPVIEW_LOG_DEST=file:///tmp/appview.log
ENV APPVIEW_ALLOW_CONSTRUCT_DBG=true

RUN echo "export PATH=/usr/local/appview:/usr/local/appview/bin:${PATH}" >/etc/profile.d/path.sh
COPY appview-profile.sh /etc/profile.d/appview.sh
COPY gdbinit /root/.gdbinit

RUN  mkdir /usr/local/appview && \
     mkdir /usr/local/appview/bin && \
     mkdir /usr/local/appview/lib && \
     ln -s /opt/appview/bin/linux/appview /usr/local/appview/bin/appview && \
     ln -s /opt/appview/lib/linux/libappview.so /usr/local/appview/lib/libappview.so

COPY go/unsupported/appview-test /usr/local/appview/appview-test

COPY docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["test"]

