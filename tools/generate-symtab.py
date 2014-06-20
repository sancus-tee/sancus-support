#!/usr/bin/env python3

import argparse
import subprocess
import tempfile
import os
from elftools.elf.elffile import ELFFile
from elftools.elf.sections import SymbolTableSection


def _find_std_lib(name):
    libc = subprocess.check_output(['msp430-gcc', '-print-file-name=libc.a'],
                                   universal_newlines=True)
    libdir = os.path.dirname(libc)
    files = (fmt.format(name) for fmt in ('{}', 'lib{}.a', '{}.a'))

    for file in files:
        path = '{}/{}'.format(libdir, file)
        if os.path.isfile(path):
            return path

    raise argparse.ArgumentTypeError('Unable to find library "{}"'.format(name))


def _find_default_libs():
    output =  subprocess.check_output(['sancus-ld', '--print-default-libs',
                                       '--standalone'], universal_newlines=True)
    return [f for f in output.split('\n') if os.path.isfile(f)]


def _extract_ar(ar_file):
    tmp_dir = tempfile.mkdtemp()
    subprocess.check_call(['ar', 'x', ar_file], cwd=tmp_dir)
    return ['{}/{}'.format(tmp_dir, file) for file in os.listdir(tmp_dir)]


def _should_add_symbol(symbol):
    sym_bind = symbol['st_info']['bind']
    sym_type = symbol['st_info']['type']
    sym_sec = symbol['st_shndx']
    return sym_bind == 'STB_GLOBAL' and \
           sym_type in ('STT_OBJECT', 'STT_FUNC', 'STT_NOTYPE') and \
           sym_sec != 'SHN_UNDEF'


def _generate_symbols_in_object(object):
    with open(object, 'rb') as file:
        for section in ELFFile(file).iter_sections():
            if isinstance(section, SymbolTableSection):
                for symbol in section.iter_symbols():
                    if _should_add_symbol(symbol):
                        yield symbol


def _generate_symbols_in_lib(lib):
    for object in _extract_ar(lib):
        yield from _generate_symbols_in_object(object)


def _generate_symbols(inputs):
    for input in inputs:
        _, ext = os.path.splitext(input)
        if ext == '.o':
            yield from _generate_symbols_in_object(input)
        elif ext == '.a':
            yield from _generate_symbols_in_lib(input)
        else:
            raise ValueError('Unknown input extension {}'.format(ext))


def _emit_symbols(inputs, ignores=[]):
    decl_lines = []
    sym_lines = []

    for symbol in _generate_symbols(inputs):
        name = symbol.name.decode('ascii')
        if not name in ignores:
            type = symbol['st_info']['type']
            if name == 'putchar':
                decl = 'extern int putchar(int c);'
            else:
                decl = 'extern char {};'.format(name)
            decl_lines.append(decl)
            sym_lines.append('    {{"{0}", &{0}}}'.format(name))
            if args.verbose:
                print('Added symbol', name)

    return (decl_lines, sym_lines)


parser = argparse.ArgumentParser()
parser.add_argument('-o', '--output',
                    help='Output file')
parser.add_argument('inputs',
                    nargs='+',
                    help='Input files')
parser.add_argument('--add-std-lib',
                    action='append',
                    default=[],
                    type=_find_std_lib,
                    dest='additional_libs',
                    help='Add a standard library to the symbol table')
parser.add_argument('-v', '--verbose',
                    action='store_true',
                    help='Be verbose')
args = parser.parse_args()

includes = ('sancus_support/private/symbol.h', 'errno.h', 'math.h', 'stdint.h',
            'stdio.h', 'stdlib.h', 'string.h', 'ctype.h', 'byteswap.h',
            'setjmp.h', 'sancus/sm_support.h')

input_decls, input_syms = _emit_symbols(args.inputs,
                                        ignores=['sancus_enable'])
_, libs_syms = _emit_symbols(args.additional_libs + _find_default_libs(),
                             ignores=['ffs', 'rindex', 'index'])

lines = []

for include in includes:
    lines.append('#include <{}>'.format(include))

lines.append('\n'.join(input_decls))
lines.append('const Symbol static_symbols[] = {')
lines.append(',\n'.join(input_syms + libs_syms))
lines.append('};')
lines.append('const unsigned num_static_symbols = ' +
             'sizeof(static_symbols) / sizeof(Symbol);')

data = '\n'.join(lines)

with open(args.output, 'wb') as f:
    f.write(data.encode('ascii'))
