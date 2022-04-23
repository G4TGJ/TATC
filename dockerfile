FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
	avr-libc \
	gcc-avr \
	git \
	make \
	wget \
&& rm -rf /var/lib/apt/lists/*

RUN git clone https://github.com/G4TGJ/TARL.git

COPY TATC TATC

WORKDIR TATC

