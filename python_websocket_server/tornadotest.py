import tornado.httpserver
import tornado.websocket
import tornado.ioloop
import tornado.web
import socket
import sys

class WSHandler(tornado.websocket.WebSocketHandler):
	def open(self):
		print 'new connection';
	
	def on_message(self, message):
		print 'Message received: %s\n' % message
		try:
			sock.sendall(message + '\0')
			print 'Message sent\n'
			data = sock.recv(4096)
			print 'Server says: %s\n' % data
			if data != "ok":
				self.write_message(data)
		finally:
			if message == 'exit !':
				exit()
	
	def on_close(self):
		print 'Connection closed'

application = tornado.web.Application([
	(r'/ws', WSHandler)
], debug=True)

if  __name__ == "__main__":
	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	server_address = ('127.0.0.1', 10001)
	print 'connecting to localhost at port 10001...'
	sock.connect(server_address)
	print 'connected'

	print "Starting Tornado server"
	http_server = tornado.httpserver.HTTPServer(application)
	print "Tornado server initialized"
	http_server.listen(8888, '0.0.0.0')
	print "Tornado server listening"
	tornado.ioloop.IOLoop.instance().start()
	print "Tornado server started"
