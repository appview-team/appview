FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt update \
    && apt install -y \
      curl \
      dnsutils \
      gdb \
      openssh-client \
      openssh-server \
      sshpass \
    && rm -rf /var/lib/apt/lists/*

# Create a user “sshuser” and group “sshgroup”
RUN groupadd sshgroup && useradd -ms /bin/bash -g sshgroup sshuser
RUN echo sshuser:sshuser | chpasswd 

RUN mkdir -p /opt/test-runner
COPY sshd/ssh_test.sh /opt/test-runner

RUN service ssh start

ENV APPVIEW_CRIBL_ENABLE=false
ENV APPVIEW_EVENT_DEST=file:///opt/test-runner/events.log
ENV APPVIEW_LOG_DEST=file:///opt/test-runner/appview.log

ENV PATH="/usr/local/appview:/usr/local/appview/bin:${PATH}"
COPY appview-profile.sh /etc/profile.d/appview.sh
COPY gdbinit /root/.gdbinit

ENV PATH="/usr/local/appview:/usr/local/appview/bin:${PATH}"
COPY appview-profile.sh /etc/profile.d/appview.sh
COPY gdbinit /root/.gdbinit
RUN  mkdir /usr/local/appview && \
     mkdir /usr/local/appview/bin && \
     mkdir /usr/local/appview/lib && \
     ln -s /opt/appview/bin/linux/$(uname -m)/appview /usr/local/appview/bin/appview && \
     ln -s /opt/appview/lib/linux/$(uname -m)/libappview.so /usr/local/appview/lib/libappview.so

COPY sshd/appview-test /usr/local/appview/appview-test

COPY docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["test"]
