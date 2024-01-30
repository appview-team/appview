FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt update \
    && apt install -y \
      bsdmainutils \
      curl \
      firefox \
      xxd \
      git-core \
      openssh-server \
      apache2 \
      apache2-utils \
      vim \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /opt/test/log && \
    chmod -R 777 /opt/test/log && \
    mkdir -p /opt/tar_test

ENV APPVIEW_CRIBL_ENABLE=false
ENV APPVIEW_LOG_LEVEL=error
ENV APPVIEW_METRIC_VERBOSITY=4
ENV APPVIEW_EVENT_LOGFILE=true
ENV APPVIEW_EVENT_CONSOLE=true
ENV APPVIEW_EVENT_METRIC=true
ENV APPVIEW_EVENT_HTTP=true
ENV APPVIEW_EVENT_DEST=file:///opt/test/log/events.log

RUN addgroup --system --gid 1234 test_group
RUN adduser --uid 3333 --system --gid 1234 --disabled-password test_user --shell /bin/bash

ENV PATH="/usr/local/appview:/usr/local/appview/bin:${PATH}"
COPY appview-profile.sh /etc/profile.d/appview.sh
COPY gdbinit /root/.gdbinit

RUN  mkdir /usr/local/appview && \
     mkdir /usr/local/appview/bin && \
     mkdir /usr/local/appview/lib && \
     ln -s /opt/appview/bin/linux/$(uname -m)/appview /usr/local/appview/bin/appview && \
     ln -s /opt/appview/lib/linux/$(uname -m)/libappview.so /usr/local/appview/lib/libappview.so

COPY ./console/appview-test /usr/local/appview/appview-test
COPY ./console/httpd.conf /etc/apache2/sites-available/git.conf

COPY docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["test"]
