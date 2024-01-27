FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt update \
    && apt install -y \
      curl \
      git \
      htop \
      netcat \
      nginx \
      memcached \
      redis \
    && rm -rf /var/lib/apt/lists/*

ENV PATH="/usr/local/appview:/usr/local/appview/bin:${PATH}"
RUN echo "export PATH=/usr/local/appview:/usr/local/appview/bin:${PATH}" >/etc/profile.d/path.sh

ENV APPVIEW_CLI_SKIP_HOST=true
COPY appview-profile.sh /etc/profile.d/appview_profile.sh
COPY gdbinit /root/.gdbinit

RUN  mkdir /usr/local/appview && \
     mkdir /usr/local/appview/bin && \
     mkdir /usr/local/appview/lib && \
     mkdir -p /opt/test-runner/logs/ && \
     ln -s /opt/appview/bin/linux/$(uname -m)/appview /usr/local/appview/bin/appview && \
     ln -s /opt/appview/lib/linux/$(uname -m)/libappview.so /usr/local/appview/lib/libappview.so

COPY cli_rules/appview-test /usr/local/appview/appview-test
COPY cli_rules/appview_rules1.yml /opt/test-runner/appview_rules1.yml
COPY cli_rules/appview_rules2.yml /opt/test-runner/appview_rules2.yml

COPY docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["test"]
