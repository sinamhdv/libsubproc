FROM ubuntu:latest

RUN apt update -y && apt upgrade -y && apt -y install gcc make python3 git

USER ubuntu
WORKDIR /home/ubuntu
RUN git clone https://github.com/sinamhdv/libsubproc

WORKDIR /home/ubuntu/libsubproc
RUN make
USER root
RUN make install

USER ubuntu
WORKDIR /home/ubuntu
