FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt update \
    && apt install -y \
      binutils \
      gcc \
      musl-dev \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /opt/errno_test
COPY ./glibc/hello_errno.c /opt/errno_test/hello_errno.c
RUN gcc -o /opt/errno_test/errno_test /opt/errno_test/hello_errno.c

COPY ./glibc/hello_fault.c /opt/fault_test/hello_fault.c
RUN gcc -O0 -g -o /opt/fault_test/fault_test /opt/fault_test/hello_fault.c -lrt

RUN mkdir -p /opt/sig_test
COPY ./glibc/sighandler.c /opt/sig_test/sighandler.c
RUN gcc -g -o /opt/sig_test/sighandler /opt/sig_test/sighandler.c
COPY ./glibc/sigusrhandler.c /opt/sig_test/sigusrhandler.c
RUN gcc -g -o /opt/sig_test/sigusrhandler /opt/sig_test/sigusrhandler.c -lrt

RUN mkdir -p /opt/msg_test
COPY ./glibc/msgtest.c /opt/msg_test/msgtest.c
RUN gcc -g -o /opt/msg_test/msg_test /opt/msg_test/msgtest.c

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
ENV APPVIEW_EVENT_FS=true
ENV APPVIEW_LOG_DEST=file:///opt/test-runner/logs/appview.log
ENV APPVIEW_EVENT_DEST=file:///opt/test-runner/logs/events.log

ENV PATH="/usr/local/appview:/usr/local/appview/bin:${PATH}"
COPY appview-profile.sh /etc/profile.d/appview.sh
COPY gdbinit /root/.gdbinit

RUN  mkdir /usr/local/appview && \
     mkdir /usr/local/appview/bin && \
     mkdir /usr/local/appview/lib && \
     mkdir -p /opt/test-runner/logs/ && \
     ln -s /opt/appview/bin/linux/$(uname -m)/appview /usr/local/appview/bin/appview && \
     ln -s /opt/appview/lib/linux/$(uname -m)/libappview.so /usr/local/appview/lib/libappview.so

COPY glibc/appview-test /usr/local/appview/appview-test

COPY docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["test"]
