/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date  : 2004-02-14
 * Description : image data interface for image plugins
 *
 * Copyright 2004-2005 by Renchi Raju, Gilles Caulier
 * Copyright 2006 by Gilles Caulier
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

#ifndef IMAGEIFACE_H
#define IMAGEIFACE_H

// Qt includes.

#include <qglobal.h>

// KDE includes.

#include <klocale.h>

// Local includes.

#include "dimg.h"
#include "dcolor.h"
#include "digikam_export.h"

#define MAX3(a, b, c) (QMAX(QMAX(a,b),b))
#define MIN3(a, b, c) (QMIN(QMIN(a,b),b))
#define ROUND(x) ((int) ((x) + 0.5))

class QPaintDevice;
class QString;

namespace Digikam
{

class ImageIfacePriv;

class DIGIKAM_EXPORT ImageIface
{
public:

    ImageIface(int w=0, int h=0);
    ~ImageIface();

    uchar* getPreviewImage();
    uchar* getImageSelection();
    uchar* getOriginalImage();
    DImg*  getOriginalImg();

    void   putOriginalImage(const QString &caller, uchar* data, int w=-1, int h=-1);
    void   putImageSelection(const QString &caller, uchar* data);
    void   putPreviewImage(uchar* data);

    /** Color point information method.*/

    DColor getColorInfoFromOriginalImage(QPoint point);
    DColor getColorInfoFromPreviewImage(QPoint point);
    DColor getColorInfoFromTargetPreviewImage(QPoint point);
    
    /** Original image informations.*/
    int  originalWidth();
    int  originalHeight();
    bool originalSixteenBit();
    bool originalHasAlpha();
    QByteArray getEmbeddedICCFromOriginalImage();

    /** Standard methods to get preview informations.*/
    int  previewWidth();
    int  previewHeight();
    bool previewHasAlpha();
    bool previewSixteenBit();
    
    uchar* setPreviewImageSize(int w, int h);

    /** Standard methods to get image selection informations.*/
    int  selectedWidth();
    int  selectedHeight();

    /** Get selected (X, Y) position on the top/left corner.*/
    int  selectedXOrg();
    int  selectedYOrg();

    void setPreviewBCG(double brightness, double contrast, double gamma, bool overIndicator=false);
    void setOriginalBCG(double brightness, double contrast, double gamma);

    void convertOriginalColorDepth(int depth);

    void paint(QPaintDevice* device, int x, int y, int w, int h);

    // FIXME : remove these methods when all image plugins will be ported to DImg.
    uint* getPreviewData();
    uint* getOriginalData();
    uint* getSelectedData();

    void  putPreviewData(uint* data);
    void  putOriginalData(const QString &caller, uint* data, int w=-1, int h=-1);
    void  putSelectedData(uint* data);
    uint* setPreviewSize(int w, int h);

private:

    ImageIfacePriv* d;
};

}

#endif /* IMAGEIFACE_H */
