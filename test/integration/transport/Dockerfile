FROM cribl/cribl:3.4.1

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get -o Acquire::Check-Valid-Until=false update && apt install -y \
  curl \
  net-tools \
  socat \
  lsof \
  golang-go \
&& rm -rf /var/lib/apt/lists/*

ENV CRIBL_NOAUTH=1
COPY transport/cribl/ /opt/cribl/local/cribl/

RUN mkdir -p /opt/test/bin
COPY transport/appview-test /opt/test/bin

COPY transport/plainClient.go /opt/test/bin
RUN cd /opt/test/bin && \
    openssl genrsa -out server.key 2048 && \
    openssl ecparam -genkey -name secp384r1 -out server.key && \
    openssl req -new -x509 -sha256 \
      -key server.key \
      -out server.crt \
      -days 3650 \
      -subj "/C=US/ST=California/L=San Francisco/O=Cribl/OU=Cribl/CN=localhost"
RUN  cd /opt/test/bin && \
     CGO_ENABLED=0 go build -o plainClientStatic plainClient.go

ENV PATH="/usr/local/appview:/usr/local/appview/bin:${PATH}"
COPY appview-profile.sh /etc/profile.d/appview.sh
COPY gdbinit /root/.gdbinit

RUN  mkdir /usr/local/appview && \
     mkdir /usr/local/appview/bin && \
     mkdir /usr/local/appview/lib && \
     ln -s /opt/appview/bin/linux/$(uname -m)/appview /usr/local/appview/bin/appview && \
     ln -s /opt/appview/lib/linux/$(uname -m)/libappview.so /usr/local/appview/lib/libappview.so

COPY transport/appview-test /usr/local/appview/

COPY docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["test"]
