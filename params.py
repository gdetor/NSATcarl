from ctypes import *
from collections import defaultdict
import weakref


class nsat(Structure):
    _fields_ = [('alpha', c_float),
                ('beta', c_float),
                ('sigma', c_float),
                ('v_th', c_float),
                ('v_reset', c_float),
                ('b', c_float),
                ('tau_ref', c_float),
                ('dopamine_mode', c_int),
                ('alphaPlus', c_float),
                ('tauPlus', c_float),
                ('alphaMinus', c_float),
                ('tauMinus', c_float),
                ('betaLTP', c_float),
                ('betaLTD', c_float),
                ('lambda', c_float),
                ('delta', c_float),
                ('gamma', c_float),
                ('stdp_fun_type', c_int),
                ('is_set', c_bool)]


class blankout(Structure):
    _fields_ = [('alphaS', c_float),
                ('prob', c_float)]


class spkg(Structure):
    _fields_ = [('rate', c_float),
                ('freq', c_float),
                ('spk_at_zero', c_bool),
                ('on_gpu', c_bool)]


class KeepRefs(object):
    __refs__ = defaultdict(list)
    def __init__(self):
        self.__refs__[self.__class__].append(weakref.ref(self))

    @classmethod
    def get_instances(cls):
        for inst_ref in cls.__refs__[cls]:
            inst = inst_ref()
            if inst is not None:
                yield inst


class new_nsat_population(KeepRefs):
    # _all = []
    def __init__(self, name, num_neurons):
        super(new_nsat_population, self).__init__()
        self.name = name
        self.num_neurons = num_neurons
        self.alpha = 0.0
        self.beta = 0.0
        self.sigma = 0.0
        self.v_th = 0.0
        self.v_reset = 0.0
        self.b = 0.0
        self.tau_ref = 0.0
        self.dopamine_mode = 0
        self.alphaPlus = 0.0
        self.tauPlus = 0.0
        self.alphaMinus = 0.0
        self.tauMinus = 0.0
        self.betaLTP = 0.0
        self.betaLTD = 0.0
        self.l = 0.0 
        self.delta = 0.0
        self.gamma = 0.0
        self.stdp_fun_type = 0
        self.is_set = 0

        self.tmp = nsat()

        self.src = self.name
        self.dest = []

    def push(self):
        self.tmp.alpha = self.alpha
        self.tmp.beta = self.beta
        self.tmp.sigma = self.sigma
        self.tmp.v_th = self.v_th
        self.tmp.v_reset = self.v_reset
        self.tmp.b = self.b
        self.tmp.tau_ref = self.tau_ref
        self.tmp.dopamine_mode = self.dopamine_mode
        self.tmp.alphaPlus = self.alphaPlus
        self.tmp.tauPlus = self.tauPlus
        self.tmp.alphaMinus = self.alphaMinus
        self.tmp.tauMinus = self.tauMinus
        self.tmp.betaLTP = self.betaLTP
        self.tmp.betaLTD = self.betaLTD
        self.tmp.l = self.l
        self.tmp.delta = self.delta
        self.tmp.gamma = self.gamma
        self.tmp.stdp_fun_type = self.stdp_fun_type
        self.tmp.is_set = self.is_set 
        # self.__class__._all.append(self.tmp)

    def connect(self, *arg):
        m = len(arg)
        for i in range(m):
            self.dest.append(arg[i])


class new_input_population(KeepRefs):
    def __init__(self, name, num_neurons):
        super(new_input_population, self).__init__()
        self.name = name
        self.num_neurons = num_neurons
        self.rate = 0.0;
        self.freq = 0.0;
        self.spk_at_zero = False;
        self.on_gpu = False;

        self.tmp = spkg()

        self.src = self.name
        self.dest = []

    def push(self):
        self.tmp.rate = self.rate
        self.tmp.freq = self.freq
        self.tmp.spk_at_zero = self.spk_at_zero
        self.tmp.on_gpu = self.on_gpu

    def connect(self, *arg):
        m = len(arg)
        for i in range(m):
            self.dest.append(arg[i])


if __name__ == '__main__':
    exc1 = new_nsat_population('excit1', 100)
    exc2 = new_nsat_population('excit2', 100)

    exc1.alpha = 1.5
    exc1.beta = 0.8

    exc2.alpha = 45.
    exc1.beta = 1.3

    exc1.connect('excit2')

    exc1.push()
    exc2.push()

    for i in new_nsat_population.get_instances():
        print("{}".format(i.name))
        print("{}".format(i.tmp.alpha))
