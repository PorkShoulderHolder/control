__author__ = 'sam.royston'
import SocketServer
import json
from constants import *
import pickle
import signal
import sys
import numpy as np
import datetime
from utils import Ingestor
from random import random, randint
from q_network import Learner, Policy, ReplayMemory, softmax

session_training_data = []
f = open(CLASSIFIER_FN)
model = pickle.load(f)

ingestor = Ingestor(5, 1)


def save():
    global session_training_data
    with open("{0}/training_data_{1}count.json".format(DATA_DIR, len(session_training_data)), "w+") as f:
        print("saving data, {0}".format(len(session_training_data)))
        json.dump(session_training_data, f)
        session_training_data = []


def on_exit(sig, frame):
    save()
    sys.exit(0)


# def route_state(data, engine_type):
#     if engine_type == FORWARD_NN:
#         # action = learner.iterate(data)
#     elif engine_type == SVM:
#         pass


class Server(SocketServer.BaseRequestHandler):
    """
    designed to handle connections from a single ./control process
    """

    def handle(self):
        global session_training_data
        while True:
            data = self.request.recv(4096)
            if data == '':
                break
            data = data.strip()
            data = json.loads(data)
            if "training" in data and data["training"] == 1:
                session_training_data.append(data)
                self.request.sendall("ok")
            elif "completed" in data:
                save()
                self.request.sendall("ok")
            else:
                x = np.array(ingestor.vectorize_json(data))
                _, x = ingestor.xy_conversion([x])

                ai = 1 if x[0][0] > 0 else 2
                self.request.sendall(str(ai))




if __name__ == "__main__":
    signal.signal(signal.SIGINT, on_exit)

    HOST, PORT = "0.0.0.0", 9998
    server = SocketServer.TCPServer((HOST, PORT), Server)
    server.serve_forever()