FROM quay.io/centos/centos:stream8

COPY detect_proto/mongodb.repo /etc/yum.repos.d/.
RUN yum -y update && \
    yum -y install epel-release && \
    yum -y install redis wget gdb openssl net-tools
RUN [ "aarch64" = "$(uname -m)" ] || yum -y install mongodb-org

RUN mkdir /opt/test-runner/
RUN mkdir /opt/test-runner/logs/
RUN mkdir /opt/test-runner/bin

COPY detect_proto/mongod.conf /etc/.
COPY detect_proto/appview.yml /opt/test-runner/bin/.
COPY detect_proto/mongo.js /opt/test-runner/bin/.

COPY detect_proto/test_protocols.sh /opt/test-runner/bin/test_protocols.sh
RUN chmod +x /opt/test-runner/bin/test_protocols.sh

ADD detect_proto/loaddata.sh /sbin/loaddata.sh
ADD detect_proto/session.csv /data/session.csv

RUN /usr/lib/systemd/systemd --system &
CMD ["start"]

ENV APPVIEW_CRIBL_ENABLE=false
ENV APPVIEW_LOG_LEVEL=info
ENV APPVIEW_LOG_DEST=file:///tmp/appview.log
ENV APPVIEW_TAG_service=redis
#ENV APPVIEW_EVENT_ENABLE=true
ENV APPVIEW_EVENT_LOGFILE=false
ENV APPVIEW_EVENT_CONSOLE=false
#ENV APPVIEW_EVENT_METRIC=true
ENV APPVIEW_EVENT_HTTP=false
ENV APPVIEW_EVENT_DEST=file:///opt/test-runner/logs/events.log
#ENV APPVIEW_METRIC_ENABLE=false
#ENV APPVIEW_METRIC_VERBOSITY=4
#ENV APPVIEW_METRIC_DEST=udp://localhost:8125
ENV APPVIEW_LOG_DEST=file:///opt/test-runner/logs/appview.log
ENV APPVIEW_HOME=/opt/test-runner/bin

ENV PATH="/usr/local/appview:/usr/local/appview/bin:${PATH}"
COPY appview-profile.sh /etc/profile.d/appview.sh
COPY gdbinit /root/.gdbinit

RUN  mkdir /usr/local/appview && \
     mkdir /usr/local/appview/bin && \
     mkdir /usr/local/appview/lib && \
     ln -s /opt/appview/bin/linux/$(uname -m)/appview /usr/local/appview/bin/appview && \
     ln -s /opt/appview/lib/linux/$(uname -m)/libappview.so /usr/local/appview/lib/libappview.so

COPY detect_proto/appview-test /usr/local/appview/appview-test

COPY docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["test"]

