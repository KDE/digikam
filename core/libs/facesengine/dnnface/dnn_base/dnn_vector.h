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

#ifndef DLIB_VECTOr_H_
#define DLIB_VECTOr_H_

#include <cmath>
//#include "vector_abstract.h"
#include "algs.h"
#include "serialize.h"
#include <functional>
#include <iostream>
#include "matrix.h"
#include <limits>

#if defined(_MSC_VER) && _MSC_VER < 1400
// Despite my efforts to disabuse visual studio of its usual nonsense I can't find a 
// way to make this warning go away without just disabling it.   This is the warning:
//   dlib\geometry\dvector.h(129) : warning C4805: '==' : unsafe mix of type 'std::numeric_limits<_Ty>::is_integer' and type 'bool' in operation
// 
#pragma warning(disable:4805)
#endif



template <
    typename T,
    long NR = 3
    >
class dvector;

// ----------------------------------------------------------------------------------------

template <typename T, typename U, typename enabled = void> 
struct vect_promote;

template <typename T, typename U, bool res = (sizeof(T) <= sizeof(U))>
struct largest_type
{
    typedef T type;
};
template <typename T, typename U>
struct largest_type<T,U,true>
{
    typedef U type;
};

template <typename T, typename U> 
struct vect_promote<T,U, typename enable_if_c<std::numeric_limits<T>::is_integer == std::numeric_limits<U>::is_integer>::type> 
{ 
    // If both T and U are both either integral or non-integral then just
    // use the biggest one
    typedef typename largest_type<T,U>::type type;
};

template <typename T, typename U> 
struct vect_promote<T,U, typename enable_if_c<std::numeric_limits<T>::is_integer != std::numeric_limits<U>::is_integer>::type> 
{ 
    typedef double type;
};

// ----------------------------------------------------------------------------------------

// This insanity here is to work around a bug in visual studio 8.   These two rebind
// structures are actually declared at a few points in this file because just having the
// one declaration here isn't enough for visual studio.  It takes the three spread around
// to avoid all its bugs. 
template <typename T, long N>
struct vc_rebind
{
    typedef dvector<T,N> type;
};
template <typename T, typename U, long N>
struct vc_rebind_promote
{
    typedef dvector<typename vect_promote<T,U>::type,N> type;
};

// ----------------------------------------------------------------------------------------

template <typename T, typename U, typename enabled = void>
struct vector_assign_helper
{
    template <long NR>
    static void assign (
        dvector<T,2>& dest,
        const dvector<U,NR>& src
    )
    {
        dest.x() = static_cast<T>(src.x());
        dest.y() = static_cast<T>(src.y());
    }

    template <long NR>
    static void assign (
        dvector<T,3>& dest,
        const dvector<U,NR>& src
    )
    {
        dest.x() = static_cast<T>(src.x());
        dest.y() = static_cast<T>(src.y());
        dest.z() = static_cast<T>(src.z());
    }

    template <typename EXP>
    static void assign (
        dvector<T,2>& dest,
        const matrix_exp<EXP>& m
    )
    {
        T x = static_cast<T>(m(0));
        T y = static_cast<T>(m(1));
        dest.x() = x;
        dest.y() = y;
    }

    template <typename EXP>
    static void assign (
        dvector<T,3>& dest,
        const matrix_exp<EXP>& m
    )
    {
        T x = static_cast<T>(m(0));
        T y = static_cast<T>(m(1));
        T z = static_cast<T>(m(2));

        dest.x() = x;
        dest.y() = y;
        dest.z() = z;
    }
};

