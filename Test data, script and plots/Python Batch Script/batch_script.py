#!/usr/bin/env python

import random
import os
from subprocess import Popen,PIPE
from multiprocessing.pool import ThreadPool
import math


def rungfpftester(cmd) :
    print cmd
    po = Popen(cmd, shell=True, stdout=PIPE)
    po.wait()
    score = 1
    return score

# evaluate a state
def run_eval(tem,rt,id) :
    jobpool = ThreadPool(processes=16)
    scorepool = []
    cmds = []
    for i in range(1) :
        #cmd = './gfpf_tester --ranges 0.2,0.2,0.1,0.2 --ns 4000 --rt 2000 %s %s %d | ./eval.py --single-eval' % (getCmdLineOptions(state),speed,subject)
        cmd = './gfpf_tester %s %s %d ' % (tem,rt,id)
        cmds.append(cmd)
        rungfpftester(cmd)

if __name__ == '__main__' :

    TOT_ITERATIONS = 3
    for it in range(TOT_ITERATIONS) :
        print 'iteration %d/%d' % (it+1,TOT_ITERATIONS)
        s1 = run_eval('user',0,it);
        s2 = run_eval('user',500,it);
        s3 = run_eval('user',1000,it);

    print ' -- THE END --'
