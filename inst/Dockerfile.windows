FROM multiarch/crossbuild:latest
ARG MAKE_OPTS=-j12
ARG SDL_VERSION
ARG ISPACK_VERSION
ARG INNOEXTRACT_VERSION

# multiarch is getting quite out of date at this point, and no longer has
# security updates
RUN grep -v http://security.debian.org /etc/apt/sources.list >/tmp/sources.list && \
    mv /tmp/sources.list /etc/apt/sources.list && \
    apt-get update && \
    apt-get install -y genisoimage pkgconf wine && \
    ln -sf /usr/bin/genisoimage /usr/bin/mkisofs

COPY vendor/innoextract-${INNOEXTRACT_VERSION}-linux.tar.xz /tmp/
RUN cd /tmp && \
    tar xf innoextract-${INNOEXTRACT_VERSION}-linux.tar.xz && \
    cp innoextract-${INNOEXTRACT_VERSION}-linux/bin/amd64/innoextract /usr/local/bin && \
    rm -rf /tmp/innoextract-${INNOEXTRACT_VERSION}*

ENV PATH="/usr/x86_64-w64-mingw32/bin:${PATH}"
COPY vendor/SDL2-${SDL_VERSION}.tar.gz /tmp/
RUN cd /tmp && \
    tar xzf SDL2-${SDL_VERSION}.tar.gz && \
    cd SDL2-${SDL_VERSION} && \
    ./configure --host=x86_64-w64-mingw32 --prefix=/usr/x86_64-w64-mingw32 && \
    make ${MAKE_OPTS} && \
    make install && \
    rm -rf /tmp/SDL2-${SDL_VERSION}*

COPY vendor/ispack-${ISPACK_VERSION}-unicode.exe /tmp/
RUN mkdir /innosetup && \
    innoextract -d /innosetup /tmp/ispack-${ISPACK_VERSION}-unicode.exe && \
    rm -f /tmp/ispack-${ISPACK_VERSION}-unicode.exe
    
