// Copyright (C) 2009  Davis E. King (davis@dlib.net)
// License: Boost Software License   See LICENSE.txt for the full license.
#ifndef DLIB_MATRIx_LA_FUNCTS_
#define DLIB_MATRIx_LA_FUNCTS_ 

//#include "matrix_la_abstract.h"
#include "matrix_utilities.h"
//#include "../sparse_vector.h"
//#include "../optimization/optimization_line_search.h"

// The 4 decomposition objects described in the matrix_la_abstract.h file are
// actually implemented in the following 4 files.  
#include "matrix_lu.h"

#include <iostream>



// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------

    enum svd_u_mode
    {
        SVD_NO_U,
        SVD_SKINNY_U,
        SVD_FULL_U
    };

    template <
        typename EXP,
        long qN, long qX,
        long uM, long uN,
        long vM, long vN,
        typename MM1,
        typename MM2,
        typename MM3,
        typename L1
        >
    long svd4 (
        svd_u_mode u_mode, 
        bool withv, 
        const matrix_exp<EXP>& a,
        matrix<typename EXP::type,uM,uN,MM1,L1>& u, 
        matrix<typename EXP::type,qN,qX,MM2,L1>& q, 
        matrix<typename EXP::type,vM,vN,MM3,L1>& v
    )
    {
        /*  
            Singular value decomposition. Translated to 'C' from the
            original Algol code in "Handbook for Automatic Computation,
            vol. II, Linear Algebra", Springer-Verlag.  Note that this
            published algorithm is considered to be the best and numerically
            stable approach to computing the real-valued svd and is referenced
            repeatedly in ieee journal papers, etc where the svd is used.

            This is almost an exact translation from the original, except that
            an iteration counter is added to prevent stalls. This corresponds
            to similar changes in other translations.

            Returns an error code = 0, if no errors and 'k' if a failure to
            converge at the 'kth' singular value.

            USAGE: given the singular value decomposition a = u * diagm(q) * trans(v) for an m*n 
                    matrix a with m >= n ...  
                    After the svd call u is an m x m matrix which is columnwise 
                    orthogonal. q will be an n element vector consisting of singular values 
                    and v an n x n orthogonal matrix. eps and tol are tolerance constants. 
                    Suitable values are eps=1e-16 and tol=(1e-300)/eps if T == double. 

                    If u_mode == SVD_NO_U then u won't be computed and similarly if withv == false
                    then v won't be computed.  If u_mode == SVD_SKINNY_U then u will be m x n instead of m x m.
        */


        DLIB_ASSERT(a.nr() >= a.nc(), 
            "\tconst matrix_exp svd4()"
            << "\n\tYou have given an invalidly sized matrix"
            << "\n\ta.nr(): " << a.nr()
            << "\n\ta.nc(): " << a.nc() 
            );


        typedef typename EXP::type T;

#ifdef DLIB_USE_LAPACK
        matrix<typename EXP::type,0,0,MM1,L1> temp(a), vtemp;

        char jobu = 'A';
        char jobvt = 'A';
        if (u_mode == SVD_NO_U)
            jobu = 'N';
        else if (u_mode == SVD_SKINNY_U)
            jobu = 'S';
        if (withv == false)
            jobvt = 'N';

        int info;
        if (jobu == jobvt)
        {
            info = lapack::gesdd(jobu, temp, q, u, vtemp);
        }
        else
        {
            info = lapack::gesvd(jobu, jobvt, temp, q, u, vtemp);
        }

        // pad q with zeros if it isn't the length we want
        if (q.nr() < a.nc())
            q = join_cols(q, zeros_matrix<T>(a.nc()-q.nr(),1));

        if (withv)
            v = trans(vtemp);

        return info;
#else
        using std::abs;
        using std::sqrt;

        T eps = std::numeric_limits<T>::epsilon();
        T tol = std::numeric_limits<T>::min()/eps;

        const long m = a.nr();
        const long n = a.nc();
        long i, j, k, l = 0, l1, iter, retval;
        T c, f, g, h, s, x, y, z;

        matrix<T,qN,1,MM2> e(n,1); 
        q.set_size(n,1);
        if (u_mode == SVD_FULL_U)
            u.set_size(m,m);
        else
            u.set_size(m,n);
        retval = 0;

        if (withv)
        {
            v.set_size(n,n);
        }

        /* Copy 'a' to 'u' */    
        for (i=0; i<m; i++) 
        {
            for (j=0; j<n; j++)
                u(i,j) = a(i,j);
        }

        /* Householder's reduction to bidiagonal form. */
        g = x = 0.0;    
        for (i=0; i<n; i++) 
        {
            e(i) = g;
            s = 0.0;
            l = i + 1;

            for (j=i; j<m; j++)
                s += (u(j,i) * u(j,i));

            if (s < tol)
                g = 0.0;
            else 
            {
                f = u(i,i);
                g = (f < 0) ? sqrt(s) : -sqrt(s);
                h = f * g - s;
                u(i,i) = f - g;

                for (j=l; j<n; j++) 
                {
                    s = 0.0;

                    for (k=i; k<m; k++)
                        s += (u(k,i) * u(k,j));

                    f = s / h;

                    for (k=i; k<m; k++)
                        u(k,j) += (f * u(k,i));
                } /* end j */
            } /* end s */

            q(i) = g;
            s = 0.0;

            for (j=l; j<n; j++)
                s += (u(i,j) * u(i,j));

            if (s < tol)
                g = 0.0;
            else 
            {
                f = u(i,i+1);
                g = (f < 0) ? sqrt(s) : -sqrt(s);
                h = f * g - s;
                u(i,i+1) = f - g;

                for (j=l; j<n; j++) 
                    e(j) = u(i,j) / h;

                for (j=l; j<m; j++) 
                {
                    s = 0.0;

                    for (k=l; k<n; k++) 
                        s += (u(j,k) * u(i,k));

                    for (k=l; k<n; k++)
                        u(j,k) += (s * e(k));
                } /* end j */
            } /* end s */

            y = abs(q(i)) + abs(e(i));                         
            if (y > x)
                x = y;
        } /* end i */

        /* accumulation of right-hand transformations */
        if (withv) 
        {
            for (i=n-1; i>=0; i--) 
            {
                if (g != 0.0) 
                {
                    h = u(i,i+1) * g;

                    for (j=l; j<n; j++)
                        v(j,i) = u(i,j)/h;

                    for (j=l; j<n; j++) 
                    {
                        s = 0.0;

                        for (k=l; k<n; k++) 
                            s += (u(i,k) * v(k,j));

                        for (k=l; k<n; k++)
                            v(k,j) += (s * v(k,i));
                    } /* end j */
                } /* end g */

                for (j=l; j<n; j++)
                    v(i,j) = v(j,i) = 0.0;

                v(i,i) = 1.0;
                g = e(i);
                l = i;
            } /* end i */
        } /* end withv, parens added for clarity */

        /* accumulation of left-hand transformations */
        if (u_mode != SVD_NO_U) 
        {
            for (i=n; i<u.nr(); i++) 
            {
                for (j=n;j<u.nc();j++)
                    u(i,j) = 0.0;

                if (i < u.nc())
                    u(i,i) = 1.0;
            }
        }

        if (u_mode != SVD_NO_U) 
        {
            for (i=n-1; i>=0; i--) 
            {
                l = i + 1;
                g = q(i);

                for (j=l; j<u.nc(); j++)  
                    u(i,j) = 0.0;

                if (g != 0.0) 
                {
                    h = u(i,i) * g;

                    for (j=l; j<u.nc(); j++) 
                    { 
                        s = 0.0;

                        for (k=l; k<m; k++)
                            s += (u(k,i) * u(k,j));

                        f = s / h;

                        for (k=i; k<m; k++) 
                            u(k,j) += (f * u(k,i));
                    } /* end j */

                    for (j=i; j<m; j++) 
                        u(j,i) /= g;
                } /* end g */
                else 
                {
                    for (j=i; j<m; j++)
                        u(j,i) = 0.0;
                }

                u(i,i) += 1.0;
            } /* end i*/
        } 

        /* diagonalization of the bidiagonal form */
        eps *= x;

        for (k=n-1; k>=0; k--) 
        {
            iter = 0;

test_f_splitting:

            for (l=k; l>=0; l--) 
            {
                if (abs(e(l)) <= eps) 
                    goto test_f_convergence;

                if (abs(q(l-1)) <= eps) 
                    goto cancellation;
            } /* end l */

            /* cancellation of e(l) if l > 0 */

cancellation:

            c = 0.0;
            s = 1.0;
            l1 = l - 1;

            for (i=l; i<=k; i++) 
            {
                f = s * e(i);
                e(i) *= c;

                if (abs(f) <= eps) 
                    goto test_f_convergence;

                g = q(i);
                h = q(i) = sqrt(f*f + g*g);
                c = g / h;
                s = -f / h;

                if (u_mode != SVD_NO_U) 
                {
                    for (j=0; j<m; j++) 
                    {
                        y = u(j,l1);
                        z = u(j,i);
                        u(j,l1) = y * c + z * s;
                        u(j,i) = -y * s + z * c;
                    } /* end j */
                } 
            } /* end i */

test_f_convergence:

            z = q(k);
            if (l == k) 
                goto convergence;

            /* shift from bottom 2x2 minor */
            iter++;
            if (iter > 300) 
            {
                retval = k;
                break;
            }
            x = q(l);
            y = q(k-1);
            g = e(k-1);
            h = e(k);
            f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2 * h * y);
            g = sqrt(f * f + 1.0);
            f = ((x - z) * (x + z) + h * (y / ((f < 0)?(f - g) : (f + g)) - h)) / x;

            /* next QR transformation */
            c = s = 1.0;

            for (i=l+1; i<=k; i++) 
            {
                g = e(i);
                y = q(i);
                h = s * g;
                g *= c;
                e(i-1) = z = sqrt(f * f + h * h);
                c = f / z;
                s = h / z;
                f = x * c + g * s;
                g = -x * s + g * c;
                h = y * s;
                y *= c;

                if (withv) 
                {
                    for (j=0;j<n;j++) 
                    {
                        x = v(j,i-1);
                        z = v(j,i);
                        v(j,i-1) = x * c + z * s;
                        v(j,i) = -x * s + z * c;
                    } /* end j */
                } /* end withv, parens added for clarity */

                q(i-1) = z = sqrt(f * f + h * h);
                if (z != 0)
                {
                    c = f / z;
                    s = h / z;
                }
                f = c * g + s * y;
                x = -s * g + c * y;
                if (u_mode != SVD_NO_U) 
                {
                    for (j=0; j<m; j++) 
                    {
                        y = u(j,i-1);
                        z = u(j,i);
                        u(j,i-1) = y * c + z * s;
                        u(j,i) = -y * s + z * c;
                    } /* end j */
                } 
            } /* end i */

            e(l) = 0.0;
            e(k) = f;
            q(k) = x;

            goto test_f_splitting;

convergence:

            if (z < 0.0) 
            {
                /* q(k) is made non-negative */
                q(k) = -z;
                if (withv) 
                {
                    for (j=0; j<n; j++)
                        v(j,k) = -v(j,k);
                } /* end withv, parens added for clarity */
            } /* end z */
        } /* end k */

        return retval;
