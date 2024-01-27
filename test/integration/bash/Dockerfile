FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get -o Acquire::Check-Valid-Until=false update && \
    apt-get install -y vim git gawk build-essential gdb emacs lsof tcpdump wget curl autoconf automake libtool && \
    apt-get clean

RUN git clone https://git.savannah.gnu.org/git/bash.git && \
    mkdir /mybin/ /mybin/nomem /mybin/mem && \
    cd bash && \
    ./configure && make && cp ./bash /mybin/mem/bash && \
    ./configure --without-bash-malloc && make && cp ./bash /mybin/nomem/bash

COPY ./bash/runbashmem.sh /mybin
COPY ./bash/runbashnomem.sh /mybin
COPY ./bash/appview_bash.yml /opt

ENV APPVIEW_CRIBL_ENABLE=false

ENV PATH="/usr/local/appview:/usr/local/appview/bin:${PATH}"
COPY appview-profile.sh /etc/profile.d/appview.sh
COPY gdbinit /root/.gdbinit

RUN  mkdir /usr/local/appview && \
     mkdir /usr/local/appview/bin && \
     mkdir /usr/local/appview/lib && \
     ln -s /opt/appview/bin/linux/$(uname -m)/appview /usr/local/appview/bin/appview && \
     ln -s /opt/appview/lib/linux/$(uname -m)/libappview.so /usr/local/appview/lib/libappview.so

COPY ./bash/appview-test /usr/local/appview/

COPY docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["test"]
