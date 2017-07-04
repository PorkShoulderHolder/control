__author__ = 'sam.royston'
import csv
import json
import uuid
from os import listdir
from os.path import isfile, join
from constants import *
import numpy as np
from sklearn.preprocessing import StandardScaler


def to_triple(x):
    mag = np.linalg.norm(x)
    unit_sphere = x / mag
    return [unit_sphere[0], unit_sphere[1], mag]


class Ingestor(object):
    def __init__(self, lag_frames=5, position_memory=2):
        self.position_memory = position_memory
        self.lag_frames = lag_frames
        self.data = []
        self.scaler = None

    def vectorize_json(self, js_dict, with_onehot=False):
        """
            [id, a0, a1, ..., x, y, x0, y0, ...]

        @param js_dict:
        @param with_onehot:
        @return:
        """
        x = []

        # observation window [location]
        for i in xrange(0, self.position_memory + self.lag_frames):
            ky = "y" + str(i)
            kx = "x" + str(i)
            ka = "a" + str(i)
            if kx not in js_dict or ky not in js_dict:
                return None
            if js_dict[ka] > 3:
                js_dict[ka] = 3
            x += [js_dict[kx]]
            x += [js_dict[ky]]
            x += [js_dict[ka]]

        x += [js_dict["id"]]
        if js_dict["id"] == -1 or js_dict["id"] == 0:
            return None
        return x

    def concat_in_directory(self, dir_name=DATA_DIR, separate=False):
        files = [join(dir_name, f) for f in listdir(dir_name) if (isfile(join(dir_name, f)) and f[-5:] == ".json")]
        output = []
        for f in files:
            print(f)
            if separate:
                output.append(self.vectorize_saved_json(f))
            else:
                output += self.vectorize_saved_json(f)
        self.data = output
        return output

    def vectorize_json_array(self, js_arr):

        output = filter(lambda x: x is not None, [self.vectorize_json(o) for o in js_arr])
        self.data = list(output)
        print("{} values filtered to give data array of length {}".format(len(js_arr) - len(self.data), len(self.data)))
        return output

    def vectorize_saved_csv(self, fn):
        with open(fn) as f:
            reader = csv.DictReader(f)
            output = self.vectorize_json(reader)
            self.data = output
            return output

    @staticmethod
    def add_device_encoding(X, labels, onehot=True):
        a = np.unique(labels)
        if onehot:
            onehot_arr = np.zeros((len(X), len(a)))
            for i, x in enumerate(a):
                idx = np.where(labels == x)
                onehot_arr[idx][:, i] = 1
            return np.concatenate([X, onehot_arr], axis=1)

        else:
            out = np.zeros((len(labels), 1))
            for i, x in enumerate(a):
                idx = np.where(labels == x)
                out[idx][:, 0] = i
            return np.concatenate([X, out], axis=1)


    def vectorize_saved_json(self, fn):
        with open(fn) as f:
            js_array = json.load(f)
            output = self.vectorize_json_array(js_array)
            self.data = output
            return output

    def xy_conversion(self, ex_data=None):

        if ex_data is not None:
            a = np.array(ex_data)
        else:
            a = np.array(self.data)
            np.random.shuffle(a)
        if self.scaler is None:
            self.scaler = StandardScaler()

        X = a[:, 3 * self.lag_frames: 3 * (self.lag_frames + self.position_memory)]
        Y = a[:, : 3 * self.position_memory]
        return X, Y


class Session(object):
    def __init__(self):
        fn = str(uuid.uuid4()[0:6])