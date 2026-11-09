"""Send credentials or scan networks over USB on a Cidade Viva Wi-Fi bench profile."""

import argparse
from getpass import GetPassWarning, getpass
import json
from time import monotonic
import warnings

import serial


def open_serial(port_name: str) -> serial.Serial:
    port = serial.Serial(baudrate=115200, timeout=0.5, write_timeout=2)
    port.dtr = False
    port.rts = False
    port.port = port_name
    port.open()
    return port


def cmd_scan(port_name: str) -> int:
    try:
        with open_serial(port_name) as port:
            port.reset_input_buffer()
            port.write(b"__WIFI_SCAN__\n")
            networks = []
            in_scan = False
            deadline = monotonic() + 15
            while monotonic() < deadline:
                raw = port.readline().decode("utf-8", errors="replace").strip()
                if not raw:
                    continue
                if raw.startswith("WIFI_SCAN_BEGIN"):
                    in_scan = True
                    continue
                if in_scan and raw.startswith("WIFI_SCAN_ITEM "):
                    payload = raw[len("WIFI_SCAN_ITEM ") :]
                    parts = dict(
                        item.split("=", 1) for item in payload.split("|") if "=" in item
                    )
                    networks.append(
                        {
                            "ssid": parts.get("ssid", ""),
                            "rssi": int(parts.get("rssi", "-90")),
                            "channel": int(parts.get("ch", "1")),
                            "auth": parts.get("auth", "SECURED"),
                        }
                    )
                if raw == "WIFI_SCAN_END":
                    break
            print(json.dumps({"port": port_name, "networks": networks}))
            return 0
    except serial.SerialException as error:
        print(json.dumps({"error": str(error), "networks": []}))
        return 1


def cmd_status(port_name: str) -> int:
    try:
        with open_serial(port_name) as port:
            port.reset_input_buffer()
            port.write(b"__WIFI_STATUS__\n")
            device_id = None
            deadline = monotonic() + 4
            while monotonic() < deadline:
                raw = port.readline().decode("utf-8", errors="replace").strip()
                if not raw:
                    continue
                if "deviceId=" in raw and not raw.startswith("WIFI_STATUS "):
                    for token in raw.split():
                        if token.startswith("deviceId="):
                            device_id = token.split("=", 1)[1]
                if raw.startswith("WIFI_STATUS "):
                    fields = dict(
                        item.split("=", 1)
                        for item in raw[len("WIFI_STATUS ") :].split()
                        if "=" in item
                    )
                    print(
                        json.dumps(
                            {
                                "port": port_name,
                                "deviceId": fields.get("deviceId") or device_id or "esp32",
                                "phase": fields.get("phase", "WAITING"),
                                "connected": fields.get("connected") == "true",
                                "ssid": fields.get("ssid", ""),
                                "ip": fields.get("ip", "0.0.0.0"),
                                "rssi": int(fields.get("rssi", "0")),
                                "apSsid": fields.get("apSsid", ""),
                                "apIp": fields.get("apIp", "0.0.0.0"),
                            }
                        )
                    )
                    return 0
                if raw == "WIFI_TEST_WAITING":
                    print(
                        json.dumps(
                            {
                                "port": port_name,
                                "deviceId": device_id or "esp32",
                                "phase": "WAITING",
                                "connected": False,
                                "ssid": "",
                                "ip": "0.0.0.0",
                                "rssi": 0,
                            }
                        )
                    )
                    return 0
            print(json.dumps({"port": port_name, "error": "TIMEOUT"}))
            return 1
    except serial.SerialException as error:
        print(json.dumps({"port": port_name, "error": str(error)}))
        return 1


def connect_credentials(port_name: str, ssid_bytes: bytes, password_bytes: bytes, json_output: bool = False) -> int:
    if not (1 <= len(ssid_bytes) <= 32 and (len(password_bytes) == 0 or 8 <= len(password_bytes) <= 63)) or any(
        character in ssid_bytes or character in password_bytes for character in (b"\n", b"\r", b"\0")
    ):
        if json_output:
            print(json.dumps({"connected": False, "error": "INVALID_INPUT"}))
        else:
            print("Informe um SSID de 1 a 32 bytes e uma senha de 8 a 63 bytes.")
        return 1

    try:
        with open_serial(port_name) as port:
            deadline = monotonic() + 10
            while monotonic() < deadline:
                line = port.readline().strip()
                if line == b"WIFI_TEST_WAITING" or b"ENERGY_ALIVE" in line or b"WATER_STATUS" in line:
                    break
            else:
                if json_output:
                    print(json.dumps({"connected": False, "error": "BOARD_NOT_RESPONDING"}))
                else:
                    print("Perfil de teste não respondeu. Confirme a gravação e pressione RST/EN.")
                return 1

            port.write(ssid_bytes + b"\n" + password_bytes + b"\n")
            deadline = monotonic() + 40
            while monotonic() < deadline:
                line = port.readline().strip()
                if line.startswith(b"WIFI_TEST_CONNECTED ip="):
                    ip = line.split(b"=", 1)[1].decode("ascii", errors="replace")
                    if json_output:
                        print(json.dumps({"connected": True, "ip": ip}))
                    else:
                        print(f"ESP32 conectado ao Wi-Fi. IP: {ip}")
                        print(f"Para testar o acesso pela rede do notebook: ping {ip}")
                    return 0
                if line == b"WIFI_TEST_CONNECTING" and not json_output:
                    print("Conectando...")
                if line in (b"WIFI_TEST_TIMEOUT", b"WIFI_TEST_INVALID_INPUT"):
                    if json_output:
                        print(json.dumps({"connected": False, "error": line.decode("ascii", errors="replace")}))
                    break
    except serial.SerialException as error:
        if json_output:
            print(json.dumps({"connected": False, "error": str(error)}))
        else:
            print(f"Falha na porta serial: {error}")
        return 1

    if not json_output:
        print("Sem IP confirmado. Confira a rede de 2,4 GHz e reinicie a placa para tentar de novo.")
    return 1


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", required=True)
    parser.add_argument("--scan", action="store_true", help="Scan 2.4 GHz networks on the ESP32 and output JSON.")
    parser.add_argument("--status", action="store_true", help="Read current Wi-Fi status from the ESP32 as JSON.")
    parser.add_argument("--ssid", help="SSID to connect (non-interactive JSON mode).")
    parser.add_argument("--password", default="", help="Wi-Fi password (non-interactive JSON mode).")
    args = parser.parse_args()

    if args.scan:
        return cmd_scan(args.port)
    if args.status:
        return cmd_status(args.port)
    if args.ssid is not None:
        return connect_credentials(args.port, args.ssid.encode("utf-8"), args.password.encode("utf-8"), json_output=True)

    ssid = input("Nome do Wi-Fi (SSID): ").encode("utf-8")
    with warnings.catch_warnings():
        warnings.simplefilter("error", GetPassWarning)
        try:
            password = getpass("Senha do Wi-Fi (oculta): ").encode("utf-8")
        except GetPassWarning:
            print("Abra um terminal interativo que suporte entrada de senha oculta.")
            return 1
    return connect_credentials(args.port, ssid, password, json_output=False)


if __name__ == "__main__":
    raise SystemExit(main())
