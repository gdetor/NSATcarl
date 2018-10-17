import numpy as np
import matplotlib.pylab as plt


if __name__ == '__main__':
    connections = ['inp_exc', 'exc_inh', 'inh_exc']
    size = [(50, 100), (100, 20), (20, 100)]
    factor = [0.5, 0.8, 1.2]
    
    for i, j in enumerate(connections):
        w = np.random.randint(0, 2, size[i]).astype('f')
        w *= factor[i]
        np.savetxt("params/"+j+".dat", w, fmt='%.2f')
