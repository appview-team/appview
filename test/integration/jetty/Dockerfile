FROM jetty:jdk17

USER root

ENV APPVIEW_CRIBL_ENABLE=false
ENV APPVIEW_LOG_LEVEL=debug
ENV APPVIEW_METRIC_VERBOSITY=4
ENV APPVIEW_EVENT_LOGFILE=true
ENV APPVIEW_EVENT_CONSOLE=true
ENV APPVIEW_EVENT_METRIC=true
ENV APPVIEW_EVENT_HTTP=true
ENV APPVIEW_EVENT_NET=true
ENV APPVIEW_EVENT_FS=true
ENV APPVIEW_LOG_DEST=file:///opt/test-runner/logs/appview.log
ENV APPVIEW_EVENT_DEST=file:///opt/test-runner/logs/events.log
ENV APPVIEW_METRIC_DEST=file:///opt/test-runner/logs/metrics.log
ENV PATH="/usr/local/appview:/usr/local/appview/bin:${PATH}"
COPY appview-profile.sh /etc/profile.d/appview.sh
COPY gdbinit /root/.gdbinit
RUN  mkdir /opt/test/
RUN  mkdir /usr/local/appview && \
     mkdir /usr/local/appview/bin && \
     mkdir /usr/local/appview/lib && \
     mkdir -p /opt/test-runner/logs/ && \
     ln -s /opt/appview/bin/linux/$(uname -m)/appview /usr/local/appview/bin/appview && \
     ln -s /opt/appview/lib/linux/$(uname -m)/libappview.so /usr/local/appview/lib/libappview.so

COPY jetty/appview-test /usr/local/appview/appview-test

COPY docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["test"]
