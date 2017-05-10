__author__ = 'sam.royston'
import math
import random
import numpy as np
from collections import namedtuple

import torch
import torch.nn as nn
import torch.optim as optim
import torch.autograd as autograd
from torch.nn import functional
from torchvision import transforms
from constants import *

steps_done = 0
Transition = namedtuple('Transition', ('state', 'action', 'next_state', 'reward'))


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
        self.input_layer = nn.Linear(2, 32)
        self.output_layer = nn.Linear(32, self.bot_count * self.action_count)

    def forward(self, x, i):
        x = functional.relu(self.input_layer(x))
        x = self.output_layer(x)
        return x


class Learner(object):
    def __init__(self):
        self.steps_done = 0
        try:
            self.policy = torch.load(MODEL_FN)
            print("loaded saved model")
        except IOError:
            self.policy = Policy(5, 4)
            print("using new model")
        self.optimizer = optim.Adam(self.policy.parameters(), lr=1e-2)
        self.state = None
        self.memory = ReplayMemory(2000)
        self.last_synch = 0
        self.t = 0
        self.reward = None
        self.last_action = None
        self.distance = 0
        self.durations = []

    def select_action(self, state):
        sample = random.random()
        eps_threshold = EPS_END + (EPS_START - EPS_END) * \
            math.exp(-1. * self.steps_done / EPS_DECAY)
        self.steps_done += 1
        if sample > eps_threshold:
            return self.policy(autograd.Variable(state, volatile=True))
        else:
            return torch.LongTensor([[random.randrange(self.policy.action_count)]])

    def optimize_model(self):
        if len(self.memory) < BATCH_SIZE:
            return
        transitions = self.memory.sample(BATCH_SIZE)
        batch = Transition(*zip(*transitions))
        non_final_mask = torch.ByteTensor(tuple(map(lambda s: s is not None, batch.next_state)))
        f_batch = torch.cat([s for s in batch.next_state if s is not None])
        non_final_next_states = autograd.Variable(f_batch, volatile=True)
        state_batch = autograd.Variable(torch.cat(batch.state))
        action_batch = autograd.Variable(torch.cat(batch.action))
        reward_batch = autograd.Variable(torch.cat(batch.reward))

        state_action_values = self.policy(state_batch).gather(1, action_batch)
        next_state_values = autograd.Variable(torch.zeros(BATCH_SIZE))
        next_state_values[non_final_mask] = self.policy(non_final_next_states).max(1)[0]
        next_state_values.volatile = False
        expected_state_action_values = (next_state_values * GAMMA) + reward_batch
    
        # Compute Huber loss
        loss = functional.smooth_l1_loss(state_action_values, expected_state_action_values)
    
        # Optimize the model
        self.optimizer.zero_grad()
        loss.backward()
        for param in self.policy.parameters():
            param.grad.data.clamp_(-1, 1)
        self.optimizer.step()

    def iterate(self, new_state, completed=False):
        if self.state is None:
            self.state = new_state
            self.distance = np.linalg.norm(self.state)
            return self.select_action(self.state)

        new_action = self.select_action(new_state)
        self.reward = np.linalg.norm(new_state)

        self.memory.push(self.state, self.last_action, new_state, self.reward)

        self.last_action = new_action
        self.state = new_state
        self.optimize_model()
        self.t += 1
        if completed:
            self.durations.append(self.distance / self.t)
            self.t = 0
            self.distance = 0
            self.state = None
            self.save()
        return new_action

    def save(self):
        torch.save(self.policy, MODEL_FN)


learner = Learner()
