#!/usr/bin/env python
from __future__ import print_function

import argparse
import errno
import logging
import os
import pwd
import subprocess
import sys


SDL_VERSION = '2.0.5'
ISPACK_VERSION = '5.5.3'

WORK_DIR = os.path.abspath(os.environ.get('WORK_DIR', 'work'))
DIST_DIR = os.path.abspath(os.environ.get('DIST_DIR', 'dist'))
PLATFORM_DIR = os.path.abspath(os.environ.get('PLATFORM_DIR', 'platform'))
VENDOR_DIR = os.path.abspath(os.environ.get('VENDOR_DIR', 'vendor'))

TARGETS = ['appimage', 'dos', 'macos', 'windows']

# 'uid:gid' user and group IDs in a string
# TODO: what's this do on windows?
UID_GID = '{2}:{3}'.format(*pwd.getpwuid(os.getuid()))

log = logging.getLogger('build')


def main():
    args = parse_args()
    logging.basicConfig(level=logging.DEBUG if args.debug else logging.INFO)

    maybe_make_dirs(WORK_DIR, DIST_DIR)
    maybe_fetch_sdl_source(SDL_VERSION)
    make_source_archive(args.version)

    source = get_source_filename(args.version)
    for target in args.targets:
        globals()['build_' + target](source, args)


def build_appimage(source, args):
    # TODO: maybe fetch appimage

    shell("""
        docker build
        -f Dockerfile.appimage -t kevedit/build_appimage
        --build-arg SDL_VERSION={sdl_version}
        .
        """,
        sdl_version=SDL_VERSION)
    shell("""
        docker run
        -v {work}:/work -v {dist}:/dist -v {platform}:/platform -v {vendor}:/vendor
        -u {uid_gid}
        kevedit/build_appimage /platform/linux/build_linux.sh {source}
        """,
        work=WORK_DIR, dist=DIST_DIR, platform=PLATFORM_DIR, vendor=VENDOR_DIR,
        source=source, uid_gid=UID_GID)
    # Need --privileged for fuse support, required by appimagetool
    shell("""
        docker run --privileged
        -v {work}:/work -v {dist}:/dist -v {vendor}:/vendor
        kevedit/build_appimage sh -c "
          /vendor/appimagetool-x86_64.AppImage /work/appdir/KevEdit.AppDir
            /dist/KevEdit-x86_64.AppImage &&
          chown {uid_gid} /dist/KevEdit-x86_64.AppImage"
        """,
        work=WORK_DIR, dist=DIST_DIR, platform=PLATFORM_DIR, vendor=VENDOR_DIR,
        uid_gid=UID_GID)


def build_macos(source, args):
    source = get_source_filename(args.version)

    shell("""
        docker build
        -f Dockerfile.macos -t kevedit/build_macos
        --build-arg SDL_VERSION={sdl_version}
        .
        """,
        sdl_version=SDL_VERSION)
    shell("""
        docker run
        -v {work}:/work -v {dist}:/dist -v {platform}:/platform -v {vendor}:/vendor
        -u {uid_gid}
        kevedit/build_macos /platform/macos/build_macos.sh {source}
        """,
        work=WORK_DIR, dist=DIST_DIR, platform=PLATFORM_DIR, vendor=VENDOR_DIR,
        source=source, uid_gid=UID_GID)


def build_windows(source, args):
    maybe_fetch_sdl_windows_runtime(SDL_VERSION)
    maybe_fetch_ispack(ISPACK_VERSION)

    source = get_source_filename(args.version)

    shell("""
        docker build
        -f Dockerfile.windows -t kevedit/build_windows
        --build-arg SDL_VERSION={sdl_version}
        --build-arg ISPACK_VERSION={ispack_version}
        .
        """,
        sdl_version=SDL_VERSION, ispack_version=ISPACK_VERSION),
    shell("""
        docker run
        -v {work}:/work -v {dist}:/dist -v {platform}:/platform -v {vendor}:/vendor
        -u {uid_gid}
        kevedit/build_windows /platform/windows/build_windows.sh {source} {sdl_version}
        """,
        work=WORK_DIR, dist=DIST_DIR, platform=PLATFORM_DIR, vendor=VENDOR_DIR,
        source=source, uid_gid=UID_GID, sdl_version=SDL_VERSION)


def build_dos(source, args):
    # TODO: maybe fetch build-djgpp

    source = get_source_filename(args.version)

    shell("""
        docker build
        -f Dockerfile.dos -t kevedit/build_dos
        .
        """)
    shell("""
        docker run
        -v {work}:/work -v {dist}:/dist -v {platform}:/platform -v {vendor}:/vendor
        -u {uid_gid}
        kevedit/build_dos /platform/dos/build_dos.sh {source} {version}
        """,
        work=WORK_DIR, dist=DIST_DIR, platform=PLATFORM_DIR, vendor=VENDOR_DIR,
        source=source, uid_gid=UID_GID, version=args.version)


