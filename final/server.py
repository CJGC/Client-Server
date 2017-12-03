import zmq
import time
import networkx as nx
import sys

port = 7777
context = zmq.Context()
socket = context.socket(zmq.REP)
socket.bind("tcp://*:%s" % port)

Ring = nx.Graph()

while True:
	request = socket.recv_multipart()
	reply = socket.send_multipart(['OK'])
	#print request

	if request[0] == 'add new node':
		print "dibujar nodo"
	elif request[0] == 'delete node':
		print "eliminar nodo"
	elif request[0] == 'upgrade node':
		print "actualizar nodo"
	elif request[0] == "upgrade node's finTbl":
		print "actualizar nodos finger table"

	