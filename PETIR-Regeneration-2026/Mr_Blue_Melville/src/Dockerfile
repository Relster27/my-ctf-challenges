FROM debian:12-slim

RUN apt-get update && \
    apt-get install -y xinetd && \
    apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* /etc/xinetd.d/*
RUN useradd -U -m -s /bin/bash blue

COPY ./xinetd.conf /etc/xinetd.d/chall
RUN chmod 644 /etc/xinetd.d/chall

COPY ./chall /home/blue/chall
COPY ./run.sh /home/blue/run
COPY ./flag.txt /home/blue/flag.txt

RUN chown -R root:blue /home/blue && chmod -R 550 /home/blue

EXPOSE 3134

CMD ["xinetd", "-dontfork", "-stayalive"]