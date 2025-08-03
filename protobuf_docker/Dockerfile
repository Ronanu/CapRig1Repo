FROM python:3.10-slim

RUN apt-get update && apt-get install -y \
    git \
    curl \
    unzip \
    build-essential \
    autoconf \
    libtool \
    cmake \
    && rm -rf /var/lib/apt/lists/*

RUN curl -OL https://github.com/protocolbuffers/protobuf/releases/download/v25.3/protoc-25.3-linux-x86_64.zip && \
    unzip protoc-25.3-linux-x86_64.zip -d /usr/local/ && \
    rm protoc-25.3-linux-x86_64.zip

RUN python3 -m pip install --upgrade pip


RUN pip install protobuf

RUN git clone https://github.com/nanopb/nanopb.git /nanopb && \
    cd /nanopb && \
    git checkout master && \
    cmake . && make

ENV PATH="/nanopb/generator:${PATH}"


COPY run_and_stay.sh /run_and_stay.sh
RUN chmod +x /run_and_stay.sh
CMD ["/run_and_stay.sh"]

WORKDIR /workspace

