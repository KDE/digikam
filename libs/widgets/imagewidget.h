/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2006-02-01
 * Description : 
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

namespace Digikam
{

class ImageIface;
class ImageWidgetPriv;

class ImageWidget : public QWidget
{
Q_OBJECT

public:

    ImageWidget(QWidget *parent=0, const QString& previewWhatsThis=QString::null);
    ~ImageWidget();

    ImageIface* imageIface();
    void updatePreview();

public slots:
        
    void slotChangeGuideColor(const QColor &color);
    void slotChangeGuideSize(int size);    

signals:

    void spotPositionChanged( const Digikam::DColor &color, bool release, const QPoint &position );
    void signalResized(void);
    
private:

    ImageWidgetPriv* d;
};

}  // namespace Digikam

#endif /* IMAGEWIDGET_H */