#endif
    }

// ----------------------------------------------------------------------------------------

    template <typename M>
    struct op_diag_inv
    {
        template <typename EXP>
        op_diag_inv( const matrix_exp<EXP>& m_) : m(m_){}


        const static long cost = 1;
        const static long NR = ((M::NC!=0)&&(M::NR!=0))? (tmax<M::NR,M::NC>::value) : (0);
        const static long NC = NR;
        typedef typename M::type type;
        typedef const type const_ret_type;
        typedef typename M::mem_manager_type mem_manager_type;
        typedef typename M::layout_type layout_type;


        // hold the matrix by value
        const matrix<type,NR,1,mem_manager_type,layout_type> m;

        const_ret_type apply ( long r, long c) const 
        { 
            if (r==c)
                return m(r);
            else
                return 0;
        }

        long nr () const { return m.size(); }
        long nc () const { return m.size(); }

        template <typename U> bool aliases               ( const matrix_exp<U>& item) const { return m.aliases(item); }
        template <typename U> bool destructively_aliases ( const matrix_exp<U>& item) const { return m.aliases(item); }
    };

    template <
        typename EXP
        >
    const matrix_diag_op<op_diag_inv<EXP> > inv (
        const matrix_diag_exp<EXP>& m
    ) 
    { 
        typedef op_diag_inv<EXP> op;
        return matrix_diag_op<op>(op(reciprocal(diag(m))));
    }

    template <
        typename EXP
        >
    const matrix_diag_op<op_diag_inv<EXP> > pinv (
        const matrix_diag_exp<EXP>& m
    ) 
    { 
        typedef op_diag_inv<EXP> op;
        return matrix_diag_op<op>(op(reciprocal(diag(m))));
    }

