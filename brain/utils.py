__author__ = 'sam.royston'
import csv
import json
import uuid
from os import listdir
from os.path import isfile, join
from constants import *
import numpy as np
from sklearn.preprocessing import StandardScaler


class Ingestor(object):
    def __init__(self, action_memory=3, position_memory=2):
        self.action_memory = action_memory
        self.position_memory = position_memory
        self.data = []

    def vectorize_json(self, js_dict, with_onehot=False):
        """
            [id, a0, a1, ..., x, y, x0, y0, ...]

        @param js_dict:
        @param with_onehot:
        @return:
        """
        if with_onehot:
            x = [0] * 6
            x[js_dict["id"]] = 1

        else:
            x = [js_dict["id"]]
        for i in xrange(0, self.action_memory):
            if js_dict["a" + str(self.action_memory - (i + 1))] > 3:
                js_dict["a" + str(self.action_memory - (i + 1))] = 3
            x += [js_dict["a" + str(self.action_memory - (i + 1))]]
        x += [js_dict["x"], js_dict["y"]]
        #
        for i in xrange(0, self.position_memory):
            x += [js_dict["x" + str(i)]]
            x += [js_dict["y" + str(i)]]

        return x

    def concat_in_directory(self, dir_name=DATA_DIR, separate=False):
        files = [join(dir_name, f) for f in listdir(dir_name) if (isfile(join(dir_name, f)) and f[-5:] == ".json")]
        output = []
        for f in files:
            if separate:
                output.append(self.vectorize_saved_json(f))
            else:
                output += self.vectorize_saved_json(f)
        self.data = output
        return output

    def vectorize_json_array(self, js_arr):
        output = [self.vectorize_json(o) for o in js_arr]
        self.data = output
        return output

    def vectorize_saved_csv(self, fn):
        with open(fn) as f:
            reader = csv.DictReader(f)
            output = self.vectorize_json(reader)
            self.data = output
            return output

    def vectorize_saved_json(self, fn):
        with open(fn) as f:
            js_array = json.load(f)
            output = self.vectorize_json_array(js_array)
            self.data = output
            return output

    def xy_conversion(self):
        a = np.array(self.data)
        np.random.shuffle(a)
        print(np.shape(a))
        X = a[:, :-1]
        X = StandardScaler().fit_transform(X)
        Y = a[:, -1]
        return X, Y


class Session(object):
    def __init__(self):
        fn = str(uuid.uuid4()[0:6])