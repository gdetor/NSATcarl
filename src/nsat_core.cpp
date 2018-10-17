#include "nsat_core.h"
#include "connx_core.h"

using namespace std;


/***************************************************************************
 * NSAT_CORE Class Implementation
 ***************************************************************************/

/***************************************************************************
 * NSAT_CORE Class Constructor - The NSAT Core Class constructor is 
 * responsible for initializing all the CARLsim and simulation parameters.
 * It instantiates CARLsim. 
 *
 * Args:
 * -----
 *  f (filenames *)  : A pointer to filenames struct.  
 *  c (carlsim *)    : A pointer to CARLsim struct.
 *  s (simulation *) : A pointer to simulation struct.
 *
 * Returns:
 * --------
 *  Void
 ***************************************************************************/
nsat_core::nsat_core(filenames *f, carlsim *c, simulation *s) {
    int flag;
    string tmp;

    // Load core parameters
    load_core_params(c, s, f);

    // Load spike generator parameters
    tmp = "spkg";
    flag = initialize_params(f->spkg_fname,
                             tmp,
                             &nsat_core::load_params);
    
    // Load NSAT neurons parameters
    tmp = "nsat";
    flag = initialize_params(f->nsat_fname,
                             tmp,
                             &nsat_core::load_params);
    
    // Initialize neural layers
    flag = initialize_layers();

    // Instantiate CARLsim and allocate memory for it
    sim = new CARLsim(carl_p.sim_name, carl_p.mode,
                      carl_p.logger, carl_p.gpu_index,
                      carl_p.random_seed);

    // Count groups that have to be monitored
    count_lies_truths();
}


/***************************************************************************
 * NSAT_CORE Class Destructor - The NSAT Core Class destructor. It 
 * deallocates the memory previously allocated for the CARLsim class
 * instance.
 *
 * Args:
 * -----
 *  Void
 *
 * Returns:
 * --------
 *  Void
 ***************************************************************************/
nsat_core::~nsat_core() {
    // Clean up CARLsim obejct - instance
    delete sim;
}


/***************************************************************************
 * NSAT_CORE C_CLEANUP - This method cleans up the memory by deallocating
 * all the previously allocated memory. 
 *
 * Args:
 * -----
 *  Void
 *
 * Returns:
 * --------
 *  - 0 if succesfully deallocates the memory, otherwise
 *  it throws bad allocation exception. 
 ***************************************************************************/
int nsat_core::c_cleanup() {
    try {
        // Cast char* to string and make convert to lowercase
        string tmp = static_cast<string>(sim_p.input_type);
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
    
        // Clean up input and nsat layers arrays
        for (int i = 0; i < num_in_groups; ++i) {
            delete grid_input_layers[i];
        }
        delete[] grid_input_layers;
        for (int i = 0; i < num_nsat_groups; ++i) {
            delete grid_nsat_layers[i];
        }
        delete[] grid_nsat_layers;

        // Clean up input arrays
        if (tmp == "poisson" ) {
            for (int i = 0; i < num_in_groups; ++i)
                delete psn_spkg[i];
            delete[] psn_spkg;
        } else if (tmp == "periodical") {
            for (int i = 0; i < num_in_groups; ++i)
                delete prd_spkg[i];
            delete[] prd_spkg;
        } else if (tmp == "vectorial") {
            for (int i = 0; i < num_in_groups; ++i)
                delete vec_spkg[i];
            delete[] vec_spkg;
        } else { cerr << "Not a recognized input type!" << endl; }

        // Clean up connections arrays
        delete connex;
    }
    catch (...) {
        cout << "Memory Exception: Memory cannot be freed!" << endl;
    }
    return 0;
}


/***************************************************************************
 * NSAT_CORE LOAD_CORE_PARAMS - This method loads the core parameters for
 * the simulation. It assigns parameters filenames, CARLsim parameters and
 * simulation parameters to the corresponding structs. 
 *
 * Args:
 * -----
 *  f (filenames *)  : A pointer to filenames struct.  
 *  c (carlsim *)    : A pointer to CARLsim struct.
 *  s (simulation *) : A pointer to simulation struct.
 *
 * Returns:
 * --------
 *  Void
 ***************************************************************************/
