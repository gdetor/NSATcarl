#ifndef _CONNX_CORE_H
#define _CONNX_CORE_H

#include <iostream>
#include <vector>

#include <carlsim.h>


using namespace std;


/***************************************************************************
 * CONNX Class - This class implements methods for connecting neural groups
 * with each other based on some predefined synaptic weight matrices and 
 * synaptic delays. 
 *
 * Attributes:
 *      - _flag      : Indicates plasticity on or off.
 *      - _nNeurPre  : Number of pre-synaptic neurons.
 *      - _nNeurPost : Number of post-synaptic neurons.
 *      - _maxWeight : Maximum value for synaptic weights.
 *      - _wt        : A 2D float vector for synaptic connections weights.
 *      - _dlt       : A 2D float vector for synaptic delays. 
 *
 * Methods: 
 *              Construction/Destruction
 *              ------------------------
 *      - nsat_core : NSAT Core constructor.
 *      - ~nsat_core : NSAT Core destructor.
 *
 *              Core Methods
 *              ------------
 *      - setWeightMatrix : Initializes the synaptic weights matrix.
 *      - setDelayMatrix  : Initializes the synaptic delays.
 *      - connect         : Connects pre- and post-synaptic neurons
 *                          according to some logical relation. 
 *
 ***************************************************************************/
class Connx : public ConnectionGenerator {
    private:
        bool _flag;
        int _nNeurPre, _nNeurPost;
        float _maxWeight;
        vector<vector<float>> _wt;
        vector<vector<float>> _dlt;
    public:
        Connx(int, int, bool&, float&);
        ~Connx();
        void setWeightMatrix(vector<vector<float>>);
        void setDelayMatrix(vector<vector<float>>);
        void connect(CARLsim *, int, int, int, int, float&, float&, float&, bool&);
};

#endif // _CONNX_CORE_H
