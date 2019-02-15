FROM alpine:3.9
ARG BUILD_DJGPP_VERSION=2.9
ARG GCC_VERSION=7.2.0
ARG DJGPP_MIRROR=http://na.mirror.garr.it/mirrors
ENV ENABLE_LANGUAGES=c

RUN apk add --no-cache autoconf automake bash bison build-base curl flex \
    libc6-compat make patch sdl2-dev tar texinfo xz zip zlib-dev

COPY vendor/build-djgpp-${BUILD_DJGPP_VERSION}.tar.gz /tmp/

# Okay, this sed command:
# 1) Replace the mirror with a fast one.
# 2) Update binutils version to 2.31.1
# 3) Update gcc version to 7.4.0
#
# These versions are not supported by the build script yet, but the old
# versions are gone from the fast mirror.
RUN cd /tmp && \
    tar xzf build-djgpp-${BUILD_DJGPP_VERSION}.tar.gz && \
    cd build-djgpp-${BUILD_DJGPP_VERSION} && \
    sed -i -e 's/\(DJGPP_DOWNLOAD_BASE\)=.*/\1="${DJGPP_MIRROR}"/; \
               s/\(GCC_VERSION_SHORT\)=.*/\1=740/; \
               s/\(GCC_VERSION\)=.*/\1=7.4.0/; \
               s/\(BINUTILS_VERSION\)=.*/\1=230/' \
              script/${GCC_VERSION} && \
    ./build-djgpp.sh ${GCC_VERSION} && \
    rm -rf /tmp/build-djgpp-${BUILD_DJGPP_VERSION} \
           /tmp/build-djgpp-${BUILD_DJGPP_VERSION}.tar.gz
ENV PATH="/usr/local/djgpp/bin:${PATH}"
