ARG UBUNTU_IMAGE=public.ecr.aws/ubuntu/ubuntu:24.04@sha256:748740465d0aadaa69ab6e6c295892f17d7a8f44a85090dbb571ec0bb8c5674f
ARG BUILD_STAGE_PLATFORM=linux/amd64

FROM --platform=${BUILD_STAGE_PLATFORM} ${UBUNTU_IMAGE} AS build

ARG DEBIAN_FRONTEND=noninteractive
ARG ANDROID_NDK_URL=https://dl.google.com/android/repository/android-ndk-r23b-linux.zip
ARG ANDROID_NDK_SHA1=f47ec4c4badd11e9f593a8450180884a927c330d
ARG ANDROID_NDK_SHA256=c6e97f9c8cfe5b7be0a9e6c15af8e7a179475b7ded23e2d1c1fa0945d6fb4382
WORKDIR /src

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    cmake \
    gcc-aarch64-linux-gnu \
    libc6-dev-arm64-cross \
    ninja-build \
    python3 \
    unzip \
    wget \
 && rm -rf /var/lib/apt/lists/*

RUN wget -q -O android-ndk-r23b-linux.zip "${ANDROID_NDK_URL}" \
 && echo "${ANDROID_NDK_SHA1}  android-ndk-r23b-linux.zip" | sha1sum -c - \
 && echo "${ANDROID_NDK_SHA256}  android-ndk-r23b-linux.zip" | sha256sum -c - \
 && unzip -q android-ndk-r23b-linux.zip \
 && rm android-ndk-r23b-linux.zip

COPY CMakeLists.txt ./
COPY VERSION ./
COPY symbol.h bridge.cpp parser.c parser.h logger.c logger.h \
    server.c layout.h ./
COPY scripts ./scripts
COPY launch.c ./
COPY rootfs ./rootfs

RUN python3 scripts/build_tzdata.py /src/rootfs/system/usr/share/zoneinfo/tzdata

RUN cmake -S . -B build -G Ninja \
 && cmake --build build -j"$(nproc)" \
 && python3 scripts/patch_elf.py /src/rootfs/system/bin/wrapper-core /src/rootfs/system/lib64/*.so \
 && test -x /src/build/wrapper \
 && test -x /src/rootfs/system/bin/wrapper-core

FROM ${UBUNTU_IMAGE}

ARG DEBIAN_FRONTEND=noninteractive
WORKDIR /app

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    libstdc++6 \
 && rm -rf /var/lib/apt/lists/*

COPY --from=build /src/build/wrapper /app/wrapper
COPY --from=build /src/rootfs /app/rootfs

RUN mkdir -p /app/rootfs/data/data/com.apple.android.music/files

EXPOSE 10020 20020 30020

ENTRYPOINT ["./wrapper"]
CMD ["--host", "0.0.0.0", "--read-2fa-file"]
