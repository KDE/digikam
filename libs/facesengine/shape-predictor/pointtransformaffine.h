#ifndef POINT_TRANSFORM_AFFINE_H
#define POINT_TRANSFORM_AFFINE_H

#include <vector>

#include "matrixoperations.h"
#include "vectoroperations.h"

#include <iostream>

using namespace std;




class pointtransformaffine
{
public:

    pointtransformaffine (
    )
    {
        //m = identity_matrix<float>(2);
        m = std::vector<std::vector<float> >(2,std::vector<float>(2,0));
        m[0][0] = 1.0;
        m[1][1] = 1.0;
        b = std::vector<float>(2,0);
    }

    pointtransformaffine (
        //const matrix<float,2,2>& m_,
        const std::vector<std::vector<float> > & m_,
        //const dlib::vector<float,2>& b_
        const std::vector<float>& b_
    ) :m(m_), b(b_)
    {
    }

    pointtransformaffine (
        const std::vector<std::vector<float> > & m_
    )
    {
        m = std::vector<std::vector<float> >(2,std::vector<float>(2,0));
        b = std::vector<float >(2,0);
        for(unsigned int i = 0;i<m_.size();i++)
            for(unsigned int j =0; j<m_[0].size();j++)
            {
                if(j == 2)
                {
                    b[i]    = m_[i][2];
                }
                else
                {
                    m[i][j] = m_[i][j];
                }
            }
    }

    //const dlib::vector<float,2> operator() (
    //    const dlib::vector<float,2>& p
    const std::vector<float> operator() (
        const std::vector<float>& p
    ) const
    {
        return m*p + b;
    }


    //const matrix<float,2,2>& get_m(
    const std::vector<std::vector<float> >& get_m(
    ) const { return m; }

    //const dlib::vector<float,2>& get_b(
    const std::vector<float>& get_b(
    ) const { return b; }

//    inline friend void serialize (const pointtransformaffine& item, std::ostream& out)
//    {
//        serialize(item.m, out);
//        serialize(item.b, out);
//    }

//    inline friend void deserialize (pointtransformaffine& item, std::istream& in)
//    {
//        deserialize(item.m, in);
//        deserialize(item.b, in);
//    }

private:
    //matrix<float,2,2> m;
    std::vector<std::vector<float> >m;
    //dlib::vector<float,2> b;
    std::vector<float> b;
};

// ----------------------------------------------------------------------------------------

inline pointtransformaffine operator* (
    const pointtransformaffine& lhs,
    const pointtransformaffine& rhs
)
{
    return pointtransformaffine(lhs.get_m()*rhs.get_m(), lhs.get_m()*rhs.get_b()+lhs.get_b());
}

// ----------------------------------------------------------------------------------------

inline pointtransformaffine inv (
    const pointtransformaffine& trans
)
{
    //matrix<float,2,2> im = inv(trans.get_m());
    std::vector<std::vector<float> > im = inv2(trans.get_m());
    return pointtransformaffine(im, -(im*trans.get_b()));
}

// ----------------------------------------------------------------------------------------

template <typename T>
pointtransformaffine find_affine_transform (
    //const std::vector<dlib::vector<T,2> >& from_points,
    const std::vector<std::vector<T> >& from_points,
    //const std::vector<dlib::vector<T,2> >& to_points
    const std::vector<std::vector<T> >& to_points
)
{
    // make sure requires clause is not broken
//    DLIB_ASSERT(from_points.size() == to_points.size() &&
//                from_points.size() >= 3,
//        "\t pointtransformaffine find_affine_transform(from_points, to_points)"
//        << "\n\t Invalid inputs were given to this function."
//        << "\n\t from_points.size(): " << from_points.size()
//        << "\n\t to_points.size():   " << to_points.size()
//        );

//    matrix<float,3,0> P(3, from_points.size());
    std::vector<std::vector<float> > P(3, std::vector<float>(from_points.size()));
//    matrix<float,2,0> Q(2, from_points.size());
    std::vector<std::vector<float> > Q(2, std::vector<float>(from_points.size()));

    for (unsigned long i = 0; i < from_points.size(); ++i)
    {
//        P(0,i) = from_points[i].x();
        P[0][i] = from_points[i][0];
//        P(1,i) = from_points[i].y();
        P[1][i] = from_points[i][1];
//        P(2,i) = 1;
        P[2][i] = 1;

        //Q(0,i) = to_points[i].x();
        Q[0][i] = to_points[i][0];

        //Q(1,i) = to_points[i].y();
        Q[1][i] = to_points[i][1];
    }

    //const matrix<float,2,3> m = Q*pinv(P);
    const std::vector<std::vector<float> > m = Q*pinv(P);
    return pointtransformaffine(m);
}



