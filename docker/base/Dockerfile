FROM ubuntu:20.04

RUN apt update -y && \
    apt install -y bash-completion ca-certificates && \
    apt upgrade -y libsystemd0 libudev1 && \
    apt clean && \
    rm -rf /var/lib/apt/lists/*

ARG TARGETARCH
COPY bin/linux/${TARGETARCH}/appview       /usr/local/bin/appview
COPY lib/linux/${TARGETARCH}/libappview.so /usr/local/lib/libappview.so

RUN mkdir /usr/local/appview /usr/local/appview/x86_64 \
 && ln -s x86_64 /usr/local/appview/amd64
   
COPY bin/linux/x86_64/* /usr/local/appview/x86_64/
COPY conf/appview.yml     /usr/local/appview/x86_64/appview.yml

RUN rm -f /usr/local/appview/*/.gitignore
RUN echo "source /etc/profile.d/bash_completion.sh" >> ~/.bashrc && \
    echo "source <(/usr/local/bin/appview completion bash)" >> ~/.bashrc
