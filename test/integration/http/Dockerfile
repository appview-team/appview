FROM cribl/cribl:3.4.1

ENV DEBIAN_FRONTEND=noninteractive
RUN apt update && apt install -y \
  curl \
  emacs \
  gdb \
  net-tools \
  nginx \
  vim \
&& rm -rf /var/lib/apt/lists/*

ENV CRIBL_NOAUTH=1
COPY http/cribl/ /opt/cribl/local/cribl/

RUN mkdir /opt/test
COPY http/conf_1 /opt/test/conf_1
COPY http/conf_2 /opt/test/conf_2

ENV PATH="/usr/local/appview:/usr/local/appview/bin:${PATH}"
COPY appview-profile.sh /etc/profile.d/appview.sh
COPY gdbinit /root/.gdbinit

RUN  mkdir /usr/local/appview && \
     mkdir /usr/local/appview/bin && \
     mkdir /usr/local/appview/lib && \
     ln -s /opt/appview/bin/linux/$(uname -m)/appview /usr/local/appview/bin/appview && \
     ln -s /opt/appview/lib/linux/$(uname -m)/libappview.so /usr/local/appview/lib/libappview.so

COPY http/appview-test /usr/local/appview/

COPY docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["test"]
