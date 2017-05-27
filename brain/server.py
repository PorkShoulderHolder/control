__author__ = 'sam.royston'
import SocketServer
import json
from constants import *
import pickle
import signal
import sys
import datetime
from sklearn.preprocessing import normalize
from utils import Ingestor
from random import random, randint
session_training_data = []
f = open(CLASSIFIER_FN)
model = pickle.load(f)
try:
    fi = open(INGESTOR_FN)
    ingestor = pickle.load(fi)
    print("loaded saved ingestor")
except IOError:
    ingestor = Ingestor(5, 0)


def save():
    global session_training_data
    with open("{0}/training_data_{1}_{2}count.json".format(DATA_DIR,
                                                           len(session_training_data),
                                                           session_training_data[0]["id"]), "w+") as f:
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
            data = self.request.recv(1024)
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
                x = ingestor.vectorize_json(data)[:-1]
                # x = normalize(x)
                action = model.predict_proba(x)[0]
                r = random()
                print(action)
                ia = 0
                s = 0
                for i, a in enumerate(action):
                    s += a
                    if s > r:
                        ia = i
                        break
                if random() < 0.1:
                    ia = randint(0, 3)
                self.request.sendall(str(ia))


if __name__ == "__main__":
    signal.signal(signal.SIGINT, on_exit)

    HOST, PORT = "0.0.0.0", 9999
    server = SocketServer.TCPServer((HOST, PORT), Server)
    server.serve_forever()