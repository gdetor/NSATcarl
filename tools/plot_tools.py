import struct
import numpy as np
import matplotlib.pylab as plt

def read_bin_file(fname):
    """ Read CARLsim spikes output binary file.

        Params:
            fname (str): Input filename

        Returns:
            fileContent (str): The contents of the file in bits
    """
    with open(fname, 'rb') as file:
        fileContent = file.read()
    return fileContent


def convert_carlbin_2_human(bin_pattern):
    """ Convert CARLsim data string into human readable format.

        Params:
            bin_pattern (str): Input string containing data

        Returns:
            head (list): The header of CARLsim spikes output file
            body (list): The body of CARLsim spikes output file
    """
    import sys

    if sys.version_info < (3, 0):
        int_size = sys.getsizeof('i')//8
    else:
        int_size = sys.getsizeof('i')//12
    size, h_size = len(bin_pattern), 5
    body_size = (size - (h_size*int_size))//int_size

    header = struct.unpack('i' * h_size, bin_pattern[:h_size*int_size])
    body = struct.unpack('i' * body_size, bin_pattern[20:])
    return body, header


def extract_times_neurons(data):
    """ Extract spikes times and neurons id labels from CARLsim
        raw data list.

        |----------------------|
        |1 | 3 | 5 | 6 | 4 | 7 |
        |----------------------|

        The first element is the first time that neuron number 3 --second
        element-- fires a spike. So this function reads a list like this
        and stores the times and the neurons labels in two different lists.

        Params:
            data (list): Contains the actual data

        Returns:
            times (array): 1D Numpy array of spikes times
            neuron_id (array): 1D Numpy array of spiked neurons labels
    """
    data = np.array(data)
    times = np.take(data, np.arange(0, data.shape[0], 2))
    neuron_id = np.take(data, np.arange(1, data.shape[0], 2))
    return times, neuron_id


def raster_plot(handler, times, neuron_id, marker_size=5.0):
    """ Plot a raster based on spikes times and neurons labels (id).

        Params:
            times (array): Numpy array of spikes times
            neuron_id (array): Numpy array of neurons labels (id)

        Returns:
    """

    handler.plot(times, neuron_id, '|k', alpha=0.7, ms=marker_size)
    handler.set_xlabel('Time (ms)')
    handler.set_ylabel('# Neuron')
