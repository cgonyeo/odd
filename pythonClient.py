#client example
#import socket
#client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#client_socket.connect(('localhost', 3357))
#print "Client started..."
#while 1:
#	print "loop"
#	client_socket.sendall("Hello, World!")
#	print "sent"
#	data = client_socket.recv(512)
#	if ( data == 'q' or data == 'Q'):
#		client_socket.close()
#		break;
#	else:
#		print "RECIEVED:" , data
#		data = raw_input ( "SEND( TYPE q or Q to Quit):" )
#		if (data <> 'Q' and data <> 'q'):
#			client_socket.send(data)
#		else:
#			client_socket.send(data)
#			client_socket.close()
#			break;

import socket
import sys

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Connect the socket to the port where the server is listening
server_address = ('localhost', 3357)
print >>sys.stderr, 'connecting to %s port %s' % server_address
sock.connect(server_address)

try:
    
    # Send data
    sock.sendall("cyloneye 0.4 1.5 10 42 42 add ")
    #sock.sendall("remove 0 ")
    #sock.sendall("time ");

    # Look for the response
    #data = sock.recv(4096)
    #print >>sys.stderr, 'received "%s"' % data

finally:
    print >>sys.stderr, 'closing socket'
    sock.close()
