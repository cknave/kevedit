# Extract macOS SDK from Xcode archive
FROM alpine:3.9
ARG MAKE_OPTS=-j12
ARG OSXCROSS_VERSION=6525b2b7d33abc371ad889f205377dc5cf81f23e
ARG PBZX_VERSION=1.0.2
ARG XAR_VERSION=1.6.1

COPY vendor/xar-${XAR_VERSION}.tar.gz /tmp
COPY vendor/pbzx-${PBZX_VERSION}.tar.gz /tmp
COPY vendor/osxcross-${OSXCROSS_VERSION}.zip /tmp
COPY platform/macos/osxcross/fix_gen_sdk_package_for_xcode8.patch /tmp
COPY platform/macos/extract_sdk.sh /
RUN apk update \
 && apk add bash cpio fts libressl libxml2 xz \
 && apk add --virtual dev-deps autoconf cpio fts-dev gcc libxml2-dev libressl-dev make musl-dev \
    xz-dev \
 && cd /tmp \
 && tar xzf /tmp/xar-${XAR_VERSION}.tar.gz \
 && cd xar-xar-${XAR_VERSION}/xar \
 && ./autogen.sh LDFLAGS=-lfts \
 && make ${MAKE_OPTS} \
 && make install \
 && cd /tmp \
 && rm -rf xar-${XAR_VERSION}.tar.gz xar-xar-${XAR_VERSION} \
 && tar xzf /tmp/pbzx-${PBZX_VERSION}.tar.gz \
 && cd pbzx-${PBZX_VERSION} \
 # Something hates a struct field called stdin, I think it's musl
 && sed -e 's/\(bool \|\.\|->\)stdin/\1stdin_/g' pbzx.c >pbzx_musl.c \
 && gcc -o /usr/local/bin/pbzx pbzx_musl.c -llzma -lxar \
 && cd .. \
 && rm -rf pbzx-${PBZX_VERSION}* \
 && unzip osxcross-${OSXCROSS_VERSION}.zip \
 && cd osxcross-${OSXCROSS_VERSION}/tools \
 # Workaround for gen_sdk_package.sh copying a symlink instead of the actual files
 && patch -p1 </tmp/fix_gen_sdk_package_for_xcode8.patch \
 && cp gen_sdk_package.sh /usr/local/bin \
 && cd /tmp \
 && rm -rf osxcross-${OSXCROSS_VERSION}* \
 && apk del dev-deps

ENTRYPOINT ["/extract_sdk.sh"]
