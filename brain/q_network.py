__author__ = 'sam.royston'
import math
import random
import numpy as np
from collections import namedtuple
import Queue
import torch
from sklearn.metrics import f1_score
import torch.nn as nn
import torch.optim as optim
from torch.autograd import Variable
from torch.nn import functional
from torchvision import transforms
from constants import *
import pickle as pkl
from utils import Ingestor


steps_done = 0
Transition = namedtuple('Transition', ('state', 'action', 'next_state', 'reward', 'ids'))


class ReplayMemory(object):

    def __init__(self, capacity):
        self.capacity = capacity
        self.memory = []
        self.position = 0

    def push(self, *args):
        """Saves a transition."""
        if len(self.memory) < self.capacity:
            self.memory.append(None)
        self.memory[self.position] = Transition(*args)
        self.position = (self.position + 1) % self.capacity

    def sample(self, batch_size):
        return random.sample(self.memory, batch_size)

    def __len__(self):
        return len(self.memory)


class Policy(nn.Module):

    def __init__(self, bot_count, action_count):
        super(Policy, self).__init__()
        self.bot_count = bot_count
        self.action_count = action_count
        self.input_layer = nn.Linear(8, 32)
        self.output_layer = nn.Linear(32, self.bot_count * self.action_count)
        self.softmax = nn.Softmax()

    def forward(self, x):
        ii = len(x)
        x = functional.relu(self.input_layer(x))
        x = self.output_layer(x)
        x = x.view(ii * self.bot_count, self.action_count)
        x = x.view(ii, self.bot_count, self.action_count)
        return x


def softmax(x, axis=0):
    """Compute softmax values for each sets of scores in x."""
    e_x = np.exp(x - np.max(x))
    return e_x / e_x.sum(axis=0)


