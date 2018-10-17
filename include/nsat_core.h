#ifndef _NSAT_CORE_H
#define _NSAT_CORE_H

#define MAKE_FLAG 1

#include <iostream>
#include <iomanip>
#include <string>
#include <iterator>
#include <sstream>
#include <fstream>
#include <regex>
#include <vector>
#include <cstdarg>

#include <carlsim.h>
#include <poisson_rate.h>

#include "connx_core.h"


using namespace std;


/* ----------------------------------
 * CARLsim simulation params struct 
 * ----------------------------------*/
typedef struct carlsim_s {
    char *sim_name;
    simMode_t mode;
    loggerMode_t logger;
    int gpu_index;
    int random_seed;
} carlsim;


/* ----------------------------------
 * Simulation parameters struct 
 * ----------------------------------*/
typedef struct simulation_s {
    integrationMethod_t int_method;
    float maxWt;
    int sim_time_sec;
    int sim_time_msec;
    int int_num_steps;      //per ms
    int num_connections;
    char *input_type;
    bool print_summary;
    bool copy_state; 
    bool remove_tmp_mem;
    bool coba_enabled;
} simulation;


/* ----------------------------------
 * Input neurons parameters struct 
 * ----------------------------------*/
typedef struct spike_generator_s {
    float rate;
    float freq;
    bool spk_at_zero;
    bool on_gpu;
} spkg;


/* ----------------------------------
 * NSAT neuron parameters struct 
 * ----------------------------------*/
typedef struct nsat_s {
    float alpha;
    float beta;
    float sigma;
    float v_th;
    float v_reset;
    float alphaS;
    float b;
    int tau_ref;
} nsat;


/* ----------------------------------
 * Parameters input files struct 
 * ----------------------------------*/
typedef struct filename_s {
    char *spkg_fname;
    char *nsat_fname;
    char *stdp_fname;
    char **conn_fname;
    char *delay_fname;
    char **finp_spikes;
} filenames;


/* ----------------------------------
 * NSAT Core struct
 * ----------------------------------*/
typedef struct nsat_unit_s {
    string unit_name;       // unique core's name
    nsat nsat_p;            // NSAT params struct
    int unit_id;            // unique core's id
    int num_neurons;        // number of neurons within core
    unsigned int unit_type; // type of neurons within core
    bool mflag;             // on/off monitoring the core
} nsat_unit;


/* ----------------------------------
 * Input  Core struct
 * ----------------------------------*/
typedef struct input_units_s {
    string unit_name;       // unique core's name
    spkg spkg_p;            // core's parameters
    int unit_id;            // unique core's id
    int num_neurons;        // number of neurons within core
    unsigned int unit_type; // type of neurons within core
    bool mflag;             // on/off monitoring the core
} input_unit;


/***************************************************************************
 * NSAT_CORE Auxilixiary Functions Declarations
 ***************************************************************************/
// Converters
int count_lies(bool);
bool str2bool(string);                  // Convert string to bool
unsigned int str2nrtype(string);        // Convert string to neuronType_t 
stdpType_t str2stdpt(string);           // Convert string to stdpType_t

// Exceptions handler
void print_exceptions(int, ...);        // Handle custom exceptions




/***************************************************************************
 * NSAT_CORE Class - This class implements methods for simulating NSAT
 * neural groups. It takes advantage of CARLsim classes and methods in
 * order to carry out all the necessary computations. 
 *
 * Attributes:
 *      - carl_p : A carlsim struct containing the parameters for properly 
 *                  initializing CARLsim.
 *      - sim_p : A simulation struct for simulation parameters.
 *      - fnames : A filenames struct that holds the names of parameter
 *                  input files. 
 *      - sim : A pointer to CARLsim class
 *      - connex : A pointer to a Connx class (inherits from
 *      - nsatc  : A pointer to struct that reflects a NSAT core
 *      - inpc   : A pointer to struct that represents a input core
 *                  ConnectionGenerator of CARLsim).
 *      - grid_input_layers : A double pointer to a Grid3D struct of
 *                  CARLsim for the input neural layers.
 *      - grid_nsat_layers : A double pointer to a Grid3D struct of
 *                  CARLsim for the NSAT neural layers.
 *      - psn_spkg : A double pointer to PoissonRate. It points to the
 *                  PoissonRate groups.
 *      - prd_spkg : A double pointer to PeriodicSpikeGenerator. It
 *                  points to periodic spike generator groups.
 *      - vec_spkg : A double pointer to SpikeGeneratorFromVector. It
 *                  points to vectorial spike generator groups. 
 *      - file_spkg : A double pointer to SpikeGeneratorFromFile. It 
 *                  points to file spike generator groups. 
 *      - spike_train : A vector that contains user-defined spike trains
 *                       for inputs to the network. 
 *
 * Methods: 
 *              Construction/Destruction
 *              ------------------------
 *      - nsat_core : NSAT Core constructor.
 *      - ~nsat_core : NSAT Core destructor.
 *
 *              Auxiliary Methods
 *              -----------------
 *      - read_core_params : Core parameters loader.
 *      - read_params : NSAT, STDP and SPKG parameters loader.
 *      - read_grid_params : Layers parameters loader.
 *      - initialize_layers : Build method for layers.
 *      - initialize_params : Build method for NSAT, SPKG and STDP.
 *      - poisson_spikes : Construct poisson spike generator neural
 *                          groups.
 *      - periodical_spikes : Construct periodic spike generator neural
 *                              groups.
 *      - vectorial_spikes : Read spike times from a double vector of ints
 *                            and construct spike generator neural groups.
 *      - file_spikes      : Read spike times and neurons ids from a binary
 *                           CARLsim file. 
 *      - load_core_params : Load the core parameters for CARLsim. It takes
 *                          three arguments (structs) passed by Python
 *                          interface. 
 *      - count_lies_truths : Counts true and false values for group 
 *                          monitors.
 *      - load_params       : Handles the loading of SPKG and NSAT parameters
 *                          to corresponding structs. 
 *      - group_index       : Returns the index of a group by searching its
 *                          name in the group names vector. 
 *      - count_lines       : Count file lines.
 *      - check_name        : Checks if a neural group name exists.
 *      - read_struct_array : Read and print to stdout a specified value
 *                          of a struct (mainly for debug).
 *      - initialize_groups : Initialize input and NSAT neural groups.
 *      - initialize_connexions : Build all the neural synaptic connections
 *                                  according to some user-defined files.
 *      - initialize_synapses : Create blankout synapses for the NSAT 
 *                              neural groups.
 *      - initialize_stdp : Assign STDP parameters to NSAT neural groups.
 *      - initialize_integration_method : Choose an integration method.
 *      - initialize_conductances : Choose COBA or CUBA simulation type.
 *
 *              Core Methods
 *              ------------
 *      - c_config_state : Performs a CARLsim Config State.
 *      - c_setup_state : Performs a CARLsim Setup State.
 *      - c_run_state : Performs a CARLsim Run State.
 *
 *
 *              CleanUp Methods
 *             ----------------
 *      - c_cleanup : Cleans up the memory. 
 * 
 ***************************************************************************/
