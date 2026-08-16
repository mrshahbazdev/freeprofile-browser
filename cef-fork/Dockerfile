# Build environment for FreeProfile CEF/Chromium fork.
# WARNING: the full Chromium/CEF build needs >200 GB disk and >32 GB RAM.
# Run on a dedicated build machine or a large cloud instance.
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
ENV WORK_DIR=/cef-build
ENV CEF_BRANCH=7922

RUN apt-get update && apt-get install -y \
    git git-svn python3 python3-pip curl wget xz-utils lbzip2 \
    g++ build-essential pkg-config ccache \
    libnss3-dev libxss1 libdbus-1-dev libatk1.0-dev libatk-bridge2.0-dev \
    libgtk-3-dev libasound2-dev libxcomposite-dev libxcursor-dev \
    libxdamage-dev libxi-dev libxrandr-dev libxtst-dev libxkbcommon-dev \
    libgbm-dev libdrm-dev libpulse-dev libffi-dev libjpeg-dev libpng-dev \
    libre2-dev libvpx-dev libevent-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /freeprofile-cef-fork
COPY . /freeprofile-cef-fork

RUN bash scripts/bootstrap.sh

# Default command fetches source only.  Run `bash scripts/build.sh` afterwards
# on a machine with enough resources.
CMD ["bash", "scripts/fetch-source.sh"]
