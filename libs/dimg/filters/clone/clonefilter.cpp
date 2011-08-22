/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-07-05
 * Description : a digiKam image plugin to clone area .
 *
 * Copyright (C) 2011-07-05 by Zhang Jie <zhangjiehangyuan2005 dot at gmail dot com>
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

#include "clonefilter.h"

//Qt includes

#include <QVariant>

// C++ includes

#include <cmath>
#include <assert.h>
#include <map>

// KDE includes

#include <kdebug.h>

// Local includes

#include "dimg.h"
#include "dcolor.h"
//#include "taucsaddon.h"

//using std::map;

namespace Digikam
{

CloneFilter::CloneFilter(QObject* parent)
    : DImgThreadedFilter(parent)
{
    initFilter();
}

CloneFilter::CloneFilter(const DImg & orgImage,const DImg & destImage, DImg* maskImage, const QPoint& dis, QObject* parent)
    :DImgThreadedFilter(parent, "CloneFilter")
{
    m_dis           = dis;
    setOriginalImage (orgImage);  
    m_destImage     = destImage;
    m_maskImage     = maskImage;
    MASK_BG         = QColor(255,255,255);

    initFilter();
}



CloneFilter::~CloneFilter()
{
    cancelFilter();
}

bool CloneFilter::inimage( DImg img, int x, int y )
{
    if ( x >= 0 && (uint)x < img.width() && y >= 0 && (uint)y < img.height() )
        return true;
    else
        return false;
}

bool CloneFilter::inBrushpixmap(QPixmap* brushmap, int x, int y)
{
    if ( x >= 0 && (uint)x < brushmap->width() && y >= 0 && (uint)y < brushmap->height() )
        return true;
    else
        return false;
}

float static inline clamp01(float f)
{
    if (f < 0.0f) return 0.0f;
    else if (f > 1.0f) return 1.0f;
    else return f;
}

bool CloneFilter::SOR(double **A, double *b, double *x, int N, double w, int maxstep, double e)
{
	double temp1, temp2;
	double *x0 = new double[N];
	for(int i = 0; i < N; i++)
		x0[i] = 0;
	int k = 0;
	while(k < maxstep)
	{
		for(int i=0; i < N; i++)
		{
			temp1=0;
			temp2=0;
			for(int j=0; j<5; j++)
			{
				int adr = A[i][j];
				if(adr >= 0)
				{
					if(adr < i)
				        temp1+=x[adr];
			        else 
						if(adr > i)
				         temp2+=x0[adr];
				}
			}
            temp1 = 0 - temp1;
			temp2 = 0 - temp2;
            x[i] = (1-w) * x0[i] + w * (temp1 + temp2 + b[i])/(-4.0);
			//cout<<x[i]<<" ";
		}
		//cout<<endl;
		temp1=0;
		for(int i=0; i<N; i++)
			temp1 += sqrt((x[i]-x0[i])*(x[i]-x0[i]));

		k++;
		for(int i=0;i<N;i++)		
			x0[i]=x[i];		
		if(temp1<e)
		{
			delete []x0;
			return true;
		}
	}
	return false;
}


void CloneFilter::filterImage()
{
    float* I[3];
    float* div[3];
    int*   OriImg[3];
    int*   M[3];
    int*   desImg[3];
    DColor col, colM;

    for (int c = 0 ; c < 3; c++)
    {
        I[c]      = 0;
        M[c]      = 0;
        div[c]    = 0;
        OriImg[c] = 0;
        desImg[c] = 0;

    }

    int width  = m_orgImage.width();
    int height = m_orgImage.height();
    float clip = m_orgImage.sixteenBit() ? 65535.0 : 255.0;

    for(int c = 0; c < 3; c++)
    {
        I[c]      = new float[width * height];
        M[c]      = new int  [width * height];
        div[c]    = new float[width * height];
        OriImg[c] = new int  [width * height];
        desImg[c] = new int  [width * height];
    }

    int j = 0;
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            col  = m_orgImage.getPixelColor(x,y);

            OriImg[0][j] = col.red();
            OriImg[1][j] = col.green();
            OriImg[2][j] = col.blue();

            desImg[0][j] = col.red();
            desImg[1][j] = col.green();
            desImg[2][j] = col.blue();

            colM    = m_maskImage->getPixelColor(x,y);
            M[0][j] = colM.red();
            M[1][j] = colM.green();
            M[2][j] = colM.blue();

            j++;
        }
    }

    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            if(inimage(m_orgImage, x-m_dis.x(), y-m_dis.y()))
            {
                desImg[0][y*width + x] = (float)OriImg[0][(y-m_dis.y())*width + x-m_dis.x()];
                desImg[1][y*width + x] = (float)OriImg[1][(y-m_dis.y())*width + x-m_dis.x()];
                desImg[2][y*width + x] = (float)OriImg[2][(y-m_dis.y())*width + x-m_dis.x()];
            }
        }

        for(int i=0; i < height*width; i++)
        {
            I[0][i] = desImg[0][i]/clip;
            I[1][i] = desImg[1][i]/clip;
            I[2][i] = desImg[2][i]/clip;
        }
    }

    divergents(I, div);

    for(int i=0; i < height*width; i++)
    {
        I[0][i] = OriImg[0][i]/clip;
        I[1][i] = OriImg[1][i]/clip;
        I[2][i] = OriImg[2][i]/clip;
    }

    int x, y;

    // Build mapping from (x,y) to variables
    int           N = 0; // variable indexer
    std::map<int, int> mp;

    for (y = 1; y < height-1; y++)
    {
        for (x = 1; x < width-1; x++)
        {
            int id = y*width+x;
            if (QColor(M[0][y*width+x],M[1][y*width+x],M[2][y*width+x])!= MASK_BG)
            {
                // Masked pixel
                mp[id] = N;
                N++;
            }
        }
    }

    if (N == 0)
    {
            kDebug() << "Solver::solve: No masked pixels found (mask color is non-black)";
            return;
    }

  ///* FIXME : this code do not compile yet

    kDebug() << "Solver::solve: Solving " << width << "x" << height << " with " << N << " unknowns" << endl;

    double **Arr;	
    Arr = new double*[N];
    for(int i=0; i < N; i++)
        Arr[i] = new double[5];
    for(int i =0 ;i <N; i++)
        for(int j=0; j<5;j++)
            Arr[i][j] = -1;
    double* Br  = new double[N];
    double* Bg  = new double[N];
    double* Bb  = new double[N];
    int index = 0, n = 0;
    for (y = 1; y < height-1; y++) {
        for (x = 1; x < width-1; x++) {
            if (QColor(M[0][y*width+x],M[1][y*width+x],M[2][y*width+x]) != MASK_BG) 
              {
                int id = y*width+x;
                float br =  div[0][y*width + x];
                float bg =  div[1][y*width + x];
                float bb =  div[2][y*width + x];                
                if (QColor(M[0][(y-1)*width+x],M[1][(y-1)*width+x],M[2][(y-1)*width+x]) != MASK_BG) 
                  {
                          Arr[index][0] =mp[id-width] ;				
                } else {
                           // pixel, update right hand side
                          br -= I[0][(y-1)*width + x];
                          bg -= I[1][(y-1)*width + x];
                          bb -= I[2][(y-1)*width + x];
                          }

              if (QColor(M[0][y*width+x-1],M[1][y*width+x-1],M[2][y*width+x-1]) != MASK_BG) 
                {
                          Arr[index][1] = mp[id-1];				
               } else {
                          br -= I[0][y*width + x-1];
                          bg -= I[1][y*width + x-1];
                          bb -= I[2][y*width + x-1];
                         }
              Arr[index][2] = mp[id];				
              if (QColor(M[0][y*width+x+1],M[1][y*width+x+1],M[2][y*width+x+1]) != MASK_BG) 
                {
                          Arr[index][3] = mp[id+1];
              } else  {				
                          br -= I[0][y*width + x+1];
                          bg -= I[1][y*width + x+1];
                          bb -= I[2][y*width + x+1];
                          }
              if (QColor(M[0][(y+1)*width+x],M[1][(y+1)*width+x],M[2][(y+1)*width+x]) != MASK_BG) 
                {					
                          Arr[index][4] = mp[id+width];	
              } else  {
                          br -= I[0][(y+1)*width + x];
                          bg -= I[1][(y+1)*width + x];
                          bb -= I[2][(y+1)*width + x];;
                          }
              int k = mp[id];//Record the location of the last mask point
              // Spread the right hand side so we can solve using TAUCS for
              Br[k] = br;
              Bg[k] = bg;
              Bb[k] = bb;
              index++;
              n++;
           }
         }
      }

    assert(n == N);
    double wv = 1.5;
    double e  = 0;
    if(N < 5000)
        e = 0.1;
        else if(N < 1000)
                e = 1;
            else if(N<20000)
                e = 2.5;
                else if(e < 30000)
                    e = 5;
                    else if(e < 50000)
                        e = 10;
                        else 
                            kDebug() << "Clone area is too large";
    int maxstep = 1000;
    double *Xr = new double[N];
    double *Xg = new double[N];
    double *Xb = new double[N];
    for(int i=0;i<N;i++)
    {
        Xr[i]=0;
        Xg[i]=0;
        Xb[i]=0;
    }
    bool SorR = SOR(Arr, Bb, Xb, N, wv, maxstep, e);
    bool SorG = SOR(Arr, Bg, Xg, N, wv, maxstep, e);
    bool SorB = SOR(Arr, Br, Xr, N, wv, maxstep, e);	
    delete []Br;
    delete []Bg;
    delete []Bb;
    if(SorR && SorG && SorB)
    {
        for (y = 1; y < height-1; y++) 
         {
            for (x = 1; x < width-1; x++) 
              {
                if (QColor(M[0][y*width+x],M[1][y*width+x],M[2][y*width+x]) != MASK_BG) 
                  {
                    int id = y * width + x;
                    int ii = mp[id];
                    I[0][y*width+x] = clamp01((float)Xr[ii]);
                    I[1][y*width+x] = clamp01((float)Xg[ii]);
                    I[2][y*width+x] = clamp01((float)Xb[ii]);
                }
            }
        }
    }

    for(int i=0; i < height*width; i++)
    {
        col.setRed((int)(I[0][y*width+x] * clip));
        col.setGreen((int)(I[1][y*width+x ]* clip));
        col.setBlue((int)(I[2][y*width+x]*clip));
        col.setAlpha(255);
        m_destImage.setPixelColor(x,y,col);
    }
}

 // Build the matrix
    // ----------------
    // We solve Ax = b for all 3 channels at once

    // Create the sparse matrix, we have at least 5 non-zero elements
    // per column

    //NOTE taucs_ccs_matrix is definded in taucs.h
  /*
  taucs_ccs_matrix* pAt = taucs_dtl(ccs_create)(N,N,5*N);
    double* b             = new double[3*N];// All 3 channels at once
    uint n                = 0;
    int index             = 0;

    double* vals = pAt->taucs_values;
    int* rowptr  = pAt->colptr;
    int* colind  = pAt->rowind;

    // Populate matrix
    for (y = 1; y < height-1; y++)
    {
            for (x = 1; x < width-1; x++)
            {
                    if (QColor(M[0][y*width+x],M[1][y*width+x],M[2][y*width+x]) != MASK_BG)
                    {
                            int id = y*width+x;
                            rowptr[n] = index;
                            float br =  div[0][y*width + x];
                            float bg =  div[1][y*width + x];
                            float bb =  div[2][y*width + x];

                            if (QColor(M[0][(y-1)*width+x],M[1][(y-1)*width+x],M[2][(y-1)*width+x]) != MASK_BG) {
                                    vals[index] = 1.0f;
                                    colind[index] = mp[id-width];
                                    index++;
                            } else {
                                    // Known pixel, update right hand side
                                    br -= I[0][(y-1)*width + x];
                                    bg -= I[1][(y-1)*width + x];
                                    bb -= I[2][(y-1)*width + x];
                            }

                            if (QColor(M[0][y*width+x-1],M[1][y*width+x-1],M[2][y*width+x-1]) != MASK_BG) {
                                    vals[index] = 1.0f;
                                    colind[index] = mp[id-1];
                                    index++;
                            } else {
                                    bb -= I[0][y*width + x - 1];
                                    bg -= I[1][y*width + x - 1];
                                    br -= I[2][y*width + x - 1];
                            }

                            vals[index] = -4.0f;
                            colind[index] = mp[id];
                            index++;

                            if (QColor(M[0][y*width+x+1],M[1][y*width+x+1],M[2][y*width+x+1]) != MASK_BG) {
                                    vals[index] = 1.0f;
                                    colind[index] = mp[id+1];
                                    index++;
                            } else
                            {
                                    br -= I[0][y*width + x + 1];
                                    bg -= I[1][y*width + x + 1];
                                    bb -= I[2][y*width + x + 1];
                            }

                            if (QColor(M[0][(y+1)*width+x],M[1][(y+1)*width+x],M[2][(y+1)*width+x]) != MASK_BG)
                            {
                                    vals[index] = 1.0f;
                                    colind[index] = mp[id+width];
                                    index++;
                            }
                            else
                            {
                                    br -= I[0][(y+1)*width + x];
                                    bg -= I[0][(y+1)*width + x];
                                    bb -= I[0][(y+1)*width + x];
                            }
                            uint i = mp[id];//Record the location of the last mask point

                            // Spread the right hand side so we can solve using TAUCS for
                            // 3 channels at once.
                            b[i] = br;
                            b[i+N] = bg;
                            b[i+2*N] = bb;
                            n++;
                    }
            }
    }

    Q_ASSERT(n == N);

    rowptr[n] = index; // mark last CRS index

    taucs_ccs_matrix *pA = MatrixTranspose(pAt); // The coefficients of linear equations in the left

    double* u = new double[3*N];

    QByteArray option = "taucs.factor.LU=true";
    char* options[] = { option.data(), NULL };

    //Solve the Linear Equations
    if (taucs_linsolve(pA, NULL, 3, u, b, options, NULL) != TAUCS_SUCCESS)
    {
        kDebug() << "Solving failed\n";
    }

    // Convert solution vector back to image
    for (y = 1; y < height-1; y++)
    {
            for (x = 1; x < width-1; x++)
            {
                    if (QColor(M[0][y*width+x],M[1][y*width+x],M[2][y*width+x]) != MASK_BG)
                    {
                            int id = y*width+x;
                            int ii = mp[id];
                            I[0][y*width+x] = clamp01((float)u[ii]);
                            I[1][y*width+x] = clamp01((float)u[ii+N]);
                            I[2][y*width+x] = clamp01((float)u[ii+2*N]);

                    }
            }
    }

    for(int y =0; y < height; y++)
        for(int x=0; x < width; x++)

    for(int i=0; i < height*width; i++)
    {
        col.setRed((int)(I[0][y*width+x] * clip));
        col.setGreen((int)(I[1][y*width+x ]* clip));
        col.setBlue((int)(I[2][y*width+x]*clip));
        col.setAlpha(255);
        m_destImage->setPixelColor(x,y,col);
    }

    // Free buffers.

    for (int c = 0; c < 3; c++)
    {
        delete [] I[c];
        delete [] M[c];
        delete [] div[c];
        delete [] OriImg[c];
        delete [] desImg[c];
    }

//FIXME
    delete [] u;
    delete [] b;
    pA = NULL;
    pAt->taucs_values = NULL;
    pAt->colptr = NULL;
    pAt->rowind = NULL;
    delete [] vals;
    delete [] rowptr ;
    delete [] colind ;

}*/

