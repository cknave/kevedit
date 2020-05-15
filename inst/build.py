#!/usr/bin/env python
"""KevEdit build script."""
import argparse
import errno
import logging
import os
import pwd
import subprocess
import sys

# Versions of 3rd party software to fetch
APPIMAGE_VERSION = '11'
APPLE_LIBTAPI_VERSION = 'e56673694db395e25b31808b4fbb9a7005e6875f'
CCTOOLS_PORT_VERSION = '1e3f614aff4eaae01b6cc4d29c3237c93f3322f8'
BUILD_DJGPP_VERSION = '2.9'
INNOEXTRACT_VERSION = '1.7'
ISPACK_VERSION = '5.5.8'
MACOS_SDK_VERSION = '10.12'
MACOS_DARWIN_VERSION = '16'
OSXCROSS_VERSION = '6525b2b7d33abc371ad889f205377dc5cf81f23e'
PBZX_VERSION = '1.0.2'
SDL_VERSION = '2.0.9'
XAR_VERSION = '1.6.1'
XCODE_VERSION = '8.3.3'


# All build targets
TARGETS = ['appimage', 'dos', 'macos', 'windows']


# Build temp directory
WORK_DIR = os.path.abspath(os.environ.get('WORK_DIR', 'work'))

# Distribution target path
DIST_DIR = os.path.abspath(os.environ.get('DIST_DIR', 'dist'))

# Platform-specific files
PLATFORM_DIR = os.path.abspath(os.environ.get('PLATFORM_DIR', 'platform'))

# 3rd party download path
VENDOR_DIR = os.path.abspath(os.environ.get('VENDOR_DIR', 'vendor'))

# 'uid:gid' user and group IDs in a string
# TODO: what's this do on windows?
UID_GID = '{2}:{3}'.format(*pwd.getpwuid(os.getuid()))

log = logging.getLogger('build')


def main():
    """Run build script."""
    args = parse_args()
    logging.basicConfig(level=logging.DEBUG if args.debug else logging.INFO)

    maybe_make_dirs(WORK_DIR, DIST_DIR, VENDOR_DIR)
    make_source_archive(args.version)

    source = get_source_filename(args.version)
    for target in args.targets:
        globals()['build_' + target](source, args)


def build_appimage(source, args, image_version='1.1'):
    """Build Linux x86_64 AppImage to DIST_DIR.

    :param str source: path to KevEdit source zip
    :param args: command line arguments namespace
    :param str image_version: docker image version
    """
    appimage_tool = 'appimagetool-{0}-x86_64.AppImage'.format(APPIMAGE_VERSION)
    maybe_fetch(
        description='AppImage tool {}'.format(APPIMAGE_VERSION),
        url='https://github.com/AppImage/AppImageKit/releases/download/{}/'
            'appimagetool-x86_64.AppImage'.format(APPIMAGE_VERSION),
        filename=appimage_tool,
        chmod=0o755)
    maybe_fetch(
        description='AppRun {}'.format(APPIMAGE_VERSION),
        url='https://github.com/AppImage/AppImageKit/releases/download/{}/'
            'AppRun-x86_64'.format(APPIMAGE_VERSION),
        filename='AppRun-{}-x86_64'.format(APPIMAGE_VERSION),
        chmod=0o755)

    if args.docker_images == 'pull':
        log.debug('Pulling AppImage docker image...')
        shell('docker pull kevedit/build_appimage:{version}', version=image_version)
    else:
        maybe_fetch_sdl_source(SDL_VERSION)
        log.debug('Building AppImage docker image...')
        shell("""
              docker build
              -f Dockerfile.appimage -t kevedit/build_appimage:{image_version}
              --build-arg SDL_VERSION={sdl_version}
              .
              """,
              image_version=image_version, sdl_version=SDL_VERSION)
        maybe_tag_latest('kevedit/build_appimage', image_version, args)

    log.debug('Compiling executable for AppImage...')
    shell("""
          docker run --rm
          -v {work}:/work -v {dist}:/dist -v {platform}:/platform -v {vendor}:/vendor
          -u {uid_gid}
          kevedit/build_appimage:{image_version}
          /platform/linux/build_linux.sh {source} {appimage_version}
          """,
          work=WORK_DIR, dist=DIST_DIR, platform=PLATFORM_DIR, vendor=VENDOR_DIR,
          image_version=image_version, source=source, uid_gid=UID_GID,
          appimage_version=APPIMAGE_VERSION)

    log.debug('Packing AppImage artifact...')
    # Need --privileged for fuse support, required by appimagetool
    shell("""
          docker run --rm --privileged
          -v {work}:/work -v {dist}:/dist -v {vendor}:/vendor
          kevedit/build_appimage sh -c "
            /vendor/{appimage_tool} /work/appdir/KevEdit.AppDir
              /dist/kevedit-{version}-x86_64.AppImage &&
            chown {uid_gid} /dist/kevedit-{version}-x86_64.AppImage"
          """,
          work=WORK_DIR, dist=DIST_DIR, platform=PLATFORM_DIR, vendor=VENDOR_DIR,
          appimage_tool=appimage_tool, uid_gid=UID_GID, version=args.version)


