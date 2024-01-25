FROM postgres:14

ENV DEBIAN_FRONTEND=noninteractive
RUN apt update \
    && apt install -y \
      binutils \
      findutils \
      less \
      psmisc \
      procps \
    && rm -rf /var/lib/apt/lists/*

RUN  mkdir /usr/local/appview && \
     mkdir /usr/local/appview/bin && \
     mkdir /usr/local/appview/lib && \
     mkdir -p /opt/test-runner/logs/ && \
     ln -s /opt/appview/bin/linux/$(uname -m)/appview /usr/local/appview/bin/appview && \
     ln -s /opt/appview/lib/linux/$(uname -m)/libappview.so /usr/local/appview/lib/libappview.so

COPY postgres/appview-test /usr/local/appview/appview-test
ENV PATH="/usr/local/appview:/usr/local/appview/bin/:/usr/lib/postgresql/14/bin:${PATH}"
RUN echo "export PATH=${PATH}" >> /etc/profile.d/path.sh

USER postgres

ENV POSTGRES_USER LoremIpsum
ENV POSTGRES_PASSWORD LoremIpsum

COPY docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["test"]
