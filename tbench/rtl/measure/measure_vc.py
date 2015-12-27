#!/usr/bin/python

import re
import subprocess
import os
import shutil

import matplotlib.pyplot as plt

start=0.05
end=0.2
step=0.05

vchannels=[1,2,4]

res_pat = re.compile("^# RESULT packets: (\d+), flits: (\d+), throughput: (\d+.\d+), acc: (\d+.\d+), net: (\d+.\d+)")

results_pack = {}
results_flits = {}
results_idx = {}
results_tp  = {}
results_acc = {}
results_net = {}
results_tot = {}

def frange(start, stop, step):
    while start <= stop:
        yield start
        start += step

def run(genrate,vc):
    cmd = subprocess.Popen("vsim -c -Gtraffic='*:uniform(%f)' -Gvchannels=%d -GnumPackets=5000 -Gxdim=4 -Gydim=4 -Gselect_router=0 work.measure -do run.do"%(genrate,vc),stdout=subprocess.PIPE,shell=True)
    for line in cmd.stdout:
        m = res_pat.match(line)
        if m:
            res = m.groups(0)
            packets = int(res[0]);
            flits = int(res[1]);
            tp = float(res[2]);
            acc = float(res[3]);
            net = float(res[4]);
            results_pack[vc].append(packets)
            results_flits[vc].append(flits)
            results_idx[vc].append(genrate*5)
            results_tp[vc].append(tp)
            results_acc[vc].append(acc)
            results_net[vc].append(net)
            results_tot[vc].append(acc + net)

if __name__ == '__main__':
    for v in vchannels:
    results_pack[v] = []
    results_flits[v] = []
    results_idx[v] = []
    results_tp[v] = []
    results_acc[v] = []
    results_net[v] = []
    results_tot[v] = []
    for r in frange(start,end,step):
        print "Run %f %d"%(r,v)
        run(r,v)

    print "results_pack=",results_pack
    print "results_flits=",results_flits
    print "results_idx=",results_idx
    print "results_tp=",results_tp
    print "results_acc=",results_acc
    print "results_net=",results_net
    print "results_tot=",results_tot

    plt.figure()
    plt.title('Throughput')
    legend = []
    for v in vchannels:
        plt.plot(results_idx[v],results_tp[v],'-x')
        legend.append("vc=%d"%v)
    plt.xlabel('Generation Rate[flits/cyc]')
    plt.ylabel('Send Rate')
    plt.legend(legend,loc=2)
    for v in vchannels:
        plt.figure()
        plt.title('System Latency (#vc=%d)'%v)
        plt.ylabel('Latency')
        plt.xlabel('Generation Rate[flits/cyc]')
        plt.plot(results_idx[v],results_acc[v],'-x',results_idx[v],results_net[v],'-o',results_idx[v],results_tot[v],'-v')
        plt.legend( ('Acc', 'Net', 'Total'),loc=2 )
    plt.show()
