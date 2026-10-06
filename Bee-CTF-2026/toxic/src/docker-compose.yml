FROM ubuntu:latest
ENV DEBIAN_FRONTEND=noninteractive

# Install core dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    wget \
    unzip \
    python3 \
    python3-pip \
    python3-gmpy2 \
    qemu-system-x86 \
    qemu-utils \
    python3-dev \
    curl \
    vim \
    socat \
    nano \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

RUN useradd -U -m -s /bin/bash ctf

WORKDIR /app

COPY ./run.sh ./start-server.sh ./image.zip ./
COPY ./flag.txt ./
COPY pow.py requirements.txt ./

# setup linux image
RUN unzip ./image.zip && rm -rf ./image.zip

# pow
RUN python3 -m pip install --no-cache-dir -r requirements.txt --break-system-packages

# Run as ctf user
USER ctf

ENV PORT_LISTENER=4919
EXPOSE 4919
ENTRYPOINT ["bash", "/app/start-server.sh"]