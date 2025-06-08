# MJPEG camera stream server

just another Motion-JPEG streaming server.
it's written in C++ and aim at supporting many clients effortlessly.
the main CPU usage is due to camera frame capture and barely moves with more clients connecting.

the main purpose was to provide an easy to integrate stream service for LAN networks, with virtual no latency.
most of the tested solutions were introducing at least a couple of seconds, which was a no-go for my use case.


## building

TL;DR:
```
./compile
```

if you want to do some more work manually, run SDK interactively with:
```
./sdk
```
and then build software:
```
mkdir build
cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja
```
when completed, the main binary is here:
```
build/release/mjpeg_cam_stream_server
```
run it with `--help` to check the usage options.

additionally `mjpeg_cam_stream_server` docker image is created to use at runtime.


## running

TL;DR after:
```
./compile
```
is used to create binary and runtime image, run server with:
```
./run
```
optional arguments to script will be passed to `mjpeg_cam_stream_server` binary directly.


## usage

by default it listens on `0.0.0.0:8080`.
for local camera, start service and point browser to [http://localhost:8080](http://localhost:8080) to get the live feed.

command line options allow to set:
* device file
* resolution
* port
run server with `--help` to check out the details.


## security

there's no built-in authn/authz or encryption mechanisms.
it is assumed to be behind a reverse-proxy like [nginx](https://nginx.org/) to provide security features.


## logs

application produces structured logs.
example output is:
```
{"But::LogLevel":"info","But::Pid":17,"But::PreciseDT":"2025-06-08T13:48:28.547702587Z","But::ThreadId":"139932860690240","Camera_config":{"capture":{"Resolution":{"x":1920,"y":1080}},"device":"/dev/video0"},"message":"cemera initialized"}
```
or in expanded form:
```
{
  "But::LogLevel": "info",
  "But::Pid": 17,
  "But::PreciseDT": "2025-06-08T13:48:28.547702587Z",
  "But::ThreadId": "139932860690240",
  "Camera_config": {
    "capture": {
      "Resolution": {
        "x": 1920,
        "y": 1080
      }
    },
    "device": "/dev/video0"
  },
  "message": "cemera initialized"
}
```

see [LogATE](https://github.com/el-bart/LogATE) for a nice structured log processing tool.