def build_macos(source, args, image_version='2.0'):
    """Build macOS x86_64 .app in a .dmg archive to DIST_DIR.

    :param str source: path to KevEdit source zip
    :param args: command line arguments namespace
    :param str image_version: docker image version
    """
    if args.docker_images == 'pull':
        log.debug('Pulling macOS docker image...')
        shell('docker pull kevedit/build_macos:{version}', version=image_version)
    else:
        maybe_extract_macos_sdk(args, MACOS_SDK_VERSION, image_version)
        maybe_tag_latest('kevedit/macos_sdk_extractor', image_version, args)
        maybe_fetch_sdl_source(SDL_VERSION)
        maybe_fetch_xar(XAR_VERSION)
        maybe_fetch(
            description='Apple libtapi {}'.format(APPLE_LIBTAPI_VERSION),
            url='https://github.com/tpoechtrager/apple-libtapi/archive/{}.zip'.format(
                APPLE_LIBTAPI_VERSION),
            filename='apple-libtapi-{}.zip'.format(APPLE_LIBTAPI_VERSION))
        maybe_fetch(
            description='cctools-port {}'.format(CCTOOLS_PORT_VERSION),
            url='https://github.com/tpoechtrager/cctools-port/archive/{}.zip'.format(
                CCTOOLS_PORT_VERSION),
            filename='cctools-port-{}.zip'.format(CCTOOLS_PORT_VERSION))
        log.debug('Building macOS docker image...')
        shell("""
              docker build
              -f Dockerfile.macos -t kevedit/build_macos:{image_version}
              --build-arg APPLE_LIBTAPI_VERSION={libtapi_version}
              --build-arg CCTOOLS_PORT_VERSION={cctools_version}
              --build-arg DARWIN_VERSION={darwin_version}
              --build-arg MACOS_SDK_VERSION={sdk_version}
              --build-arg OSXCROSS_VERSION={osxcross_version}
              --build-arg SDL_VERSION={sdl_version}
              --build-arg XAR_VERSION={xar_version}
              .
              """,
              image_version=image_version, libtapi_version=APPLE_LIBTAPI_VERSION,
              cctools_version=CCTOOLS_PORT_VERSION, darwin_version=MACOS_DARWIN_VERSION,
              sdk_version=MACOS_SDK_VERSION, osxcross_version=OSXCROSS_VERSION,
              sdl_version=SDL_VERSION, xar_version=XAR_VERSION)
        maybe_tag_latest('kevedit/build_macos', image_version, args)

    log.debug('Building macOS .dmg artifact...')
    shell("""
          docker run --rm
          -v {work}:/work -v {dist}:/dist -v {platform}:/platform -v {vendor}:/vendor
          -u {uid_gid}
          kevedit/build_macos:{image_version}
          /platform/macos/build_macos.sh {source} {version}
          """,
          work=WORK_DIR, dist=DIST_DIR, platform=PLATFORM_DIR, vendor=VENDOR_DIR,
          image_version=image_version, source=source, uid_gid=UID_GID, version=args.version)


