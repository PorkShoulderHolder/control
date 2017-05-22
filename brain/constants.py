__author__ = 'sam.royston'
import os

MSG_TYPE_KEY = "msg_type"
REWARD_MSG = "reward"
STATE_MSG = "state"
INIT_MSG = "init"
SVM = "svm"

DATA_DIR = os.path.dirname(os.path.abspath(__file__)) + "/archive"
FORWARD_NN = "forward_nn"
BATCH_SIZE = 256
GAMMA = 0.999
EPS_START = 0.9
EPS_END = 0.15
EPS_DECAY = 200
MODEL_FN = "archive/q_model"