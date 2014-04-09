#!/usr/bin/env python3

import argparse
import subprocess
import tempfile
import os
from elftools.elf.elffile import ELFFile
from elftools.elf.sections import SymbolTableSection

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

def _generate_symbols(inputs):
    for input in inputs:
        for object in _extract_ar(input):
            with open(object, 'rb') as file:
                for section in ELFFile(file).iter_sections():
                    if isinstance(section, SymbolTableSection):
                        for symbol in section.iter_symbols():
                            if _should_add_symbol(symbol):
                                yield symbol.name.decode('ascii')


parser = argparse.ArgumentParser()
parser.add_argument('-o', '--output',
                    help='Output file')
parser.add_argument('inputs',
                    nargs='+',
                    help='Input files')
parser.add_argument('-v', '--verbose',
                    help='Be verbose')
args = parser.parse_args()

lines = []
lines.append('#include <stdlib.h>')
lines.append('#include <sancus_support/private/symbol.h>')
decl_lines = []
sym_lines = []

for symbol in _generate_symbols(args.inputs):
    decl_lines.append('extern char {};'.format(symbol))
    sym_lines.append('    {{"{0}", &{0}, 0}}'.format(symbol))
    if args.verbose:
        print('Added symbol', symbol)

lines.append('\n'.join(decl_lines))
lines.append('const Symbol static_symbols[] = {')
lines.append(',\n'.join(sym_lines))
lines.append('};')
lines.append('const size_t num_static_symbols = ' +
             'sizeof(static_symbols) / sizeof(Symbol);')
data = '\n'.join(lines)

with open(args.output, 'wb') as f:
    f.write(data.encode('ascii'))