def build_windows(source, args, image_version='1.1'):
    """Build windows x64 .exe in a self-executing installer to DIST_DIR.

    :param str source: path to KevEdit source zip
    :param args: command line arguments namespace
    :param str image_version: docker image version
    """
    maybe_fetch(description='SDL {} windows runtime'.format(SDL_VERSION),
                url='https://www.libsdl.org/release/SDL2-{}-win32-x64.zip'.format(SDL_VERSION))

    if args.docker_images == 'pull':
        log.debug('Pulling Windows docker image...')
        shell('docker pull kevedit/build_windows:{version}', version=image_version)
    else:
        maybe_fetch_sdl_source(SDL_VERSION)
        maybe_fetch(
            description='ispack {}'.format(ISPACK_VERSION),
            url='http://files.jrsoftware.org/ispack/ispack-{}-unicode.exe'.format(ISPACK_VERSION))
        inex_url = ('https://github.com/dscharrer/innoextract/releases/download/{0}/'
                    'innoextract-{0}-linux.tar.xz'.format(INNOEXTRACT_VERSION))
        maybe_fetch(description='innoextract {}'.format(INNOEXTRACT_VERSION),
                    url=inex_url,
                    signature_url=inex_url+'.sig')
        log.debug('Building Windows docker image...')
        shell("""
              docker build
              -f Dockerfile.windows -t kevedit/build_windows:{image_version}
              --build-arg SDL_VERSION={sdl_version}
              --build-arg ISPACK_VERSION={ispack_version}
              --build-arg INNOEXTRACT_VERSION={innoextract_version}
              .
              """,
              image_version=image_version, sdl_version=SDL_VERSION, ispack_version=ISPACK_VERSION,
              innoextract_version=INNOEXTRACT_VERSION)
        maybe_tag_latest('kevedit/build_windows', image_version, args)

    log.debug('Bulding Windows setup.exe artifact...')
    shell("""
          docker run --rm
          -v {work}:/work -v {dist}:/dist -v {platform}:/platform -v {vendor}:/vendor
          -u {uid_gid}
          kevedit/build_windows:{image_version}
          /platform/windows/build_windows.sh {source} {version} {sdl_version}
          """,
          work=WORK_DIR, dist=DIST_DIR, platform=PLATFORM_DIR, vendor=VENDOR_DIR,
          image_version=image_version, source=source, uid_gid=UID_GID, version=args.version,
          sdl_version=SDL_VERSION)


def build_dos(source, args, image_version='1.1'):
    """Build DOS 32-bit .exe in a .zip file to DIST_DIR.

    :param str source: path to KevEdit source zip
    :param args: command line arguments namespace
    :param str image_version: docker image version
    """
    if args.docker_images == 'pull':
        log.debug('Pulling DOS docker image...')
        shell('docker pull kevedit/build_dos:{version}', version=image_version)
    else:
        maybe_fetch(
            description='build-djgpp {}'.format(BUILD_DJGPP_VERSION),
            url='https://github.com/andrewwutw/build-djgpp/archive/v{}.tar.gz'.format(
                BUILD_DJGPP_VERSION),
            filename='build-djgpp-{}.tar.gz'.format(BUILD_DJGPP_VERSION))
        log.debug('Building DOS docker image...')
        shell("""
              docker build
              -f Dockerfile.dos -t kevedit/build_dos:{image_version}
              .
              """,
              image_version=image_version)
        maybe_tag_latest('kevedit/build_dos', image_version, args)

    log.debug('Building DOS zip artifact...')
    shell("""
          docker run --rm
          -v {work}:/work -v {dist}:/dist -v {platform}:/platform -v {vendor}:/vendor
          -u {uid_gid}
          kevedit/build_dos:{image_version}
          /platform/dos/build_dos.sh {source} {version}
          """,
          work=WORK_DIR, dist=DIST_DIR, platform=PLATFORM_DIR, vendor=VENDOR_DIR,
          image_version=image_version, source=source, uid_gid=UID_GID, version=args.version)