void nsat_core::load_core_params(carlsim *c,
                                 simulation *s,
                                 filenames *f) {
    // Assign filenames
    fnames.spkg_fname = f->spkg_fname;
    fnames.nsat_fname = f->nsat_fname;
    fnames.stdp_fname = f->stdp_fname;
    fnames.conn_fname = f->conn_fname;
    fnames.delay_fname = f->delay_fname;
    fnames.finp_spikes = f->finp_spikes;

    // Assign CARLsim parameters
    carl_p.sim_name = c->sim_name;
    carl_p.mode = c->mode;
    carl_p.logger = c->logger;
    carl_p.gpu_index = c->gpu_index;
    carl_p.random_seed = c->random_seed;
    
    // Assign Simulation parameters
    sim_p.int_method = s->int_method;
    sim_p.maxWt = s->maxWt;
    sim_p.sim_time_sec = s->sim_time_sec;
    sim_p.sim_time_msec = s->sim_time_msec;
    sim_p.int_num_steps = s->int_num_steps;
    sim_p.num_connections = s->num_connections;
    sim_p.input_type = s->input_type;
    sim_p.print_summary = s->print_summary;
    sim_p.copy_state = s->copy_state;
    sim_p.remove_tmp_mem = s->remove_tmp_mem;
    sim_p.coba_enabled = s->coba_enabled;
}


/***************************************************************************
 * NSAT_CORE READ_STRUCT_ARRAY - This method takes as arguments a void 
 * pointer and prints to the stdout its content. 
 * 
 * Args:
 * -----
 *  *ptr (void *)   : Void pointer containing the data
 *  size (int)      : Size of data chunk
 *
 * Returns:
 * --------
 *  Void
 * 
 * Exceptions: 
 * -----------
 * 
 * Caveats:
 *  This method is used only for debugging purposes!
 ***************************************************************************/
void nsat_core::read_struct_array(void *ptr, int size) {
    const nsat * tmp = (nsat *) ptr;

    for (int i = 0; i < size; ++i) {
        cout << tmp[i].alpha << endl;
    }
}


/***************************************************************************
 * NSAT_CORE LOAD_PARAMS - This method loads spike generator, NSAT and 
 * STDP parameters to the corresponding data structs. 
 * 
 * Args:
 * -----
 *  file (ifstream &)   : Input file stream.
 *  count_lines (int &) : Counter of file lines - used by 
 *                        print_exceptions function.
 *  flag (string &)     : Indicates the type of parameters 
 *                        to load. spkg, nsat or stdp.
 *
 * Returns:
 * --------
 *  0 if succesfully all the parameters are loaded, otherwise it throws
 *  exceptions depending on the underlying error. 
 * 
 * Exceptions: 
 * -----------
 *  10 : An empty line is detected in the input file.
 *  20 : An non-numeric character is detected in the input file.
 *  30 : A parameter value is missing.
 *  40 : Not a valid type of neuron (NSAT).
 *  90 : Not a valid Dopamine type.
 ***************************************************************************/
int nsat_core::load_params(ifstream &file, int &count_lines, string &flag) {
    int num_ingroups = 0, num_nsatgroups = 0;
    string line;
    regex com_sec("^\\s #.|^#.|\\[|^#");

    // Read Spike Generator parameters - Input
    if (flag == "spkg") {
        input_unit tmp_unit;

        // Read line by line the input file and assign values
        // to parameters
        count_lines = 0;
        while (getline(file, line)) {
            count_lines++;
            if (line.empty()) { continue; }     // Empty lines
            else if (regex_search(line, com_sec)) { continue; } // Non-numerics
            else {
                istringstream iss(line);
                vector<string> tokens{istream_iterator<string>{iss},
                                      istream_iterator<string>{}};
                if (tokens.size() != 8) { throw 30; }    

                tmp_unit.unit_name = tokens[0];
                inp_names.push_back(tmp_unit.unit_name);

                tmp_unit.num_neurons = stoi(tokens[1]);
                tmp_unit.unit_type = str2nrtype(tokens[2]);
                
                if (tmp_unit.unit_type == 135) { throw 40; }

                tmp_unit.spkg_p.on_gpu = str2bool(tokens[3]);
                tmp_unit.spkg_p.rate = stof(tokens[4]);
                tmp_unit.spkg_p.freq = stof(tokens[5]);
                tmp_unit.spkg_p.spk_at_zero = str2bool(tokens[6]);
                tmp_unit.mflag = str2bool(tokens[7]);

                inpc.push_back(tmp_unit);
                num_ingroups++;
            }
        }
        num_in_groups = num_ingroups;
    // Read NSAT parameters    
    } else if (flag == "nsat") {
        nsat_unit tmp_unit;

        count_lines = 0;
        while (getline(file, line)) {
            count_lines++;
            if (line.empty()) { continue; }
            else if (regex_search(line, com_sec)) { continue; }
            else {
                istringstream iss(line);
                vector<string> tokens{istream_iterator<string>{iss},
                                      istream_iterator<string>{}};

                if (tokens.size() != 12) { throw 30; }
                
                tmp_unit.unit_name = tokens[0];
                nsat_names.push_back(tmp_unit.unit_name);

                tmp_unit.num_neurons = stoi(tokens[1]);
                tmp_unit.unit_type = str2nrtype(tokens[2]);
                if (tmp_unit.unit_type == 135) { throw 40; }
                
                tmp_unit.nsat_p.alpha = stof(tokens[3]);
                tmp_unit.nsat_p.beta = stof(tokens[4]);
                tmp_unit.nsat_p.sigma = stof(tokens[5]);
                tmp_unit.nsat_p.v_th = stof(tokens[6]);
                tmp_unit.nsat_p.v_reset = stof(tokens[7]);
                tmp_unit.nsat_p.b = stof(tokens[8]);
                tmp_unit.nsat_p.tau_ref = stoi(tokens[9]);
                tmp_unit.nsat_p.alphaS = stof(tokens[10]);
                tmp_unit.mflag = str2bool(tokens[11]);

                nsatc.push_back(tmp_unit);
                num_nsatgroups++;
            }
        }
        num_nsat_groups = num_nsatgroups;
    } else {
        throw 2;
    }
    return 0;
}


