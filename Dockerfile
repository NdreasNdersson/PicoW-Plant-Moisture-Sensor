FROM python:3.12-slim

RUN apt-get update && \
    apt-get install -y cmake ninja-build curl xz-utils build-essential

# install arm toolchain
ARG ARM_TOOLCHAIN_VERSION=12.3.Rel1
RUN curl -Lo gcc-arm-none-eabi.tar.xz "https://developer.arm.com/-/media/Files/downloads/gnu/${ARM_TOOLCHAIN_VERSION}/binrel/arm-gnu-toolchain-${ARM_TOOLCHAIN_VERSION}-x86_64-arm-none-eabi.tar.xz"
RUN mkdir -p /opt/gcc-arm-none-eabi
RUN tar xf gcc-arm-none-eabi.tar.xz --strip-components=1 -C /opt/gcc-arm-none-eabi
ENV PATH="/opt/gcc-arm-none-eabi/bin:${PATH}"

# test arm-none-gcc
RUN arm-none-eabi-gcc --version
RUN arm-none-eabi-g++ --version

ENTRYPOINT ["/bin/bash"]
