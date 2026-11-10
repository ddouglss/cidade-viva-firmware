"""Provision one telemetry-only ESP32 over USB; credentials never appear in CLI args."""

import argparse
import getpass
import ipaddress
import os
import sys
import time

import serial


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", required=True, help="Serial port of the selected board, e.g. COM5")
    parser.add_argument("--device-id", choices=("esp32-energy", "esp32-water"), required=True)
    parser.add_argument("--broker-ip", required=True, help="Private IPv4 of the local VPN gateway")
    parser.add_argument("--from-env", action="store_true", help="Read credentials from short-lived environment variables")
    args = parser.parse_args()
    address = ipaddress.ip_address(args.broker_ip)
    private_lan = any(address in ipaddress.ip_network(block) for block in (
        "10.0.0.0/8", "172.16.0.0/12", "192.168.0.0/16"))
    if address.version != 4 or not private_lan:
        parser.error("broker-ip must be a reachable private LAN IPv4 address")

    if args.from_env:
        ssid = os.environ.pop("CV_WIFI_SSID", "")
        wifi_password = os.environ.pop("CV_WIFI_PASSWORD", "")
        mqtt_password = os.environ.pop("CV_MQTT_PASSWORD", "")
    else:
        ssid = input("Wi-Fi 2.4 GHz SSID: ")
        wifi_password = getpass.getpass("Wi-Fi password: ")
        mqtt_password = getpass.getpass(f"MQTT password for {args.device_id}: ")
    if not 1 <= len(ssid.encode()) <= 32 or "\n" in ssid or "\r" in ssid:
        parser.error("SSID must be 1-32 bytes without newline")
    if len(wifi_password.encode()) >= 64 or "\n" in wifi_password or "\r" in wifi_password:
        parser.error("Wi-Fi password must be under 64 bytes without newline")
    if not 1 <= len(mqtt_password.encode()) < 65 or "\n" in mqtt_password or "\r" in mqtt_password:
        parser.error("MQTT password must be 1-64 bytes without newline")

    fields = ("CVNET1", ssid, wifi_password, args.broker_ip, args.device_id, mqtt_password)
    with serial.Serial(args.port, 115200, timeout=1) as board:
        time.sleep(2)  # Opening the serial port resets ESP32.
        for field in fields:
            board.write(field.encode() + b"\n")
            board.flush()
            time.sleep(0.05)
        deadline = time.monotonic() + 15
        while time.monotonic() < deadline:
            line = board.readline().decode(errors="replace").strip()
            if line.startswith("CV_MQTT_CONFIGURED"):
                print(f"Configured {args.device_id} on {args.port}; credentials stored on the board.")
                return 0
            if line.startswith(("CV_MQTT_CONFIG_INVALID", "CV_MQTT_CONFIG_FAILED")):
                print(line, file=sys.stderr)
                return 1
    print("No configuration acknowledgement from the board.", file=sys.stderr)
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
