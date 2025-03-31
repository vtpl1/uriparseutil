FROM ubuntu:20.04

RUN apt update \
    && DEBIAN_FRONTEND=noninteractive apt install -y \
    ca-certificates \
    software-properties-common \
    build-essential \
    wget curl git gdb ninja-build \
    lsb-release \
    pkg-config zip unzip tar ccache nasm autoconf-archive autotools-dev automake \
    cmake \
    python3-tk python3-pip python3-setuptools \
    python3-dev python3-venv python3-jinja2 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*


ENV SHELL=/bin/bash

ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=$USER_UID

# Create the user
RUN groupadd --gid $USER_GID $USERNAME && \
    useradd --uid $USER_UID --gid $USER_GID -m $USERNAME

RUN groupmod --gid $USER_GID $USERNAME && \
    usermod --uid $USER_UID --gid $USER_GID $USERNAME && \
    chown -R $USER_UID:$USER_GID /home/$USERNAME

USER $USERNAME
RUN python3 -m pip install --upgrade pip
RUN python3 -m pip install --no-cache-dir loguru meson jinja2 bump2version