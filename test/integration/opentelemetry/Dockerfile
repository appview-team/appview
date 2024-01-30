FROM alpine:latest

RUN apk add --no-cache \
    autoconf \
    automake \
    bash \
    binutils \
    c-ares-dev \
    cgdb  \
    cmake \
    curl \
    gcc \
    g++ \
    git \
    grpc-dev \
    libtool \
    linux-headers \
    make \
    musl-dbg \
    protobuf-dev \
    re2-dev

# Comment install protobuf-dev grpc-dev to debug
# RUN mkdir -p /opt/protobuf/ && \
#     cd /opt/protobuf/ && \
#     git clone https://github.com/protocolbuffers/protobuf.git --recurse-submodules -b v3.21.12 . && \
#     ./autogen.sh && \
#     ./configure --prefix=/usr CFLAGS="-fPIC -g3 -O0"  CXXFLAGS="-fPIC -g3 -O0" && \
#     make -j$(nproc) && \
#     make install

RUN mkdir -p /opt/telemetry-demo && \
    cd /opt/telemetry-demo && \
    git clone https://github.com/open-telemetry/opentelemetry-demo.git .


# RUN git clone --recurse-submodules -b v1.52.2 --depth 1 --shallow-submodules https://github.com/grpc/grpc \
#     && cd grpc \
#     && mkdir -p cmake/build \
#     && cd cmake/build \
#     && cmake -DgRPC_INSTALL=ON \
#       -DgRPC_BUILD_TESTS=OFF \
#       -DCMAKE_BUILD_TYPE=Debug \
#       ../.. \ 
#     &&  make -j$(nproc) \
#     &&  make install \
#     &&  cd ../../..
ARG OPENTELEMETRY_CPP_VERSION=1.9.0

RUN mkdir -p /opt/opentelemetry-cpp \
    && cd /opt/opentelemetry-cpp/ \
    && git clone https://github.com/open-telemetry/opentelemetry-cpp . \
    && git checkout tags/v${OPENTELEMETRY_CPP_VERSION} -b v${OPENTELEMETRY_CPP_VERSION} \
    && mkdir build \
    && cd build \
    && cmake .. -DCMAKE_CXX_STANDARD=17 -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
              -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=OFF \
              -DWITH_EXAMPLES=OFF -DWITH_OTLP=ON -DWITH_OTLP_GRPC=ON \
    && make -j$(nproc) install && cd ../..

RUN cd /opt/telemetry-demo/src/currencyservice/ \
    && mkdir -p build && cd build \
    && cmake -DCMAKE_BUILD_TYPE=Release .. \
    && make -j$(nproc) install

ENV APPVIEW_CRIBL_ENABLE=false

ENV PATH="/usr/local/appview:/usr/local/appview/bin:${PATH}"
RUN echo "export PATH=/usr/local/appview:/usr/local/appview/bin:${PATH}" >/etc/profile.d/path.sh

COPY appview-profile.sh /etc/profile.d/appview.sh
COPY gdbinit /root/.gdbinit

RUN  mkdir /usr/local/appview && \
     mkdir /usr/local/appview/bin && \
     mkdir /usr/local/appview/lib && \
     mkdir -p /opt/test-runner/logs/ && \
     ln -s /opt/appview/bin/linux/$(uname -m)/appview /usr/local/appview/bin/appview

COPY opentelemetry/appview-test /usr/local/appview/appview-test

COPY docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["test"]
