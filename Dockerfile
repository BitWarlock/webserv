FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y lsof \
    valgrind \
    build-essential \
    git \
    vim \
    wrk \
    siege \
    openssh-server \
    curl \
    zlib1g-dev \
    nodejs \
    php \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /var/run/sshd && \
    echo "root:password" | chpasswd && \
    sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config

RUN mkdir -p /project

WORKDIR /project

EXPOSE 22

CMD ["/usr/sbin/sshd", "-D"]
