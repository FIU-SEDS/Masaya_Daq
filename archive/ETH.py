import socket
import time

UDP_IP_REMOTE = "192.168.1.200"  # CH9121 module IP
UDP_PORT_SEND = 2000             # CH9121 local port (it listens here)
UDP_PORT_RECV = 5005             # Your PC port (replies come back here)

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("", UDP_PORT_RECV))   # Bind to receive replies on port 5005
sock.settimeout(1.0)

while True:
    print("Sending Open...")
    sock.sendto(b"O", (UDP_IP_REMOTE, UDP_PORT_SEND))

    try:
        data, addr = sock.recvfrom(1024)
        print(f"Response: {data.decode()}")
    except socket.timeout:
        print("No response from STM32")

    time.sleep(1)

    print("Sending Close...")
    sock.sendto(b"C", (UDP_IP_REMOTE, UDP_PORT_SEND))

    try:
        data, addr = sock.recvfrom(1024)
        print(f"Response: {data.decode()}")
    except socket.timeout:
        print("No response from STM32")

    time.sleep(1)