// This is an overload for the case where you are converting from a floating point
// type to an integral type.  These overloads make sure values are rounded to 
// the nearest integral value.
template <typename T, typename U>
struct vector_assign_helper<T,U, typename enable_if_c<std::numeric_limits<T>::is_integer == true && 
                                                      std::numeric_limits<U>::is_integer == false>::type>
{
    template <long NR>
    static void assign (
        dvector<T,2>& dest,
        const dvector<U,NR>& src
    )
    {
        dest.x() = static_cast<T>(std::floor(src.x() + 0.5));
        dest.y() = static_cast<T>(std::floor(src.y() + 0.5));
    }

    template <long NR>
    static void assign (
        dvector<T,3>& dest,
        const dvector<U,NR>& src
    )
    {
        dest.x() = static_cast<T>(std::floor(src.x() + 0.5));
        dest.y() = static_cast<T>(std::floor(src.y() + 0.5));
        dest.z() = static_cast<T>(std::floor(src.z() + 0.5));
    }

    template <typename EXP>
    static void assign (
        dvector<T,3>& dest,
        const matrix_exp<EXP>& m
    )
    {
        dest.x() = static_cast<T>(std::floor(m(0) + 0.5));
        dest.y() = static_cast<T>(std::floor(m(1) + 0.5));
        dest.z() = static_cast<T>(std::floor(m(2) + 0.5));
    }

    template <typename EXP>
    static void assign (
        dvector<T,2>& dest,
        const matrix_exp<EXP>& m
    )
    {
        dest.x() = static_cast<T>(std::floor(m(0) + 0.5));
        dest.y() = static_cast<T>(std::floor(m(1) + 0.5));
    }

};

// ----------------------------------------------------------------------------------------

template <typename T>
class dvector<T,3> : public matrix<T,3,1>
{
    /*!
        INITIAL VALUE
            - x() == 0
            - y() == 0
            - z() == 0

        CONVENTION
            - (*this)(0) == x() 
            - (*this)(1) == y() 
            - (*this)(2) == z() 

    !*/

    // This insanity here is to work around a bug in visual studio 8.  
    template <typename V, long N>
    struct vc_rebind
    {
        typedef dvector<V,N> type;
    };
        template <typename V, typename U, long N>
    struct vc_rebind_promote
    {
        typedef dvector<typename vect_promote<V,U>::type,N> type;
    };

public:

    typedef T type;
    
    dvector (
    ) 
    {
        x() = 0;
        y() = 0;
        z() = 0;
    }

    // ---------------------------------------

    dvector (
        const T _x,
        const T _y,
        const T _z
    ) 
    {
        x() = _x;
        y() = _y;
        z() = _z;
    }

    // ---------------------------------------

    dvector (
        const dvector& item
    ) : matrix<T,3,1>(item)
    {
    }

    // ---------------------------------------

    template <typename U>
    dvector (
        const dvector<U,2>& item
    )
    {
        // Do this so that we get the appropriate rounding depending on the relative
        // type of T and U.
        dvector<T,2> temp(item);
        x() = temp.x();
        y() = temp.y();
        z() = 0;
    }

    // ---------------------------------------

    dvector (
        const dvector<T,2>& item
    )
    {
        x() = item.x();
        y() = item.y();
        z() = 0;
    }

    // ---------------------------------------

    template <typename U>
    dvector (
        const dvector<U,3>& item
    )
    {
        (*this) = item;
    }

    // ---------------------------------------

    template <typename EXP>
    dvector ( const matrix_exp<EXP>& m)
    {
        (*this) = m;
    }

    // ---------------------------------------

    template <typename EXP>
    dvector& operator = (
        const matrix_exp<EXP>& m
    )
    {
        // you can only assign vectors with 3 elements to a  dvector<T,3> object
        COMPILE_TIME_ASSERT(EXP::NR*EXP::NC == 3 || EXP::NR*EXP::NC == 0);

        // make sure requires clause is not broken
        DLIB_ASSERT((m.nr() == 1 || m.nc() == 1) && (m.size() == 3),
            "\t dvector(const matrix_exp& m)"
            << "\n\t the given matrix is of the wrong size"
            << "\n\t m.nr():   " << m.nr() 
            << "\n\t m.nc():   " << m.nc() 
            << "\n\t m.size(): " << m.size() 
            << "\n\t this: " << this
            );

        vector_assign_helper<T, typename EXP::type>::assign(*this, m);
        return *this;
    }

    // ---------------------------------------

    template <typename U, long N>
    dvector& operator = (
        const dvector<U,N>& item
    )
    {
        vector_assign_helper<T,U>::assign(*this, item);
        return *this;
    }

    // ---------------------------------------

    dvector& operator= (
        const dvector& item
    )
    {
        x() = item.x();
        y() = item.y();
        z() = item.z();
        return *this;
    }

    // ---------------------------------------

    double length(
    ) const 
    { 
        return std::sqrt((double)(x()*x() + y()*y() + z()*z())); 
    }

