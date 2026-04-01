import socket

# BIND EXPLICITLY to the Ethernet Adapter IP
UDP_IP = "192.168.1.100" 
UDP_PORT = 5005

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
try:
    sock.bind((UDP_IP, UDP_PORT))
    print(f"Listening on {UDP_IP}:{UDP_PORT}")
except OSError as e:
    print(f"Error binding to IP: {e}")
    print("Check that your Ethernet adapter is actually assigned this IP!")

while True:
    data, addr = sock.recvfrom(1024)
    print(f"Received: {data.decode('utf-8', errors='ignore').strip()} from {addr}")