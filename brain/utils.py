__author__ = 'sam.royston'
import csv
import uuid

class Ingestor(object):
    def __init__(self):
        self.action_memory = 2
        self.position_memory = 0

    def vectorize_json(self, js_dict):
        x = [js_dict["x"], js_dict["y"]]

        for i in xrange(0, self.action_memory):
            x += [js_dict["a" + str(i)]]

        for i in xrange(0, self.position_memory):
            x += [js_dict["x" + str(i)]]
            x += [js_dict["y" + str(i)]]

        return x

    def vectorize_json_array(self, js_arr):
        return [self.vectorize_json(o) for o in js_arr]

    def vectorize_saved_csv(self, fn):
        with open(fn) as f:
            reader = csv.DictReader(f)
            return self.vectorize_json(reader)


class Session(object):
    def __init__(self):
        fn = str(uuid.uuid4()[0:6])