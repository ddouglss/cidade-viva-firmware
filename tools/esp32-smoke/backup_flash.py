"""Read the observed 4 MiB ESP32 flash in bounded ROM-only chunks; never writes flash."""
import argparse
from pathlib import Path
import subprocess
import sys


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", required=True)
    parser.add_argument("--output-dir", required=True)
    args = parser.parse_args()
    target = Path(args.output_dir)
    target.mkdir(parents=True, exist_ok=False)
    esptool = Path.home() / ".platformio/packages/tool-esptoolpy/esptool.py"
    size, block = 0x400000, 0x10000
    with (target / "flash-incomplete.tmp").open("xb") as combined:
        for offset in range(0, size, block):
            chunk = target / f"chunk-{offset:06x}.bin"
            success = False
            for attempt in range(1, 4):
                command = [sys.executable, str(esptool), "--port", args.port,
                           "--baud", "460800", "--no-stub", "read_flash",
                           hex(offset), hex(block), str(chunk)]
                try:
                    result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                            timeout=25, check=False)
                    (target / f"read-{offset:06x}-{attempt}.log").write_bytes(result.stdout)
                    success = result.returncode == 0 and chunk.is_file() and chunk.stat().st_size == block
                except subprocess.TimeoutExpired:
                    print(f"ROM read timeout offset={offset:#x} attempt={attempt}", flush=True)
                if success:
                    break
                print(f"ROM read retry offset={offset:#x} attempt={attempt}", flush=True)
            if not success:
                print(f"BACKUP_FAILED offset={offset:#x}; incomplete file is NOT a backup", flush=True)
                return 1
            combined.write(chunk.read_bytes())
            combined.flush()
            print(f"BACKUP_PROGRESS {offset + block}/{size}", flush=True)
    final = target / "flash-original.bin"
    (target / "flash-incomplete.tmp").rename(final)
    print(f"BACKUP_COMPLETE bytes={final.stat().st_size}", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
