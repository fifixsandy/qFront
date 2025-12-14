FROM fedora:latest

RUN dnf install -y cmake make git g++ gcc wget java-25-openjdk antlr4 antlr4-runtime antlr4-cpp-runtime antlr4-cpp-runtime-devel
COPY . /qfront
WORKDIR qfront
# RUN make runtime
RUN make antlr
RUN make
