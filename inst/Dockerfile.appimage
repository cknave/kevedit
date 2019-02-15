FROM centos:6.10
ARG MAKE_OPTS=-j12
ARG SDL_VERSION

RUN yum groupinstall -y "Development Tools" && \
    yum install -y alsa-lib-devel dbus-devel fuse-devel libX11-devel \
                libXcursor-devel libXrandr-devel libXScrnSaver-devel \
                libXinerama-devel libXi-devel mesa-libEGL-devel \
                mesa-libGL-devel mkisofs pulseaudio-libs-devel sudo unzip

COPY vendor/SDL2-${SDL_VERSION}.tar.gz /tmp/
RUN cd /tmp && \
    tar xzf SDL2-${SDL_VERSION}.tar.gz && \
    cd SDL2-${SDL_VERSION} && \
    ./configure && \
    make ${MAKE_OPTS} && \
    make install && \
    rm -rf /tmp/SDL2-${SDL_VERSION}*
