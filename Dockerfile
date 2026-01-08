FROM ubuntu:22.04

# Install build tools and clangd
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    clangd-15 \
    && ln -s /usr/bin/clangd-15 /usr/bin/clangd \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

# Keep container running
CMD ["sleep", "infinity"]
