__author__ = 'sam.royston'
import SocketServer
import json
from constants import *
from q_network import learner


class Server(SocketServer.BaseRequestHandler):
    """
    designed to handle connections from a single ./control process
    """

    def handle(self):
        data = self.request.recv(1024).strip()
        data = json.loads(data)
        if data[MSG_TYPE_KEY] == STATE_MSG:
            action = learner.iterate(data["state"])
            self.request.sendall(action)
        elif data[MSG_TYPE_KEY] == INIT_MSG:
            pass

if __name__ == "__main__":
    HOST, PORT = "0.0.0.0", 9999
    server = SocketServer.TCPServer((HOST, PORT), Server)
    server.serve_forever()