// ----------------------------------------------------------------------------------------

    template <
        typename EXP
        >
    const matrix_diag_op<op_diag_inv<EXP> > pinv (
        const matrix_diag_exp<EXP>& m,
        double tol
    ) 
    { 
        DLIB_ASSERT(tol >= 0, 
            "\tconst matrix_exp::type pinv(const matrix_exp& m)"
            << "\n\t tol can't be negative"
            << "\n\t tol: "<<tol 
            );
        typedef op_diag_inv<EXP> op;
        return matrix_diag_op<op>(op(reciprocal(round_zeros(diag(m),tol))));
    }

// ----------------------------------------------------------------------------------------

    template <
        typename EXP,
        long uNR, 
        long uNC,
        long wN, 
        long vN,
        long wX,
        typename MM1,
        typename MM2,
        typename MM3,
        typename L1
        >
    inline void svd3 (
        const matrix_exp<EXP>& m,
        matrix<typename matrix_exp<EXP>::type, uNR, uNC,MM1,L1>& u,
        matrix<typename matrix_exp<EXP>::type, wN, wX,MM2,L1>& w,
        matrix<typename matrix_exp<EXP>::type, vN, vN,MM3,L1>& v
    )
    {
        typedef typename matrix_exp<EXP>::type T;
        const long NR = matrix_exp<EXP>::NR;
        const long NC = matrix_exp<EXP>::NC;

        // make sure the output matrices have valid dimensions if they are statically dimensioned
        COMPILE_TIME_ASSERT(NR == 0 || uNR == 0 || NR == uNR);
        COMPILE_TIME_ASSERT(NC == 0 || uNC == 0 || NC == uNC);
        COMPILE_TIME_ASSERT(NC == 0 || wN == 0 || NC == wN);
        COMPILE_TIME_ASSERT(NC == 0 || vN == 0 || NC == vN);
        COMPILE_TIME_ASSERT(wX == 0 || wX == 1);

#ifdef DLIB_USE_LAPACK
        // use LAPACK but only if it isn't a really small matrix we are taking the SVD of.
        if (NR*NC == 0 || NR*NC > 3*3)
        {
            matrix<typename matrix_exp<EXP>::type, uNR, uNC,MM1,L1> temp(m);
            lapack::gesvd('S','A', temp, w, u, v);
            v = trans(v);
            // if u isn't the size we want then pad it (and v) with zeros
            if (u.nc() < m.nc())
            {
                w = join_cols(w, zeros_matrix<T>(m.nc()-u.nc(),1));
                u = join_rows(u, zeros_matrix<T>(u.nr(), m.nc()-u.nc()));
            }
            return;
        }
#endif
        if (m.nr() >= m.nc())
        {
            svd4(SVD_SKINNY_U,true, m, u,w,v);
        }
        else
        {
            svd4(SVD_FULL_U,true, trans(m), v,w,u);

            // if u isn't the size we want then pad it (and v) with zeros
            if (u.nc() < m.nc())
            {
                w = join_cols(w, zeros_matrix<T>(m.nc()-u.nc(),1));
                u = join_rows(u, zeros_matrix<T>(u.nr(), m.nc()-u.nc()));
            }
        }
    }

