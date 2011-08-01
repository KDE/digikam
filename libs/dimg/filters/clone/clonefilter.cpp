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


#include"clonefilter.h"

//Local includes

#include "taucsaddon.h"
#incldue"dimg.h"
#include"dcolor.h"


//C++ includes
#include<cmath>
#include<cstdlib>
#include <map>
#include <iostream>
using std::cout;
using std::endl;
using std::map;
#include <assert.h>
typedef unsigned int uint;


namespace Digikam
{

    CloneFilter::CloneFilter(QObject* parent):DImgThreadedFilter(parent)
    {
        initFilter();
    }

    CloneFilter::CloneFilter(const DImg* orgImage, const  DImg *Mask, const QPoint disP, QObject *parent)
        :DImgThreadedFilter(orgImage,parent,"CloneFilter")
    {
        dis = disP;
        originalImage =orgImage;      
        maskImage = Mask;
        MASK_BG = new QColor(255,255,255);

        initFilter();
    }

    CloneFilter::~CloneFilter()
    {
        cancelFilter();
    }


   bool CloneFilter::inimage( DImg *img, int x, int y )
    {
            if ( x >= 0 && x < img->width() && y >= 0 && y < img->height() )
                    return true;
            else
                    return false;
    }

   bool CloneFilter::inBrushpixmap(QPixmap *brushmap, int x, int y)
   {
       if ( x >= 0 && x < brushmap->width() && y >= 0 && y < brushmap->height() )
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

    void CloneFilter::filterImage()
    {
        float*      I[3];
        float*      div[3];
        int*        OriImg[3];
        int*        M[3];
        int*       desImg[3];
        DColor col;
        for (int c = 0 ; c < 3; c++)
        {
            I[c]   = 0;
            M[c] = 0;
            div[c] = 0;
            OriImg[c]   = 0;
            desImg[c] = 0;

        }
        int width  = originalImage.width();

        int height = originalImage.height();

        float clip = originalImage.sixteenBit() ? 65535.0 : 255.0;
        for(int a = 0; a < 3; a++)
        {
            I[c] = new float[width * height];
            M[c] = new int[width * height];
            div[c] = new float[width * height];
            OriImg[c] = new int[width * height];
            desImg[c] = new int[width * height];
        }
        int j = 0;
        for(int y = 0; y < height; y++)
        for(int x = 0; x < width; x++)
        {
           col  = originalImage.getPixelColor(x,y);

           OriImg[0][j] = col.red();
           OriImg[1][j] = col.green();
           OriImg[2][j] = col.blue();

           desImg[0][j] = col.red();
           desImg[1][j] = col.green();
           desImg[2][j] = col.blue();

           col = maskImage.getPixelColor(x,y);
           M[0][j] = colM.red();
           M[1][j] = colM.green();
           M[2][j] = colM.blue();

           j++;
        }

        for(int y = 0; y < height; y++)
            for(int x = 0; x < width; x++)
            {
            if(inimage(originalImage, x-dis.x(), y-dis.y()))
            {
                desImg[0][y*width + x] = (float)OriImg[0][(y-dis.y())*width + x-dis.x()];
                desImg[1][y*width + x] = (float)OriImg[1][(y-dis.y())*width + x-dis.x()];
                desImg[2][y*width + x] = (float)OriImg[2][(y-dis.y())*width + x-dis.x()];
            }
            }
        for(int i=0; i < height*width; i++)
        {
            I[0][i] = desImg[0][i]/clip;
            I[1][i] = desImg[1][i]/clip;
            I[2][i] = desImg[2][i]/clip;

         }

        divergents(I,div);

        for(int i=0; i < height*width; i++)
        {
            I[0][i] = OriImg[0][i]/clip;
            I[1][i] = OriImg[1][i]/clip;
            I[2][i] = OriImg[2][i]/clip;
        }

        int x,y;       

        // Build mapping from (x,y) to variables
        int N = 0; // variable indexer
        map<int,int> mp;
        for (y = 1; y < height-1; y++) {
                for (x = 1; x < width-1; x++) {
                        int id = y*width+x;
                        if (QColor(M[0][y*width+x],M[1][y*width+x],M[2][y*width+x])!= MASK_BG) { // Masked pixel
                                mp[id] = N;
                                N++;
                        }
                }
        }

        if (N == 0) {
                cout << "Solver::solve: No masked pixels found (mask color is non-black)\n";
                return;
        }

        cout << "Solver::solve: Solving " << w << "x" << h << " with " << N << " unknowns" << endl;

        // Build the matrix
        // ----------------
        // We solve Ax = b for all 3 channels at once

        // Create the sparse matrix, we have at least 5 non-zero elements
        // per column

        taucs_ccs_matrix *pAt = taucs_ccs_create(N,N,5*N,TAUCS_DOUBLE);
        double* b = new double[3*N];// All 3 channels at once
        uint n = 0;
        int index = 0;

        double* vals = pAt->taucs_values;
        int* rowptr = pAt->colptr;
        int* colind = pAt->rowind;

        // Populate matrix
        for (y = 1; y < heiht-1; y++) {
                for (x = 1; x < width-1; x++) {
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

        assert(n == N);
        rowptr[n] = index; // mark last CRS index

        taucs_ccs_matrix *pA = MatrixTranspose(pAt); // The coefficients of linear equations in the left

        double* u = new double[3*N];

        char* options[] = { "taucs.factor.LU=true", NULL };

//Solve the Linear Equations
        if (taucs_linsolve(pA, NULL, 3, u, b, options, NULL) != TAUCS_SUCCESS) {
                cout << "Solving failed\n";
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
            resultImage.setPixelColor(x,y,col);

        }

        // Free buffers.
        I[c]   = 0;
        M[c] = 0;
        div[c] = 0;
        OriImg[c]   = 0;
        desImg[c] = 0;

        for (int c = 0; c < 3; c++)
            {
                delete [] I[c];
                delete [] M[c];
                delete [] div[c];
                delete [] OriImg[c];
                delete [] desImg[c];

            }

            delete [] u;
            delete [] b;
            pA = NULL;
            pAt->taucs_values = NULL;
            pAt->colptr = NULL;
            pAt->rowind = NULL;
            delete [] vals;
            delete [] rowptr ;
            delete [] colind ;
    }

DImg* CloneFilter::getResultImg()
    {
         return resultImage;
    }

void  CloneFilter::divergents(float* I, float* O)
{
    int h = originalImage->height();
    int w = originalImage->width();
    for (int y = 1; y < h-1; y++)
            for (int x = 1; x < w-1; x++)
            {
               O[0][y*w+x] = I[0][y*w+x+1] + I[0][y*w+x-1] + I[0][(y-1)*w+x] + I[0][(y+1)+x] - 4*I[0][y*w+x];
               O[1][y*w+x] = I[1][y*w+x+1] + I[1][y*w+x-1] + I[1][(y-1)*w+x] + I[1][(y+1)+x] - 4*I[1][y*w+x];
               O[2][y*w+x] = I[2][y*w+x+1] + I[2][y*w+x-1] + I[2][(y-1)*w+x] + I[2][(y+1)+x] - 4*I[2][y*w+x];

           }

}


