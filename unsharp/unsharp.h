/* ============================================================
 * File  : unsharp.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : Unsharp Mask threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier
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
  
#ifndef UNSHARPMASK_H
#define UNSHARPMASK_H

// Digikam includes.

#include <digikamheaders.h>

namespace DigikamUnsharpMaskImagesPlugin
{

class UnsharpMask : public Digikam::ThreadedFilter
{

public:
    
    UnsharpMask(QImage *orgImage, QObject *parent=0, double radius=5.0, 
                double amount=0.5, int threshold=0);
    
    ~UnsharpMask(){};
    
private:  // Unsharp Mask filter data.

    double m_radius;
    double m_amount;
    int    m_threshold;
    
private:  // Unsharp Mask filter methods.


    virtual void filterImage(void);
    
    void unsharpImage(uint* data, int w, int h, double radius, 
                      double amount, int threshold);
                 
    inline void blur_line (double *ctable, double *cmatrix, int cmatrix_length,
                           uchar *cur_col, uchar *dest_col, int y, long bytes);  
                           
    int gen_convolve_matrix (double radius, double **cmatrix_p);    
    
    double* gen_lookup_table (double *cmatrix, int cmatrix_length);
    
};    

}  // NameSpace DigikamUnsharpMaskImagesPlugin

#endif /* UNSHARPMASK_H */