// ----------------------------------------------------------------------------------------

    template <
        typename EXP
        >
    const matrix<typename EXP::type,EXP::NC,EXP::NR,typename EXP::mem_manager_type> pinv_helper ( 
        const matrix_exp<EXP>& m,
        double tol
    )
    /*!
        ensures
            - computes the results of pinv(m) but does so using a method that is fastest
              when m.nc() <= m.nr().  So if m.nc() > m.nr() then it is best to use
              trans(pinv_helper(trans(m))) to compute pinv(m).
    !*/
    { 
        typename matrix_exp<EXP>::matrix_type u;
        typedef typename EXP::mem_manager_type MM1;
        typedef typename EXP::layout_type layout_type;
        matrix<typename EXP::type, EXP::NC, EXP::NC,MM1, layout_type > v;

        typedef typename matrix_exp<EXP>::type T;

        matrix<T,matrix_exp<EXP>::NC,1,MM1, layout_type> w;

        svd3(m, u,w,v);

        const double machine_eps = std::numeric_limits<typename EXP::type>::epsilon();
        // compute a reasonable epsilon below which we round to zero before doing the
        // reciprocal.  Unless a non-zero tol is given then we just use tol*max(w).
        const double eps = (tol!=0) ? tol*max(w) :  machine_eps*std::max(m.nr(),m.nc())*max(w);

        // now compute the pseudoinverse
        return tmp(scale_columns(v,reciprocal(round_zeros(w,eps))))*trans(u);
    }

    template <
        typename EXP
        >
    const matrix<typename EXP::type,EXP::NC,EXP::NR,typename EXP::mem_manager_type> pinv ( 
        const matrix_exp<EXP>& m,
        double tol = 0
    )
    { 
        DLIB_ASSERT(tol >= 0, 
            "\tconst matrix_exp::type pinv(const matrix_exp& m)"
            << "\n\t tol can't be negative"
            << "\n\t tol: "<<tol 
            );
        // if m has more columns then rows then it is more efficient to
        // compute the pseudo-inverse of its transpose (given the way I'm doing it below).
        if (m.nc() > m.nr())
            return trans(pinv_helper(trans(m),tol));
        else
            return pinv_helper(m,tol);
    }

