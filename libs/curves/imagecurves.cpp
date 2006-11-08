/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-12-01
 * Description : image curves manipulation methods.
 * 
 * Copyright 2004-2006 by Gilles Caulier
 *
 * Some code parts are inspired from gimp 2.0
 * app/base/curves.c, gimplut.c, and app/base/gimpcurvetool.c 
 * source files.
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#define CLAMP(x,l,u) ((x)<(l)?(l):((x)>(u)?(u):(x)))

// C++ includes.
 
#include <cstdio>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <cerrno>

// Qt includes.

#include <qfile.h>

// Local includes.

#include "ddebug.h"
#include "imagecurves.h"

namespace Digikam
{

class ImageCurvesPriv
{
    
public:

    struct _Curves
    {
        ImageCurves::CurveType curve_type[5];     // Curve types by channels (Smooth or Free).
        int                    points[5][17][2];  // Curve main points in Smooth mode ([channel][point id][x,y]).
        unsigned short         curve[5][65536];   // Curve values by channels.
    };
    
    struct _Lut
    {
        unsigned short **luts;
        int              nchannels;
    };

public:

    ImageCurvesPriv()
    {
        curves = 0;
        lut    = 0;
    }

    // Curves data.
    struct _Curves *curves;
    
    // Lut data.
    struct _Lut    *lut;

