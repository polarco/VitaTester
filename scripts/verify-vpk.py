#!/usr/bin/env python3
"""Validate package identity, assets and executable from this same build."""
import pathlib
import struct
import sys
import zipfile


def verify(path):
    path = pathlib.Path(path)
    root = pathlib.Path(__file__).resolve().parent.parent
    with zipfile.ZipFile(path) as z:
        assert z.testzip() is None, 'ZIP CRC failure'
        names = z.namelist()
        assert len(names) == len(set(names)), 'duplicate ZIP entry'
        allowed = {'eboot.bin', 'sce_sys/param.sfo'} | {str(p.relative_to(root)) for p in (root / 'icons').glob('*.png')}
        assert set(names) == allowed, f'unexpected/missing package entries: {set(names) ^ allowed}'
        for name in names:
            assert not name.startswith('/') and '..' not in pathlib.PurePosixPath(name).parts
        for p in (root / 'icons').glob('*.png'):
            assert z.read(str(p.relative_to(root))) == p.read_bytes(), f'asset mismatch: {p.name}'
        eboot = z.read('eboot.bin')
        assert eboot[:4] == b'SCE\0', 'not a Vita SELF'
        assert eboot == (path.parent / 'VitaTester.self').read_bytes(), 'SELF from another build'
        elf = (path.parent / 'VitaTester').read_bytes()
        assert elf[:5] == b'\x7fELF\x01' and struct.unpack_from('<H', elf, 18)[0] == 40, 'not ELF32 ARM'
        data = z.read('sce_sys/param.sfo')
        magic, version, keys, values, count = struct.unpack_from('<5I', data)
        assert magic == 0x46535000 and version == 0x101
        fields = {}
        for i in range(count):
            k, fmt, length, capacity, offset = struct.unpack_from('<HHIII', data, 20 + i * 16)
            name = data[keys+k:data.index(b'\0', keys+k)].decode()
            assert length <= capacity and values + offset + capacity <= len(data)
            fields[name] = data[values+offset:values+offset+length].rstrip(b'\0')
        assert fields['APP_VER'] == b'01.51', fields
        assert fields['TITLE_ID'] == b'VITATESTR', fields
        assert fields['TITLE'] == b'VitaTester', fields
    print('VPK: CRC, exact asset inventory, ELF/SELF, Title ID and SFO 01.51 OK')


if __name__ == '__main__':
    verify(sys.argv[1])
