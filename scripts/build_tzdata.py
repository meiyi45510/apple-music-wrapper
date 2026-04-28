#!/usr/bin/env python3

import argparse
import pathlib
import struct
import sys

MAGIC_PREFIX = b"tzdata"
TZDATA_VERSION = b"2026a"
INDEX_NAME_SIZE = 40
INDEX_ENTRY_SIZE = INDEX_NAME_SIZE + 12


def output_value(subject: str, value: str, *, stream=sys.stdout) -> None:
    print(f"{subject}: {value}", file=stream)


class ProjectArgumentParser(argparse.ArgumentParser):
    def error(self, message: str) -> None:
        self.print_usage(sys.stderr)
        output_value("argument error", message, stream=sys.stderr)
        raise SystemExit(2)


def build_entry(
    zone_id: str, offset: int, length: int, raw_utc_offset: int
) -> bytes:
    encoded_name = zone_id.encode("ascii")
    if len(encoded_name) >= INDEX_NAME_SIZE:
        raise ValueError(f"zone id long: {zone_id}")
    name_field = encoded_name + b"\0" + (
        b"\0" * (INDEX_NAME_SIZE - len(encoded_name) - 1)
    )
    return name_field + struct.pack(">iii", offset, length, raw_utc_offset)


def parse_args() -> argparse.Namespace:
    parser = ProjectArgumentParser(
        description="build tzdata",
        add_help=False,
    )
    parser._positionals.title = "arg"
    parser._optionals.title = "opt"
    parser.add_argument("-h", "--help", action="help", help="help")
    parser.add_argument("output", help="path")
    return parser.parse_args()


def main() -> int:
    args = parse_args()

    zone_sources = [
        ("GMT", pathlib.Path("/usr/share/zoneinfo/GMT"), 0),
        ("posixrules", pathlib.Path("/usr/share/zoneinfo/posixrules"), 0),
    ]

    for _, path, _ in zone_sources:
        if not path.exists():
            output_value(
                "tzdata build failed",
                f"zoneinfo missing: {path}",
                stream=sys.stderr,
            )
            return 1

    output_path = pathlib.Path(args.output)
    try:
        output_path.parent.mkdir(parents=True, exist_ok=True)
    except OSError as exc:
        output_value("tzdata build failed", str(exc), stream=sys.stderr)
        return 1

    zone_blobs = []
    current_offset = 0
    for zone_id, source_path, raw_offset in zone_sources:
        data = source_path.read_bytes()
        if len(data) < 44:
            output_value(
                "tzdata build failed",
                f"tzif invalid: {source_path}",
                stream=sys.stderr,
            )
            return 1
        zone_blobs.append((zone_id, current_offset, len(data), raw_offset, data))
        current_offset += len(data)

    zone_blobs.sort(key=lambda item: item[0])

    header_size = 12 + 4 + 4 + 4
    index_size = len(zone_blobs) * INDEX_ENTRY_SIZE
    data_offset = header_size + index_size

    body = bytearray()
    data_blob = bytearray()
    for zone_id, relative_offset, length, raw_offset, data in zone_blobs:
        body.extend(build_entry(zone_id, relative_offset, length, raw_offset))
        data_blob.extend(data)

    zone_tab_offset = data_offset + len(data_blob)
    header = MAGIC_PREFIX + TZDATA_VERSION + b"\0" + struct.pack(
        ">iii", header_size, data_offset, zone_tab_offset
    )

    try:
        output_path.write_bytes(header + body + data_blob)
    except OSError as exc:
        output_value("tzdata build failed", str(exc), stream=sys.stderr)
        return 1

    output_value("tzdata saved", f"{output_path} {output_path.stat().st_size}b")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
