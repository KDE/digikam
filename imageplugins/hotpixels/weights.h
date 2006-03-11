/* ============================================================
 * File  : weights.h
 * Author: Unai Garro <ugarro at users dot sourceforge dot net>
 * Date  : 2005-03-27
 * Description : a class to calculate filter weights
 * 
 * Copyright 2005 by Unai Garro
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
 * ============================================================
 * The algorithm for calculating the weights was based on
 * the code of jpegpixi, which was released under the GPL license,
 * and is Copyright (C) 2003, 2004 Martin Dickopp
 * ============================================================*/

#ifndef WEIGHTS_H
#define WEIGHTS_H

// Qt includes.

#include <qrect.h>
#include <qvaluelist.h>

namespace DigikamHotPixelsImagesPlugin
{

class Weights 
{
public:

    Weights(){}
    Weights(const Weights &w);
    void operator=(const Weights &w);
    
    ~Weights()
    {    
        if (!mWeightMatrices) return;
        for (unsigned int i=0; i<mPositions.count(); i++)
            {
                for (unsigned int j=0; j<mHeight; j++) delete[] mWeightMatrices[i][j];
            }
        }
    
    unsigned int      height(void)        const   { return mHeight; }
    unsigned int      polynomeOrder(void) const   { return mPolynomeOrder; }
    bool     twoDim(void)        const   { return mTwoDim; }
    unsigned int      width(void)         const   { return mWidth; }
    
    void     setHeight(int h)            { mHeight=h; };
    void     setPolynomeOrder(int order) { mPolynomeOrder=order; }
    void     setTwoDim(bool td)          { mTwoDim=td; }
    void     setWidth(int w)             { mWidth=w; }
    
    void     calculateWeights();
    bool     operator==(const Weights& ws) const;
    double** operator[](int n) const            { return mWeightMatrices[n]; }
    const QValueList <QPoint> positions() const { return mPositions; }

protected:

    int       coefficientNumber() const { return mCoefficientNumber; }
    
    double*** weightMatrices() const    { return mWeightMatrices; }
    
private:

    unsigned int                 mHeight,mWidth;
    QValueList <QPoint> mPositions;
    unsigned int                 mCoefficientNumber;
    bool                mTwoDim;
    unsigned int                 mPolynomeOrder;
    double ***          mWeightMatrices; //Stores a list of weight matrices
    
private:
    
    double polyTerm (const size_t i_coeff, const int x, const int y, const int poly_order);
    void   matrixInv (double *const a, const size_t size);
};

}  // NameSpace DigikamHotPixelsImagesPlugin

#endif  // WEIGHTS_H
