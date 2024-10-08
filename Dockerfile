FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    sudo build-essential cmake cmake-curses-gui git

RUN apt-get update && apt-get install -y \
    curl zip unzip tar xorg-dev libgl1-mesa-dev libglu1-mesa-dev libtool

# Install necessary packages
RUN apt-get update && apt-get install -y \
    software-properties-common \
    && add-apt-repository ppa:ubuntu-toolchain-r/test \
    && apt-get update \
    && apt-get install -y gcc-13 g++-13 \
    && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 60 --slave /usr/bin/g++ g++ /usr/bin/g++-13

# Verify GCC version
RUN gcc --version

ARG USER_ID 
ARG GROUP_ID
ARG USERNAME

RUN groupadd -g ${GROUP_ID} ${USERNAME} && \
    useradd -m -u ${USER_ID} -g ${GROUP_ID} -s /bin/bash ${USERNAME} && \
    echo "${USERNAME} ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

USER ${USERNAME}

WORKDIR /home/${USERNAME}/megamol

CMD mkdir build || true && cd build && \ 
    cmake -DCMAKE_CXX_COMPILER=/usr/bin/g++-13 -DCMAKE_C_COMPILER=/usr/bin/gcc-13 -DVCPKG_APPLOCAL_DEPS=OFF \
          -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_MAKE_PROGRAM=/usr/bin/make -DMEGAMOL_VCPKG_DOWNLOAD_CACHE=ON .. && \
    make -j 16 && make -j 16 install

