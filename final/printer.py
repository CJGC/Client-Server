import zmq
import sys
import os.path
import thread
import pygame
from graphviz import Digraph

def drawGraph(info, window, clock):
	dot = Digraph(format='png')

	for key in info:
		node = key[0:5]
		dot.node(node)
		after = info[key][0:5]
		dot.node(after)
		dot.edge(node, after)
	
	white = (255,255,255)
	dot.render("ring", view=False)
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
	socket.bind("tcp://*:%s" % port)

	pygame.init()
	clock = pygame.time.Clock()
	pygame.mouse.set_visible(True)
	pygame.display.set_caption("Torrent RING")
	window = pygame.display.set_mode((300,300),0,32)


	info = {}

	while True:
		events = pygame.event.get()
		for event in events:
			if event.type == pygame.QUIT:
				break

		request = socket.recv_multipart()
		reply = socket.send_multipart(['OK'])
		
		if request[0] == 'add new node':
			nodeID = request[1]
			afterID = request[2]
			info[nodeID] = afterID
			drawGraph(info,window,clock)
			print "NodeID: " + nodeID +  "-> " + "SuccesorID: " + afterID 
		elif request[0] == 'delete node':
			nodeID =  request[1]
			del info[nodeID]
			afterID = request[2]
			beforeID = request[3]
			info[beforeID] = afterID
			drawGraph(info,window,clock)
		elif request[0] == 'upgrade node':
			nodeID = request[1]
			afterID = request[2]
			info[nodeID] = afterID
			drawGraph(info,window,clock)
		elif request[0] == "upgrade node's finTbl":
			print ""

if __name__ == '__main__':
	main()


	

	