def parse_args():
    parser = argparse.ArgumentParser(
        description='Build KevEdit for multiple platforms')
    parser.add_argument('-v', '--version', metavar='VERSION', help='KevEdit version to build')
    parser.add_argument('-d', '--debug', action='store_true', help='Enable debug logging')

    target_choices = ['all'] + TARGETS
    target_list = ', '.join(sorted(target_choices))
    parser.add_argument(
            'targets', nargs='*', metavar='TARGET', choices=target_choices, default='all',
            help='Target platforms to build: {}'.format(target_list))

    args = parser.parse_args()

    args.targets = args.targets if isinstance(args.targets, list) else [args.targets]
    if 'all' in args.targets:
        if len(args.targets) > 1:
            print('Target "all" is not allowed with other targets\n')
            parser.print_usage()
            sys.exit(1)
        args.targets = TARGETS

    if args.version is None:
        args.version = check_output('git rev-parse --verify HEAD')
        log.info('Version not specified, using current git version %s', args.version)

    return args


def maybe_fetch_sdl_source(version):
    sdl_filename = 'SDL2-{}.tar.gz'.format(version)
    sdl_src = os.path.join(VENDOR_DIR, sdl_filename)
    sdl_sig = os.path.join(VENDOR_DIR, sdl_filename + '.sig')
    if os.path.exists(sdl_src):
        log.debug('SDL file %s already exists; will not fetch', sdl_src)
        return

    # Validate that we can fetch and check the signature first.
    validate_runs(['wget', '--version'], 'wget is required to fetch SDL')
    validate_runs(['gpg', '--version'], 'gpg is required to fetch SDL')

    log.debug('Fetching SDL source...')
    subprocess.check_call(
        ['wget', 'https://www.libsdl.org/release/' + sdl_filename, '-O', sdl_src])
    log.debug('Fetching SDL signature...')

    subprocess.check_call(
        ['wget', 'https://www.libsdl.org/release/{}.sig'.format(sdl_filename), '-O', sdl_sig])

    log.debug('Checking SDL signature...')
    subprocess.check_call(['gpg', '--verify', sdl_sig, sdl_src])
    log.info('Fetched SDL %s', version)


def maybe_fetch_sdl_windows_runtime(version):
    sdl_filename = 'SDL2-{}-win32-x64.zip'.format(version)
    sdl_zip = os.path.join(VENDOR_DIR, sdl_filename)
    if os.path.exists(sdl_zip):
        log.debug('SDL windows runtime file %s already exists; will not fetch', sdl_zip)
        return

    validate_runs(['wget', '--version'], 'wget is required to fetch SDL windows runtime')

    log.debug('Fetching SDL windows runtime...')
    subprocess.check_call(
        ['wget', 'https://www.libsdl.org/release/' + sdl_filename, '-O', sdl_zip])
    log.info('Fetched SDL windows runtime %s', version)


def maybe_fetch_ispack(version):
    filename = 'ispack-{version}-unicode.exe'.format(version=version)
    ispack_exe = os.path.join(VENDOR_DIR, filename)
    if os.path.exists(ispack_exe):
        log.debug('ispack file %s already exists; will not fetch', ispack_exe)
        return

    validate_runs(['wget', '--version'], 'wget is required to fetch ispack')

    url = 'http://files.jrsoftware.org/ispack/{filename}'.format(filename=filename)
    log.debug('Fetching ispack exe...')
    subprocess.check_call(['wget', url, '-O', ispack_exe])
    log.info('Fetched ispack %s', version)


def make_source_archive(version, path=VENDOR_DIR):
    filename = get_source_filename(version)
    abs_path = os.path.abspath(path)

    # Run in the project root to archive all files
    log.debug('Changing directory to the project top level...')
    cwd = os.getcwd()
    root = check_output('git rev-parse --show-toplevel')
    os.chdir(root)

    log.debug('Making source archive...')
    subprocess.check_call(
        ['git', 'archive', version,
         '--format', 'zip',
         '--output', os.path.join(abs_path, filename)])

    log.debug('Restoring directory...')
    os.chdir(cwd)


def get_source_filename(version):
    return 'kevedit-{}.zip'.format(version)


def check_output(cmd):
    if isinstance(cmd, str):
        cmd = cmd.split(' ')
    result = subprocess.check_output(cmd)
    result = result.decode('utf-8')
    if result.count('\n') == 1:
        result = result.rstrip()
    return result


def shell(cmd, **kwargs):
    cmd = cmd.replace('\n', '')
    cmd = cmd.format(**kwargs)
    return subprocess.check_call(cmd, shell=True)


def validate_runs(command, message):
    try:
        check_output(command)
    except OSError:
        log.exception(message)
        print(message, file=sys.stderr)
        sys.exit(2)


def maybe_make_dirs(*dirs):
    for d in dirs:
        try:
            os.mkdir(d)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise


if __name__ == '__main__':
    main()
