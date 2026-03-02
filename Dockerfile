# Based Image File
FROM ubuntu:22.04

# Prevent interactive message during the install process.
ENV DEBIAN_FRONTEND=noninteractive

# Update package list and install the core build tools.
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    gdb \
    ninja-build \
    valgrind \
    && rm -rf /var/lib/apt/lists/*

# Set up the work directory.
WORKDIR /app

# Default Start Up Command
CMD ["/bin/bash"]