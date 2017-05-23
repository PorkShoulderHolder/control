__author__ = 'sam.royston'

import numpy as np
from constants import *
from utils import Ingestor
import pickle as pkl
from sklearn.linear_model import LogisticRegression
from sklearn.svm import SVC, LinearSVC
from sklearn.neighbors import KNeighborsClassifier
from sklearn.model_selection import cross_val_score
from sklearn.metrics import f1_score, classification_report, accuracy_score
from sklearn.neural_network import MLPClassifier
from sklearn.neighbors import KNeighborsClassifier
from sklearn.svm import SVC
from sklearn.gaussian_process import GaussianProcessClassifier
from sklearn.gaussian_process.kernels import RBF
from sklearn.tree import DecisionTreeClassifier
from sklearn.ensemble import RandomForestClassifier, AdaBoostClassifier
from sklearn.naive_bayes import GaussianNB
from sklearn.discriminant_analysis import QuadraticDiscriminantAnalysis

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
    knn = KNeighborsClassifier()
    rfc = RandomForestClassifier()
    abc = AdaBoostClassifier()
    nn = MLPClassifier()
    gnb = GaussianNB()
    svm = SVC(class_weight='balanced', probability=True)
    best = 0
    ret_val = None

    for cls in [logistic_reg, knn, gnb, rfc]:#,  nn,  abc, gp, svm]:

        scores = perform_crossval(cls, data, target)
        sum_score = sum(list(scores)) / len(scores)
        print("avg score: " + str(sum_score))
        if sum_score > best:
            ret_val = cls
            best = sum_score
    print("winner: " + ret_val.__class__.__name__)
    ret_val.fit(data, target)
    return ret_val, best

if __name__ == '__main__':
    # ingestor = Ingestor()
    # ingestor.concat_in_directory(dir_name=DATA_DIR)
    # data, target = ingestor.xy_conversion()
    # winner, _ = evaluate_models(data, target)
    #
    # ingestor_less = Ingestor(2, 1)
    # ingestor_less.concat_in_directory(dir_name=DATA_DIR)
    # data_less, target_less = ingestor_less.xy_conversion()
    # winner, _ = evaluate_models(data_less, target_less)

    ingestor_more = Ingestor(1, 0)
    ingestor_more.concat_in_directory(dir_name=DATA_DIR)
    data_more, target_more = ingestor_more.xy_conversion()
    winner, _ = evaluate_models(data_more, target_more)


    with open(CLASSIFIER_FN, "w+") as f:
        pkl.dump(winner, f)

    print("saved model at " + CLASSIFIER_FN)