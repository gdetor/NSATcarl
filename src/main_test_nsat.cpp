#include "nsat_core.h"

int main(int argc, char **argv) {
    int res;                    // return flag
    filenames *fins;            // pointer to a filenames struct
    simulation *sim;            // pointer to a simulation struct
    carlsim *carl;              // pointer to a carlsim struct

    // Allocate space for structs
    carl = new carlsim;
    sim = new simulation;
    fins = new filenames;

    // Assign filenames to fins
    fins->spkg_fname = "params/spkg_params.dat";
    fins->nsat_fname = "params/nsat_params.dat";
    fins->stdp_fname = "params/stdp_params.dat";
    fins->conn_fname = "params/conn_params.dat";
    fins->delay_fname = "params/delay_params.dat";

    // Assign CARLsim parameters to carl
    carl->sim_name = "test_nsat";       // Simulation name
    carl->mode = CPU_MODE;              // CPU- or GPU-based simulation
    carl->logger = USER;                // Type of logger
    carl->gpu_index = 0;                // GPU indexing
    carl->random_seed = 42;             // Random number generator seed
    
    // Assign Simulation parameters to sim
    sim->int_method = FORWARD_EULER;    // Integration method
    sim->num_in_layers = 1;             // Number of input layers
    sim->num_nsat_layers = 1;           // Number of NSAT layers
    sim->num_in_groups = 1;             // Number of input groups
    sim->num_nsat_groups = 1;           // Number of NSAT groups
    sim->sim_time_sec = 1;              // Simulation time in seconds
    sim->sim_time_msec = 0;             // Simulation time in mseconds
    sim->int_num_steps = 2;             // Integration method steps (in ms)
    sim->print_summary = true;         // Print a summary at the end
    sim->input_type = "poisson";        // Type of input
    sim->copy_state = false;            // Copy data from devide to host
    sim->remove_tmp_mem = true;         // Remove temporary memory after building net 
    sim->coba_enabled = false;          // CUBA or COBA simulation

    // Run actual simulation code - if an exception is thrown 
    // the execution terminates
    try {
        nsat_core test(fins, carl, sim);    // NSAT instance
        res = test.c_config_state();        // CARLsim config state
        res = test.c_setup_state();         // CARLsim setup state
        res = test.c_run_state();           // CARLsim run state
        res = test.c_cleanup();             // Cleanup method
    }
    catch (int &e) {
        print_exceptions(e);                // Custom exceptions - see auxiliary.cpp
    }
    catch(runtime_error &e) {
        cout << e.what() << endl;           // Generic runtime exceptions
    }
    catch(bad_alloc &ba) {
        cout << ba.what() << endl;          // Bad allocation exceptions
    }

    // Deallocate memory
    delete carl;
    delete sim;
    delete fins;
    
    return 0;
}
