FROM ubuntu:latest

RUN apt update && apt install -y \
  curl \
  emacs \
  gdb \
  net-tools \
  vim \
  apache2-utils \
  nginx \
&& rm -rf /var/lib/apt/lists/*

RUN mkdir /opt/test && \
    mkdir /usr/local/appview && \
    mkdir /usr/local/appview/bin && \
    mkdir /usr/local/appview/lib

COPY conf_1 /opt/test/conf_1
COPY conf_2 /opt/test/conf_2
COPY conf_3 /opt/test/conf_3
COPY tcpserver /usr/bin/tcpserver
COPY appview-test /usr/local/appview/

RUN  ln -s /opt/appview/bin/linux/$(uname -m)/appview /usr/local/appview/bin/appview && \
     ln -s /opt/appview/bin/linux/$(uname -m)/ldappview /usr/local/appview/bin/ldappview && \
     ln -s /opt/appview/lib/linux/$(uname -m)/libappview.so /usr/local/appview/lib/libappview.so

ENV PATH="/usr/local/appview:/usr/local/appview/bin:${PATH}"

COPY docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["test"]
