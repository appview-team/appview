FROM cribl/cribl:3.4.1

RUN apt-get update && \
    apt-get install --no-install-recommends -y \
        net-tools \
        python3.8 \
        python3-setuptools \
        python3-pip \
    && apt-get autoremove && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

COPY logstream/http_status.csv /opt/cribl/data/lookups/http_status.csv
COPY logstream/scripts/ /opt/cribl/scripts/
COPY logstream/cribl/ /opt/cribl/local/cribl/

RUN mkdir -p /opt/test-runner/logs/

ENV APPVIEW_CRIBL_ENABLE=false
ENV APPVIEW_METRIC_DEST=udp://127.0.0.1:8125
ENV APPVIEW_LOG_LEVEL=info
ENV APPVIEW_LOG_DEST=file:///opt/test-runner/logs/appview.log
ENV APPVIEW_METRIC_VERBOSITY=4
ENV APPVIEW_EVENT_LOGFILE=true
ENV APPVIEW_EVENT_CONSOLE=true
ENV APPVIEW_EVENT_METRIC=true
ENV APPVIEW_EVENT_HTTP=true
#ENV APPVIEW_EVENT_DEST=tcp://172.16.198.132:9109

COPY ./test_runner/requirements.txt /opt/test-runner/requirements.txt
RUN pip3 install wheel
RUN pip3 install -r /opt/test-runner/requirements.txt

COPY ./test_runner/ /opt/test-runner/

# Switching to Python 3.8 required this hack. Not sure where the kafka packages are coming from.
RUN sed -i 's/\basync\b/is_async/g' /usr/local/lib/python3.8/dist-packages/kafka/producer/*.py

ENV PATH="/usr/local/appview:/usr/local/appview/bin:${PATH}"
COPY appview-profile.sh /etc/profile.d/appview.sh
COPY gdbinit /root/.gdbinit

RUN  mkdir /usr/local/appview && \
     mkdir /usr/local/appview/bin && \
     mkdir /usr/local/appview/lib && \
     ln -s /opt/appview/bin/linux/$(uname -m)/appview /usr/local/appview/bin/appview && \
     ln -s /opt/appview/lib/linux/$(uname -m)/libappview.so /usr/local/appview/lib/libappview.so

COPY logstream/appview-test /usr/local/appview/appview-test

COPY docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["test"]

