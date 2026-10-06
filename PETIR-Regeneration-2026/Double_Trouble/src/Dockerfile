FROM debian:12-slim

RUN apt-get update && \
    apt-get install -y xinetd && \
    apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* /etc/xinetd.d/*
RUN useradd -U -m -s /bin/bash ctf

COPY ./xinetd.conf /etc/xinetd.d/chall
RUN chmod 644 /etc/xinetd.d/chall

COPY ./double_trouble /home/ctf/double_trouble
COPY ./run.sh /home/ctf/run
COPY ./flag.txt /home/ctf/flag.txt

RUN chown -R root:ctf /home/ctf && chmod -R 550 /home/ctf

EXPOSE 3134

CMD ["xinetd", "-dontfork", "-stayalive"]