// ----------------------------------------------------------------------------------------

    template <
        typename EXP,
        long uNR, 
        long uNC,
        long wN, 
        long vN,
        typename MM1,
        typename MM2,
        typename MM3,
        typename L1
        >
    inline void svd (
        const matrix_exp<EXP>& m,
        matrix<typename matrix_exp<EXP>::type, uNR, uNC,MM1,L1>& u,
        matrix<typename matrix_exp<EXP>::type, wN, wN,MM2,L1>& w,
        matrix<typename matrix_exp<EXP>::type, vN, vN,MM3,L1>& v
    )
    {
        typedef typename matrix_exp<EXP>::type T;
        const long NR = matrix_exp<EXP>::NR;
        const long NC = matrix_exp<EXP>::NC;

        // make sure the output matrices have valid dimensions if they are statically dimensioned
        COMPILE_TIME_ASSERT(NR == 0 || uNR == 0 || NR == uNR);
        COMPILE_TIME_ASSERT(NC == 0 || uNC == 0 || NC == uNC);
        COMPILE_TIME_ASSERT(NC == 0 || wN == 0 || NC == wN);
        COMPILE_TIME_ASSERT(NC == 0 || vN == 0 || NC == vN);

        matrix<T,matrix_exp<EXP>::NC,1,MM1, L1> W;
        svd3(m,u,W,v);
        w = diagm(W);
    }

