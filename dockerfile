FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
	bzip2 \
	git \
	make \
	wget \
&& rm -rf /var/lib/apt/lists/*

RUN wget https://github.com/ZakKemble/avr-gcc-build/releases/download/v12.1.0-1/avr-gcc-12.1.0-x64-linux.tar.bz2
RUN tar jxf avr-gcc-12.1.0-x64-linux.tar.bz2

RUN git clone https://github.com/G4TGJ/TARL.git

ENV PATH=$PATH:/avr-gcc-12.1.0-x64-linux/bin

COPY TATC TATC

WORKDIR TATC