    int             segmentMax;
};

ImageCurves::CRMatrix CR_basis =
{
    { -0.5,  1.5, -1.5,  0.5 },
    {  1.0, -2.5,  2.0, -0.5 },
    { -0.5,  0.0,  0.5,  0.0 },
    {  0.0,  1.0,  0.0,  0.0 },
};

ImageCurves::ImageCurves(bool sixteenBit)
{
    d = new ImageCurvesPriv;
    d->lut        = new ImageCurvesPriv::_Lut;
    d->curves     = new ImageCurvesPriv::_Curves;
    d->segmentMax = sixteenBit ? 65535 : 255;

    curvesReset();
}

ImageCurves::~ImageCurves()
{ 
    if (d->lut)
    {
       if (d->lut->luts)
       {
          for (int i = 0 ; i < d->lut->nchannels ; i++)
              delete [] d->lut->luts[i];

          delete [] d->lut->luts;
       }
       
       delete d->lut;
    }
    
    if (d->curves)
       delete d->curves;
       
    delete d;
}

void ImageCurves::curvesReset(void)
{
    memset(d->curves, 0, sizeof(struct ImageCurvesPriv::_Curves));
    d->lut->luts      = NULL;
    d->lut->nchannels = 0;

    for (int channel = 0 ; channel < 5 ; channel++)
    {
       setCurveType(channel, CURVE_SMOOTH);
       curvesChannelReset(channel);
    }
}

void ImageCurves::curvesChannelReset(int channel)
{
    int j;
    
    if (!d->curves) return;

    // Contruct a linear curve.
    
    for (j = 0 ; j <= d->segmentMax ; j++)
       d->curves->curve[channel][j] = j;

    // Init coordinates points to null.
       
    for (j = 0 ; j < 17 ; j++)
    {
       d->curves->points[channel][j][0] = -1;
       d->curves->points[channel][j][1] = -1;
    }

    // First and last points init.
       
    d->curves->points[channel][0][0]  = 0;
    d->curves->points[channel][0][1]  = 0;
    d->curves->points[channel][16][0] = d->segmentMax;
    d->curves->points[channel][16][1] = d->segmentMax;
}

void ImageCurves::curvesCalculateCurve(int channel)
{
    int i;
    int points[17];
    int num_pts;
    int p1, p2, p3, p4;

    if (!d->curves) return;

    switch (d->curves->curve_type[channel])
    {
       case CURVE_FREE:
          break;

       case CURVE_SMOOTH:
       {
          //  Cycle through the curves  
          
          num_pts = 0;
          
          for (i = 0 ; i < 17 ; i++)
             if (d->curves->points[channel][i][0] != -1)
                points[num_pts++] = i;

          //  Initialize boundary curve points 
          
          if (num_pts != 0)
          {
             for (i = 0 ; i < d->curves->points[channel][points[0]][0] ; i++)
             {
                d->curves->curve[channel][i] = d->curves->points[channel][points[0]][1];
             }
             
             for (i = d->curves->points[channel][points[num_pts - 1]][0] ; i <= d->segmentMax ; i++)
             {
                d->curves->curve[channel][i] = d->curves->points[channel][points[num_pts - 1]][1];
             }
          }

          for (i = 0 ; i < num_pts - 1 ; i++)
          {
             p1 = (i == 0) ? points[i] : points[(i - 1)];
             p2 = points[i];
             p3 = points[(i + 1)];
             p4 = (i == (num_pts - 2)) ? points[(num_pts - 1)] : points[(i + 2)];

             curvesPlotCurve(channel, p1, p2, p3, p4);
          }

          // Ensure that the control points are used exactly 
      
          for (i = 0 ; i < num_pts ; i++)
          {
             int x, y;

             x = d->curves->points[channel][points[i]][0];
             y = d->curves->points[channel][points[i]][1];
             d->curves->curve[channel][x] = y;
          }

          break;
       }
    }
}

float ImageCurves::curvesLutFunc(int n_channels, int channel, float value)
{
    float  f;
    int    index;
    double inten;
    int    j;

    if (!d->curves) return 0.0;

    if (n_channels == 1)
       j = 0;
    else
       j = channel + 1;

    inten = value;

    // For color images this runs through the loop with j = channel +1
    // the first time and j = 0 the second time.
    
    // For bw images this runs through the loop with j = 0 the first and
    // only time.
    
    for ( ; j >= 0 ; j -= (channel + 1))
    {
       // Don't apply the overall curve to the alpha channel.
       
       if (j == 0 && (n_channels == 2 || n_channels == 4) && channel == n_channels -1)
          return inten;

       if (inten < 0.0)
          inten = d->curves->curve[j][0]/(float)d->segmentMax;
       else if (inten >= 1.0)
          inten = d->curves->curve[j][d->segmentMax]/(float)(d->segmentMax);
       else       // interpolate the curve.
       {
          index = (int)floor(inten * (float)(d->segmentMax));
          f = inten * (float)(d->segmentMax) - index;
          inten = ((1.0 - f) * d->curves->curve[j][index    ] +
                   (      f) * d->curves->curve[j][index + 1] ) / (float)(d->segmentMax);
       }
    }

    return inten;
}

void ImageCurves::curvesPlotCurve(int channel, int p1, int p2, int p3, int p4)
{
    CRMatrix geometry;
    CRMatrix tmp1, tmp2;
    CRMatrix deltas;
    double   x, dx, dx2, dx3;
    double   y, dy, dy2, dy3;
    double   d1, d2, d3;
    int      lastx, lasty;
    int      newx, newy;
    int      i;
    int      loopdiv = d->segmentMax * 3;

    if (!d->curves) return;

    // Construct the geometry matrix from the segment.
  
    for (i = 0 ; i < 4 ; i++)
    {
       geometry[i][2] = 0;
       geometry[i][3] = 0;
    }

    for (i = 0 ; i < 2 ; i++)
    {
       geometry[0][i] = d->curves->points[channel][p1][i];
       geometry[1][i] = d->curves->points[channel][p2][i];
       geometry[2][i] = d->curves->points[channel][p3][i];
       geometry[3][i] = d->curves->points[channel][p4][i];
    }

    // Subdivide the curve 1000 times.
    // n can be adjusted to give a finer or coarser curve.
    
    d1  = 1.0 / loopdiv;
    d2 = d1 * d1;
    d3 = d1 * d1 * d1;

    // Construct a temporary matrix for determining the forward differencing deltas.
  
    tmp2[0][0] = 0;     tmp2[0][1] = 0;     tmp2[0][2] = 0;    tmp2[0][3] = 1;
    tmp2[1][0] = d3;    tmp2[1][1] = d2;    tmp2[1][2] = d1;   tmp2[1][3] = 0;
    tmp2[2][0] = 6*d3;  tmp2[2][1] = 2*d2;  tmp2[2][2] = 0;    tmp2[2][3] = 0;
    tmp2[3][0] = 6*d3;  tmp2[3][1] = 0;     tmp2[3][2] = 0;    tmp2[3][3] = 0;

    // Compose the basis and geometry matrices.
  
    curvesCRCompose(CR_basis, geometry, tmp1);

    // Compose the above results to get the deltas matrix.
  
    curvesCRCompose(tmp2, tmp1, deltas);

    // Extract the x deltas.
    
    x   = deltas[0][0];
    dx  = deltas[1][0];
    dx2 = deltas[2][0];
    dx3 = deltas[3][0];

    // Extract the y deltas.
    
    y   = deltas[0][1];
    dy  = deltas[1][1];
    dy2 = deltas[2][1];
    dy3 = deltas[3][1];

    lastx = (int)CLAMP (x, 0, d->segmentMax);
    lasty = (int)CLAMP (y, 0, d->segmentMax);

    d->curves->curve[channel][lastx] = lasty;

    // Loop over the curve.
    
    for (i = 0 ; i < loopdiv ; i++)
    {
       // Increment the x values.
       
       x   += dx;
       dx  += dx2;
       dx2 += dx3;

       // Increment the y values.
      
       y   += dy;
       dy  += dy2;
       dy2 += dy3;

       newx = CLAMP(ROUND (x), 0, d->segmentMax);
       newy = CLAMP(ROUND (y), 0, d->segmentMax);

       // If this point is different than the last one...then draw it.
      
       if ((lastx != newx) || (lasty != newy))
          d->curves->curve[channel][newx] = newy;

       lastx = newx;
       lasty = newy;
    }
}

void ImageCurves::curvesCRCompose(CRMatrix a, CRMatrix b, CRMatrix ab)
{
    int i, j;

    for (i = 0 ; i < 4 ; i++)
    {
       for (j = 0 ; j < 4 ; j++)
       {
          ab[i][j] = (a[i][0] * b[0][j] +
                      a[i][1] * b[1][j] +
                      a[i][2] * b[2][j] +
                      a[i][3] * b[3][j]);
       }
    }
}

void ImageCurves::curvesLutSetup(int nchannels, bool overIndicator)
{
    int    i; 
    uint   v;
    double val;

    if (d->lut->luts)
    {
       for (i = 0 ; i < d->lut->nchannels ; i++)
           delete [] d->lut->luts[i];

       delete [] d->lut->luts;
    }

    d->lut->nchannels = nchannels;
    d->lut->luts      = new unsigned short*[d->lut->nchannels];
    
    for (i = 0 ; i < d->lut->nchannels ; i++)
    {
       d->lut->luts[i] = new unsigned short[d->segmentMax+1];

       for (v = 0 ; v <= (uint)d->segmentMax ; v++)
       {
          // To add gamma correction use func(v ^ g) ^ 1/g instead. 
          
          val = (float)(d->segmentMax) * curvesLutFunc( d->lut->nchannels, i, v / (float)(d->segmentMax)) + 0.5;
                        
          if (overIndicator && val > d->segmentMax)
              val = 0;
              
          d->lut->luts[i][v] = (unsigned short)CLAMP (val, 0, d->segmentMax);
       }
    }
}

void ImageCurves::curvesLutProcess(uchar *srcPR, uchar *destPR, int w, int h)
{
    unsigned short *lut0 = NULL, *lut1 = NULL, *lut2 = NULL, *lut3 = NULL;
    
    int       i;

    if (d->lut->nchannels > 0)
       lut0 = d->lut->luts[0];
    if (d->lut->nchannels > 1)
       lut1 = d->lut->luts[1];
    if (d->lut->nchannels > 2)
       lut2 = d->lut->luts[2];
    if (d->lut->nchannels > 3)
       lut3 = d->lut->luts[3];
    
    if (d->segmentMax == 255)        // 8 bits image.
    {
        uchar red, green, blue, alpha;
        uchar *ptr = srcPR;
        uchar *dst = destPR;
        
        for (i = 0 ; i < w*h ; i++)
        {
            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];
            alpha = ptr[3];

            if ( d->lut->nchannels > 0 )
               red = lut0[red];
            
            if ( d->lut->nchannels > 1 )
               green = lut1[green];
            
            if ( d->lut->nchannels > 2 )
               blue = lut2[blue];
        
            if ( d->lut->nchannels > 3 )
               alpha = lut3[alpha];
                       
            dst[0] = blue;
            dst[1] = green;
            dst[2] = red;
            dst[3] = alpha;

            ptr += 4;
            dst += 4;
        }
    }
    else               // 16 bits image.
    {
        unsigned short red, green, blue, alpha;
        unsigned short *ptr = (unsigned short *)srcPR;
        unsigned short *dst = (unsigned short *)destPR;

        for (i = 0 ; i < w*h ; i++)
        {
            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];
            alpha = ptr[3];
        
            if ( d->lut->nchannels > 0 )
               red = lut0[red];
            
            if ( d->lut->nchannels > 1 )
               green = lut1[green];
            
            if ( d->lut->nchannels > 2 )
               blue = lut2[blue];
        
            if ( d->lut->nchannels > 3 )
               alpha = lut3[alpha];
                                
            dst[0] = blue;
            dst[1] = green;
            dst[2] = red;
            dst[3] = alpha;

            ptr += 4;
            dst += 4;
        }
    }    
}

