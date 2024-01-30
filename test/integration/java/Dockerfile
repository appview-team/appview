ARG JDK_IMAGE
FROM $JDK_IMAGE

ARG TOMCAT_MAJOR_VERSION=8
ARG TOMCAT_MINOR_VERSION=8.0.49
ARG KEYTOOL=keytool
ARG DNLD_HEXDUMP

ENV CATALINA_HOME /opt/tomcat
ENV PATH="${PATH}:${CATALINA_HOME}/bin"

ADD https://archive.apache.org/dist/tomcat/tomcat-${TOMCAT_MAJOR_VERSION}/v${TOMCAT_MINOR_VERSION}/bin/apache-tomcat-${TOMCAT_MINOR_VERSION}.tar.gz /apache-tomcat.tar.gz

# for hexdump.  java 11 and under seems to use apt-get, 12 and 13 uses yum, 14 uses microdnf
RUN eval $DNLD_HEXDUMP

RUN tar zxf apache-tomcat.tar.gz && \
    rm apache-tomcat.tar.gz && \
    mv apache-tomcat* $CATALINA_HOME

RUN mkdir /opt/tomcat/certs && \
    mkdir -p /opt/test-runner/logs && \
    mkdir -p /opt/test-runner/bin

RUN ${KEYTOOL} -genkey -alias "tomcat" -dname "CN=appview,O=cribl" -keyalg RSA -storetype pkcs12 -keystore /opt/tomcat/certs/tomcat.p12 -storepass changeit -keypass changeit

COPY ./java/server.xml $CATALINA_HOME/conf/server.xml
COPY ./java/test-ssl.sh /opt/test-runner/bin/test-ssl.sh
RUN chmod +x /opt/test-runner/bin/test-ssl.sh

RUN mkdir -p /opt/javassl
COPY ./java/SSLSocketClient.java /opt/javassl/SSLSocketClient.java
RUN javac -d /opt/javassl/ /opt/javassl/SSLSocketClient.java

RUN mkdir -p /opt/java_http
COPY ./java/SimpleHttpServer.java /opt/java_http/SimpleHttpServer.java
RUN javac -d /opt/java_http/ /opt/java_http/SimpleHttpServer.java

ENV APPVIEW_CRIBL_ENABLE=false
ENV APPVIEW_LOG_LEVEL=info
ENV APPVIEW_METRIC_VERBOSITY=4
ENV APPVIEW_EVENT_LOGFILE=false
ENV APPVIEW_EVENT_CONSOLE=false
ENV APPVIEW_EVENT_METRIC=false
ENV APPVIEW_EVENT_HTTP=true
ENV APPVIEW_SUMMARY_PERIOD=1
ENV APPVIEW_EVENT_DEST=file:///opt/test-runner/logs/events.log
ENV APPVIEW_METRIC_DEST=udp://localhost:8125
ENV APPVIEW_LOG_DEST=file:///opt/test-runner/logs/appview.log

ENV PATH="/usr/local/appview:/usr/local/appview/bin:${PATH}"
RUN echo "export PATH=${PATH}" >> /etc/profile.d/path.sh
COPY appview-profile.sh /etc/profile.d/appview.sh
COPY gdbinit /root/.gdbinit

RUN  mkdir /usr/local/appview && \
     mkdir /usr/local/appview/bin && \
     mkdir /usr/local/appview/lib && \
     ln -s /opt/appview/bin/linux/$(uname -m)/appview /usr/local/appview/bin/appview && \
     ln -s /opt/appview/lib/linux/$(uname -m)/libappview.so /usr/local/appview/lib/libappview.so

COPY java/appview-test /usr/local/appview/appview-test

COPY docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["test"]

