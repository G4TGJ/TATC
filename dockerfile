FROM ubuntu:20.04

RUN apt-get update && apt-get install -y \
	git \
	make \
	wget \
&& rm -rf /var/lib/apt/lists/*

RUN wget https://blog.zakkemble.net/download/avr-gcc-10.1.0-x64-linux.tar.bz2
RUN tar jxf avr-gcc-10.1.0-x64-linux.tar.bz2

RUN git clone https://github.com/G4TGJ/TARL.git

ENV PATH=$PATH:/avr-gcc-10.1.0-x64-linux/bin

COPY TATC TATC

WORKDIR TATC

