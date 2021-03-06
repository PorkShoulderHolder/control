__author__ = 'sam.royston'
import os

MSG_TYPE_KEY = "msg_type"
REWARD_MSG = "reward"
STATE_MSG = "state"
INIT_MSG = "init"
SVM = "svm"

DATA_DIR = os.path.dirname(os.path.abspath(__file__)) + "/archive"
CLASSIFIER_FN = DATA_DIR + "/best_sarsa"
FORWARD_NN = "forward_nn"
BATCH_SIZE = 1000
GAMMA = 0.999
EPS_START = 0.9
EPS_END = 0.15
EPS_DECAY = 200
MODEL_FN = DATA_DIR + "/q_model"
