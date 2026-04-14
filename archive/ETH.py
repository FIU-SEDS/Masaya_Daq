import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("0.0.0.0", 5005))
sock.settimeout(5.0)

print("Listening...")
while True:
    try:
        data, addr = sock.recvfrom(1024)
        print(f"Got {len(data)} bytes from {addr}: {data.hex()}")
    except socket.timeout:
        print("Timeout - nothing received")