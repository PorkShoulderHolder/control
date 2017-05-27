__author__ = 'sam.royston'
import csv
import json
import uuid
from os import listdir
from os.path import isfile, join
from constants import *
import numpy as np
from sklearn.preprocessing import StandardScaler, normalize


class Ingestor(object):
    def __init__(self, action_memory=3, position_memory=2):
        self.action_memory = action_memory
        self.position_memory = position_memory
        self.data = []
        self.preprocessor = StandardScaler()

    def vectorize_json(self, js_dict):
        x = [0, 0, 0, 0, 0, 0]
        x[js_dict["id"]] = 1
        x0 = js_dict["x"]
        y0 = js_dict["y"]
        if x0 == 0 and y0 == 0:
            return
        x += [x0, y0]
        for i in xrange(0, self.position_memory):
            x_i = js_dict["x" + str(i)]
            y_i = js_dict["y" + str(i)]
            if x_i == 0 and y_i == 0:
                return
            x += [x_i]
            x += [y_i]
        for i in xrange(0, self.action_memory):
            if js_dict["a" + str(self.action_memory - (i + 1))] > 3:
                js_dict["a" + str(self.action_memory - (i + 1))] = 3
            x += [js_dict["a" + str(self.action_memory - (i + 1))]]

        return x

    def concat_in_directory(self, dir_name=DATA_DIR):
        files = [join(dir_name, f) for f in listdir(dir_name) if (isfile(join(dir_name, f)) and f[-5:] == ".json")]
        output = []
        for f in files:
            output += self.vectorize_saved_json(f)
        self.data = output
        return output

    def vectorize_json_array(self, js_arr):
        output = [self.vectorize_json(o) for o in js_arr]
        output = [o for o in filter(lambda x: x is not None, output)]
        self.data = output
        return output


    def vectorize_saved_json(self, fn):
        with open(fn) as f:
            js_array = json.load(f)
            output = self.vectorize_json_array(js_array)
            self.data = output
            return output

    def xy_conversion(self):
        """
        intended for train time only - fits the normalizer to the training data
        @return: a tuple containing the data matrix + labels
        """
        a = np.array(self.data)
        np.random.shuffle(a)
        X = a[:, :-1]
        X = self.preprocessor.fit_transform(X)
        # X = normalize(X)
        Y = a[:, -1]
        return X, Y

    def transform(self, x):
        if type(x) == list:
            x_trans = self.vectorize_json_array(x)
        else:
            x_trans = [self.vectorize_json(x)[:-1]]
        x_trans = self.preprocessor.transform(x_trans)
        return x_trans



class Session(object):
    def __init__(self):
        fn = str(uuid.uuid4()[0:6])