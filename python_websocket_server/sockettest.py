import socket
import sys
from time import sleep

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_address = ('127.0.0.1', 10001)
print('connecting to localhost at port 10001...')
sock.connect(server_address)
print('connected')
sock.sendall(('{ "action": "add", "animation": { "name": "cylonEye", "modifier": "add", "params": [ 0.5, 13 ], "colors": [ { "r": 1000, "g": 400, "b": 0 } ] } }').encode(encoding="ascii", errors="strict"))
