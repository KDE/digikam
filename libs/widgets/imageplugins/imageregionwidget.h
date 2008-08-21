/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-17
 * Description : a widget to draw an image clip region.
 * 
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEREGIONWIDGET_H
#define IMAGEREGIONWIDGET_H

// Qt includes.

#include <qrect.h>

// Local includes.

#include "previewwidget.h"
#include "dimg.h"
#include "digikam_export.h"

namespace Digikam
{

class ImageRegionWidgetPriv;

class DIGIKAM_EXPORT ImageRegionWidget : public PreviewWidget
{
Q_OBJECT

public:
    
    enum SeparateViewMode 
    {
        SeparateViewHorizontal=0,
        SeparateViewVertical,
        SeparateViewNone,
        SeparateViewDuplicateVert,
        SeparateViewDuplicateHorz
    };
    
public:
    
    ImageRegionWidget(int wp, int hp, QWidget *parent=0, bool scrollBar=true);
    ~ImageRegionWidget();
    
    void   setContentsPosition(int x, int y, bool targetDone);    
    void   setCenterContentsPosition();

    /** To get image region including original or/and target area depending of separate view mode.
        The region is given using not scaled image unit.*/
    QRect  getImageRegion();
    
    /** To get target image region area to render */
    QRect  getImageRegionToRender();
    
    /** To get target image region image to use for render operations */
    DImg   getImageRegionImage();

    void   updatePreviewImage(DImg *img);

    void   backupPixmapRegion();
    void   restorePixmapRegion();
    
    void   setHighLightPoints(const QPointArray& pointsList);
    void   drawSeparateView();
             
public slots:

    void slotSeparateViewToggled(int mode);

private slots:
    
    void slotZoomFactorChanged();

private:

    void  updatePixmap(DImg& img);
    QRect getLocalTargetImageRegion();
    QRect getLocalImageRegionToRender();
    void  viewportPaintExtraData();
    int   previewWidth();
    int   previewHeight();
    bool  previewIsNull();
    void  resetPreview();
    void  setContentsSize();
    void  resizeEvent(QResizeEvent *);
    void  contentsWheelEvent(QWheelEvent *);

    inline void paintPreview(QPixmap *pix, int sx, int sy, int sw, int sh);

private:
    
    ImageRegionWidgetPriv* d;
};

}  // NameSpace Digikam

#endif /* IMAGEREGIONWIDGET_H */
