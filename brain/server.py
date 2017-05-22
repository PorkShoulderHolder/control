__author__ = 'sam.royston'
import SocketServer
import json
from constants import *
from q_network import learner
import signal
import sys

session_training_data = []


def on_exit(sig, frame):
    with open("data.json", "w+") as f:
        print("saving data")
        json.dump(session_training_data, f)
    sys.exit(0)

class Server(SocketServer.BaseRequestHandler):
    """
    designed to handle connections from a single ./control process
    """

    def handle(self):
        while True:
            data = self.request.recv(1024)
            if data == '':
                break
            data = data.strip()
            data = json.loads(data)
            session_training_data.append(data)
            if "completed" in data:
                learner.iterate(data, completed=True)
                self.request.sendall("ok")
            else:
                action = learner.iterate(data)
                self.request.sendall(str(action))


if __name__ == "__main__":
    signal.signal(signal.SIGINT, on_exit)

    HOST, PORT = "0.0.0.0", 9999
    server = SocketServer.TCPServer((HOST, PORT), Server)
    server.serve_forever()