int ImageCurves::getCurveValue(int channel, int bin)
{    
    if ( d->curves &&
         channel>=0 && channel<5 && 
         bin>=0 && bin<=d->segmentMax )
       return(d->curves->curve[channel][bin]);
    
    return 0;
}

QPoint ImageCurves::getCurvePoint(int channel, int point)
{
    if ( d->curves &&
         channel>=0 && channel<5 && 
         point>=0 && point<=17 )
       return(QPoint(d->curves->points[channel][point][0],
                     d->curves->points[channel][point][1]) );
    
    return QPoint(-1, -1);
}

int ImageCurves::getCurvePointX(int channel, int point)
{
    if ( d->curves &&
         channel>=0 && channel<5 && 
         point>=0 && point<=17 )
       return(d->curves->points[channel][point][0]);
    
    return(-1);
}

int ImageCurves::getCurvePointY(int channel, int point)
{
    if ( d->curves &&
         channel>=0 && channel<5 && 
         point>=0 && point<=17 )
       return(d->curves->points[channel][point][1]);
    
    return (-1);
}

int ImageCurves::getCurveType(int channel)
{
    if ( d->curves &&
         channel>=0 && channel<5 )
       return ( d->curves->curve_type[channel] );
    
    return (-1);
}

void ImageCurves::setCurveValue(int channel, int bin, int val)
{
    if ( d->curves &&
         channel>=0 && channel<5 && 
         bin>=0 && bin<=d->segmentMax )
       d->curves->curve[channel][bin] = val;
}

