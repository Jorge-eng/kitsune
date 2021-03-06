#!/usr/bin/python
import sys
sys.path.append('.')

import MyGmm
import MyPca
import json
import numpy as np

np.set_printoptions(precision=3, suppress=True, threshold=np.nan)

min_maxloglik = -50
class GmmPcaEvalutator():
    def __init__(self):
        self.pca = MyPca.MyPca()
        self.ens = MyGmm.MyGmmEnsemble()
        
    def SetFromJsonFile(self, filename):
        f = open(filename, 'r')
        me = json.load(f)
        
        self.pca.setFromDict(me['pca'])
        self.ens.setFromDict(me['gmmsensemble'])
        
    def evaluate(self, x):
        x2 = self.pca.transform(x)
        probs = self.ens.evaluate(x2,min_maxloglik)
        return probs
