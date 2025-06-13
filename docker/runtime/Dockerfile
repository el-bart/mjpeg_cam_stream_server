FROM debian:12
RUN apt-get update && apt-get dist-upgrade -y

RUN apt-get update && apt-get install -y \
    libboost-program-options1.74.0

COPY mjpeg_cam_stream_server /usr/local/bin/
ENTRYPOINT ["mjpeg_cam_stream_server"]
