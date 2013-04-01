import tornado.httpserver
import tornado.websocket
import tornado.ioloop
import tornado.web
import socket
import sys

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_address = ('localhost', 3357)
print 'connecting to localhost at port 3357...'
sock.connect(server_address)

class WSHandler(tornado.websocket.WebSocketHandler):
	def open(self):
		print 'new connection';
		self.write_message('Hello, World!');
	
	def on_message(self, message):
		print 'Message received: %s\n' % message
		try:
			sock.sendall(message + '\0')
			print 'Message sent\n'
			#data = sock.recv(4096)
			#print 'Server says: %s\n' % data
			#if data != "ok":
			#	self.write_message(data)
		finally:
			pass
	
	def on_close(self):
		print 'Connection closed'

application = tornado.web.Application([
	(r'/ws', WSHandler)
])

if  __name__ == "__main__":
	http_server = tornado.httpserver.HTTPServer(application)
	http_server.listen(8888)
	tornado.ioloop.IOLoop.instance().start()