/***************************************************************************
 * NSAT_CORE LOAD_GRID_PARAMS - This method loads the layers related
 * parameters. These parameters define network topology according to
 * CARLsim methods. 
 *
 * Args:
 * -----
 *  file (ifstream &)   : Input file stream.
 *  count_lines (int &) : Counter of file lines - used by print_exceptions
 *                        function.
 *
 * Returns:
 * --------
 *  0 if succesfully all the parameters are loaded, otherwise it throws
 *  exceptions depending on the underlying error. 
 * 
 * Exceptions: 
 * -----------
 *  7  : Not a valid number of neural input groups.
 ***************************************************************************/
int nsat_core::initialize_layers() {
    // Initialize input layers
    // Check the number of input layers
    if (num_in_groups <= 0) { throw 7; } 
    grid_input_layers = new Grid3D*[num_in_groups]; 

    for (int i = 0; i < num_in_groups; ++i)
        grid_input_layers[i] = new Grid3D(inpc[i].num_neurons, 1, 1);

    // Initialize NSAT layers
    // Check the number of NSAT layers
    if (num_nsat_groups <= 0) { throw 7; } 
    grid_nsat_layers = new Grid3D*[num_nsat_groups]; 

    for (int i = 0; i < num_nsat_groups; ++i)
        grid_nsat_layers[i] = new Grid3D(nsatc[i].num_neurons, 1, 1);
    return 0;
}


/***************************************************************************
 * NSAT_CORE PARAMS_INITIALIZATION - This method handles LOAD_PARAMS 
 * method. It opens an input filestream and tries to read from it passing 
 * the handler to LOAD_PARAMS method. If it fails, it throws an exception.
 *
 * Args:
 * -----
 *  fname (char *)  : Input file stream.
 *  type (string &) : Type of parameters SPKG, NSAT, STDP.
 *  fun (int)       : A pointer to a function. User can provide its own 
 *                    function for loading more parameters than these 
 *                    already provided. 
 *
 * Returns:
 * --------
 *  Flag (int), which in a succesful parameters initialization is 0, 
 *  otherwise it throws an exception. 
 *
 * Exceptions:
 * -----------
 *  Read I/O failure or all the possible exceptions described in 
 *  print_exception() function (see auxiliary.cpp). 
 ***************************************************************************/
int nsat_core::initialize_params(char *fname, string &type,
                     int(nsat_core::*fun)(ifstream &, int &, string &)) {
    int flag;
    int count_lines;
    ifstream infile;
    infile.exceptions(ifstream::badbit);

    // Try to open a file and read parameters
    try {
        infile.open(fname, fstream::in);
        flag = (this->*fun)(infile, count_lines, type);
    }
    // Catch file I/O exceptions
    catch (ifstream::failure &e){
        cout << e.what() << endl;
    }
    // Catch other exceptions - see auxiliary.cpp
    catch (int &e) {
        print_exceptions(e, count_lines, fname);
    }
    infile.close();
    return flag;
}


/***************************************************************************
 * NSAT_CORE INITIALIZE_GROUPS - This method initializes the neural groups.
 * It takes care of input neural groups initialization as well as NSAT 
 * neural groups.
 *
 * Args:
 * -----
 *  Void
 *
 * Returns:
 * --------
 * It returns 0 if succesfully all the neural groups are initialized, 
 * otherwise it throws an exception.
 *
 * Exceptions:
 * -----------•••••
 *  7  : Not a valid number of neural input groups.
 ***************************************************************************/
