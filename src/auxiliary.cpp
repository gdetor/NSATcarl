#include "nsat_core.h"


/***************************************************************************
 * NSAT_CORE Auxiliary Functions
 ***************************************************************************/


/***************************************************************************
 * str2bool - Converts a string to a bool.
 *
 * Args:
 * -----
 *  str (string) : Input string to be converted (true of false).
 *
 * Returns:
 * --------
 *  tmp (bool) : Returns a boolean (true of false) depending on the input
 *  string. 
 ***************************************************************************/
bool str2bool(string str) {
    bool tmp;

    transform(str.begin(), str.end(), str.begin(), ::tolower);

    istringstream iss(str);
    iss >> boolalpha >> tmp;
    return tmp;
}


/***************************************************************************
 * str2nrtype - Converts a string to a CARLsim binary neuron type
 *
 * Args:
 * -----
 *  str (string) : Input string to be converted. Acceptable input strings
 *                 are poisson_neuron, excitatory_neuron, inhibitory_neuron,
 *                 dopaminergic_neuron, excitatory_poisson,
 *                 inhibitory_poisson.
 *
 * Returns:
 * --------
 *  tmp (unsigned int) : Is a binary pattern that defines the type of a 
 *                       neuron according to CARLsim's definitions. If 
 *                       a non-recognized input string is given it returns
 *                       135. 
 ***************************************************************************/
unsigned int str2nrtype(string str) {
    unsigned int tmp;

    transform(str.begin(), str.end(), str.begin(), ::tolower);

    if (str == "poisson_neuron") {
        tmp = (1 << 0);
        return tmp;
    } else if (str == "excitatory_neuron") {
        tmp = ((1 << 2)|(1 << 1));
        return tmp;
    } else if (str == "inhibitory_neuron") {
        tmp = ((1 << 3)|(1 << 4));
        return tmp;
    } else if (str == "dopaminergic_neuron") {
        tmp = ((1 << 5) | ((1 << 2)|(1 << 1)));
        return tmp;
    } else if (str == "excitatory_poisson") {
        tmp = (((1 << 2)|(1 << 1))|(1 << 0));
        return tmp;
    } else if (str == "inhibitory_poisson") {
        tmp = (((1 << 3)|(1 << 4))|(1 << 0));
        return tmp;
    } else {
        return 135;
    }
}


/***************************************************************************
 * str2stdpt - This function converts a string to a stdpType_t enumeration
 * type. 
 *
 * Args:
 * -----
 *  str (string) : Input string to be converted.
 *
 * Returns:
 * --------
 *  A stdpType_t enumeration type depending on the input string. If the 
 *  input is not a valid STDP type, it returns UNKNOWN_STDP.
 ***************************************************************************/
stdpType_t str2stdpt(string str) {
    stdpType_t tmp;

    transform(str.begin(), str.end(), str.begin(), ::tolower);

    if (str == "standard") return STANDARD;
    else if (str == "da_mod") return DA_MOD;
    else return UNKNOWN_STDP;
}


/***************************************************************************
 * print_exceptions - This function writes to the standard output an 
 * error message depending on the number of exception it receives as input.
 *
 * Args:
 * -----
 *  e (int) : An integer that indicates the exception.
 *  ...     : Can receive two more inputs. The number of line of the input
 *            file, where an issue has been detected by the handler function
 *            and/or the name of the input file. 
 *
 * Returns:
 * --------
 *  Void 
 ***************************************************************************/
void print_exceptions(int e, ...) {
    int tmp_int;
    char *tmp_str;
    va_list args;

    va_start(args, e);

    switch(e) {
        case 2:
            tmp_int = va_arg(args, int);
            tmp_str = va_arg(args, char *);
            cout << "Exception 2: Not valid parameters found at line [" 
                 << tmp_int << "] in file [" << static_cast<string>(tmp_str)
                 << "]" << endl;
            break;
        case 5: 
            cout << "Exception 5: Failed to deallocate memory!" << endl;
            break;
        case 6:
            cout << "Exception 6: Mismatch between group names (connections)!" << endl;
            break;
        case 7:
            cout << "Exception 7: Not a valid number of neural groups/layers!" << endl;
            break;
        case 8:
            cout << "Exception 8: Not a valid input type!" << endl;
            break;
        case 9:
            cout << "Exception 9: Mismatch of file lines and number of neurons!" << endl;
            break;
        case 10:
            cout << "Exception 10: Missing blankout probability!" << endl;
            break;
        case 11:
            cout << "Exception 11: Wrong group/type in STDP parameters file!" << endl;
            break;
        case 12:
            cout << "Exception 12: Missing parameters in STDP parameters file!" << endl;
            break;
        case 30:
            tmp_int = va_arg(args, int);
            tmp_str = va_arg(args, char *);
            cout << "Exception 30: Too few/more parameters are given at line ["
                 << tmp_int << "] in file ["
                 << static_cast<string>(tmp_str) << "]" << endl;
            break;
        case 40:
            tmp_int = va_arg(args, int);
            tmp_str = va_arg(args, char *);
            cout << "Exception 40: Wrong neuron type found at line ["
                 << tmp_int << "] in file [" << static_cast<string>(tmp_str)
                 << "]" << endl;
            break;
        case 50:
            cout << "Exception 50: Not a valid STDP curve function!" << endl;
            break;
        case 60:
            cout << "Exception 60: Not a valid Integration Method!" << endl;
            break;
        case 70:
            cout << "Exception 70: Not allowed Integration Steps Size!" << endl;
            break;
        case 80:
            cout << "Exception 80: Not a valid Conductance flag!" << endl;
            break;
        case 90:
            cout << "Exception 100: Not a valid Dopamine Mode!" << endl;
            break;
        default:
            break;
    }
    va_end(args);
}
