"""Bounded USB smoke or energy-disabled capture; closes COM, never writes flash."""
import argparse
import re
import time

import serial


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", required=True)
    parser.add_argument("--profile", choices=("smoke", "energy-disabled", "led26", "water-disabled", "float27", "soil34", "water-monitor"), default="smoke")
    parser.add_argument("--seconds", type=float, default=12)
    parser.add_argument("--reset", action="store_true", help="Reset using RTS/EN; no flash writes")
    args = parser.parse_args()
    if not 3 <= args.seconds <= 60:
        parser.error("--seconds must be between 3 and 60")
    boot_seen = False
    heartbeats = 0
    disabled_confirmed = True
    marker = "CIDADE_VIVA_USB_SMOKE v1" if args.profile == "smoke" else "CIDADE_VIVA_ENERGY v0.1.0-disabled"
    heartbeat = "USB_SMOKE_ALIVE uptimeMs=" if args.profile == "smoke" else "ENERGY_ALIVE uptimeMs="
    if args.profile == "led26":
        marker, heartbeat = "CIDADE_VIVA_ENERGY_LED26_TEST v1", "LED26_REQUEST uptimeMs="
    elif args.profile == "water-disabled":
        marker, heartbeat = "CIDADE_VIVA_WATER v0.1.0-disabled", "WATER_ALIVE uptimeMs="
    elif args.profile == "float27":
        marker, heartbeat = "CIDADE_VIVA_WATER_FLOAT27_TEST v1", "FLOAT27 uptimeMs="
    elif args.profile == "soil34":
        marker, heartbeat = "CIDADE_VIVA_WATER_SOIL34_TEST v1", "SOIL34 uptimeMs="
    elif args.profile == "water-monitor":
        marker, heartbeat = "CIDADE_VIVA_WATER v0.2.0-pump-disabled", "WATER_STATUS uptimeMs="
    contact_states = set()
    soil_values = []
    high_seen = low_seen = False
    port = serial.Serial(port=None, baudrate=115200, timeout=0.25)
    port.dtr = False
    port.rts = False
    port.port = args.port
    with port:
        if args.reset:
            port.rts = True
            time.sleep(0.1)
            port.rts = False
        deadline = time.monotonic() + args.seconds
        while time.monotonic() < deadline:
            line = port.readline().decode("utf-8", errors="replace").strip()
            if not line:
                continue
            print(line, flush=True)
            boot_seen |= marker in line
            if line.startswith(heartbeat):
                heartbeats += 1
                if args.profile == "energy-disabled":
                    disabled_confirmed &= "externalOutputs=DISABLED" in line and "measurements=NONE" in line
                elif args.profile == "led26":
                    disabled_confirmed &= "gpio=26" in line and "physicalFeedback=NONE" in line
                    high_seen |= "level=HIGH" in line
                    low_seen |= "level=LOW" in line
                elif args.profile == "water-disabled":
                    disabled_confirmed &= all(value in line for value in (
                        "pumpRequested=OFF", "pumpOutput=DISABLED", "reservoir=UNKNOWN",
                        "state=FAULT_SENSOR", "measurements=NONE"))
                elif args.profile == "float27":
                    disabled_confirmed &= all(value in line for value in (
                        "gpio=27", "reservoir=UNKNOWN", "pumpOutput=DISABLED"))
                    state = line.split("contact=", 1)[-1].split()[0]
                    disabled_confirmed &= state in ("OPEN", "CLOSED", "SETTLING")
                    contact_states.add(state)
                elif args.profile == "soil34":
                    disabled_confirmed &= all(value in line for value in (
                        "gpio=34", "unit=ADC_COUNT", "calibration=NONE",
                        "humidityPct=UNAVAILABLE", "pumpOutput=DISABLED"))
                    match = re.search(r"\braw=(\d+)\b", line)
                    if match and 0 <= int(match[1]) <= 4095:
                        soil_values.append(int(match[1]))
                    else:
                        disabled_confirmed = False
                elif args.profile == "water-monitor":
                    disabled_confirmed &= all(value in line for value in (
                        "pumpRequested=OFF", "pumpOutput=DISABLED", "mode=MANUAL",
                        "calibration=NONE", "humidityPct=UNAVAILABLE"))
                    expected = {"PRESENT": "IDLE", "LOW": "BLOCKED_LOW_WATER", "UNKNOWN": "FAULT_SENSOR"}
                    level = re.search(r"\breservoir=(\w+)\b", line)
                    state = re.search(r"\bstate=(\w+)\b", line)
                    disabled_confirmed &= bool(level and state and expected.get(level[1]) == state[1])
                    raw = re.search(r"\bsoilRaw=(\d+)\b", line)
                    disabled_confirmed &= bool(raw and 0 <= int(raw[1]) <= 4095)
    if args.profile == "led26":
        disabled_confirmed &= high_seen and low_seen
    if args.profile == "float27":
        disabled_confirmed &= bool(contact_states & {"OPEN", "CLOSED"})
        print("FLOAT_CONTACT_STATES " + ",".join(sorted(contact_states)))
    if args.profile == "soil34":
        disabled_confirmed &= len(soil_values) >= 2
        if soil_values:
            print(f"SOIL_ADC_RANGE min={min(soil_values)} max={max(soil_values)} samples={len(soil_values)}")
    print(f"CAPTURE_RESULT profile={args.profile} boot_marker={boot_seen} messages={heartbeats} profile_status_ok={disabled_confirmed}")
    return 0 if heartbeats >= 2 and (boot_seen or not args.reset) and disabled_confirmed else 1


if __name__ == "__main__":
    raise SystemExit(main())
