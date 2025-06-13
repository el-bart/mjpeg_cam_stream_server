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
    libboost-program-options-dev

# BUT installation
RUN apt-get update && apt-get install -y \
    wget \
    nlohmann-json3-dev \
    catch2

ENV BUT_VER=2.1.0
RUN cd /usr/src/ && \
    wget "https://github.com/el-bart/but/archive/v${BUT_VER}.tar.gz" && \
    tar xzvf "v${BUT_VER}.tar.gz" && \
    cd "but-${BUT_VER}/" && \
    mkdir -p "build" && \
    cd "build" && \
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../src/cmake/toolchains/gcc.cmake .. && \
    ninja && \
    ninja install && \
    cd /usr/src/ && \
    rm "v${BUT_VER}.tar.gz" && \
    cd "but-${BUT_VER}"