int nsat_core::initialize_groups() {
    // Check numbers of input and NSAT neural groups
    if (num_in_groups <= 0 &&
        num_nsat_groups <= 0) { throw 7; }
    
    // Input - Spike Generators - groups creation
    for (int i = 0; i < num_in_groups; ++i) {
        // Use CARLsim createSpikeGeneratorGroup method
        inpc[i].unit_id = sim->createSpikeGeneratorGroup(inpc[i].unit_name,
                                                         *grid_input_layers[i],
                                                         inpc[i].unit_type);
    }

    // NSAT groups creation
    for (int i = 0; i < num_nsat_groups; ++i) {
        // Use CaRLsim createGroupNSAT method
        nsatc[i].unit_id = sim->createGroupNSAT(nsatc[i].unit_name,
                                                *grid_nsat_layers[i],
                                                nsatc[i].unit_type);
    
        // Set NSAT parameters to each group
        sim->setNeuronParametersNSAT(nsatc[i].unit_id,
                                     nsatc[i].nsat_p.alphaS,
                                     nsatc[i].nsat_p.alpha,
                                     nsatc[i].nsat_p.beta,
                                     nsatc[i].nsat_p.sigma,
                                     nsatc[i].nsat_p.v_th,
                                     nsatc[i].nsat_p.v_reset,
                                     nsatc[i].nsat_p.tau_ref,
                                     nsatc[i].nsat_p.b); 
    }
    return 0;
}


/***************************************************************************
 * NSAT_CORE GROUP_INDEX - This method takes as arguments a vector of 
 * strings and a key string, searches for the key in the string vector and
 * returns the position (int) of the key in the vector. 
 *
 * Args:
 * -----
 *  str (vector<string>)    : Input vector string
 *  key (string)            : Key string 
 *
 * Returns:
 * --------
 *  The position of the key in the vector 
 *  -1 if the key is not found
 *
 * Exceptions:
 * -----------
 *
 ***************************************************************************/
int nsat_core::group_index(vector<string> str, string key) {
    int len = str.size();
    int pos = find(str.begin(), str.end(), key) - str.begin();
    if (pos > len) {
        return -1;
    }else{
        return pos;
    }
}


/***************************************************************************
 * NSAT_CORE CHECK_NAME - This method takes as a vector string and a string
 * key and checks if the string exists in the vector. 
 *
 * Args:
 * -----
 *  str (vector<string>)    : Input vector string
 *  key (string)            : Search key string
 *
 * Returns:
 * --------
 *  True if the key exists in the str vector, False otherwise. 
 *
 * Exceptions:
 * -----------
 *
 ***************************************************************************/
bool nsat_core::check_name(vector<string> str, string key) {
    bool tmp;
    tmp = (find(str.begin(), str.end(), key) != str.end());
    return tmp;
}


/***************************************************************************
 * NSAT_CORE COUNT_LINES - This method takes as input a file input stream
 * and counts the lines of the file. 
 *
 * Args:
 * -----
 *  file (ifstream &)       : Input file stream
 *
 * Returns:
 * --------
 *  NUmber of lines
 *
 * Exceptions:
 * -----------
 *
 ***************************************************************************/
int nsat_core::count_lines(ifstream &file) {
    int tmp = count(istreambuf_iterator<char>(file),
                    istreambuf_iterator<char>(), '\n');
    return tmp;
}


/***************************************************************************
 * NSAT_CORE INITIALIZE_CONNEXIONS - This method takes as input two 2D
 * vectors, one for synaptic connections and a second one for delays, 
 * and initializes the synaptic weights and delays according to CARLsim'
 * ConnectionGenerator methods.
 *
 * Args:
 * -----
 *  Void
 *
 * Returns:
 * --------
 *  0 if all synaptic connections are succesfully initialized, otherwise
 *  it throws an exception.
 *
 * Exceptions:
 * -----------
 *  6  : Mismatch between group names.
 *  7  : Not a valid number of neural input groups.
 *  10 : Missing blankout probability.
 ***************************************************************************/