    // ---------------------------------------

    double length_squared(
    ) const 
    { 
        return (double)(x()*x() + y()*y() + z()*z()); 
    }

    // ---------------------------------------

    typename vc_rebind<double,3>::type normalize (
    ) const 
    {
        const double tmp = std::sqrt((double)(x()*x() + y()*y() + z()*z()));
        return dvector<double,3> ( x()/tmp,
                                  y()/tmp,
                                  z()/tmp
        );
    }

    // ---------------------------------------

    T& x (
    ) 
    { 
        return (*this)(0);
    }

    // ---------------------------------------

    T& y (
    ) 
    { 
        return (*this)(1);
    }

    // ---------------------------------------

    T& z (
    ) 
    { 
        return (*this)(2);
    }

    // ---------------------------------------

    const T& x (
    ) const
    { 
        return (*this)(0);
    }

    // ---------------------------------------

    const T& y (
    ) const 
    { 
        return (*this)(1);
    }

    // ---------------------------------------

    const T& z (
    ) const
    { 
        return (*this)(2);
    }

    // ---------------------------------------

    T dot (
        const dvector& rhs
    ) const 
    { 
        return x()*rhs.x() + y()*rhs.y() + z()*rhs.z();
    }

    // ---------------------------------------

    template <typename U, long N>
    typename vect_promote<T,U>::type dot (
        const dvector<U,N>& rhs
    ) const 
    { 
        return x()*rhs.x() + y()*rhs.y() + z()*rhs.z();
    }

    // ---------------------------------------

    template <typename U, long N>
    typename vc_rebind_promote<T,U,3>::type cross (
        const dvector<U,N>& rhs
    ) const
    {
        typedef dvector<typename vect_promote<T,U>::type,3> ret_type;

        return ret_type (
            y()*rhs.z() - z()*rhs.y(),
            z()*rhs.x() - x()*rhs.z(),
            x()*rhs.y() - y()*rhs.x()
            );
    }

    // ---------------------------------------

    dvector& operator += (
        const dvector& rhs
    )
    {
        x() += rhs.x();
        y() += rhs.y();
        z() += rhs.z();
        return *this;
    }

    // ---------------------------------------

    dvector& operator -= (
        const dvector& rhs
    )
    {
        x() -= rhs.x();
        y() -= rhs.y();
        z() -= rhs.z();
        return *this;
    }

    // ---------------------------------------

    dvector& operator /= (
        const T& rhs
    )
    {
        x() /= rhs;
        y() /= rhs;
        z() /= rhs;
        return *this;
    }

    // ---------------------------------------

    dvector& operator *= (
        const T& rhs
    )
    {
        x() *= rhs;
        y() *= rhs;
        z() *= rhs;
        return *this;
    }

    // ---------------------------------------

    dvector operator - (
    ) const
    {
        return dvector(-x(), -y(), -z());
    }

    // ---------------------------------------

    template <typename U>
    typename vc_rebind_promote<T,U,3>::type operator / (
        const U& val
    ) const
    {
        typedef dvector<typename vect_promote<T,U>::type,3> ret_type;
        return ret_type(x()/val, y()/val, z()/val);
    }

    // ---------------------------------------

    template <typename U, long NR2>
    bool operator== (
        const dvector<U,NR2>& rhs
    ) const
    {
        return x()==rhs.x() && y()==rhs.y() && z()==rhs.z();
    }

    // ---------------------------------------

    template <typename U, long NR2>
    bool operator!= (
        const dvector<U,NR2>& rhs
    ) const
    {
        return !(*this == rhs);
    }

    // ---------------------------------------

    void swap (
        dvector& item
    )
    {
         exchange(x(), item.x());
         exchange(y(), item.y());
         exchange(z(), item.z());
    }

    // ---------------------------------------

};

// ----------------------------------------------------------------------------------------

template <typename T>
class dvector<T,2> : public matrix<T,2,1>
{
    /*!
        INITIAL VALUE
            - x() == 0
            - y() == 0

        CONVENTION
            - (*this)(0) == x() 
            - (*this)(1) == y() 
            - z() == 0
    !*/

    // This insanity here is to work around a bug in visual studio 8.  
    template <typename V, long N>
    struct vc_rebind
    {
        typedef dvector<V,N> type;
    };
        template <typename V, typename U, long N>
    struct vc_rebind_promote
    {
        typedef dvector<typename vect_promote<V,U>::type,N> type;
    };


public:

