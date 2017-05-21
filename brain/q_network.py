__author__ = 'sam.royston'
import math
import random
import numpy as np
from collections import namedtuple
import Queue
import torch
import torch.nn as nn
import torch.optim as optim
import torch.autograd as autograd
from torch.nn import functional
from torchvision import transforms
from constants import *

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
        self.input_layer = nn.Linear(3, 32)
        self.output_layer = nn.Linear(32, self.bot_count * self.action_count)
        self.softmax = nn.Softmax()

    def forward(self, x):
        ii = len(x)
        x = functional.relu(self.input_layer(x))
        x = self.output_layer(x)
        x = x.view(ii * self.bot_count, self.action_count)
        x = self.softmax(x)
        x = x.view(ii, self.bot_count, self.action_count)
        return x


def softmax(x):
    """Compute softmax values for each sets of scores in x."""
    e_x = np.exp(x - np.max(x))
    return e_x / e_x.sum(axis=0)


class Learner(object):
    def __init__(self):
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
            output = self.policy(autograd.Variable(state, volatile=True))
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

        non_final_mask = torch.ByteTensor(tuple(map(lambda s: s is not None, batch.next_state)))
        non_null_actions = torch.ByteTensor(tuple(map(lambda s: s is not None, batch.action))).repeat(3, 1).transpose(1, 0)
        f_batch = torch.cat([s for s in batch.next_state if s is not None])
        non_final_next_states = autograd.Variable(f_batch, volatile=True)
        device_array = autograd.Variable(torch.cat([torch.LongTensor([[s]]) for s in batch.ids if s is not None]))

        state_batch = autograd.Variable(torch.cat(batch.state))
        action_batch = autograd.Variable(torch.cat([torch.LongTensor([[int(a)]]) for a in batch.action if a is not None]))
        reward_batch = autograd.Variable(torch.cat([torch.FloatTensor([[float(a)]]) for a in batch.reward]))

        s = state_batch[non_null_actions].view(len(state_batch[non_null_actions]) / 3, 3)
        devices_base = autograd.Variable(self.policy.bot_count * self.policy.action_count * torch.LongTensor(xrange(0, len(action_batch))))

        device_indices = devices_base + (device_array[non_null_actions[:, 0]] * self.policy.action_count) + action_batch.squeeze()
        op = self.policy(s)
        op = op.view(len(op) * self.policy.bot_count * self.policy.action_count)
        state_action_values = op[device_indices.data.squeeze()]
        next_state_values = autograd.Variable(torch.zeros(BATCH_SIZE))
        next_state_values[non_final_mask] = self.policy(non_final_next_states).max(1)[0]
        next_state_values.volatile = False
        expected_state_action_values = (next_state_values * GAMMA) + reward_batch

        # Compute Huber loss
        loss = functional.smooth_l1_loss(state_action_values, expected_state_action_values[non_null_actions[:, 0]])
    
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


learner = Learner()
