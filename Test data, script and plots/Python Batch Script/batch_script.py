#!/usr/bin/env python

import random
import os
from subprocess import Popen,PIPE
from multiprocessing.pool import ThreadPool
import math



# get string form of command line options to be added to the gf recognizer invocation
def getCmdLineOptions(s) :
    return '--sigmas %f,%f,%f,%f --smoothing %f --student-nu %f --reference-pos-exp-param %f' % (s[0],s[1],s[2],s[3],s[4],s[5],s[6])

def runbubi(cmd) :
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
    print 'twant'
    for i in range(1) :
        #cmd = './gfpf_tester --ranges 0.2,0.2,0.1,0.2 --ns 4000 --rt 2000 %s %s %d | ./eval.py --single-eval' % (getCmdLineOptions(state),speed,subject)
        cmd = './gfpf_tester %s %s %d ' % (tem,rt,id)
        cmds.append(cmd)
        runbubi(cmd)

if __name__ == '__main__' :

    TOT_ITERATIONS = 3
    for it in range(TOT_ITERATIONS) :
        print 'iteration %d/%d' % (it+1,TOT_ITERATIONS)
        s1 = run_eval('user',0,it);
        s2 = run_eval('user',500,it);
        s3 = run_eval('user',1000,it);

    print ' -- THE END --'
