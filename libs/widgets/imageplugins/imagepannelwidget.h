/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-01
 * Description : a widget to draw a control pannel image tool.
 * 
 * Copyright (C) 2005-2008 Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEPANNELWIDGET_H
#define IMAGEPANNELWIDGET_H

// Qt includes.

#include <QPolygon>
#include <QImage>
#include <QRect>
#include <QString>
#include <QResizeEvent>

// KDE includes.

#include <khbox.h>

// Local includes.

#include "dimg.h"
#include "digikam_export.h"

class QProgressBar;

namespace Digikam
{

class ImagePannelWidgetPriv;

class DIGIKAM_EXPORT ImagePannelWidget : public KHBox
{
Q_OBJECT

public:

    enum SeparateViewOptions 
    {
        SeparateViewNormal=0,
        SeparateViewDuplicate,
        SeparateViewAll
    };
    
public:

    ImagePannelWidget(uint w, uint h, const QString& settingsSection, QWidget *parent=0,
                      int separateViewMode=SeparateViewAll);
    ~ImagePannelWidget();
    
    QRect  getOriginalImageRegion();
    QRect  getOriginalImageRegionToRender();
    DImg   getOriginalRegionImage();
    void   setPreviewImage(DImg img);
    void   setPreviewImageWaitCursor(bool enable);
    void   setCenterImageRegionPosition();
    
    void   setEnable(bool b);
    
    void   setProgress(int val);
    void   setProgressVisible(bool b);
    void   setProgressWhatsThis(const QString& desc);

    void   setUserAreaWidget(QWidget *w);
    
    void   setPanIconHighLightPoints(const QPolygon& pt);
    
    QProgressBar *progressBar();

signals:

    void signalOriginalClipFocusChanged();
    void signalResized();
           
public slots:

    // Set the top/Left conner clip position.
    void slotSetImageRegionPosition(const QRect& rect, bool targetDone);
    
    // Slot used when the original image clip focus is changed by the user.
    void slotOriginalImageRegionChanged(bool target);

protected:
    
    void resizeEvent(QResizeEvent *e);

private slots:

    void slotPanIconTakeFocus();    
    void slotInitGui();
    void slotZoomSliderChanged(int);
    void slotZoomFactorChanged(double);

private:
        
    void updateSelectionInfo(const QRect& rect);
    void readSettings();
    void writeSettings();

private:

    ImagePannelWidgetPriv *d;
};

}  // NameSpace Digikam

#endif /* IMAGEPANNELWIDGET_H */