// FIXIT: Need to be more generic
int nsat_core::initialize_connexions() {
    int prob_flag;
    int idx_src, idx_dest;
    int row, col;
    int src_id, dest_id;
    bool flag(false);
    float prob(0.0), std(0.0);
    string line;

    connex = new Connx*[sim_p.num_connections];
    
    for (int k = 0; k < sim_p.num_connections; ++k) {
        vector<vector<float>> wt_ij;
        ifstream infile(static_cast<string>(fnames.conn_fname[k]));
        getline(infile, line);
        istringstream iss(line);
        vector<string> tokens{istream_iterator<string>{iss},
                              istream_iterator<string>{}};

        if ((tokens.size() == 4) || (tokens.size() == 5)) {
            // Source: Input group
            if (tokens[2] == "true") {
                // Either check_name for checking for existence of -1 from group_index
                if (!check_name(inp_names, tokens[0])) { throw 6; }
                idx_src = group_index(inp_names, tokens[0]);
                row = inpc[idx_src].num_neurons;
                src_id = inpc[idx_src].unit_id;
            // Source: NSAT group
            } else {
                if (!check_name(nsat_names, tokens[0])) { throw 6; }
                idx_src = group_index(nsat_names, tokens[0]);
                row = nsatc[idx_src].num_neurons;
                src_id = nsatc[idx_src].unit_id;
            }
            // Destination: NSAT group
            if (!check_name(nsat_names, tokens[1])) { throw 6; }
            idx_dest = group_index(nsat_names, tokens[1]);
            col = nsatc[idx_dest].num_neurons;
            dest_id = nsatc[idx_dest].unit_id;

            // Blankout probability
            prob_flag = tokens.size();
            if (prob_flag == 4) { prob = stof(tokens[3]); }
            else {
                prob = stof(tokens[3]);
                std = stof(tokens[4]);
            }

            // Assign synaptic strengths
            for(int i = 0; i < row; ++i) {
                getline(infile, line);
                istringstream iss(line);
                vector <string> tokens = {istream_iterator<string>{iss},
                                          istream_iterator<string>{}};

                if (tokens.size() != col) {
                    throw 7;
                }
                vector<float> temp;
                for (auto &t: tokens) {
                    temp.push_back(stof(t));
                }
                wt_ij.push_back(temp);
            }

            // Allocate space for the new connection
            connex[k] = new Connx(row, col, flag, sim_p.maxWt);
            connex[k]->setWeightMatrix(wt_ij);

            // Create the new connection
            if (prob_flag == 4) {
                sim->connectNSAT(src_id, dest_id, connex[k],
                                 BlankOutProb(prob), 
                                 SYN_PLASTIC);
            }else{
                sim->connectNSAT(src_id, dest_id, connex[k],
                                 BlankOutProb(prob, std),
                                 SYN_PLASTIC);
            }

            // Close the file :p
            infile.close();
        }else{ throw 10; }
    }
    return 0;
}


/***************************************************************************
 * NSAT_CORE INITIALIZE_STDP - This method reads STDP parameters and 
 * initializes all the STDP methods on NSAT groups. 
 *
 * Args:
 * -----
 *  Void
 *
 * Returns:
 * --------
 *  (int) 0 if succesfully assigns all the STDP parameters, otherwise it
 *  throws an exception. 
 *
 * Exceptions:
 * -----------
 *  7  : Not a valid number of neural input groups.
 *  11 : Wrong group/type in STDP parameters file.
 *  12 : Missing parameters in STDP parameters file.
 *  50 : Not a valid STDP curve function.
 ***************************************************************************/
int nsat_core::initialize_stdp() {
    int group_id;
    string line;
    ifstream infile(static_cast<string>(fnames.stdp_fname));

    while(getline(infile, line)) {
        istringstream iss(line);
        vector<string> tokens{istream_iterator<string>{iss},
                              istream_iterator<string>{}};

        if (tokens[0] == "#") {
            continue;
        }else {
            if (tokens.size() != 14) {
                throw 12;
            }
            group_id = group_index(nsat_names, tokens[0]);
            if (tokens[1] == "E") {
                switch(stoi(tokens[3])) {
                    case 0:
                        sim->setESTDP(nsatc[group_id].unit_id,
                                      str2bool(tokens[4]),
                                      str2stdpt(tokens[2]),
                                      ExpCurve(stof(tokens[5]),
                                               stof(tokens[6]),
                                               -stof(tokens[7]),
                                               stof(tokens[8])));
                        break;
                    // Time-based STDP curve
                    case 1:
                        sim->setESTDP(nsatc[group_id].unit_id,
                                      str2bool(tokens[4]),
                                      str2stdpt(tokens[2]),
                                      TimingBasedCurve(stof(tokens[5]),
                                                       stof(tokens[6]),
                                                       -stof(tokens[7]),
                                                       stof(tokens[8]),
                                                       stof(tokens[13])));
                        break;
                    // Not valid STDP curve - throws exception
                    default:
                        throw 50;
                        break;
                }
            }else if (tokens[1] == "I") {
                switch(stoi(tokens[3])) {
                    case 0:
                        sim->setISTDP(nsatc[group_id].unit_id,
                                      str2bool(tokens[4]),
                                      str2stdpt(tokens[2]),
                                      ExpCurve(-stof(tokens[5]),
                                               stof(tokens[6]),
                                               stof(tokens[7]),
                                               stof(tokens[8])));
                        break;
                    // Time-based STDP curve
                    case 1:
                        sim->setISTDP(nsatc[group_id].unit_id,
                                      str2bool(tokens[4]),
                                      str2stdpt(tokens[2]),
                                      PulseCurve(stof(tokens[9]),
                                                 stof(tokens[10]),
                                                 stof(tokens[11]),
                                                 stof(tokens[12])));
                        break;
                    // Not valid STDP curve - throws exception
                    default:
                        throw 50;
                        break;
                }
            }else{
                throw 11;
            }
        }
    }
    infile.close();
    return 0;
}


