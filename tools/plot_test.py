from plot_tools import *


def read_result_fnames(sim_name):
    import os, sys
    
    if sys.version_info < (3, 0):
        name = sim_name
    else:
        name = sim_name.decode("utf-8")
    fnames = []
    for file in os.listdir("results/"):
        if file.endswith(".dat"):
            if file != "sim_"+name+".dat":
                fnames.append("results/"+file)
    return fnames


if __name__ == '__main__':
    # Plot the results
    outputs = read_result_fnames(b"test_spike_trains")
    
    fig = plt.figure(figsize=(11, 11))
    for j, i in enumerate(outputs):
        print(i)
        bin_data = read_bin_file(i)
        data, _ = convert_carlbin_2_human(bin_data)
        times, neuron_id = extract_times_neurons(data)

        ax = fig.add_subplot(3, 1, j+1)
        raster_plot(ax, times, neuron_id)

    plt.show()
