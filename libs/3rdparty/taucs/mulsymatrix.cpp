#pragma warning( disable : 4786 )
#pragma warning( disable : 4503 )

#include <algorithm>
<<<<<<< HEAD
=======
#include <cstring>
>>>>>>> 594b29498695ab0d05bd26529c53062e7b364d79
#include <iostream>
#include <map>
#include <vector>

#include "taucsaddon.h"

typedef std::map<std::pair<int,int>, taucsType> Pos2ValueMap;
inline std::pair<int,int> GetPos(int i, int j);

inline std::pair<int,int> GetPos(int i, int j) {
	return (i > j)? std::pair<int,int>(j, i)  :  std::pair<int,int>(i, j);
}

/// Computes the transpose of a matrix.
taucs_ccs_matrix *MatrixTranspose(const taucs_ccs_matrix *mat) {
	taucs_ccs_matrix* ret;
	ret = taucs_dtl(ccs_create)(mat->n, mat->m, mat->colptr[mat->n]);
	if (! ret)
		return NULL;

	
	if (mat->flags & TAUCS_SYMMETRIC) {
		// symmetric - just copy the matrix

		memcpy(ret->colptr, mat->colptr, sizeof(int) * (mat->n + 1));
		memcpy(ret->rowind, mat->rowind, sizeof(int) * (mat->colptr[mat->n]));
		memcpy(ret->taucs_values, mat->taucs_values, sizeof(taucsType) * (mat->colptr[mat->n]));

		return ret;
	}

	// non-symmetric matrix -> need to build data structure.
	// we'll go over the columns and build the rows
	std::vector< std::vector<int> >       rows(mat->m);
	std::vector< std::vector<taucsType> > values(mat->m);
	for (int c = 0; c < mat->n; ++c) {
		for (int rowi = mat->colptr[c]; rowi < mat->colptr[c+1]; ++rowi) {
			rows[mat->rowind[rowi]].push_back(c);
			values[mat->rowind[rowi]].push_back(mat->taucs_values[rowi]);
		}
	}

	// copying the rows as columns in ret
	int cind = 0;
	for (int r = 0; r < mat->m; ++r) {
		ret->colptr[r] = cind;

		for (int j = 0; j < (int)rows[r].size(); ++j) {
			ret->rowind[cind] = rows[r][j];
			ret->taucs_values[cind] = values[r][j];
			cind++;
		}
	}
	ret->colptr[mat->m] = cind;

//	assert(cind == mat->colptr[mat->n]);

	return ret;
}

