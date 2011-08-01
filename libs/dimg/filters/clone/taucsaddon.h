#ifndef __MY_TAUCS_ADDON
#define __MY_TAUCS_ADDON

/****************************************************************/
// Sivan's library!!!
#ifndef WIN32
#include <unistd.h>
#include <pthread.h>
#endif
#define TAUCS_CORE_DOUBLE
extern "C" {
#include "taucs.h"
}

#include <vector>
#include <map>

typedef double taucsType;
/****************************************************************/

/// Multiplies two symmetric lower matrices. If dimensions don't
/// match or if the arguments are not good in any other way
// returns NULL. Otherwise returns the new matrix.
taucs_ccs_matrix *Mul2SymmetricMatrices(const taucs_ccs_matrix *mat0,
										const taucs_ccs_matrix *mat1);




taucs_ccs_matrix *Mul2NonSymmetricMatrices(const taucs_ccs_matrix *matA,
										   const taucs_ccs_matrix *matB);

// for usage when it's known that the result is symmetric,
// like A^T * A
taucs_ccs_matrix *Mul2NonSymmMatSymmResult(const taucs_ccs_matrix *matA,
                                           const taucs_ccs_matrix *matB);

/// Computes the transpose of a matrix.
taucs_ccs_matrix *MatrixTranspose(const taucs_ccs_matrix *mat);

taucs_ccs_matrix * CreateTaucsMatrixFromColumns(const std::vector< std::map<int,taucsType> > & cols, 
												int nRows,
												int flags);

// Multiplies matA by x and stores the result
// in b. Assumes all memory has been allocated
// and the sizes match; assumes matA is not
// symmetric!!
void MulNonSymmMatrixVector(const taucs_ccs_matrix *matA,
					        const taucsType * x,
							taucsType * b);

taucs_ccs_matrix * MatrixCopy(const taucs_ccs_matrix *mat);

#endif // __MY_TAUCS_ADDON


