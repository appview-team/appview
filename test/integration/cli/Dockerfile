FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt update \
    && apt install -y \
      emacs \
      gdb \
      jq \
      lsof \
      netcat \
      python3 \
      vim \
      libbpf0 \
    && rm -rf /var/lib/apt/lists/*

# Tests
COPY ./cli/test_cli.sh /opt/test/bin/test_cli.sh
COPY ./cli/test_cli_dest.sh /opt/test/bin/test_cli_dest.sh
COPY ./cli/test_edge.sh /opt/test/bin/test_edge.sh
COPY ./cli/test_options.sh /opt/test/bin/test_options.sh
COPY ./cli/test_inspect.sh /opt/test/bin/test_inspect.sh
COPY ./cli/expected.yml /
RUN mkdir /opt/test/conf
COPY ./cli/payload_conf.yml /opt/test/bin/payload_conf.yml
COPY ./cli/update_log_dest.yml /opt/test/bin/update_log_dest.yml

RUN mkdir /opt/test-runner && \
    mkdir -p /var/run/appview/ && \
    mkdir -p /opt/cribl/state/ && \
    mkdir -p /opt/cribl/home/state && \
    touch /opt/test-runner/empty_file

ENV APPVIEW_CRIBL_ENABLE=false
ENV APPVIEW_LOG_LEVEL=error
ENV APPVIEW_METRIC_VERBOSITY=4
ENV APPVIEW_EVENT_LOGFILE=true
ENV APPVIEW_EVENT_CONSOLE=true
ENV APPVIEW_EVENT_METRIC=true
ENV APPVIEW_EVENT_HTTP=true
ENV APPVIEW_CLI_SKIP_HOST=true
ENV APPVIEW_RULES=false

ENV PATH="/usr/local/appview:/usr/local/appview/bin:${PATH}"
COPY appview-profile.sh /etc/profile.d/appview.sh
COPY gdbinit /root/.gdbinit

RUN  mkdir /usr/local/appview && \
     mkdir /usr/local/appview/bin && \
     mkdir /usr/local/appview/lib && \
     ln -s /opt/appview/bin/linux/$(uname -m)/appview /usr/local/appview/bin/appview && \
     ln -s /opt/appview/lib/linux/$(uname -m)/libappview.so /usr/local/appview/lib/libappview.so

COPY cli/appview-test /usr/local/appview/appview-test

COPY docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["test"]

