#!/bin/bash
# System dependencies for building CEF/Chromium on Ubuntu/Debian.
set -e

sudo apt-get update
sudo apt-get install -y \
  git \
  git-svn \
  python3 \
  python3-pip \
  curl \
  wget \
  xz-utils \
  lbzip2 \
  g++ \
  build-essential \
  pkg-config \
  libnss3-dev \
  libxss1 \
  libdbus-1-dev \
  libatk1.0-dev \
  libatk-bridge2.0-dev \
  libgtk-3-dev \
  libasound2-dev \
  libxcomposite-dev \
  libxcursor-dev \
  libxdamage-dev \
  libxi-dev \
  libxrandr-dev \
  libxtst-dev \
  libxkbcommon-dev \
  libgbm-dev \
  libdrm-dev \
  libpulse-dev \
  libffi-dev \
  libjpeg-dev \
  libpng-dev \
  libre2-dev \
  libvpx-dev \
  libevent-dev \
  ccache

# CEF build needs at least Python 3.11 if building very recent branches; otherwise 3.8+ is okay.
python3 --version
