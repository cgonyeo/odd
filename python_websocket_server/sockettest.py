import socket
import sys
from time import sleep

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_address = ('127.0.0.1', 10001)
print('connecting to localhost at port 10001...')
sock.connect(server_address)
print('connected')
sock.sendall(('Hello, World!' + '\0').encode(encoding="ascii", errors="strict"))
sleep(0.5)
sock.sendall(('My name is Derek' + '\0').encode(encoding="ascii", errors="strict"))
sleep(0.5)
sock.sendall(('Purple' + '\0').encode(encoding="ascii", errors="strict"))
