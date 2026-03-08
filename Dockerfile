FROM ubuntu:22.04

# Prevent interactive message during the install process.
ENV DEBIAN_FRONTEND=noninteractive

# Update package list and install the build tools.
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    ca-certificates \
    curl \
    git \
    gdb \
    ninja-build \
    valgrind \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
CMD ["/bin/bash"]