FROM alpine:latest

RUN apk add bash binutils curl gcc gdb musl-dev openssl socat

RUN mkdir -p /opt/fwrite
COPY ./musl/fwrite.c /opt/fwrite/fwrite.c
RUN gcc -o /opt/fwrite/fwrite /opt/fwrite/fwrite.c

RUN mkdir -p /opt/fault_test
COPY ./musl/hello_fault.c /opt/fault_test/hello_fault.c
RUN gcc -O0 -g -o /opt/fault_test/fault_test /opt/fault_test/hello_fault.c -lrt

RUN mkdir -p /opt/sig_test
COPY ./musl/sigusrhandler.c /opt/sig_test/sigusrhandler.c
RUN gcc -g -o /opt/sig_test/sigusrhandler /opt/sig_test/sigusrhandler.c

RUN mkdir -p /opt/extract_appview && \
    mkdir -p /opt/patch_libappview

ENV APPVIEW_CRIBL_ENABLE=false
ENV APPVIEW_LOG_LEVEL=error
ENV APPVIEW_METRIC_VERBOSITY=4
ENV APPVIEW_EVENT_LOGFILE=true
ENV APPVIEW_EVENT_CONSOLE=true
ENV APPVIEW_EVENT_METRIC=true
ENV APPVIEW_EVENT_HTTP=true
ENV APPVIEW_EVENT_NET=true
ENV APPVIEW_EVENT_FS=false
ENV APPVIEW_LOG_DEST=file:///opt/test-runner/logs/appview.log
ENV APPVIEW_EVENT_DEST=file:///opt/test-runner/logs/events.log

ENV PATH="/usr/local/appview:/usr/local/appview/bin:${PATH}"
RUN echo "export PATH=/usr/local/appview:/usr/local/appview/bin:${PATH}" >/etc/profile.d/path.sh

COPY appview-profile.sh /etc/profile.d/appview.sh
COPY gdbinit /root/.gdbinit

RUN  mkdir /usr/local/appview && \
     mkdir /usr/local/appview/bin && \
     mkdir /usr/local/appview/lib && \
     mkdir -p /opt/test-runner/logs/ && \
     ln -s /opt/appview/bin/linux/$(uname -m)/appview /usr/local/appview/bin/appview && \
     ln -s /opt/appview/lib/linux/$(uname -m)/libappview.so /usr/local/appview/lib/libappview.so

COPY musl/appview-test /usr/local/appview/appview-test


RUN (cd /usr/local/appview &&  \
     openssl req -nodes -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem \
      -subj "/C=US/ST=MN/L=Excelsior/O=Cribl/OU=Engineering/CN=cribl.io")


COPY docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["test"]
