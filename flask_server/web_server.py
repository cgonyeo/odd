import socket
import sys 
from time import sleep
from flask import Flask

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_address = ('dgonyeoraspi.csh.rit.edu', 10001)

app = Flask(__name__)

@app.route('/')
def hello_world():
    return 'Hello World!'

if __name__ == '__main__':
    print('connecting')
    sock.connect(server_address)
    print('connected')
    app.run(host='0.0.0.0', debug=True)
