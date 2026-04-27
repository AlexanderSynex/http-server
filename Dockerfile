FROM gcc AS builder
ARG DEBIAN_FRONTEND=noninteractive
RUN apt update \
    && apt install -y cmake
COPY . /workdir
WORKDIR /workdir 
RUN mkdir -p build \
    && cd build \
    && cmake -S .. -B . -DCMAKE_BUILD_TYPE=Release \
    && cmake --build .

FROM busybox:latest
COPY --from=builder /workdir/build/server /usr/bin
ENTRYPOINT [ "/usr/bin/server" ]