FROM alpine:3.9
ARG APPLE_LIBTAPI_VERSION=e56673694db395e25b31808b4fbb9a7005e6875f
ARG CCTOOLS_PORT_VERSION=1e3f614aff4eaae01b6cc4d29c3237c93f3322f8
ARG DARWIN_VERSION
ARG MACOS_SDK_VERSION
ARG MAKE_OPTS=-j12
ARG OSXCROSS_VERSION=6525b2b7d33abc371ad889f205377dc5cf81f23e
ARG XAR_VERSION=1.6.1

COPY platform/macos/osxcross /tmp
COPY vendor/MacOSX${MACOS_SDK_VERSION}.sdk.tar.xz /tmp
COPY vendor/apple-libtapi-${APPLE_LIBTAPI_VERSION}.zip /tmp
COPY vendor/cctools-port-${CCTOOLS_PORT_VERSION}.zip /tmp
COPY vendor/osxcross-${OSXCROSS_VERSION}.zip /tmp
COPY vendor/xar-${XAR_VERSION}.tar.gz /tmp

# So this is in a bit of a sorry state:
# https://github.com/tpoechtrager/osxcross/issues/103#issuecomment-300769589
RUN apk update \
 && apk add autoconf automake cdrkit clang clang-dev fts libressl libuuid llvm-dev make pkgconf \
    xz-libs \
 && apk add --virtual dev-deps autoconf automake bsd-compat-headers cmake dev-deps fts-dev g++ \
    libressl-dev libtool libxml2-dev linux-headers musl-dev patch python tar util-linux-dev xz \
    xz-dev zlib-dev \
 && ln -s /usr/include/linux/sysctl.h /usr/include/sys \
 && cd /tmp \
 && tar xzf /tmp/xar-${XAR_VERSION}.tar.gz \
 && cd xar-xar-${XAR_VERSION}/xar \
 && ./autogen.sh LDFLAGS=-lfts \
 && make ${MAKE_OPTS} \
 && make install \
 && cd /tmp \
 && rm -rf xar-${XAR_VERSION}.tar.gz xar-xar-${XAR_VERSION} \
 # 1. Bulid osxcross.  Its test compile will fail since we don't have a good linker yet.
 && unzip osxcross-${OSXCROSS_VERSION}.zip \
 && cd osxcross-${OSXCROSS_VERSION} \
 && mv /tmp/MacOSX${MACOS_SDK_VERSION}.sdk.tar.xz tarballs \
 && cp /tmp/patches/* patches \
 && patch -p0 </tmp/build-new-patches.patch \
 # Build script always fails:
 && UNATTENDED=1 ./build.sh || true \
 # 2. Build apple-libtapi, required by the new linker to use newer frameworks
 && cd /tmp \
 && unzip apple-libtapi-${APPLE_LIBTAPI_VERSION}.zip \
 && cd apple-libtapi-${APPLE_LIBTAPI_VERSION} \
 && INSTALLPREFIX=/tmp/osxcross-${OSXCROSS_VERSION}/target ./build.sh \
 && ./install.sh \
 && cd /tmp \
 # 3. Build the replacement linker
 && unzip cctools-port-${CCTOOLS_PORT_VERSION}.zip \
 && cd cctools-port-${CCTOOLS_PORT_VERSION}/cctools \
 && libtoolize -c -i --force \
 && ./autogen.sh \
 # musl-related patches (same as those used when building osxcross the first time)
 && patch -p0 </tmp/patches/cctools-ld64-3.patch \
 && patch -p0 </tmp/patches/cctools-ar-1.patch \
 && patch -p0 </tmp/patches/cctools-ar-2.patch \
 && patch -p0 </tmp/patches/cctools-libstuff-1.patch \
 && ./configure --prefix=/tmp/osxcross-${OSXCROSS_VERSION}/target \
    --with-libtapi=/tmp/osxcross-${OSXCROSS_VERSION}/target \
    --target=x86_64-apple-darwin${DARWIN_VERSION} \
 && make ${MAKE_OPTS} \
 && make install \
 && mkdir /opt/osxcross \
 && cp -a /tmp/osxcross-${OSXCROSS_VERSION}/target/* /opt/osxcross \
 # 4. Clean up
 && cd /tmp \
 && rm -rf apple-libtapi-${APPLE_LIBTAPI_VERSION}* \
    build-new-patches.patch \
    cctools-port-${CCTOOLS_PORT_VERSION}* \
    fix_gen_sdk_package_for_xcode8.patch \
    osxcross-${OSXCROSS_VERSION}* \
    patches \
 && apk del dev-deps \
 && rm /usr/include/sys/sysctl.h

ENV PATH=$PATH:/opt/osxcross/bin
ENV LD_LIBRARY_PATH=/opt/osxcross/lib


# Now that we have a toolchain, we can build SDL

ARG SDL_VERSION
COPY vendor/SDL2-${SDL_VERSION}.tar.gz /tmp
RUN cd /tmp \
 && tar xvzf SDL2-${SDL_VERSION}.tar.gz \
 && cd SDL2-${SDL_VERSION} \
 && export MACOSX_DEPLOYMENT_TARGET=10.6 \
 && ./configure --host=x86_64-apple-darwin16 --prefix=/opt/osxcross CC=o64-clang AR=llvm-ar RANLIB=llvm-ranlib \
 && make ${MAKE_OPTS} \
 && make install \
 && rm -rf /tmp/SDL2-${SDL_VERSION}*