///*FIXME
FilterAction CloneFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    //FIXME QImage m_oriImage = m_orgImage.copyQImage();
    QImage m_mask     = m_maskImage->copyQImage();

    action.addParameter("distancePoint", m_dis);
    action.addParameter("originalImage", m_orgImage.copyQImage());  //need fixed, maybe parameters cannot be transfered !!!!
    action.addParameter("maskImage",     m_mask);

    return action;
}

void CloneFilter::readParameters(const FilterAction& action)
{
    m_dis           = action.parameter("distancePoint").toPoint();
    m_orgImage      = DImg(action.parameter("originalImage").value<QImage>());
    m_maskImage     = new DImg(action.parameter("maskImage").value<QImage>());   
 /*   QVariant tempImg = action.parameter("originalImage").value<QImage>();
    tempImg.convert(QVariant::Image);
    m_originalImage = new DImg(tempImg);

    tempImg = action.parameter("maskImage");
    tempImg.convert(QVariant::Image);*/

}
//---------use getTargetImage () frome DImgThreadedFilter instead-----
/*
DImg* CloneFilter::getResultImg() const
{
    return m_resultImage;
}
*/
//===================================================================================
void CloneFilter::divergents(float* I[3], float* O[3])
{

    int h = m_orgImage.height();
    int w = m_orgImage.width();

    for (int y = 1; y < h-1; y++)
    {
        for (int x = 1; x < w-1; x++)
        {
            O[0][y*w+x] = I[0][y*w+x+1] + I[0][y*w+x-1] + I[0][(y-1)*w+x] + I[0][(y+1)+x] - 4*I[0][y*w+x];
            O[1][y*w+x] = I[1][y*w+x+1] + I[1][y*w+x-1] + I[1][(y-1)*w+x] + I[1][(y+1)+x] - 4*I[1][y*w+x];
            O[2][y*w+x] = I[2][y*w+x+1] + I[2][y*w+x-1] + I[2][(y-1)*w+x] + I[2][(y+1)+x] - 4*I[2][y*w+x];
        }
    }

}

} // namespace Digikam