void ImageCurves::setCurvePoint(int channel, int point, QPoint val)
{
    if ( d->curves &&
         channel>=0 && channel<5 && 
         point>=0 && point<=17 &&
         val.x()>=-1 && val.x()<=d->segmentMax && // x can be egal to -1
         val.y()>=0 && val.y()<=d->segmentMax)    // if the current point is disable !!!
    {
       d->curves->points[channel][point][0] = val.x();
       d->curves->points[channel][point][1] = val.y();
    }
}

void ImageCurves::setCurvePointX(int channel, int point, int x)
{
    if ( d->curves &&
         channel>=0 && channel<5 && 
         point>=0 && point<=17 &&
         x>=-1 && x<=d->segmentMax) // x can be egal to -1 if the current point is disable !!!
    {
       d->curves->points[channel][point][0] = x;
    }
}

void ImageCurves::setCurvePointY(int channel, int point, int y)
{
    if ( d->curves &&
         channel>=0 && channel<5 && 
         point>=0 && point<=17 &&
         y>=0 && y<=d->segmentMax)
    {
       d->curves->points[channel][point][1] = y;
    }
}

void ImageCurves::setCurveType(int channel, CurveType type)
{
    if ( d->curves &&
         channel>=0 && channel<5 && 
         type>=CURVE_SMOOTH && type<=CURVE_FREE )
       d->curves->curve_type[channel] = type;
}

