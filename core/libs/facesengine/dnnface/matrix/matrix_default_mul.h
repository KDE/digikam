/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2017-08-08
 * Description : Base functions for dnn module, can be used for face recognition, 
 *               all codes are ported from dlib library (http://dlib.net/)
 *
 * Copyright (C) 2006-2016 by Davis E. King <davis at dlib dot net>
 * Copyright (C) 2017      by Yingjie Liu <yingjiewudi at gmail dot com>
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DLIB_MATRIx_DEFAULT_MULTIPLY_
#define DLIB_MATRIx_DEFAULT_MULTIPLY_

#include "../dnn_base/rectangle.h"
#include "matrix.h"
#include "matrix_utilities.h"
#include "../dnn_base/enable_if.h"
//#include <omp.h>
#include <cstring>

//namespace dlib
//{

// ------------------------------------------------------------------------------------

    namespace ma
    {
        template < typename EXP, typename enable = void >
        struct matrix_is_vector { static const bool value = false; };
        template < typename EXP >
        struct matrix_is_vector<EXP, typename enable_if_c<EXP::NR==1 || EXP::NC==1>::type > { static const bool value = true; };
    }

// ------------------------------------------------------------------------------------

    /*!  This file defines the default_matrix_multiply() function.  It is a function 
         that conforms to the following definition:

        template <
            typename matrix_dest_type,
            typename EXP1,
            typename EXP2
            >
        void default_matrix_multiply (
            matrix_dest_type& dest,
            const EXP1& lhs,
            const EXP2& rhs
        );
            requires
                - (lhs*rhs).destructively_aliases(dest) == false
                - dest.nr() == (lhs*rhs).nr()
                - dest.nc() == (lhs*rhs).nc()
            ensures
                - #dest == dest + lhs*rhs
    !*/

// ------------------------------------------------------------------------------------

    template <
        typename matrix_dest_type,
        typename EXP1,
        typename EXP2
        >
    typename enable_if_c<ma::matrix_is_vector<EXP1>::value == true || ma::matrix_is_vector<EXP2>::value == true>::type 
    default_matrix_multiply (
        matrix_dest_type& dest,
        const EXP1& lhs,
        const EXP2& rhs
    )
    {//std::cout << "default matrix multiply 1\n";
        matrix_assign_default(dest, lhs*rhs, 1, true);
    }

// ------------------------------------------------------------------------------------

    template <
        typename matrix_dest_type,
        typename EXP1,
        typename EXP2
        >
    typename enable_if_c<ma::matrix_is_vector<EXP1>::value == false && ma::matrix_is_vector<EXP2>::value == false>::type 
    default_matrix_multiply (
        matrix_dest_type& dest,
        const EXP1& lhs,
        const EXP2& rhs
    )
    {//std::cout << "default matrix multiply 2\n";
        const long bs = 90;

        // if the matrices are small enough then just use the simple multiply algorithm
        if (lhs.nc() <= 2 || rhs.nc() <= 2 || lhs.nr() <= 2 || rhs.nr() <= 2 || (lhs.size() <= bs*10 && rhs.size() <= bs*10) )
        {//std::cout << "default matrix multiply 2 b 1\n";
            matrix_assign_default(dest, lhs*rhs, 1, true);
        }
        else
        {//std::cout << "default matrix multiply 2 b 2 size: \n" << lhs.nr() << " " << lhs.nc() << " " << rhs.nr() << " " << rhs.nc() << std::endl;
            // if the lhs and rhs matrices are big enough we should use a cache friendly
            // algorithm that computes the matrix multiply in blocks.  
            //omp_set_num_threads(4);
            double lhs_mat[lhs.nr()+5][lhs.nc()+5];
            double rhs_mat[rhs.nr()+5][rhs.nc()+5];
            double dst_mat[dest.nr()+5][dest.nc()+5];
            for(int i = 0; i < lhs.nr(); i++)
            {
                for(int j = 0; j < lhs.nc(); j++)
                {
                    lhs_mat[i][j] = lhs(i, j);
                }
            }
            for(int i = 0; i < rhs.nr(); i++)
            {
                for(int j = 0; j < rhs.nc(); j++)
                {
                    rhs_mat[i][j] = rhs(i, j);
                }
            }
            std::memset(dst_mat, 0, sizeof(dst_mat));

            // Loop over all the blocks in the lhs matrix
            //#pragma omp parallel num_threads(2)
            for (long r = 0; r < lhs.nr(); r+=bs)
            {////std::cout << "omp get thread num: " << omp_get_thread_num() << " " << omp_get_num_threads() << std::endl;
                //#pragma omp parallel num_threads(4)
                for (long c = 0; c < lhs.nc(); c+=bs)
                {
                    // make a rect for the block from lhs 
                    rectangle lhs_block(c, r, std::min(c+bs-1,lhs.nc()-1), std::min(r+bs-1,lhs.nr()-1));

                    // now loop over all the rhs blocks we have to multiply with the current lhs block
                    for (long i = 0; i < rhs.nc(); i += bs)
                    {
                        // make a rect for the block from rhs 
                        rectangle rhs_block(i, c, std::min(i+bs-1,rhs.nc()-1), std::min(c+bs-1,rhs.nr()-1));

                        // make a target rect in res
                        rectangle res_block(rhs_block.left(),lhs_block.top(), rhs_block.right(), lhs_block.bottom());

                        // This loop is optimized assuming that the data is laid out in 
                        // row major order in memory.
                        for (long r = lhs_block.top(); r <= lhs_block.bottom(); ++r)
                        {
                            for (long c = lhs_block.left(); c<= lhs_block.right(); ++c)
                            {
                                const typename EXP2::type temp = lhs_mat[r][c];//lhs(r,c);
                                for (long i = rhs_block.left(); i <= rhs_block.right(); ++i)
                                {
                                    // += rhs(c,i)*temp;
                                    dst_mat[r][i] += rhs_mat[c][i]*temp;
                                }
                            }
                        }
                    }
                }
            }
            for(int i = 0; i < dest.nr(); i++)
            {
                for(int j = 0; j < dest.nc(); j++)
                {
                    dest(i,j) = dst_mat[i][j];
                }
            }
        }


    }

// ------------------------------------------------------------------------------------

//}

#endif // DLIB_MATRIx_DEFAULT_MULTIPLY_