/***************************************************************************
 * NSAT_CORE INITIALIZE_INTEGRATION_METHOD - This method sets the numerical 
 * integration method. The user can choose between Forward Euler's method
 * and Runge-Kutta 4. 
 *for auto iterator c++
 * Args:
 * -----
 *  Void
 *
 * Returns:
 * --------
 *  (int) 0 if succesfully initializes the integration method, otherwise
 *  it throws an exception. 
 *
 * Exceptions:
 * -----------
 *  60 : Not a valid integration method.
 *  70 : Not allowed integration steps size.
 ***************************************************************************/
int nsat_core::initialize_integration_method() {
    if ((sim_p.int_method != FORWARD_EULER) && 
        (sim_p.int_method != RUNGE_KUTTA4)) throw 60;
    else if (1 > sim_p.int_num_steps || sim_p.int_num_steps > 100) throw 70;
    else sim->setIntegrationMethod(sim_p.int_method, sim_p.int_num_steps);
    return 0;
}


/***************************************************************************
 * NSAT_CORE INITIALIZE_CONDUCTANCES - This method sets the conductances 
 * on or off. It permits for conductance-based or current-based simulations
 * (COBA or CUBA).
 *
 * Args:
 * -----
 *  Void
 *
 * Returns:
 * --------
 *  (int) 0 if a succesful choice has been made, otherwise it throws an
 *  exception. 
 *
 * Exceptions:
 * -----------
 *  80 : Not a valid conductance flag (true or false).
 ***************************************************************************/
int nsat_core::initialize_conductances() {
    // Check if conductances flag is valid (true of false)
    if (sim_p.coba_enabled != true &&
        sim_p.coba_enabled != false) { throw 80; }
    
    // Enable (true) or disable (false) conductances
    sim->setConductances(sim_p.coba_enabled);
    return 0;
}


/***************************************************************************
 * SAT_CORE Class C_CONFIG_STATE - This method implements CARLsim's 
 * Config State. Here the initialization of the whole neural network 
 * takes place. 
 *
 * Args:
 * -----
 *  Void
 *
 * Returns:
 * --------
 *  flag (int), which is normally 0, if successfully initializes the 
 *  network, otherwise it throws an exception. 
 *
 * Exceptions:
 * -----------
 *  See auxiliary.cpp for more about exceptions. 
 ***************************************************************************/
int nsat_core::c_config_state() {
    int flag;

    // Try to initialize everything
    try {
        flag = initialize_groups();             
        flag = initialize_connexions();                
        flag = initialize_stdp();               
        flag = initialize_conductances();
        flag = initialize_integration_method(); 
    }
    // Catch possible exceptions - see auxiliary.cpp
    catch (int &e) {
        print_exceptions(e);
    }

    return flag;
}


/***************************************************************************
 * NSAT_CORE POISSON_SPIKES - This method initializes Poisson spike 
 * generators as inputs. Each input group can be associated with one 
 * Poisson spike generator.
 *
 * Args:
 * -----
 *  Void
 *
 * Returns:
 * --------
 *  0 (int) if successfully creates Poisson spike generators. Otherwise
 *  it throws an exception. 
 *
 * Exceptions:
 * -----------
 *  7  : Not a valid number of neural input groups.
 ***************************************************************************/