class nsat_core {
    private:
        // NSAT Class Fields (attributes)
        // Simulation attributes

        int num_in_groups;
        int num_nsat_groups;

        carlsim carl_p;
        simulation sim_p;
        filenames fnames;

        CARLsim *sim;
        Connx **connex;
        
        // Network attributes
        Grid3D **grid_input_layers, **grid_nsat_layers;

        vector<input_unit> inpc;
        vector<nsat_unit> nsatc;

        vector<int> inp_monitors, nsat_monitors; // NA TO VALW STH LISTA PARAPANW
        vector<string> inp_names, nsat_names;

        // Input attributes
        PoissonRate **psn_spkg;
        PeriodicSpikeGenerator **prd_spkg;
        SpikeGeneratorFromVector **vec_spkg;
        SpikeGeneratorFromFile **file_spkg;

        vector<vector<int>> spike_trains;

    public:
        // NSAT Class constructor and destructor
        nsat_core(filenames *, carlsim *, simulation *);  // Constructor
        ~nsat_core();                        // Destructor

        // NSAT Class core params loader
        void load_core_params(carlsim *c, simulation *s, filenames *);

        // NSAT Class auxiliary methods - load parameters
        void count_lies_truths();
        int load_params(ifstream &, int &, string &);
        int group_index(vector<string>, string);
        int count_lines(ifstream &);
        bool check_name(vector<string>, string);
        void read_struct_array(void *, int);

        // NSAT layers and parameters initialization
        int initialize_layers();
        int initialize_params(char *,
                              string &,
                              int (nsat_core::*)(ifstream &, int &, string &));

        // NSAT input generation methods
        int poisson_spikes();
        int periodical_spikes();
        int vectorial_spikes();
        int file_spikes();
    
        // NSAT Initialization Class Methods
        int initialize_groups();
        int initialize_connexions();
        int initialize_stdp();
        int initialize_integration_method();
        int initialize_conductances();
        void initialize_custom_input(void *, int, int);

        // NSAT Main CARLsim Interface Methods
        int c_config_state();          // CARLsim config state
        int c_setup_state();           // CARLsim setup state
        int c_run_state();             // CARLsim run state
        int c_cleanup();               // Clean up memory
};


/***************************************************************************
 * Substitute of Python ctypes ABI (since it does not exist one for C++)
 * Set MAKE_FLAG to 0 in order to compile as standalone running software
 * Set MAKE_FLAG to 1 in order to compile as a dynamic shared library
 **************************************************************************/
#if (MAKE_FLAG == 1)
    extern "C" {
        nsat_core *NSAT_Core_New(carlsim *carl, simulation *simu, filenames *files) { 
            return new nsat_core(files, carl, simu);
        }
        void NSAT_Core_InitInput(nsat_core *obj, void *ptr, int nspkt, int length) {
            obj->initialize_custom_input(ptr, nspkt, length);
        }
        void NSAT_Core_ReadStructArray(nsat_core *obj, void *ptr, int size) {
            obj->read_struct_array(ptr, size);
        }
        int NSAT_Core_Config(nsat_core *obj){ obj->c_config_state(); }
        int NSAT_Core_Setup(nsat_core *obj){ obj->c_setup_state(); }
        int NSAT_Core_Run(nsat_core *obj){ obj->c_run_state(); }
        int NSAT_Core_CleanUp(nsat_core *obj){ obj->c_cleanup(); }
        void NSAT_Core_Exit(nsat_core *obj){ delete obj; }
    }
#endif

#endif // _NSAT_CORE_H
