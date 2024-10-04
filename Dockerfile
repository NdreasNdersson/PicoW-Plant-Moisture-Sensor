FROM python:3.11-slim

RUN apt-get update && \
    apt-get install -y cmake ninja-build curl xz-utils build-essential git cppcheck clang clang-tidy clang-tools

# install arm toolchain
ARG ARM_TOOLCHAIN_VERSION=12.3.Rel1
RUN curl -Lo gcc-arm-none-eabi.tar.xz --create-dirs --output-dir /temp "https://developer.arm.com/-/media/Files/downloads/gnu/${ARM_TOOLCHAIN_VERSION}/binrel/arm-gnu-toolchain-${ARM_TOOLCHAIN_VERSION}-x86_64-arm-none-eabi.tar.xz"
RUN mkdir -p /opt/gcc-arm-none-eabi
RUN tar xf /temp/gcc-arm-none-eabi.tar.xz --strip-components=1 -C /opt/gcc-arm-none-eabi
ENV PATH="/opt/gcc-arm-none-eabi/bin:${PATH}"

RUN git clone --depth 1 --branch V11.1.0 https://github.com/FreeRTOS/FreeRTOS-Kernel.git /repos/FreeRTOS-Kernel
RUN cd /repos/FreeRTOS-Kernel; git submodule update --init --recursive
ENV FREERTOS_KERNEL_PATH="/repos/FreeRTOS-Kernel"

RUN git clone --depth 1 --branch 2.0.0 https://github.com/raspberrypi/pico-sdk.git /repos/pico-sdk
RUN cd /repos/pico-sdk; git submodule update --init --recursive
ENV PICO_SDK_PATH="/repos/pico-sdk"

# test arm-none-gcc
RUN arm-none-eabi-gcc --version
RUN arm-none-eabi-g++ --version

ADD requirements.txt /temp/requirements.txt
RUN pip install -r /temp/requirements.txt

RUN rm -r /temp

RUN apt-get update && \
    apt-get install -y gcc g++ gcc-multilib g++-multilib gdb lcov

ENTRYPOINT ["/bin/bash"]
