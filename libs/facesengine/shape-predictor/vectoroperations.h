#ifndef VECTOROPERATIONS_H
#define VECTOROPERATIONS_H

#include <vector>


template<class T>
inline T length_squared(const std::vector<T> & diff)
{
    T sum = 0;
    for(unsigned int i = 0;i<diff.size();i++)
    {
        sum += diff[i] * diff[i];
    }
    return sum;
}


template<class T>
std::vector<T> operator-(const std::vector<T>& v1, const std::vector<T>& v2)
{
    assert(v1.size() == v2.size());

    std::vector<T> result(v1.size());
    for(unsigned int i = 0;i<v1.size();i++)
    {
        result[i] = v1[i] - v2[i];
    }
    return result;
}

template<class T>
std::vector<T> operator-(const std::vector<T>& v1)
{
    std::vector<T> result(v1.size());
    for(unsigned int i = 0;i<v1.size();i++)
    {
        result[i] = -v1[i];
    }
    return result;
}

template<class T>
std::vector<T> operator/(const std::vector<T>& v1, int divisor)
{
    std::vector<T> result(v1.size());
    for(unsigned int i = 0;i<v1.size();i++)
    {
        result[i] = v1[i]/divisor;
    }
    return result;
}
template<class T>
std::vector<std::vector<T> > operator/(const std::vector<std::vector<T> >& v1, int divisor)
{
    //assert(v1[0].size() != v2.size());

    std::vector<std::vector<T> > result(v1.size(),std::vector<T>(v1[0].size(),0));
    for(unsigned int i = 0;i<v1.size();i++)
    {
        for(unsigned int j = 0;j<v1[0].size();j++)
            result[i][j] = v1[i][j] / divisor;
    }
    return result;
}


template<class T>
std::vector<T> operator+(const std::vector<T>& v1, const std::vector<T>& v2)
{
    assert(v1.size() == v2.size());

    std::vector<T> result(v1.size());
    for(unsigned int i = 0;i<v1.size();i++)
    {
        result[i] = v1[i] + v2[i];
    }
    return result;
}

template<class T>
std::vector<std::vector<T> > operator+(const std::vector<std::vector<T> >& v1,
                                       const std::vector<std::vector<T> >& v2)
{
    assert(v1.size()    == v2.size() &&
           v1[0].size() == v2[0].size());

    std::vector<std::vector<T> > result(v1.size(),
                                             std::vector<T>(v1[0].size(),0));
    for(unsigned int i = 0;i<v1.size();i++)
    {
        for(unsigned int j = 0;j<v2[0].size();j++)
        {
            result[i][j] += v1[i][j] + v2[i][j];
        }

    }
    return result;
}

template<class T>
std::vector<T> operator*(const std::vector<std::vector<T> >& v1,
                         const std::vector<T>& v2)
{
    assert(v1[0].size() == v2.size());

    std::vector<T> result(v1.size());
    for(unsigned int i = 0;i<v1.size();i++)
    {
        result[i] = 0;
        for(unsigned int j = 0;j<v1[0].size();j++)
            result[i] += v1[i][j] * v2[j];
    }
    return result;
}

template<class T>
std::vector<std::vector<T> > operator*(const std::vector<std::vector<T> >& v1,
                                       const std::vector<std::vector<T> >& v2)
{
    assert(v1[0].size() == v2.size());

    std::vector<std::vector<T> > result(v1.size(),
                                             std::vector<T>(v2[0].size(),0));
    for(unsigned int i = 0;i<v1.size();i++)
    {
        for(unsigned int k = 0;k<v1[0].size();k++)
        {
            // swapping j and k loops for cache optimization
            for(unsigned int j = 0;j<v2[0].size();j++)
            {

                result[i][j] += v1[i][k] * v2[k][j];
            }
        }
    }
    return result;
}


template<class T>
std::vector<std::vector<T> > operator*(const std::vector<T>& v1,
                                       const std::vector<T>& v2)
{
    assert(v1.size() == v2.size());

    std::vector<std::vector<T> > result(v1.size(),
                                             std::vector<T>(v2.size(),0));
    for(unsigned int i = 0;i<v1.size();i++)
    {
        for(unsigned int j = 0;j<v1.size();j++)
        {
            result[i][j] = v1[i]*v2[j];
        }
    }
    return result;
}

template<class T>
std::vector<std::vector<T> > operator+(const std::vector<std::vector<T> >& v1,
                                       float d)
{
//    assert(v1.size()    == v2.size() &&
//           v1[0].size() == v2[0].size());

    std::vector<std::vector<T> > result(v1.size(),
                                             std::vector<T>(v1[0].size(),0));
    for(unsigned int i = 0;i<v1.size();i++)
    {
        for(unsigned int j = 0;j<v1[0].size();j++)
        {
            result[i][j] = v1[i][j]*d;
        }

    }
    return result;
}

template<class T>
std::vector<T> operator*(const std::vector<T>& v1,
                                       float d)
{
//    assert(v1.size()    == v2.size() &&
//           v1[0].size() == v2[0].size());

    std::vector<T> result(v1.size());
    for(unsigned int i = 0;i<v1.size();i++)
    {
        result[i] = v1[i] * d;

    }
    return result;
}


#endif // VECTOROPERATIONS_H