int nsat_core::poisson_spikes() {
    // Check if the number of input groups is valid
    if (num_in_groups <= 0) { throw 7; }

    psn_spkg = new PoissonRate*[num_in_groups];

    // Construct PoissonRate objects
    for (int i = 0; i < num_in_groups; ++i)
        psn_spkg[i] = new PoissonRate(inpc[i].num_neurons,
                                      inpc[i].spkg_p.on_gpu);

    // Assign PoissonRates to input neural groups
    for (int i = 0; i < num_in_groups; ++i) {
        psn_spkg[i]->setRates(inpc[i].spkg_p.rate);
        sim->setSpikeRate(inpc[i].unit_id, psn_spkg[i]);
    }
    return 0;
}


/***************************************************************************
 * NSAT_CORE PERIODICAL_SPIKES - This method builds periocic spike 
 * generators. Each input group can be associated with one periodic
 * spike generator.
 *
 * Args:
 * -----
 *  Void
 *
 * Returns:
 * --------
 *  0 (int) if successfully creates periodic spike generators. Otherwise
 *  it throws an exception. 
 *
 * Exceptions:
 * -----------
 *  7  : Not a valid number of neural input groups.
 ***************************************************************************/
int nsat_core::periodical_spikes() {
    // Check if the number of input groups is valid
    if (num_in_groups <= 0) { throw 7; }

    prd_spkg = new PeriodicSpikeGenerator*[num_in_groups];

    // Construct PeriodicSpikeGenerator objects
    for (int i = 0; i < num_in_groups; ++i)
        prd_spkg[i] = new PeriodicSpikeGenerator(inpc[i].spkg_p.freq,
                                                 inpc[i].spkg_p.spk_at_zero);

    // Assign PeriodicSpikeGenerators to input neural groups
    for (int i = 0; i < num_in_groups; ++i)
        sim->setSpikeGenerator(inpc[i].unit_id, prd_spkg[i]);
    return 0;
}


/***************************************************************************
 * NSAT_CORE VECTORIAL_SPIKES - This method builds spike generators from a
 * input 2D vector. Each input group can be associated with a 1D vector of
 * spike trains.
 *
 * Args:
 * -----
 *  Void
 *
 * Returns:
 * --------
 *  0 (int) if successfully builds spike generators. Otherwise it throws
 *  an exception. 
 *
 * Exceptions:
 * -----------
 *  7  : Not a valid number of neural input groups.
 ***************************************************************************/
int nsat_core::vectorial_spikes() {
    // Check if the number of input groups is valid
    if (num_in_groups <= 0) { throw 7; }

    vec_spkg = new SpikeGeneratorFromVector*[num_in_groups];

    // Construct SpikeGeneratorFromVector objects
    for (int i = 0; i < num_in_groups; ++i)
        vec_spkg[i] = new SpikeGeneratorFromVector(spike_trains[i]);

    // Assign SpikeGeneratorFromVector to input neural groups
    for (int i = 0; i < num_in_groups; ++i)
        sim->setSpikeGenerator(inpc[i].unit_id, vec_spkg[i]);
    return 0;
}


/* FIXME : Put some comments here :p */
int nsat_core::file_spikes() {
    string str;
    // Check if the number of input groups is valid
    if (num_in_groups <= 0) { throw 7; }

    file_spkg = new SpikeGeneratorFromFile*[num_in_groups];

    // For each input group set its spike gen
    for (int i = 0; i < num_in_groups; ++i) {
        str = static_cast<string>(fnames.finp_spikes[i]);
        file_spkg[i] = new SpikeGeneratorFromFile(str);
    }

    for(int i = 0; i < num_in_groups; ++i) {
        sim->setSpikeGenerator(inpc[i].unit_id, file_spkg[i]);
    }
    return 0;
}


/***************************************************************************
 * NSAT_CORE INITIALIZE_CUSTOM_SPIKES - C-Python interface function
 *
 * Args:
 * -----
 *  *ptr (void *)           : Abstract data pointer
 *  num_spike_trains (int)  : Number of spike trains
 *  length (int)            : Length of spike trains
 *
 * Returns:
 * --------
 *      Void
 *
 * Exceptions:
 * -----------
 * 
 ***************************************************************************/
void nsat_core::initialize_custom_input(void *ptr,
                                        int num_spike_trains,
                                        int length) {
    const int * tmp = (int *) ptr;
    for (int i = 0; i < num_spike_trains; ++i) {
        vector<int> temp;
        for (int j = 0; j < length; ++j) {
            temp.push_back(tmp[i*num_spike_trains+j]);
        }
        spike_trains.push_back(temp);
    }
}


