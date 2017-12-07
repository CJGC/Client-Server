import zmq
import sys
import os.path
import thread
import pygame
from pygraphviz import *
from graphviz import Digraph

ip = "*"

def drawGraph(info, window, clock):
	#dot = Digraph(format='png')
	dot = AGraph(directed=True,layout='circo')

	for key in info:
		dot.add_node(key)
		after = info[key]
		dot.add_node(after)
		dot.add_edge(key, after)
	
	white = (255,255,255)
	dot.write("ring")
	dot.draw('ring.png',prog='circo')
	#dot.render("ring", view=False)
	imageGraph = pygame.image.load('ring.png')
	size = imageGraph.get_rect().size
	imageGraph = imageGraph.convert()
	window.fill((white))
	window.blit(imageGraph,(0,0))
	pygame.display.update()
	pygame.display.flip()
	clock.tick(25)
	
def main():
	
	port = 7777
	context = zmq.Context()
	socket = context.socket(zmq.REP)
	socket.bind("tcp://"+ip+":%s" % port)

	pygame.init()
	clock = pygame.time.Clock()
	pygame.mouse.set_visible(True)
	pygame.display.set_caption("Torrent RING")
	window = pygame.display.set_mode((400,300),0,32)


	info = {}

	while True:
		events = pygame.event.get()
		for event in events:
			if event.type == pygame.QUIT:
				break

		request = socket.recv_multipart()
		reply = socket.send_multipart(['OK'])

		#print request
		
		if request[0] == 'add new node':
			nodeId = request[1][0:5]
			afterId = request[2][0:5]
			beforeId = request[3][0:5]
			keys = request[4]
			idsFinTabl = request[5]
			finTabl = idsFinTabl.split(" ")
			info[nodeId] = afterId
			drawGraph(info,window,clock)
			print "Operation = " +request[0] + "\n"
			print "NodeID: " + nodeId +  "-> " + "AfterID: " + afterId \
				+ "-> " + "BeforeID: " + beforeId + "-> "+ "Domain keys: " + keys\
				+ "-> " + "IdsFinTabl: "+ idsFinTabl + "\n\n"
		elif request[0] == 'delete node':
			nodeId =  request[1][0:5]
			del info[nodeId]
			afterId = request[2]
			beforeId = request[3]
			info[beforeId] = afterId
			drawGraph(info,window,clock)
			print "Operation = " + request[0] + "\n"
			print "NodeID: " + nodeId + "\n\n"
		elif request[0] == 'upgrade node':
			nodeId = request[1][0:5]
			afterId = request[2][0:5]
			keys = request[4]
			info[nodeId] = afterId
			drawGraph(info,window,clock)
			print "Operation = "+request[0] + "\n"
			print "NodeID: " + nodeId + "-> " + "New AfterID: " + afterId \
				+ "-> " + "NewDomainKeys:" + keys + "\n\n"
		elif request[0] == "upgrade node's finTbl":
			nodeId = request[1][0:5]
			idsFinTabl = request[5]
			print "Operation = "+ request[0] + "\n"
			print "NodeID: " + nodeId + "-> " + "NewIdsFinTabl: " + idsFinTabl\
				+ "\n\n"

if __name__ == '__main__':
	main()


	

	