// ----------------------------------------------------------------------------------------

    template <
        typename EXP
        >
    const typename matrix_exp<EXP>::type trace (
        const matrix_exp<EXP>& m
    ) 
    { 
        COMPILE_TIME_ASSERT(matrix_exp<EXP>::NR == matrix_exp<EXP>::NC ||
                            matrix_exp<EXP>::NR == 0 ||
                            matrix_exp<EXP>::NC == 0 
                            );
        DLIB_ASSERT(m.nr() == m.nc(), 
            "\tconst matrix_exp::type trace(const matrix_exp& m)"
            << "\n\tYou can only apply trace() to a square matrix"
            << "\n\tm.nr(): " << m.nr()
            << "\n\tm.nc(): " << m.nc() 
            );
        return sum(diag(m));
    }

// ----------------------------------------------------------------------------------------

    template <
        typename EXP,
        long N = EXP::NR
        >
    struct det_helper
    {
        static const typename matrix_exp<EXP>::type det (
            const matrix_exp<EXP>& m
        )
        {
            COMPILE_TIME_ASSERT(matrix_exp<EXP>::NR == matrix_exp<EXP>::NC ||
                                matrix_exp<EXP>::NR == 0 ||
                                matrix_exp<EXP>::NC == 0 
                                );
            DLIB_ASSERT(m.nr() == m.nc(), 
                "\tconst matrix_exp::type det(const matrix_exp& m)"
                << "\n\tYou can only apply det() to a square matrix"
                << "\n\tm.nr(): " << m.nr()
                << "\n\tm.nc(): " << m.nc() 
                );

            return lu_decomposition<EXP>(m).det();
        }
    };

    template <
        typename EXP
        >
    struct det_helper<EXP,1>
    {
        static const typename matrix_exp<EXP>::type det (
            const matrix_exp<EXP>& m
        )
        {
            COMPILE_TIME_ASSERT(matrix_exp<EXP>::NR == matrix_exp<EXP>::NC);

            return m(0);
        }
    };

    template <
        typename EXP
        >
    struct det_helper<EXP,2>
    {
        static const typename matrix_exp<EXP>::type det (
            const matrix_exp<EXP>& m
        )
        {
            COMPILE_TIME_ASSERT(matrix_exp<EXP>::NR == matrix_exp<EXP>::NC);

            return m(0,0)*m(1,1) - m(0,1)*m(1,0);
        }
    };

    template <
        typename EXP
        >
    struct det_helper<EXP,3>
    {
        static const typename matrix_exp<EXP>::type det (
            const matrix_exp<EXP>& m
        )
        {
            COMPILE_TIME_ASSERT(matrix_exp<EXP>::NR == matrix_exp<EXP>::NC);
            typedef typename matrix_exp<EXP>::type type;

            type temp = m(0,0)*(m(1,1)*m(2,2) - m(1,2)*m(2,1)) -
                        m(0,1)*(m(1,0)*m(2,2) - m(1,2)*m(2,0)) +
                        m(0,2)*(m(1,0)*m(2,1) - m(1,1)*m(2,0));
            return temp;
        }
    };


    template <
        typename EXP
        >
    inline const typename matrix_exp<EXP>::type det (
        const matrix_exp<EXP>& m
    ) { return det_helper<EXP>::det(m); }


    template <
        typename EXP
        >
    struct det_helper<EXP,4>
    {
        static const typename matrix_exp<EXP>::type det (
            const matrix_exp<EXP>& m
        )
        {
            COMPILE_TIME_ASSERT(matrix_exp<EXP>::NR == matrix_exp<EXP>::NC);
            typedef typename matrix_exp<EXP>::type type;

            type temp = m(0,0)*(det(removerc<0,0>(m))) -
                        m(0,1)*(det(removerc<0,1>(m))) +
                        m(0,2)*(det(removerc<0,2>(m))) -
                        m(0,3)*(det(removerc<0,3>(m)));
            return temp;
        }
    };



#endif // DLIB_MATRIx_LA_FUNCTS_