    typedef T type;
    
    dvector (
    ) 
    {
        x() = 0;
        y() = 0;
    }

    // ---------------------------------------

    dvector (
        const T _x,
        const T _y
    ) 
    {
        x() = _x;
        y() = _y;
    }

    // ---------------------------------------

    template <typename U>
    dvector (
        const dvector<U,3>& item
    )
    {
        // Do this so that we get the appropriate rounding depending on the relative
        // type of T and U.
        dvector<T,3> temp(item);
        x() = temp.x();
        y() = temp.y();
    }

    // ---------------------------------------

    dvector (
        const dvector& item
    ) : matrix<T,2,1>(item)
    {
    }

    // ---------------------------------------

    dvector (
        const dvector<T,3>& item
    )
    {
        x() = item.x();
        y() = item.y();
    }

    // ---------------------------------------

    template <typename U>
    dvector (
        const dvector<U,2>& item
    )
    {
        (*this) = item;
    }

    // ---------------------------------------

    template <typename EXP>
    dvector ( const matrix_exp<EXP>& m)
    {
        (*this) = m;
    }

    // ---------------------------------------

    template <typename EXP>
    dvector& operator = (
        const matrix_exp<EXP>& m
    )
    {
        // you can only assign vectors with 2 elements to a  dvector<T,2> object
        COMPILE_TIME_ASSERT(EXP::NR*EXP::NC == 2 || EXP::NR*EXP::NC == 0);

        // make sure requires clause is not broken
        DLIB_ASSERT((m.nr() == 1 || m.nc() == 1) && (m.size() == 2),
            "\t dvector(const matrix_exp& m)"
            << "\n\t the given matrix is of the wrong size"
            << "\n\t m.nr():   " << m.nr() 
            << "\n\t m.nc():   " << m.nc() 
            << "\n\t m.size(): " << m.size() 
            << "\n\t this: " << this
            );

        vector_assign_helper<T, typename EXP::type>::assign(*this, m);
        return *this;
    }

    // ---------------------------------------

    template <typename U, long N>
    dvector& operator = (
        const dvector<U,N>& item
    )
    {
        vector_assign_helper<T,U>::assign(*this, item);
        return *this;
    }

    // ---------------------------------------

    dvector& operator= (
        const dvector& item
    )
    {
        x() = item.x();
        y() = item.y();
        return *this;
    }

    // ---------------------------------------

    double length(
    ) const 
    { 
        return std::sqrt((double)(x()*x() + y()*y())); 
    }

    // ---------------------------------------

    double length_squared(
    ) const 
    { 
        return (double)(x()*x() + y()*y()); 
    }

    // ---------------------------------------

    typename vc_rebind<double,2>::type normalize (
    ) const 
    {
        const double tmp = std::sqrt((double)(x()*x() + y()*y()));
        return dvector<double,2> ( x()/tmp,
                     y()/tmp
        );
    }

    // ---------------------------------------

    T& x (
    ) 
    { 
        return (*this)(0);
    }

    // ---------------------------------------

    T& y (
    ) 
    { 
        return (*this)(1);
    }

    // ---------------------------------------

    const T& x (
    ) const
    { 
        return (*this)(0);
    }

    // ---------------------------------------

    const T& y (
    ) const 
    { 
        return (*this)(1);
    }

    // ---------------------------------------

    const T z (
    ) const
    {
        return 0;
    }

    // ---------------------------------------

    T dot (
        const dvector& rhs
    ) const 
    { 
        return x()*rhs.x() + y()*rhs.y();
    }

    // ---------------------------------------

    template <typename U, long N>
    typename vect_promote<T,U>::type dot (
        const dvector<U,N>& rhs
    ) const 
    { 
        return x()*rhs.x() + y()*rhs.y() + z()*rhs.z();
    }

    // ---------------------------------------

    dvector& operator += (
        const dvector& rhs
    )
    {
        x() += rhs.x();
        y() += rhs.y();
        return *this;
    }

    // ---------------------------------------

    dvector& operator -= (
        const dvector& rhs
    )
    {
        x() -= rhs.x();
        y() -= rhs.y();
        return *this;
    }

