FROM docker.elastic.co/elasticsearch/elasticsearch:8.0.0
ENV discovery.type=single-node
ENV xpack.security.enabled=false
USER root

RUN apt update && \
    apt install python3-pip -y
RUN pip3 install virtualenv

RUN chown elasticsearch:elasticsearch /opt/ /usr/local

USER elasticsearch

RUN mkdir /opt/test-runner/ /opt/test-runner/logs/

RUN virtualenv -p $(which python3) /opt/test-runner/

ENV APPVIEW_CRIBL_ENABLE=false
ENV APPVIEW_OUT_DEST=udp://localhost:8125
ENV APPVIEW_LOG_LEVEL=info
ENV APPVIEW_LOG_DEST=file:///opt/test-runner/logs/appview.log
ENV APPVIEW_OUT_VERBOSITY=4
ENV APPVIEW_EVENT_LOGFILE=true
ENV APPVIEW_EVENT_CONSOLE=true
ENV APPVIEW_EVENT_METRIC=true
ENV APPVIEW_EVENT_HTTP=true

COPY ./test_runner/requirements.txt /opt/test-runner/requirements.txt
RUN /opt/test-runner/bin/pip install -r /opt/test-runner/requirements.txt

COPY ./test_runner/ /opt/test-runner/

# Switching to Python 3.8 required this hack. Not sure where the kafka packages are coming from.
RUN sed -i 's/\basync\b/is_async/g' /opt/test-runner/lib/python3.8/site-packages/kafka/producer/*.py

ENV PATH="/usr/local/appview:/usr/local/appview/bin:${PATH}"
COPY appview-profile.sh /etc/profile.d/appview.sh
COPY gdbinit /root/.gdbinit

RUN  mkdir /usr/local/appview && \
     mkdir /usr/local/appview/bin && \
     mkdir /usr/local/appview/lib && \
     ln -s /opt/appview/bin/linux/$(uname -m)/appview /usr/local/appview/bin/appview && \
     ln -s /opt/appview/lib/linux/$(uname -m)/libappview.so /usr/local/appview/lib/libappview.so

COPY elastic/appview-test /usr/local/appview/appview-test

COPY docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["test"]

