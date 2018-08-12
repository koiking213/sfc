FROM ubuntu:18.04
RUN apt update
RUN apt install -y emacs25 llvm-6.0 git clang-6.0 tmux
RUN apt install -y cmake g++ glibc++ zlib1g-dev gdb

RUN update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-6.0 1

RUN mkdir source
VOLUME /source