def parse_args():
    """Parse command line arguments.

    :return: arguments namespace
    """
    parser = argparse.ArgumentParser(
        description='Build KevEdit for multiple platforms')

    # optional arguments
    parser.add_argument('-v', '--version', metavar='VERSION', help='KevEdit version to build')
    parser.add_argument('-d', '--debug', action='store_true', help='Enable debug logging')

    # optional maintainer arguments
    mc_group = parser.add_argument_group('maintainer arguments')
    mc_group.add_argument('-i', '--docker-images', choices=['build', 'pull'], default='pull',
                          help='Method to get docker build images')
    mc_group.add_argument('-t', '--tag', action='store_true',
                          help="Tag docker image as 'latest' after building")

    # positional arguments
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

    if args.tag and args.docker_images != 'build':
        print('The --tag argument can only be used with --docker-images=build')
        parser.print_usage()
        sys.exit(1)

    if args.version is None:
        args.version = check_output('git describe --tags HEAD')
        log.info('Version not specified, using current git version %s', args.version)

    return args


def maybe_fetch(description, url, filename=None, signature_url=None, chmod=None):
    """If a filename doesn't exist in VENDOR_DIR, fetch it.

    If a `signature_url` is given, also fetch that and check the file's PGP signature.

    :param str description: file description
    :param str url: source URL
    :param str filename: optional destination filename inside VENDOR_DIR, defaults to the last path
        component of url
    :param str signature_url: optional URL to the PGP signature for the source file
    :param int chmod: optional mode to chmod the destination file
    """
    if not filename:
        filename = url.rsplit('/', 1)[-1]

    dest_path = os.path.join(VENDOR_DIR, filename)
    if os.path.exists(dest_path):
        log.debug('%s file %s already exists; will not fetch', description, dest_path)
        return

    # Validate that we can fetch the file (and verify its signature if a signature_url is given)
    validate_runs(['wget', '--version'], 'wget is required to fetch {}'.format(description))
    if signature_url:
        validate_runs(['gpg', '--version'], 'gpg is required to fetch {}'.format(description))

    if 'Xcode' in description:
        log.info('********** Downloading Xcode! Hold on to your butts!')

    log.debug('Fetching %s...', description)
    subprocess.check_call(['wget', url, '-O', dest_path])

    if signature_url:
        log.debug('Fetching %s signature...', description)
        sig_path = '{}.sig'.format(dest_path)
        subprocess.check_call(['wget', signature_url, '-O', sig_path])

        log.debug('Checking %s signature...', description)
        subprocess.check_call(['gpg', '--verify', sig_path, dest_path])
    log.info('Fetched %s', description)

    if chmod:
        log.debug('Chmod %s to %03o', dest_path, chmod)
        os.chmod(dest_path, chmod)


def maybe_fetch_sdl_source(version):
    """Fetch the SDL source code if it doesn't already exist in VENDOR_DIR.

    :param str version: SDL2 version to fetch
    """
    sdl_filename = 'SDL2-{}.tar.gz'.format(version)
    maybe_fetch(description='SDL {} source'.format(version),
                url='https://www.libsdl.org/release/{}'.format(sdl_filename),
                filename=sdl_filename,
                signature_url='https://www.libsdl.org/release/{}.sig'.format(sdl_filename))


def maybe_fetch_osxcross(version):
    """Fetch the OSXCross git snapshot if it doesn't already exist in VENDOR_DIR.

    :param str version: OSXCross git commit hash
    """
    maybe_fetch(description='OSXCross {}'.format(version),
                url='https://github.com/tpoechtrager/osxcross/archive/{}.zip'.format(version),
                filename='osxcross-{}.zip'.format(version))


def maybe_fetch_xar(version):
    """Fetch the xar sources git snapshot if it doesn't already exist in VENDOR_DIR.

    :param str version: xar git commit hash
    """
    maybe_fetch(description='xar {} sources'.format(version),
                url='https://github.com/mackyle/xar/archive/xar-{}.tar.gz'.format(version))


