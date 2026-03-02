FROM fedora:latest

RUN dnf install -y cmake make git g++ gcc wget java-25-openjdk antlr4 antlr4-runtime antlr4-cpp-runtime antlr4-cpp-runtime-devel gmp-devel catch2-devel

RUN git clone https://github.com/VeriFIT/algebraic-complex-numbers.git /algebraic-complex-numbers
WORKDIR /algebraic-complex-numbers/build
RUN cmake -DCMAKE_BUILD_TYPE=Debug ../src
RUN make -j4

COPY . /qfront
WORKDIR /qfront
RUN cp /algebraic-complex-numbers/build/libalgebraic_complex_numbers.a ./
# RUN make runtime
RUN make antlr
RUN make -j4
