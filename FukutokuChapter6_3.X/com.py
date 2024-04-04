import sys
from pprint import pprint
import serial
import io


def list_ports():
    from serial.tools import list_ports

    for port in list_ports.comports():
        pprint(vars(port))


def detect_port():
    from serial.tools import list_ports

    for port in list_ports.comports():
        if "USB Serial Port" in port.description:
            return port.device


def receiver(device, baud=None):
    baud = baud or 9600
    with serial.Serial(device, baud, timeout=1) as ser:
        while True:
            print(ser.read_all().decode("utf-8", errors="ignore"), end="")


def sender(device, baud=None, end=None):
    baud = baud or 9600
    end = (end or "\n").encode("utf-8")
    with serial.Serial(device, baud, timeout=1) as ser:
        while True:
            data = input(" > ").encode("utf-8")
            ser.write(data)
            ser.write(end)


def main(argv):
    if len(argv) >= 1:
        command = argv.pop(0)
        if command == "list":
            list_ports()
            return
        if command == "recv":
            if len(argv) >= 1:
                device = argv.pop(0)
            else:
                device = detect_port()
            if len(argv) >= 1:
                baud = int(argv.pop(0))
            else:
                baud = None
            receiver(device, baud)
            return
        if command == "send":
            if len(argv) >= 1:
                device = argv.pop(0)
            else:
                device = detect_port()
            if len(argv) >= 1:
                baud = int(argv.pop(0))
            else:
                baud = None
            if len(argv) >= 1:
                end = argv.pop(0)
            else:
                end = None
            sender(device, baud, end)
    raise ValueError("Invalid command")


if __name__ == "__main__":
    main(sys.argv[1:])
