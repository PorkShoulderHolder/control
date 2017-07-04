__author__ = 'sam.royston'

import numpy as np
from constants import *
from utils import Ingestor
import pickle as pkl
from sklearn.linear_model import LogisticRegression
from sklearn.svm import SVC, LinearSVC, SVR
from sklearn.neighbors import KNeighborsClassifier
from sklearn.model_selection import cross_val_score
from sklearn.metrics import f1_score, classification_report, accuracy_score
from sklearn.neural_network import MLPClassifier
from sklearn.neighbors import KNeighborsClassifier, KNeighborsRegressor
from sklearn.svm import SVC
from sklearn.gaussian_process import GaussianProcessClassifier
from sklearn.gaussian_process.kernels import RBF
from sklearn.tree import DecisionTreeClassifier
from sklearn.ensemble import RandomForestClassifier, AdaBoostClassifier
from sklearn.naive_bayes import GaussianNB
from sklearn.discriminant_analysis import QuadraticDiscriminantAnalysis
from matplotlib import pyplot as plt

class Sarsa(object):
    def __init__(self, q_appoximator, discount_factor=0.85, learning_rate=0.05):
        self.q = q_appoximator
        self.gamma = discount_factor
        self.alpha = learning_rate
        self.convergence = []

    @staticmethod
    def reward(state):
        return -1 * np.linalg.norm(state[:, 0: 2], axis=1)

    def sarsa_iter(self, X, Y):
        rewards = self.reward(Y)
        current_q_values = self.q.predict(X)
        resultant_q_values = self.q.predict(Y)
        new_q_values = current_q_values + self.alpha * \
            (rewards + self.gamma * resultant_q_values - current_q_values)
        self.q.fit(X, new_q_values)
        self.convergence.append(np.linalg.norm(current_q_values - new_q_values))

    def init_q(self, X):
        rewards = self.reward(X)
        print rewards
        self.q.fit(X, rewards)



def perform_crossval(cls, data, target):
    print(cls.__class__.__name__)

    def scorer(estimator, X, y):
        pred = estimator.predict(X)
        return f1_score(y, pred, average='micro')

    def scorer_macro(estimator, X, y):
        pred = estimator.predict(X)
        # print(classification_report(y, pred))
        # print(accuracy_score(y, pred))
        return f1_score(y, pred, average='micro')

    scores = cross_val_score(cls, data, target, scoring=scorer_macro, cv=5)
    print(scores)
    return scores


def evaluate_models(data, target):
    logistic_reg = LogisticRegression(penalty='l2', class_weight='balanced')
    logistic_reg_sparse = LogisticRegression(penalty='l1', class_weight='balanced')
    linear_svm = LinearSVC(class_weight='balanced')
    gp = GaussianProcessClassifier(1.0 * RBF(1.0), warm_start=True)
    knn = KNeighborsClassifier(n_neighbors=6)
    rfc = RandomForestClassifier()
    abc = AdaBoostClassifier()
    nn = MLPClassifier()
    gnb = GaussianNB()
    svm = SVC(probability=True, class_weight='balanced', gamma=0.1)
    svm2 = SVC(probability=True, class_weight='balanced',gamma=1)
    svm3 = SVC(probability=True, class_weight='balanced',gamma=10)
    best = 0
    ret_val = None

    for cls in [ knn ]:#,  nn,  abc, gp, svm]:

        scores = perform_crossval(cls, data, target)
        sum_score = sum(list(scores)) / len(scores)
        print("avg score: " + str(sum_score))
        if sum_score > best:
            ret_val = cls
            best = sum_score
    print("winner: " + ret_val.__class__.__name__)
    ret_val.fit(data, target)
    return ret_val, best


def plot_action(action_i, device_j, clf):
    h = .02
    cm = plt.cm.RdBu

    x_min, x_max = data[:, 0].min() - .5, data[:, 0].max() + .5
    y_min, y_max = data[:, 1].min() - .5, data[:, 1].max() + .5
    xx, yy = np.meshgrid(np.arange(x_min, x_max, h),
                         np.arange(y_min, y_max, h))

    a2 = plt.subplot(2, 4, 4 + (8 * device_j) + action_i + 1)
    ax = plt.subplot(2, 4, (8 * device_j) + action_i + 1)
    len_r = len(xx.ravel())

    dev0 = np.ones(len_r) if device_j == 0 else np.zeros(len_r)
    dev1 = np.ones(len_r) if device_j == 1 else np.zeros(len_r)

    Z_0 = clf.predict(np.c_[xx.ravel(), yy.ravel(), np.ones(len_r) * action_i])
    Z_1 = clf.predict(np.c_[xx.ravel(), yy.ravel(), np.ones(len_r) * 0])
    Z_2 = clf.predict(np.c_[xx.ravel(), yy.ravel(), np.ones(len_r) * 1])
    Z_3 = clf.predict(np.c_[xx.ravel(), yy.ravel(), np.ones(len_r) * 2])
    Z_4 = clf.predict(np.c_[xx.ravel(), yy.ravel(), np.ones(len_r) * 3])
    zx = np.concatenate([Z_1, Z_2, Z_3, Z_4])
    zz = np.vstack([Z_1, Z_2, Z_3, Z_4])

    actions = np.argmax(zz, axis=0).reshape(xx.shape)
    print actions.shape
    levels = np.linspace(0, 1, 100)
    # Put the result into a color plot
    Z = Z_0.reshape(xx.shape)
    z_min, z_max = zx.min() - .5, zx.max() + .5

    print xx.shape, yy.shape, Z.shape
    ax.contourf(xx, yy, Z,  np.arange(z_min, z_max, 0.1))
    z_min, z_max = actions.min() - .5, actions.max() + .5
    a2.contourf(xx, yy, actions,  np.arange(z_min, z_max, 0.1))


    ax.set_xlim(xx.min(), xx.max())
    ax.set_ylim(yy.min(), yy.max())
    ax.set_xticks(())
    ax.set_yticks(())

    a2.set_xlim(xx.min(), xx.max())
    a2.set_ylim(yy.min(), yy.max())
    a2.set_xticks(())
    a2.set_yticks(())

    ax.text(xx.max() - .3, yy.min() + .3, ('%.2f' % action_i).lstrip('0'),
            size=15, horizontalalignment='right')
    action_i += 1
    return Z_0

if __name__ == '__main__':


    ingestor = Ingestor(5, 1)
    ingestor.concat_in_directory(dir_name=DATA_DIR)
    knn = KNeighborsRegressor(n_neighbors=15, weights='distance')
    svr = SVR()

    sarsa = Sarsa(q_appoximator=knn)
    data, target = ingestor.xy_conversion()

    sarsa.init_q(data)


    for i in xrange(0, 367):
        data, target = ingestor.xy_conversion()
        sarsa.sarsa_iter(data, target)
        print "{}: change {}".format(i, sarsa.convergence[-1])

    pkl.dump(sarsa.q, open("best_sarsa", "w+"))

    plt.plot(sarsa.convergence)
    plt.show()
    for j in xrange(0, 1):
        for i in xrange(0, 4):
            plot_action(i, 0, sarsa.q)

    plt.show()


