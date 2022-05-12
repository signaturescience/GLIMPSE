FROM ubuntu:20.04
ADD static_bins/ /usr/local/bin/
RUN mkdir /GLIMPSE && ln -sr /usr/local/bin/* /GLIMPSE
WORKDIR /data