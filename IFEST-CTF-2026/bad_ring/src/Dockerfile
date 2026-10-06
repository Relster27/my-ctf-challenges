FROM ubuntu:latest
ENV DEBIAN_FRONTEND=noninteractive

# Install core dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    wget \
    unzip \
    qemu-system-x86 \
    qemu-utils \
    curl \
    vim \
    socat \
    nano \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

RUN useradd -U -m -s /bin/bash ctf

WORKDIR /app

COPY ./run.sh ./start-server.sh ./image.zip ./
COPY ./flag ./

# setup linux image
RUN unzip ./image.zip && rm -rf ./image.zip

# Run as ctf user
USER ctf

ENV PORT_LISTENER=4919
EXPOSE 4919
ENTRYPOINT ["bash", "/app/start-server.sh"]
