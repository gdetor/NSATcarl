#include "connx_core.h"

/***************************************************************************
 * Connx Class Implementation
 ***************************************************************************/


/***************************************************************************
 * CONNX Class Constructor - This method constructs a Connx instance. 
 *
 * Args:
 * -----
 *  num_neurons_pre (int)  : Number of presynaptic neurons.
 *  num_neurons_post (int) : Number of postsynaptic neurons.
 *  flag (bool)            : Specifies if plasticity is enabled or not.
 *  maxWeight (float)      : Maximum value for synaptic weights.
 *
 * Returns:
 * --------
 *  Void
 *
 * Exceptions:
 * -----------
 ***************************************************************************/
Connx::Connx(int num_neurons_pre,
             int num_neurons_post,
             bool& flag,
             float& maxWeight) {
    _nNeurPre = num_neurons_pre;
    _nNeurPost = num_neurons_post;
    _flag = flag;
    _maxWeight = maxWeight;
}


/***************************************************************************
 * CONNX Class Destructor - This method destroys a Connx instance. 
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
 ***************************************************************************/
Connx::~Connx() {
    // void
}


/***************************************************************************
 * CONNX Class SETWEIGHTMATRIX - This method takes as input a 2D vector of
 * floats representing the synaptic connectivity matrix and assigns the 
 * synaptic strength values to the intrinsic synaptic matrix. 
 *
 * Args:
 * -----
 *  wt (2D float vector) : Input 2D vector containing the neural synaptic 
 *  strengths.
 *
 * Returns:
 * --------
 *  Void
 *
 * Exceptions:
 * -----------
 ***************************************************************************/
void Connx::setWeightMatrix(vector<vector<float>> wt) {
    //FIXME: Remove assert (ingenious way indeed)
    assert(wt.size() == _nNeurPre);
    assert(wt[0].size() == _nNeurPost);
    _wt = wt;
}


/***************************************************************************
 * CONNX Class SETDeLAYMATRIX - This method takes as input a 2D vector of
 * floats representing the synaptic delays and assigns the synaptic
 * strength values to the intrinsic synaptic matrix. 
 *
 * Args:
 * -----
 *  dlt (2D float vector) : Input 2D vector containing the neural synaptic 
 *  delays.
 *
 * Returns:
 * --------
 *  Void
 *
 * Exceptions:
 * -----------
 ***************************************************************************/
void Connx::setDelayMatrix(vector<vector<float>> dlt) {
    //FIXME: Remove assert (ingenious way indeed)
    assert(dlt.size() == _nNeurPre);
    assert(dlt[0].size() == _nNeurPost);
    _dlt = dlt;

}


/***************************************************************************
 * CONNX Class CONNECT - This method implements CARLsim's connect 
 * method. This method is used in order to define custom synaptic 
 * connections.
 *
 * Args:
 * -----
 *  sim (CARLsim *)  : A pointer to a CARLsim class instance.
 *  srcGrp (int)     : Source neural group.
 *  i (int)          : i-th neuron of source group. 
 *  destGrp (int)    : Destination neural group.
 *  j (int)          : j-th neuron of destination group.
 *  weight (float)   : Synaptic weight value for pair (i, j).
 *  maxWt (float)    : Maximum synaptic weight (fixed if no plasticity).
 *  delay (float)    : Synaptic delay in ms.
 *  connected (bool) : Specifies if i and j neurons are connected.
 *
 * Returns:
 * --------
 *  Void
 *
 * Exceptions:
 * -----------
 ***************************************************************************/
void Connx::connect(CARLsim *sim,                       
                    int srcGrp,
                    int i,             
                    int destGrp,
                    int j,            
                    float& weight,                 
                    float& maxWt,                  
                    float& delay,
                    bool& connected) {
    connected = fabsf(_wt[i][j]) > 0.0f; // connect if nonzero weight
    weight = _wt[i][j];
    maxWt = _maxWeight;
    if (_flag == true) {
        maxWt = _wt[i][j];
    }else{
        maxWt = _maxWeight;
    }
    delay = 1;
}