// ----------------------------------------------------------------------------------------

//template <typename T>
pointtransformaffine find_similarity_transform (
    //const std::vector<dlib::vector<T,2> >& from_points,
    const std::vector<std::vector<float> >& from_points,
    //const std::vector<dlib::vector<T,2> >& to_points
    const std::vector<std::vector<float> >& to_points
)
{
    // make sure requires clause is not broken
//    DLIB_ASSERT(from_points.size() == to_points.size() &&
//                from_points.size() >= 2,
//        "\t pointtransformaffine find_similarity_transform(from_points, to_points)"
//        << "\n\t Invalid inputs were given to this function."
//        << "\n\t from_points.size(): " << from_points.size()
//        << "\n\t to_points.size():   " << to_points.size()
//        );

    // We use the formulas from the paper: Least-squares estimation of transformation
    // parameters between two point patterns by Umeyama.  They are equations 34 through
    // 43.

    //dlib::vector<float,2> mean_from, mean_to;
    std::vector<float> mean_from(2,0), mean_to(2,0);
    float sigma_from = 0, sigma_to = 0;
    std::vector<std::vector<float> > cov(2,std::vector<float>(2,0));
    //cov = 0;

    for (unsigned long i = 0; i < from_points.size(); ++i)
    {
        mean_from = mean_from + from_points[i];
        mean_to   = mean_to   + to_points[i];
    }
    mean_from = mean_from / from_points.size();
    mean_to   = mean_to   / from_points.size();

    for (unsigned long i = 0; i < from_points.size(); ++i)
    {
        sigma_from = sigma_from + length_squared(from_points[i] - mean_from);
        sigma_to   = sigma_to   + length_squared(to_points[i]   - mean_to);
        cov        = cov + (to_points[i] - mean_to)*(from_points[i] - mean_from);
    }

    sigma_from = sigma_from / from_points.size();
    sigma_to   = sigma_to   / from_points.size();
    cov        = cov        / from_points.size();
    std::vector<std::vector<float> >  u(2,std::vector<float>(2));
    std::vector<std::vector<float> >  v(2,std::vector<float>(2));
    std::vector<std::vector<float> > vt(2,std::vector<float>(2));
    std::vector<std::vector<float> >  d(2,std::vector<float>(2));
    std::vector<std::vector<float> >  s(2,std::vector<float>(2,0));
//    cv::Mat covcv(2,2,CV_64FC1);

//    stdmattocvmat(cov,covcv);

//    //matrix<float,2,2> u, v, s, d;
//    cv::Mat u(2,2,CV_64FC1),    v(2,2,CV_64FC1),
//            s(2,2,CV_64FC1), diag(2,2,CV_64FC1);
    svd(cov, u,d,vt);
//    s = cv::Mat::eye(2,2,CV_64FC1);
    s[0][0] = 1;
    s[1][1] = 1;
    if (determinant(cov) < 0 ||
            (determinant(cov) == 0 && determinant(u)*determinant(v)<0))
    {
        if (d[1][1] < d[0][0])
            s[1][1] = -1;
        else
            s[0][0] = -1;
    }
    transpose(vt,v);
    std::vector<std::vector<float> >  r = u*s*v;
//    cout<<"cov"<<covcv<<endl<<"diag"<<diag<<endl<<"u"<<u<<endl<<"v"<<v<<endl
//       <<"r"<<r<<endl;
    float c = 1;
    if (sigma_from != 0)
        c = 1.0/sigma_from * trace(d*s);
//    std::vector<std::vector<float> > rstd(r.size(),std::vector<float>(r[0].size()));
//    cvmattostdmat(r,rstd);
    std::vector<float> t = mean_to - r*mean_from * c;

    return pointtransformaffine(r*c, t);
}

#endif // POINT_TRANSFORM_AFFINE_H