    // ---------------------------------------

    dvector& operator /= (
        const T& rhs
    )
    {
        x() /= rhs;
        y() /= rhs;
        return *this;
    }

    // ---------------------------------------

    dvector& operator *= (
        const T& rhs
    )
    {
        x() *= rhs;
        y() *= rhs;
        return *this;
    }

    // ---------------------------------------

    dvector operator - (
    ) const
    {
        return dvector(-x(), -y());
    }

    // ---------------------------------------

    template <typename U>
    typename vc_rebind_promote<T,U,2>::type operator / (
        const U& val
    ) const
    {
        typedef dvector<typename vect_promote<T,U>::type,2> ret_type;
        return ret_type(x()/val, y()/val);
    }

    // ---------------------------------------

    template <typename U, long NR2>
    bool operator== (
        const dvector<U,NR2>& rhs
    ) const
    {
        return x()==rhs.x() && y()==rhs.y() && z()==rhs.z();
    }

    // ---------------------------------------

    bool operator== (
        const dvector& rhs
    ) const
    {
        return x()==rhs.x() && y()==rhs.y();
    }

    // ---------------------------------------

    template <typename U, long NR2>
    bool operator!= (
        const dvector<U,NR2>& rhs
    ) const
    {
        return !(*this == rhs);
    }

    // ---------------------------------------

    bool operator!= (
        const dvector& rhs
    ) const
    {
        return !(*this == rhs);
    }

    // ---------------------------------------

    void swap (
        dvector& item
    )
    {
         exchange(x(), item.x());
         exchange(y(), item.y());
    }

    // ---------------------------------------

    template <typename U, long N>
    typename vc_rebind_promote<T,U,3>::type cross (
        const dvector<U,N>& rhs
    ) const
    {
        typedef dvector<typename vect_promote<T,U>::type,3> ret_type;
        return ret_type (
            y()*rhs.z(),
            - x()*rhs.z(),
            x()*rhs.y() - y()*rhs.x()
            );
    }

    // ---------------------------------------

};

// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------

template <typename T, typename U>
inline const typename vc_rebind_promote<T,U,2>::type operator+ (
    const dvector<T,2>& lhs,
    const dvector<U,2>& rhs 
)
{
    typedef typename vc_rebind_promote<T,U,2>::type ret_type;
    return ret_type(lhs.x()+rhs.x(), lhs.y()+rhs.y());
}

// ----------------------------------------------------------------------------------------

template <typename T, typename U>
inline const typename vc_rebind_promote<T,U,3>::type operator+ (
    const dvector<T,3>& lhs,
    const dvector<U,3>& rhs 
)
{
    typedef typename vc_rebind_promote<T,U,3>::type ret_type;
    return ret_type(lhs.x()+rhs.x(), lhs.y()+rhs.y(), lhs.z()+rhs.z());
}

// ----------------------------------------------------------------------------------------

template <typename T, typename U>
inline const typename vc_rebind_promote<T,U,3>::type operator+ (
    const dvector<T,2>& lhs,
    const dvector<U,3>& rhs 
)
{
    typedef typename vc_rebind_promote<T,U,3>::type ret_type;
    return ret_type(lhs.x()+rhs.x(), lhs.y()+rhs.y(), rhs.z());
}

// ----------------------------------------------------------------------------------------

template <typename T, typename U>
inline const typename vc_rebind_promote<T,U,3>::type operator+ (
    const dvector<T,3>& lhs,
    const dvector<U,2>& rhs 
)
{
    typedef typename vc_rebind_promote<T,U,3>::type ret_type;
    return ret_type(lhs.x()+rhs.x(), lhs.y()+rhs.y(), lhs.z());
}

// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------

template <typename T, typename U>
inline const typename vc_rebind_promote<T,U,2>::type operator- (
    const dvector<T,2>& lhs,
    const dvector<U,2>& rhs 
)
{
    typedef typename vc_rebind_promote<T,U,2>::type ret_type;
    return ret_type(lhs.x()-rhs.x(), lhs.y()-rhs.y());
}

// ----------------------------------------------------------------------------------------

