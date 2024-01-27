FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt update \
    && apt install -y \
    nginx \
    openssl \
    curl \
    emacs \
    gdb \
    lsof \
    netcat \
    vim \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /var/run/appview/ && \
    mkdir /usr/local/appview && \
    mkdir /usr/local/appview/bin && \
    mkdir /usr/local/appview/lib && \
    ln -s /opt/appview/bin/linux/$(uname -m)/appview /usr/local/appview/bin/appview && \
    ln -s /opt/appview/lib/linux/$(uname -m)/libappview.so /usr/local/appview/lib/libappview.so

RUN openssl genrsa -out ca.key 2048 && \
    openssl req -new -key ca.key -out ca.csr -subj "/C=US/ST=California/L=San Francisco/O=Cribl/OU=Cribl/CN=localhost" && \
    openssl x509 -req -days 3650 -in ca.csr -signkey ca.key -out ca.crt && \
    cp ca.crt /etc/ssl/certs/ && \
    cp ca.key /etc/ssl/private/ && \
    cp ca.csr /etc/ssl/private/

COPY appview-profile.sh /etc/profile.d/appview.sh
COPY gdbinit /root/.gdbinit
COPY ./nginx/appview-test /usr/local/appview/appview-test
COPY ./nginx/test_nginx.sh /opt/test/bin/test_nginx.sh
COPY ./nginx/nginx.conf /etc/nginx/nginx.conf
COPY docker-entrypoint.sh /

ENV PATH="/usr/local/appview:/usr/local/appview/bin:${PATH}"
ENV APPVIEW_LOG_DEST=file:///tmp/appview.log
ENV APPVIEW_EVENT_DEST=file:///tmp/events.json
ENV APPVIEW_METRIC_DEST=file:///tmp/metrics.json
ENV APPVIEW_CRIBL_ENABLE=false
ENV APPVIEW_LOG_LEVEL=error
ENV APPVIEW_METRIC_VERBOSITY=4
ENV APPVIEW_EVENT_LOGFILE=true
ENV APPVIEW_EVENT_CONSOLE=true
ENV APPVIEW_EVENT_METRIC=true
ENV APPVIEW_EVENT_HTTP=true

ENTRYPOINT ["/docker-entrypoint.sh"]

CMD ["test"]

