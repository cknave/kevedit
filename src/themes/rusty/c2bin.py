#!/usr/bin/env python
"""Script to convert ANSI panel *.C source code to editable *.BIN files."""
import argparse
import os.path
import re
import sys

parser = argparse.ArgumentParser(description='Convert an ANSI image from .c to .bin')
parser.add_argument('input', help='input .c file, or - for stdin')
parser.add_argument('output',
                    nargs=argparse.OPTIONAL,
                    help='output .bin file, renaming *.c to *.bin if not given, or - for stdout')
args = parser.parse_args()

if args.input == '-':
    fin = sys.stdin
else:
    fin = open(args.input, 'rt')
source = fin.read()

match = re.search(r'unsigned char [^{]+\{([^}]+)}', source, re.MULTILINE)
if not match:
    print('Could not find unsigned char array in input', file=sys.stderr)
    sys.exit(1)
items = [item.strip() for item in match.group(1).split(',')]


def parse_array_item(s: str) -> int:
    original = s
    if m := re.match(r"'(.)'", s):
        result = ord(m.group(1))
    else:
        if m := re.match(r"'\\x([0-9a-f]{1,2})'", s):
            s = f'0x{m.group(1)}'
        try:
            result = int(s, 0)
        except ValueError:
            print(f'Could not parse {original} as integer', file=sys.stderr)
            sys.exit(1)
    if result < 0 or result > 255:
        print(f'Value {original} out of range', file=sys.stderr)
        sys.exit(1)
    return result


values = bytes(parse_array_item(item) for item in items)
output = args.output
if output is None:
    output = os.path.splitext(args.input)[0] + '.bin'
if output == '-':
    fout = sys.stdout
else:
    fout = open(output, 'wb')
fout.write(values)
