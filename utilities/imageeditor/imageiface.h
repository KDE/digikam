/* ============================================================
 * File  : imageiface.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-02-14
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#ifndef IMAGEIFACE_H
#define IMAGEIFACE_H

#include <qglobal.h>

class QWidget;

namespace Digikam
{

class ImageIfacePriv;

class ImageIface
{
public:

    ImageIface(int w=0, int h=0);
    ~ImageIface();

    uint* getPreviewData();
    uint* getOriginalData();
    uint* getSelectedData();

    void putPreviewData(uint* data);
    void putOriginalData(uint* data);
    void putSelectedData(uint* data);

    int  previewWidth();
    int  previewHeight();

    int  originalWidth();
    int  originalHeight();

    int  selectedWidth();
    int  selectedHeight();
    
    void setPreviewBCG(double brightness, double contrast, double gamma);
    void setOriginalBCG(double brightness, double contrast, double gamma);
    
    void paint(QWidget* widget, int x, int y, int w, int h);
    
private:

    ImageIfacePriv* d;
};

}

#endif /* IMAGEIFACE_H */
