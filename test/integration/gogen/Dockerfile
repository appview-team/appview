# FROM alpine:latest
FROM frolvlad/alpine-glibc

WORKDIR /gogen
RUN apk update && apk add \
    ca-certificates \
    openssl \
    wget \
    bash
RUN wget https://api.gogen.io/linux/gogen --no-check-certificate
RUN chmod 755 ./gogen

# tcpserver is an app built from git@bitbucket.org:cribl/appview.git
# gcc -g test/manual/tcpserver.c -lpthread -o tcpserver
COPY gogen/tcpserver /usr/bin/tcpserver

ENV APPVIEW_CRIBL_ENABLE=false
ENV APPVIEW_LOG_LEVEL=error
ENV APPVIEW_METRIC_VERBOSITY=4
#ENV APPVIEW_EVENT_LOGFILE=true
#ENV APPVIEW_EVENT_CONSOLE=true
ENV APPVIEW_EVENT_METRIC=true
#ENV APPVIEW_EVENT_METRIC_NAME=net\.tx
ENV APPVIEW_SUMMARY_PERIOD=1
#ENV APPVIEW_EVENT_HTTP=true
ENV APPVIEW_EVENT_DEST=file:///gogen/events.log

#ENV PATH="/usr/local/appview:/usr/local/appview/bin:${PATH}"
RUN echo "export PATH=/usr/local/appview:/usr/local/appview/bin:${PATH}" >/etc/profile.d/path.sh
COPY appview-profile.sh /etc/profile.d/appview.sh
COPY gdbinit /root/.gdbinit

RUN  mkdir /usr/local/appview && \
     mkdir /usr/local/appview/bin && \
     mkdir /usr/local/appview/lib && \
     ln -s /opt/appview/bin/linux/$(uname -m)/appview /usr/local/appview/bin/appview && \
     ln -s /opt/appview/lib/linux/$(uname -m)/libappview.so /usr/local/appview/lib/libappview.so

COPY gogen/appview-test /usr/local/appview/appview-test
COPY gogen/weblog.yml /gogen/weblog.yml

COPY docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["test"]