/***************************************************************************
 * NSAT_CORE C_SETUP_STATE - This method implements the CARLsim's setup
 * state. In this method the neural network is setup. 
 *
 * Args:
 * -----
 *  Void
 *
 * Returns:
 * --------
 *  flag (int), which normally is 0, when the input spike generators and the 
 *  network have been set up properly. Otherwise it throws an exception. 
 *
 * Exceptions:
 * -----------
 *  8  : Not a valid input type.  
 ***************************************************************************/
int nsat_core::c_setup_state() {
    int flag;
    string tmp = static_cast<string>(sim_p.input_type);
    // Convert all characters to lowercase  !!
    transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);

    // Poisson spikes
    if (tmp == "poisson") {
        sim->setupNetwork(sim_p.remove_tmp_mem);
        flag = poisson_spikes();
    }
    // Periodical spike trains
    else if (tmp == "periodical") {
        flag = periodical_spikes();
        sim->setupNetwork(sim_p.remove_tmp_mem);
    }
    // Spikes from c++ vector 
    else if (tmp == "vectorial") {
        flag = vectorial_spikes();
        sim->setupNetwork(sim_p.remove_tmp_mem);
    }
    // Spikes from file
    else if (tmp == "fromfile") {
        flag = file_spikes();
        sim->setupNetwork(sim_p.remove_tmp_mem);
    }
    // Throw an exception
    else { throw 8; }
    return flag;
}


/***************************************************************************
 * SAT_CORE Class COUNT_LIES_TRUTHS - This method counts the number of 
 * bool flags for momitoring neural populations. If a flag is true then
 * the corresponding index is saved to a vector (like a list). 
 *
 * Args:
 * -----
 *  Void
 *
 * Returns:
 * --------
 *  Void
 *
 * Exceptions:
 * -----------
 *  Runtime errors subjecting to vector class.
 ***************************************************************************/
void nsat_core::count_lies_truths() {
    // Count input neural groups monitors flags
    for (int i = 0; i < num_in_groups; ++i) {
        if (inpc[i].mflag == true) {
            inp_monitors.push_back(i);
        }
    }

    // Count NSAT neural groups monitors flags
    for (int i = 0; i < num_nsat_groups; ++i) {
        if (nsatc[i].mflag == true) {
            nsat_monitors.push_back(i);
        }
    }
}


/***************************************************************************
 * SAT_CORE Class RUN_STATE - This method implements CARLsim's Run State.
 * In this state the neural network is simulated and the results are 
 * saved according to previously given parameters. 
 *
 * Args:
 * -----
 *  Void
 *
 * Returns:
 * --------
 *  flag (int), which is normally 0, when the network has been simulated
 *  successfully. Otherwise it throws an exception. 
 *
 * Exceptions:
 * -----------
 *  Runtime errors according to CARLsim methods. 
 ***************************************************************************/
int nsat_core::c_run_state() {
    int flag, ii = 0;
    int inp_size = inp_monitors.size();
    int nsat_size = nsat_monitors.size();

    // Set the external current to NSAT groups
    // FIXME This can be neglected later - only for test purposes here
	// sim->setExternalCurrent(nsatc[0].unit_id, 0.1f);

    // ConnectionMonitor * CM = sim->setConnectionMonitor(inpc[0].unit_id,
    //                                                    nsatc[0].unit_id,
    //                                                    "DEFAULT");
    // vector<vector<float>> weights = CM->takeSnapshot();
    // cout << weights[0][0] << endl;

    SpikeMonitor **inSM, **nsatSM;

    inSM = new SpikeMonitor*[inp_size];
    nsatSM = new SpikeMonitor*[nsat_size];
    
    // Setup and start spike monitors for input groups
    for (auto &i : inp_monitors) {
        inSM[ii] = sim->setSpikeMonitor(inpc[i].unit_id, "DEFAULT");
        inSM[ii]->startRecording();
        ii++;
    }

    // Setup and start spike monitors for NSAT groups
    ii = 0;
    for (auto &i : nsat_monitors) {
        nsatSM[ii] = sim->setSpikeMonitor(nsatc[i].unit_id, "DEFAULT");
        nsatSM[ii]->startRecording();
        ii++;
    }

    // Run the network
	flag = sim->runNetwork(sim_p.sim_time_sec,
                           sim_p.sim_time_msec,
                           sim_p.print_summary,
                           sim_p.copy_state);

    // Stop spike monitors for input and NSAT
    for (int i = 0; i < inp_size; ++i)
        inSM[i]->stopRecording();
    for (int i = 0; i < nsat_size; ++i)
        nsatSM[i]->stopRecording();

    // Cleanup memory for spike monitors
    delete[] inSM;
    delete[] nsatSM;

    return flag;
}
