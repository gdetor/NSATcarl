import numpy as np


if __name__ == '__main__':
    N, M, K = 5, 8, 2
    
    inp_exc_ = np.random.randint(0, 10, (N, M))
    np.savetxt('inp_exc_.dat', inp_exc_, fmt='%.1f', delimiter='  ')

    exc_inh_ = np.random.randint(0, 15, (M, K))
    np.savetxt('exc_inh_.dat', exc_inh_, fmt='%.1f', delimiter='  ')

    inh_exc_ = np.random.randint(0, 20, (N, M))
    np.savetxt('inh_exc_.dat', inh_exc_, fmt='%.1f', delimiter='  ')