template <typename T, typename U>
inline const typename vc_rebind_promote<T,U,3>::type operator- (
    const dvector<T,3>& lhs,
    const dvector<U,3>& rhs 
)
{
    typedef typename vc_rebind_promote<T,U,3>::type ret_type;
    return ret_type(lhs.x()-rhs.x(), lhs.y()-rhs.y(), lhs.z()-rhs.z());
}

// ----------------------------------------------------------------------------------------

template <typename T, typename U>
inline const typename vc_rebind_promote<T,U,3>::type operator- (
    const dvector<T,2>& lhs,
    const dvector<U,3>& rhs 
)
{
    typedef typename vc_rebind_promote<T,U,3>::type ret_type;
    return ret_type(lhs.x()-rhs.x(), lhs.y()-rhs.y(), -rhs.z());
}

// ----------------------------------------------------------------------------------------

template <typename T, typename U>
inline const typename vc_rebind_promote<T,U,3>::type operator- (
    const dvector<T,3>& lhs,
    const dvector<U,2>& rhs 
)
{
    typedef typename vc_rebind_promote<T,U,3>::type ret_type;
    return ret_type(lhs.x()-rhs.x(), lhs.y()-rhs.y(), lhs.z());
}

// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------

template <typename T, typename U>
inline typename disable_if<is_matrix<U>, const typename vc_rebind_promote<T,U,2>::type >::type operator* (
    const dvector<T,2>& v,
    const U& s
)
{
    typedef typename vc_rebind_promote<T,U,2>::type ret_type;
    return ret_type(v.x()*s, v.y()*s);
}

// ----------------------------------------------------------------------------------------

template <typename T, typename U>
inline typename disable_if<is_matrix<U>, const typename vc_rebind_promote<T,U,2>::type >::type operator* (
    const U& s,
    const dvector<T,2>& v
)
{
    typedef typename vc_rebind_promote<T,U,2>::type ret_type;
    return ret_type(v.x()*s, v.y()*s);
}

// ----------------------------------------------------------------------------------------

template <typename T, typename U>
inline typename disable_if<is_matrix<U>, const typename vc_rebind_promote<T,U,3>::type >::type operator* (
    const dvector<T,3>& v,
    const U& s
)
{
    typedef typename vc_rebind_promote<T,U,3>::type ret_type;
    return ret_type(v.x()*s, v.y()*s, v.z()*s);
}

// ----------------------------------------------------------------------------------------

template <typename T, typename U>
inline typename disable_if<is_matrix<U>, const typename vc_rebind_promote<T,U,3>::type >::type operator* (
    const U& s,
    const dvector<T,3>& v
)
{
    typedef typename vc_rebind_promote<T,U,3>::type ret_type;
    return ret_type(v.x()*s, v.y()*s, v.z()*s);
}

// ----------------------------------------------------------------------------------------

template<typename T, long NR>
inline void swap (
    dvector<T,NR> & a, 
    dvector<T,NR> & b 
) { a.swap(b); }   

// ----------------------------------------------------------------------------------------

template<typename T>
inline void serialize (
    const dvector<T,3>& item,  
    std::ostream& out
)
{
    try
    {
        serialize(item.x(),out);
        serialize(item.y(),out);
        serialize(item.z(),out);
    }
    catch (serialization_error& e)
    { 
        throw serialization_error(e.info + "\n   while serializing object of type dvector"); 
    }
}

template<typename T>
inline void deserialize (
    dvector<T,3>& item,  
    std::istream& in
)
{
    try
    {
        deserialize(item.x(),in);
        deserialize(item.y(),in);
        deserialize(item.z(),in);
    }
    catch (serialization_error& e)
    { 
        item.x() = 0;
        item.y() = 0;
        item.z() = 0;
        throw serialization_error(e.info + "\n   while deserializing object of type dvector"); 
    }
}

// ----------------------------------------------------------------------------------------

template<typename T>
inline void serialize (
    const dvector<T,2>& item,  
    std::ostream& out
)
{
    try
    {
        serialize(item.x(),out);
        serialize(item.y(),out);
    }
    catch (serialization_error& e)
    { 
        throw serialization_error(e.info + "\n   while serializing object of type dvector"); 
    }
}

// ----------------------------------------------------------------------------------------

template<typename T>
std::ostream& operator<< (
    std::ostream& out, 
    const dvector<T,3>& item 
)
{
    out << "(" << item.x() << ", " << item.y() << ", " << item.z() << ")";
    return out;
}

