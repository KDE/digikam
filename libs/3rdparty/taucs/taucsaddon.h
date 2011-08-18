#ifndef __MY_TAUCS_ADDON
#define __MY_TAUCS_ADDON

/****************************************************************/
// Sivan's library!!!
#ifndef WIN32
#include <unistd.h>
#include <pthread.h>
#endif

#include <map>
#include <vector>

#define TAUCS_CORE_DOUBLE
//#define TAUCS_CONFIG_DREAL
//#define TAUCS_CORE_GENERAL
extern "C" {
#include "taucs.h"

}


typedef double taucsType;
/****************************************************************/

/// Computes the transpose of a matrix.
taucs_ccs_matrix *MatrixTranspose(const taucs_ccs_matrix *mat);

#endif // __MY_TAUCS_ADDON


