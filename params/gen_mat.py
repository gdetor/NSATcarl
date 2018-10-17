import numpy as np
import matplotlib.pylab as plt


def hinton(matrix, max_weight=None, ax=None):
    """Draw Hinton diagram for visualizing a weight matrix."""
    ax = ax if ax is not None else plt.gca()

    if not max_weight:
        max_weight = 2**np.ceil(np.log(np.abs(matrix).max())/np.log(2))

    ax.patch.set_facecolor('gray')
    ax.set_aspect('equal', 'box')
    ax.xaxis.set_major_locator(plt.NullLocator())
    ax.yaxis.set_major_locator(plt.NullLocator())

    for (x, y), w in np.ndenumerate(matrix):
        print(w)
        color = 'white' if w > 0 else 'black'
        size = np.sqrt(np.abs(w))
        rect = plt.Rectangle([x - size / 2, y - size / 2], size, size,
                             facecolor=color, edgecolor=color)
        ax.add_patch(rect)

    ax.autoscale_view()
    ax.invert_yaxis()


if __name__ == '__main__':
    N, M = 18, 46
    
    exc_input2visible = np.zeros((N, N))
    np.fill_diagonal(exc_input2visible, np.random.randint(1, 5, (N,)))
    np.savetxt('excinput2visible.dat', exc_input2visible, fmt='%.1f', delimiter='  ')

    inh_input2visible = np.zeros((N, N))
    np.fill_diagonal(inh_input2visible, np.random.randint(5, 30, (N,)))
    np.savetxt('inhinput2visible.dat', inh_input2visible, fmt='%.1f', delimiter='  ')

    visible2hidden = np.random.randint(1, 10, (N, M))
    np.savetxt('visible2hidden.dat', visible2hidden, fmt='%.1f', delimiter='  ')

    hidden2visible = visible2hidden.T
    np.savetxt('hidden2visible.dat', hidden2visible, fmt='%.1f', delimiter='  ')
