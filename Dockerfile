FROM gcc AS builder
ARG DEBIAN_FRONTEND=noninteractive
RUN apt update \
    && apt install -y cmake
COPY . /workdir
WORKDIR /workdir 
RUN mkdir -p build \
    && cd build \
    && cmake -S .. -B . \
    && cmake --build .

FROM ubuntu:22.04
COPY --from=builder /workdir/build/server /usr/bin
ENTRYPOINT [ "/usr/bin/server" ]