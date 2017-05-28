__author__ = 'sam.royston'
import math
import random
import numpy as np
from collections import namedtuple
import Queue
import torch
import torch.nn as nn
import torch.optim as optim
from torch.autograd import Variable
from torch.nn import functional
from torchvision import transforms
from constants import *
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
        self.input_layer = nn.Linear(10, 32)
        self.output_layer = nn.Linear(32, self.bot_count * self.action_count)
        self.softmax = nn.Softmax()

    def forward(self, x):
        ii = len(x)
        x = functional.relu(self.input_layer(x))
        x = self.output_layer(x)
        x = x.view(ii * self.bot_count, self.action_count)
        x = x.view(ii, self.bot_count, self.action_count)
        return x


def softmax(x):
    """Compute softmax values for each sets of scores in x."""
    e_x = np.exp(x - np.max(x))
    return e_x / e_x.sum(axis=0)


class Learner(object):
    def __init__(self, action_memory=3, position_memory=2):
        self.steps_done = 0
        self.devices = 5
        self.actions = 4
        # try:
        #     self.policy = torch.load(MODEL_FN)
        #     print("loaded saved model")
        # except IOError:
        self.policy = Policy(5, 4)
        print("using new model")
        self.optimizer = optim.Adam(self.policy.parameters(), lr=1e-2)
        self.state = None
        self.memory = ReplayMemory(2000)
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

    def select_action(self, state, i):
        sample = random.random()
        eps_threshold = EPS_END + (EPS_START - EPS_END) * \
            math.exp(-1. * self.steps_done / EPS_DECAY)
        self.steps_done += 1
        if sample > 0.15:
            output = self.policy(Variable(state, volatile=True))
            output = output.data.numpy()
            output = np.argmax(output[0][i])
            return output
        else:
            output = random.randrange(self.policy.action_count)
            return output

    def optimize_model(self):
        if len(self.memory) < BATCH_SIZE:
            return
        transitions = self.memory.sample(BATCH_SIZE)
        batch = Transition(*zip(*transitions))
        f_batch = torch.FloatTensor(np.array([s.tolist() for s in batch.next_state]))
        this_batch = torch.FloatTensor(np.array([s.tolist() for s in batch.state]))
        non_final_next_states = Variable(f_batch, volatile=True)

        device_tensor = np.array([i * self.policy.bot_count + int(s) for i, s in enumerate(batch.ids)]).squeeze()
        state_batch = Variable(this_batch)
        action_batch = Variable(
            torch.from_numpy(
                np.array([i * self.policy.action_count + int(s) for i, s in enumerate(batch.action)]).squeeze()
            )
        )
        reward_batch = Variable(torch.cat([torch.FloatTensor([[float(a)]]) for a in batch.reward]))

        op = self.policy(state_batch)

        flat_op = op.view(len(op) * self.policy.bot_count, self.policy.action_count).data.numpy()[device_array]
        print(flat_op.shape)

        flat_op = np.reshape(flat_op, len(flat_op) * self.policy.action_count)
        print(flat_op.shape)
        flat_op = Variable(torch.from_numpy(flat_op[action_array]))
        print(flat_op)
        next_state_values = self.policy(non_final_next_states).max(2)[0]
        flat_nsv = next_state_values.view(len(next_state_values) * self.policy.bot_count).data.numpy()[device_array]
        flat_nsv = Variable(torch.from_numpy(flat_nsv))

        next_state_values.volatile = False
        expected_state_action_values = (flat_nsv * GAMMA) + reward_batch

        # Compute Huber loss
        print(flat_op)
        print(flat_nsv)
        loss = functional.smooth_l1_loss(flat_op, expected_state_action_values)
        print("epoch {} loss: {}".format(self.t, loss))
        # Optimize the model
        self.optimizer.zero_grad()
        loss.backward()
        for param in self.policy.parameters():
            param.grad.data.clamp_(-1, 1)
        self.optimizer.step()

    @staticmethod
    def clean(data):
        return torch.Tensor([[data["x"], data["y"]]])

    @staticmethod
    def append_action(vec_data, a):
        return torch.Tensor([[vec_data[0][0], vec_data[0][1], a]])

    def train(self, data, epochs=10):
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



if __name__ == '__main__':
    ingestor = Ingestor(5, 1)
    ingestor.concat_in_directory(dir_name=DATA_DIR, separate=True)
    data = [np.array(d) for d in ingestor.data]
    print (":::::")
    test_size = 10000
    learner = Learner(action_memory=ingestor.action_memory, position_memory=ingestor.position_memory)
    learner.train(data[:-1])







