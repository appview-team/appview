FROM quay.io/centos/centos:stream9

RUN yum -y update && \
    yum -y groupinstall 'Development Tools' && \
    yum -y install python pip && \
    yum -y install epel-release && \
    yum -y install gnutls-devel openssl-devel wget automake python-pip && \
    yum -y install ruby php httpd mod_ssl && \
    curl -sL https://rpm.nodesource.com/setup_12.x | bash - && \
    yum -y install nodejs

RUN mkdir /opt/test /opt/test-runner/ /opt/test-runner/logs /opt/test-runner/bin

RUN cd /opt/test && \
      wget https://curl.haxx.se/download/curl-7.69.1.tar.gz && \
      tar xvzf curl-7.69.1.tar.gz && \
      mv /opt/test/curl-7.69.1 /opt/test/curlssl && \
    cd /opt/test/curlssl && \
      ./buildconf && \
      ./configure --with-ssl --without-gnutls && \
      make && \
    cd /opt/test && \
      tar xvzf curl-7.69.1.tar.gz && \
      mv /opt/test/curl-7.69.1 /opt/test/curltls && \
    cd /opt/test/curltls && \
      ./buildconf && \
      ./configure --with-gnutls --without-ssl && \
      make
      
RUN curl https://sh.rustup.rs -sSf | sh -s -- -y
COPY ./tls/rust/http_test/target/debug/http_test /opt/test-runner/bin/.

RUN rm -f /opt/test/curl-7.69.1.tar.gz
COPY ./tls/test_cert.pem /opt/test/.

COPY ./tls/nodehttp.ts /opt/test-runner/bin/nodehttp.ts
COPY ./tls/test_tls.sh /opt/test-runner/bin/test_tls.sh
RUN chmod +x /opt/test-runner/bin/test_tls.sh

RUN mkdir /opt/test-runner/ruby
COPY ./tls/ruby/server.rb /opt/test-runner/ruby
COPY ./tls/ruby/client.rb /opt/test-runner/ruby
RUN chmod u+x /opt/test-runner/ruby/*rb

COPY ./tls/testssl.py /opt/test-runner/bin/testssl.py

RUN mkdir /opt/test-runner/php
COPY ./tls/php/sslclient.php /opt/test-runner/php

COPY ./tls/alias /root/.alias
COPY ./tls/gdbinit /root/.gdbinit

RUN openssl genrsa -out ca.key 2048 && \
    openssl req -new -key ca.key -out ca.csr -subj "/C=US/ST=California/L=San Francisco/O=Cribl/OU=Cribl/CN=localhost" && \
    openssl x509 -req -days 3650 -in ca.csr -signkey ca.key -out ca.crt && \
    cp ca.crt /etc/pki/tls/certs/localhost.crt && \
    cp ca.key /etc/pki/tls/private/localhost.key

ENV APPVIEW_CRIBL_ENABLE=false
ENV APPVIEW_LOG_LEVEL=info
ENV APPVIEW_METRIC_VERBOSITY=4
ENV APPVIEW_EVENT_LOGFILE=false
ENV APPVIEW_EVENT_CONSOLE=false
ENV APPVIEW_EVENT_METRIC=false
ENV APPVIEW_EVENT_HTTP=true
ENV APPVIEW_EVENT_DEST=file:///opt/test-runner/logs/events.log
ENV APPVIEW_METRIC_DEST=udp://localhost:8125
ENV APPVIEW_LOG_DEST=file:///opt/test-runner/logs/appview.log
#ENV LD_PRELOAD=/usr/local/appview/lib/libappview.so

ENV PATH="/usr/local/appview:/usr/local/appview/bin:${PATH}"
COPY appview-profile.sh /etc/profile.d/appview.sh
COPY gdbinit /root/.gdbinit

RUN mkdir /usr/local/appview && \
    mkdir /usr/local/appview/bin && \
    mkdir /usr/local/appview/lib && \
    ln -s /opt/appview/bin/linux/$(uname -m)/appview /usr/local/appview/bin/appview && \
    ln -s /opt/appview/lib/linux/$(uname -m)/libappview.so /usr/local/appview/lib/libappview.so

COPY tls/appview-test /usr/local/appview/appview-test

COPY docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["test"]
