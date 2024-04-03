FROM quay.io/centos/centos:stream9

RUN yum -y update && \
    yum -y groupinstall "Development Tools" && \
    yum -y install python pip && \
    yum -y install epel-release && \
    yum -y install java-1.8.0-openjdk && \
    yum -y install mc net-tools lsof vim emacs gdb procps

RUN mkdir -p /opt/test-runner/logs/

# Originally used 2.1.1 and 2.11, but they're too old now.
ENV KAFKA_VERSION=3.7.0 KAFKA_SCALA_VERSION=2.13
ENV KAFKA_RELEASE_ARCHIVE kafka_${KAFKA_SCALA_VERSION}-${KAFKA_VERSION}.tgz

RUN mkdir /kafka /data /logs

# Download Kafka binary distribution
#COPY ./kafka/kafka_2.13-2.5.0.tgz /tmp/kafka_2.13-2.5.0.tgz

ADD https://archive.apache.org/dist/kafka/${KAFKA_VERSION}/${KAFKA_RELEASE_ARCHIVE} /tmp/
ADD https://archive.apache.org/dist/kafka/${KAFKA_VERSION}/${KAFKA_RELEASE_ARCHIVE}.md5 /tmp/

WORKDIR /tmp

# Check artifact digest integrity
RUN echo VERIFY CHECKSUM: && \
  gpg --print-md MD5 ${KAFKA_RELEASE_ARCHIVE} 2>/dev/null && \
  cat ${KAFKA_RELEASE_ARCHIVE}.md5

# Install Kafka to /kafka
RUN tar -zx -C /kafka --strip-components=1 -f ${KAFKA_RELEASE_ARCHIVE} && \
  rm -rf kafka_*

ADD ./kafka/config /kafka/config

# Set up a user to run Kafka
RUN groupadd kafka && \
  useradd -d /kafka -g kafka -s /bin/false kafka && \
  chown -R kafka:kafka /kafka /data /logs

#ENV APPVIEW_OUT_DEST=udp://localhost:8125
#ENV APPVIEW_OUT_VERBOSITY=4
#ENV APPVIEW_EVENT_DEST=tcp://172.16.198.132:9109
#ENV APPVIEW_METRIC_DEST=udp://localhost:8125
ENV APPVIEW_CRIBL_ENABLE=false
ENV APPVIEW_LOG_LEVEL=info
ENV APPVIEW_LOG_DEST=file:///opt/test-runner/logs/appview.log
ENV APPVIEW_METRIC_VERBOSITY=4
ENV APPVIEW_EVENT_LOGFILE=true
ENV APPVIEW_EVENT_CONSOLE=true
ENV APPVIEW_EVENT_METRIC=true
ENV APPVIEW_EVENT_HTTP=true

COPY ./test_runner/requirements.txt /opt/test-runner/requirements.txt
RUN pip install -r /opt/test-runner/requirements.txt
RUN pip install kafka-python

COPY ./test_runner/ /opt/test-runner/

# Switching to Python 3.8 required this hack. Not sure where the kafka packages are coming from.
RUN sed -i 's/\basync\b/is_async/g' /usr/local/lib//python3.9/site-packages/kafka/producer/*.py

ENV PATH /kafka/bin:$PATH
WORKDIR /kafka

# broker, jmx
VOLUME [ "/data", "/logs" ]

ENV PATH="/usr/local/appview:/usr/local/appview/bin:${PATH}"
COPY appview-profile.sh /etc/profile.d/appview.sh
COPY gdbinit /root/.gdbinit

RUN  mkdir /usr/local/appview && \
     mkdir /usr/local/appview/bin && \
     mkdir /usr/local/appview/lib && \
     ln -s /opt/appview/bin/linux/$(uname -m)/appview /usr/local/appview/bin/appview && \
     ln -s /opt/appview/lib/linux/$(uname -m)/libappview.so /usr/local/appview/lib/libappview.so

COPY kafka/appview-test /usr/local/appview/appview-test

COPY docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["test"]
