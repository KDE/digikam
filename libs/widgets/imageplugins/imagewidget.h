/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-01
 * Description : a widget to display an image preview with some 
 *               modes to compare effect results.
 * 
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

#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

// Qt includes.

#include <qwidget.h>
#include <qpoint.h>
#include <qcolor.h>
#include <qstring.h>

// Local includes

#include "dcolor.h"
#include "imageguidewidget.h"
#include "digikam_export.h"

namespace Digikam
{

class ImageIface;
class ImageWidgetPriv;

class DIGIKAM_EXPORT ImageWidget : public QWidget
{
Q_OBJECT

public:

    ImageWidget(QWidget *parent=0, const QString& previewWhatsThis=QString::null,
                bool prevModeOptions=true, int guideMode=ImageGuideWidget::PickColorMode, 
                bool guideVisible=true);
    ~ImageWidget();

    ImageIface* imageIface();

    QPoint getSpotPosition(void);
    DColor getSpotColor(int getColorFrom);
    void   setSpotVisible(bool spotVisible, bool blink=false);    
    int    getRenderingPreviewMode();
    void   resetSpotPosition();
    void   updatePreview();

    void   setRenderingPreviewMode(int mode);

public slots:
        
    void slotChangeGuideColor(const QColor &color);
    void slotChangeGuideSize(int size);    

signals:

    void spotPositionChangedFromOriginal( const Digikam::DColor &color, const QPoint &position );
    void spotPositionChangedFromTarget( const Digikam::DColor &color, const QPoint &position );    
    void signalResized(void);
    
private slots:
    
    void slotUpdateSpotInfo(const Digikam::DColor &col, const QPoint &point);

private:

    ImageWidgetPriv* d;
};

}  // namespace Digikam

#endif /* IMAGEWIDGET_H */