def maybe_extract_macos_sdk(args, sdk_version, docker_image_version):
    """If the macOS SDK doesn't exist, fetch Xcode, build an extractor in docker, and run it.

    :param args: arguments namespace
    :param str sdk_version: macOS SDK version
    :param str docker_image_version: macos_sdk_extractor image version
    """
    sdk_file = 'MacOSX{}.sdk.tar.xz'.format(sdk_version)
    sdk_path = os.path.join(VENDOR_DIR, sdk_file)
    if os.path.exists(sdk_path):
        log.debug('MacOS SDK file %s already exists; will not extract', sdk_path)
        return

    maybe_make_dirs(os.path.join(VENDOR_DIR, 'xcode'))
    maybe_fetch(
        description='Xcode {}'.format(XCODE_VERSION),
        url='https://download.developer.apple.com/Developer_Tools/Xcode_{0}/Xcode{0}.xip'.format(
            XCODE_VERSION),
        filename=os.path.join('xcode', 'Xcode{}.xip'.format(XCODE_VERSION)))

    if args.docker_images == 'pull':
        log.debug('Pulling macOS SDK extractor image...')
        shell('docker pull kevedit/macos_sdk_extractor:{version}', version=docker_image_version)
    else:
        maybe_fetch_osxcross(OSXCROSS_VERSION)
        maybe_fetch_xar(XAR_VERSION)
        maybe_fetch(
            description='pbzx {}'.format(PBZX_VERSION),
            url='https://github.com/NiklasRosenstein/pbzx/archive/v{}.tar.gz'.format(PBZX_VERSION),
            filename='pbzx-{}.tar.gz'.format(PBZX_VERSION))
        log.debug('Building macOS SDK extractor docker image...')
        shell("""
              docker build
              -f Dockerfile.macos_sdk_extractor -t kevedit/macos_sdk_extractor:{image_version}
              --build-arg OSXCROSS_VERSION={osxcross_version}
              --build-arg PBZX_VERSION={pbzx_version}
              --build-arg XAR_VERSION={xar_version}
              .
              """,
              image_version=docker_image_version, osxcross_version=OSXCROSS_VERSION,
              pbzx_version=PBZX_VERSION, xar_version=XAR_VERSION)
        maybe_tag_latest('kevedit/macos_sdk_extractor', docker_image_version, args)

    log.debug('Extracting macOS SDK...')
    shell("""
          docker run --rm
          -v {vendor}:/vendor -u {uid_gid}          
          kevedit/macos_sdk_extractor:{image_version}
          {xcode_version}
          """,
          vendor=VENDOR_DIR, uid_gid=UID_GID, image_version=docker_image_version,
          xcode_version=XCODE_VERSION)

    if not os.path.exists(sdk_path):
        log.error('Extractor ran on Xcode %s but expected SDK file was not created: %s',
                  XCODE_VERSION, sdk_path)
        sys.exit(1)


def make_source_archive(version, path=VENDOR_DIR):
    """Retrieve the selected version from git and save as a zip file.

    :param str version: version to use
    :param str path: directory to save the zip file
    """
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


def maybe_tag_latest(name, version, args):
    """Tag a docker image 'latest' if the --tag option was set.

    :param str name: docker image name
    :param str version: version number tag
    :param args: arguments namespace
    """
    if not args.tag:
        return
    shell('docker tag {name}:{version} {name}:latest', name=name, version=version)


def get_source_filename(version):
    """Get the KevEdit source zip filename for a given version.

    :param str version: KevEdit version
    :rtype: str
    """
    return 'kevedit-{}.zip'.format(version)


def check_output(cmd):
    """Execute a command, check its return code, and return its output.

    The command will not be executed in a shell.

    If only one line of output is returned, the newline will be stripped.  For multi-line
    output, the newlines will be preserved.

    :param cmd: command line str or list
    :rtype: str
    """
    if isinstance(cmd, str):
        cmd = cmd.split(' ')
    result = subprocess.check_output(cmd)
    result = result.decode('utf-8')
    if result.count('\n') == 1:
        result = result.rstrip()
    return result


def shell(cmd, **kwargs):
    """Format a command line, and run it using a shell.

    :param str cmd: command line format string; newlines will be stripped.
    :param kwargs: format keyword arguments
    """
    cmd = cmd.replace('\n', '')
    cmd = cmd.format(**kwargs)
    subprocess.check_call(cmd, shell=True)


def validate_runs(command, message):
    """Validate that a command runs, exiting on failure.

    :param command: command line str or list
    :param message: error message on failure
    """
    try:
        check_output(command)
    except OSError:
        log.exception(message)
        print(message, file=sys.stderr)
        sys.exit(2)


def maybe_make_dirs(*dirs):
    """Make directories if they don't already exist.

    :param dirs: path strs
    """
    for d in dirs:
        try:
            os.mkdir(d)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise
        else:
            log.debug('Created directory %s', d)


if __name__ == '__main__':
    main()
