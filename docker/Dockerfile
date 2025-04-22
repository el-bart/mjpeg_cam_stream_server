FROM debian:12
RUN apt-get update && apt-get dist-upgrade -y

RUN apt-get update && apt-get install -y \
    cmake \
    make \
    ninja-build \
    g++

RUN apt-get update && apt-get install -y \
    gdb \
    vim

RUN apt-get update && apt-get install -y \
    libopencv-dev

RUN apt-get update && apt-get install -y \
    libboost-program-options-dev