template<typename T>
std::istream& operator>>(
    std::istream& in, 
    dvector<T,3>& item 
)   
{

    // eat all the crap up to the '(' 
    while (in.peek() == ' ' || in.peek() == '\t' || in.peek() == '\r' || in.peek() == '\n')
        in.get();

    // there should be a '(' if not then this is an error
    if (in.get() != '(')
    {
        in.setstate(in.rdstate() | std::ios::failbit);
        return in;
    }

    // eat all the crap up to the first number 
    while (in.peek() == ' ' || in.peek() == '\t')
        in.get();
    in >> item.x();

    if (!in.good())
        return in;
          
    // eat all the crap up to the next number
    while (in.peek() == ' ' || in.peek() == '\t' || in.peek() == ',')
        in.get();
    in >> item.y();

    if (!in.good())
        return in;
          
    // eat all the crap up to the next number
    while (in.peek() == ' ' || in.peek() == '\t' || in.peek() == ',')
        in.get();
    in >> item.z();

    if (!in.good())
        return in;
          
    // eat all the crap up to the ')'
    while (in.peek() == ' ' || in.peek() == '\t')
        in.get();

    // there should be a ')' if not then this is an error
    if (in.get() != ')')
        in.setstate(in.rdstate() | std::ios::failbit);
    return in;
}

// ----------------------------------------------------------------------------------------


template<typename T>
std::ostream& operator<< (
    std::ostream& out, 
    const dvector<T,2>& item 
)
{
    out << "(" << item.x() << ", " << item.y() << ")";
    return out;
}

template<typename T>
std::istream& operator>>(
    std::istream& in, 
    dvector<T,2>& item 
)   
{

    // eat all the crap up to the '(' 
    while (in.peek() == ' ' || in.peek() == '\t' || in.peek() == '\r' || in.peek() == '\n')
        in.get();

    // there should be a '(' if not then this is an error
    if (in.get() != '(')
    {
        in.setstate(in.rdstate() | std::ios::failbit);
        return in;
    }

    // eat all the crap up to the first number 
    while (in.peek() == ' ' || in.peek() == '\t')
        in.get();
    in >> item.x();

    if (!in.good())
        return in;
          
    // eat all the crap up to the next number
    while (in.peek() == ' ' || in.peek() == '\t' || in.peek() == ',')
        in.get();
    in >> item.y();

    if (!in.good())
        return in;
          
    // eat all the crap up to the ')'
    while (in.peek() == ' ' || in.peek() == '\t')
        in.get();

    // there should be a ')' if not then this is an error
    if (in.get() != ')')
        in.setstate(in.rdstate() | std::ios::failbit);
    return in;
}

// ----------------------------------------------------------------------------------------

typedef dvector<long,2> point;
typedef dvector<double,2> dpoint;

// ----------------------------------------------------------------------------------------



namespace std
{
    /*!
        Define std::less<dvector<T,3> > so that you can use vectors in the associative containers.
    !*/
    template<typename T>
    struct less< dvector<T,3> > : public binary_function< dvector<T,3> , dvector<T,3> ,bool>
    {
        inline bool operator() (const  dvector<T,3> & a, const  dvector<T,3> & b) const
        { 
            if      (a.x() < b.x()) return true;
            else if (a.x() > b.x()) return false;
            else if (a.y() < b.y()) return true;
            else if (a.y() > b.y()) return false;
            else if (a.z() < b.z()) return true;
            else if (a.z() > b.z()) return false;
            else                    return false;
        }
    };

    /*!
        Define std::less<dvector<T,2> > so that you can use dvector<T,2>s in the associative containers.
    !*/
    template<typename T>
    struct less< dvector<T,2> > : public binary_function< dvector<T,2> , dvector<T,2> ,bool>
    {
        inline bool operator() (const  dvector<T,2> & a, const  dvector<T,2> & b) const
        { 
            if      (a.x() < b.x()) return true;
            else if (a.x() > b.x()) return false;
            else if (a.y() < b.y()) return true;
            else if (a.y() > b.y()) return false;
            else                    return false;
        }
    };
}

#if defined(_MSC_VER) && _MSC_VER < 1400
// turn this warning back on
#pragma warning(default:4805)
#endif

#endif // DLIB_VECTOr_H_

