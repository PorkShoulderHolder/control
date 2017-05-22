__author__ = 'sam.royston'
import SocketServer
import json
from constants import *
from q_network import learner
import signal
import sys
import datetime
session_training_data = []


def on_exit(sig, frame):
    with open("{0}/training_data_{1}count.json".format(DATA_DIR, len(session_training_data)), "w+") as f:
        print("saving data, {0}".format(len(session_training_data)))
        json.dump(session_training_data, f)
    sys.exit(0)


def route_state(data, engine_type):
    if engine_type == FORWARD_NN:
        action = learner.iterate(data)
    elif engine_type == SVM:
        pass


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
            if "training" in data and data["training"] == 1:
                session_training_data.append(data)
                self.request.sendall("ok")
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