#!/usr/bin/env python3

import argparse
import pathlib
import struct
import sys

ELF_MAGIC = b"\x7fELF"
ELFCLASS64 = 2
ELFDATA2LSB = 1
PT_DYNAMIC = 2

DT_NULL = 0
DT_RUNPATH = 0x1D
DT_GNU_HASH = 0x6FFFFEF5
DT_FLAGS_1 = 0x6FFFFFFB
DT_VERNEED = 0x6FFFFFFE
DT_VERNEEDNUM = 0x6FFFFFFF

DF_1_PIE = 0x08000000

REMOVE_TAGS = {
    DT_RUNPATH: "DT_RUNPATH",
    DT_GNU_HASH: "DT_GNU_HASH",
    DT_VERNEED: "DT_VERNEED",
    DT_VERNEEDNUM: "DT_VERNEEDNUM",
}


def output_value(subject: str, value: str, *, stream=sys.stdout) -> None:
    print(f"{subject}: {value}", file=stream)


class ProjectArgumentParser(argparse.ArgumentParser):
    def error(self, message: str) -> None:
        self.print_usage(sys.stderr)
        output_value("argument error", message, stream=sys.stderr)
        raise SystemExit(2)


def parse_args() -> argparse.Namespace:
    parser = ProjectArgumentParser(
        description="patch elf tags",
        add_help=False,
    )
    parser._positionals.title = "arg"
    parser._optionals.title = "opt"
    parser.add_argument("-h", "--help", action="help", help="help")
    parser.add_argument("paths", nargs="+", help="path")
    return parser.parse_args()


def patch_elf_file(path: pathlib.Path) -> bool:
    data = bytearray(path.read_bytes())
    if data[:4] != ELF_MAGIC:
        raise ValueError("not elf")
    if data[4] != ELFCLASS64:
        raise ValueError("elf64 only")
    if data[5] != ELFDATA2LSB:
        raise ValueError("lsb only")

    e_phoff = struct.unpack_from("<Q", data, 32)[0]
    e_phentsize = struct.unpack_from("<H", data, 54)[0]
    e_phnum = struct.unpack_from("<H", data, 56)[0]

    dynamic_offset = None
    dynamic_size = None
    for idx in range(e_phnum):
        off = e_phoff + idx * e_phentsize
        p_type = struct.unpack_from("<I", data, off)[0]
        if p_type != PT_DYNAMIC:
            continue
        dynamic_offset = struct.unpack_from("<Q", data, off + 8)[0]
        dynamic_size = struct.unpack_from("<Q", data, off + 32)[0]
        break

    if dynamic_offset is None or dynamic_size is None:
        raise ValueError("pt_dynamic miss")

    slot_count = dynamic_size // 16
    entries = []
    null_index = None
    for idx in range(slot_count):
        tag, value = struct.unpack_from("<QQ", data, dynamic_offset + idx * 16)
        entries.append((tag, value))
        if tag == DT_NULL:
            null_index = idx
            break

    if null_index is None:
        raise ValueError("dt_null miss")

    updated = []
    changed = False
    for tag, value in entries[:null_index]:
        if tag in REMOVE_TAGS:
            changed = True
            continue
        if tag == DT_FLAGS_1:
            patched_value = value & ~DF_1_PIE
            if patched_value != value:
                changed = True
            updated.append((tag, patched_value))
            continue
        updated.append((tag, value))

    if not changed:
        return False

    updated.append((DT_NULL, 0))
    while len(updated) < slot_count:
        updated.append((DT_NULL, 0))

    for idx, (tag, value) in enumerate(updated):
        struct.pack_into("<QQ", data, dynamic_offset + idx * 16, tag, value)

    path.write_bytes(data)
    return True


def main() -> int:
    args = parse_args()
    for raw_path in args.paths:
        path = pathlib.Path(raw_path)
        try:
            changed = patch_elf_file(path)
        except Exception as exc:
            output_value("elf patch failed", f"{path}: {exc}", stream=sys.stderr)
            return 1
        subject = "elf patched" if changed else "elf kept"
        output_value(subject, str(path))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