bool ImageCurves::loadCurvesFromGimpCurvesFile(KURL fileUrl)
{
    // TODO : support KURL !
    
    FILE *file;
    int   i, j;
    int   fields;
    char  buf[50];
    int   index[5][17];
    int   value[5][17];

    file = fopen(QFile::encodeName(fileUrl.path()), "r");
    if (!file)
       return false;
    
    if (! fgets (buf, sizeof (buf), file))
    {
       fclose(file);
       return false;
    }

    if (strcmp (buf, "# GIMP Curves File\n") != 0)
       return false;

    for (i = 0 ; i < 5 ; i++)
    {
       for (j = 0 ; j < 17 ; j++)
       {
          fields = fscanf (file, "%d %d ", &index[i][j], &value[i][j]);
          if (fields != 2)
          {
             DWarning() <<  "Invalid Gimp curves file!" << endl;
             fclose(file);
             return false;
          }
       }
    }

    curvesReset();
    
    for (i = 0 ; i < 5 ; i++)
    {
       d->curves->curve_type[i] = CURVE_SMOOTH;

       for (j = 0 ; j < 17 ; j++)
       {
          d->curves->points[i][j][0] = ((d->segmentMax == 65535) && (index[i][j] !=-1) ?
                                         index[i][j]*255 : index[i][j]);
          d->curves->points[i][j][1] = ((d->segmentMax == 65535) && (value[i][j] !=-1) ?
                                         value[i][j]*255 : value[i][j]);
       }
    }

    for (i = 0 ; i < 5 ; i++)
       curvesCalculateCurve(i);

    fclose(file);
    return true;
}

bool ImageCurves::saveCurvesToGimpCurvesFile(KURL fileUrl)
{
    // TODO : support KURL !
    
    FILE *file;
    int   i, j;
    int   index;

    file = fopen(QFile::encodeName(fileUrl.path()), "w");
    
    if (!file)
       return false;
    
    for (i = 0 ; i < 5 ; i++)
    {
       if (d->curves->curve_type[i] == CURVE_FREE)
       {
          //  Pick representative points from the curve and make them control points.
   
          for (j = 0 ; j <= 8 ; j++)
          {
             index = CLAMP(j * 32, 0, d->segmentMax);
             d->curves->points[i][j * 2][0] = index;
             d->curves->points[i][j * 2][1] = d->curves->curve[i][index];
          }
       }
    }

    fprintf (file, "# GIMP Curves File\n");

    for (i = 0 ; i < 5 ; i++)
    {
       for (j = 0 ; j < 17 ; j++)
       {
          fprintf (file, "%d %d ",
                   ((d->segmentMax == 65535) && (d->curves->points[i][j][0]!=-1) ?
                     d->curves->points[i][j][0]/255 : d->curves->points[i][j][0]),
                   ((d->segmentMax == 65535) && (d->curves->points[i][j][1]!=-1) ?
                     d->curves->points[i][j][1]/255 : d->curves->points[i][j][1]));

          fprintf (file, "\n");
       }
    }
    
    fflush(file);
    fclose(file);

    return true;
}

}  // NameSpace Digikam
