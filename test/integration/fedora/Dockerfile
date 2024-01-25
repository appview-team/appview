FROM fedora:39

RUN dnf update -y && dnf install -y \
  gcc \
  ncurses-devel \
	&& dnf clean all

ENV PATH="/usr/local/appview:/usr/local/appview/bin:${PATH}"
COPY appview-profile.sh /etc/profile.d/appview.sh
COPY gdbinit /root/.gdbinit

RUN  mkdir /opt/test
COPY ./fedora/nsterm.c /opt/test/nsterm.c
RUN gcc -g -o /opt/test/nsterm /opt/test/nsterm.c -lncurses

RUN  mkdir /usr/local/appview && \
     mkdir /usr/local/appview/bin && \
     mkdir /usr/local/appview/lib && \
     mkdir -p /opt/test-runner/logs/ && \
     ln -s /opt/appview/bin/linux/$(uname -m)/appview /usr/local/appview/bin/appview && \
     ln -s /opt/appview/lib/linux/$(uname -m)/libappview.so /usr/local/appview/lib/libappview.so

# This is for GitHub runner
ENV TERM=xterm

COPY fedora/appview-test /usr/local/appview/appview-test

COPY docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["test"]
