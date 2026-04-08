import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("", 5005))
sock.settimeout(2.0)

# Build a valid frame: SEV01F (ID=0), CMD_OPEN_FAST (0x01), checksum = 0^1 = 1
frame = bytes([0x00, 0x01, 0x01])
sock.sendto(frame, ("192.168.1.200", 2000))

try:
    data, _ = sock.recvfrom(1024)
    print(f"Got {len(data)} bytes: {list(data)}")
except socket.timeout:
    print("No telemetry received")