class Learner(object):
    def __init__(self, action_memory=3, position_memory=2):
        self.steps_done = 0
        self.devices = 5
        self.actions = 4
        try:
            self.policy = torch.load(MODEL_FN)
            print("loaded saved model")
        except IOError:
            self.policy = Policy(5, 4)
            print("using new model")
        self.optimizer = optim.Adam(self.policy.parameters(), lr=0.000012)
        self.state = None
        self.memory = ReplayMemory(43000)
        self.last_synch = 0
        self.t = 0
        self.action_memory = action_memory
        self.position_memory = position_memory
        self.reward = None
        self.last_action = None
        self.a1 = None
        self.a2 = None
        self.distance = 0
        self.durations = []

    def predict_proba(self, x):
        id = x[0]

        d = torch.FloatTensor([x])

        output = self.policy(Variable(self.clean(d), volatile=True))
        output = softmax(output.data.numpy()[0])

        return output[id]

    def select_action(self, state, i, eps=0.15):
        sample = random.random()
        eps_threshold = EPS_END + (EPS_START - EPS_END) * \
            math.exp(-1. * self.steps_done / EPS_DECAY)
        self.steps_done += 1
        if sample > eps:
            output = self.policy(Variable(self.clean(state), volatile=True))
            output = output.data.numpy()
            output = np.argmax(output[0][int(i)])
            return output
        else:
            output = random.randrange(self.policy.action_count)
            return output

    def device_mask(self, ids):
        dev_index = np.zeros(BATCH_SIZE * self.policy.bot_count)
        xx = np.array([i * self.policy.bot_count + int(bid) for i, bid in enumerate(ids)])
        dev_index[xx] = 1
        oo = np.reshape(dev_index, (BATCH_SIZE, self.policy.bot_count))
        dev_index = np.array([[d] * self.policy.action_count for line in oo for d in line])
        dev_index = np.reshape(dev_index, (BATCH_SIZE, self.policy.bot_count, self.policy.action_count))
        return torch.from_numpy(oo).byte(), torch.from_numpy(dev_index).byte()

    def action_mask(self, actions):
        dev_index = np.zeros(BATCH_SIZE * self.policy.action_count)
        xx = np.array([i * self.policy.action_count + int(bid) for i, bid in enumerate(actions)])
        dev_index[xx] = 1
        oo = np.reshape(dev_index, (BATCH_SIZE, self.policy.action_count))
        dev_index = np.array([[d] * self.policy.bot_count for line in oo for d in line])
        dev_index = np.reshape(dev_index, (BATCH_SIZE, self.policy.action_count, self.policy.bot_count))
        dev_index = dev_index.transpose((0, 2, 1))
        return torch.from_numpy(oo).byte(), torch.from_numpy(dev_index).byte()

    def optimize_model(self):
        if len(self.memory) < BATCH_SIZE:
            return
        transitions = self.memory.sample(BATCH_SIZE)
        batch = Transition(*zip(*transitions))
        next_state_batch = Variable(torch.FloatTensor(np.array([self.clean(s.tolist()) for s in batch.next_state])),
                                    volatile=True)
        state_batch = Variable(torch.FloatTensor(np.array([self.clean(s.tolist()) for s in batch.state])))

        oo_devices, device_tensor = self.device_mask(batch.ids)
        oo_action, action_tensor = self.action_mask(batch.action)
        reward_batch = Variable(torch.cat([torch.FloatTensor([[float(a)]]) for a in batch.reward]))

        # run forward pass on current state
        state_batch = state_batch.view(len(state_batch), -1)
        op = self.policy(state_batch)
        flat_op = op[device_tensor].view(BATCH_SIZE, self.policy.action_count)

        # run forward pass on next state
        next_state_values = self.policy(next_state_batch).max(2)[0]
        flat_nsv = next_state_values[oo_devices]

        flat_nsv.volatile = False
        expected_state_action_values = (flat_nsv * GAMMA) + reward_batch


        # Compute Huber loss
        loss = functional.smooth_l1_loss(flat_op[oo_action], expected_state_action_values)
        if self.t % 100 == 0:
            print("epoch {} loss: {}".format(self.t, loss.data.numpy()[0]))

        # Optimize the model
        self.optimizer.zero_grad()
        loss.backward()
        for param in self.policy.parameters():
            param.grad.data.clamp_(-1, 1)
        self.optimizer.step()

    @staticmethod
    def clean(data):
        if type(data) is not list:
            # shape = data.numpy().shape
            # data = data.view(shape[0] * shape[1], shape[2])
            return data[:, 2:]
        else:
            data = np.array([data])
            if len(data.shape) > 2:
                data = np.reshape(data, (data.shape[0] * data.shape[1], data.shape[2]))
            return data[:, 2:]

    @staticmethod
    def append_action(vec_data, a):
        return torch.Tensor([[vec_data[0][0], vec_data[0][1], a]])

    def train(self, data, epochs=1000):
        data = filter(lambda x: len(x) > 0, data)
        concat = np.concatenate(data, axis=0)
        norms = np.linalg.norm(concat[:, 1 + self.action_memory :3 + self.action_memory], axis=1)
        max_norm = np.max(norms)
        for session in data:
            for i, state in enumerate(session[1:]):
                prev_state = session[i]
                reward = 1 - np.linalg.norm(state[1 + self.action_memory: 3 + self.action_memory])/max_norm
                last_action = prev_state[1]
                self.memory.push(prev_state, last_action, state, reward, state[0])
        for i in xrange(0, epochs):
            self.optimize_model()
            self.t = i

    def iterate(self, new_state, completed=False):
        if completed:
            self.durations.append(self.distance / self.t)
            self.t = 0
            self.state = None
            self.save()
            return
        vec_state = self.clean(new_state)
        if self.state is None:
            self.distance = np.linalg.norm(vec_state.numpy())
            self.a1 = 0
            full_state = self.append_action(vec_state, self.a1)
            self.state = full_state
            a = self.select_action(full_state, new_state["id"])
            self.last_action = a
            return a

        full_state = self.append_action(vec_state, self.a1)
        new_action = self.select_action(full_state, new_state["id"])
        self.reward = -1 * np.linalg.norm(vec_state.numpy())
        self.memory.push(self.state, self.last_action, full_state, self.reward, new_state["id"])
        self.a1 = self.last_action
        self.last_action = new_action
        self.state = full_state
        self.optimize_model()
        self.t += 1
        return new_action

    def save(self):
        torch.save(self.policy, MODEL_FN)

    # def load(self):
    #     with open(MODEL_FN) as f:
    #         a = torch.load(f)
    #         self.policy = pkl.load(f)


if __name__ == '__main__':
    ingestor = Ingestor(5, 1)
    ingestor.concat_in_directory(dir_name=DATA_DIR, separate=True)
    data = [np.array(d) for d in ingestor.data]
    d = data[:-1]

    learner = Learner(action_memory=ingestor.action_memory, position_memory=ingestor.position_memory)
    prediction = [learner.select_action(torch.FloatTensor(np.array([d.tolist()])), i=d[0], eps=0) for d in data[-1]]
    print f1_score(data[-1][:, 1], prediction, average='micro')
    learner.train(data[:-1], epochs=2000)
    learner.save()
    prediction = [learner.select_action(torch.FloatTensor(np.array([d.tolist()])), i=d[0], eps=0) for d in data[-1]]
    print f1_score(data[-1][:, 1], prediction, average='micro')
    learner.save()
    learner.train(data[:-1], epochs=10000)
    learner.save()
    prediction = [learner.select_action(torch.FloatTensor(np.array([d.tolist()])), i=d[0], eps=0) for d in data[-1]]
    print f1_score(data[-1][:, 1], prediction, average='micro')








