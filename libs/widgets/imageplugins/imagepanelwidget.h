/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-01
 * Description : a widget to draw a control panel image tool.
 *
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEPANELWIDGET_H
#define IMAGEPANELWIDGET_H

// Qt includes.

#include <qwidget.h>
#include <qimage.h>
#include <qrect.h>
#include <qstring.h>

// Local includes.

#include "dimg.h"
#include "digikam_export.h"

class KProgress;

namespace Digikam
{

class ImagePanelWidgetPriv;
class ImageRegionWidget;
class ImagePanIconWidget;

class DIGIKAM_EXPORT ImagePanelWidget : public QWidget
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

    ImagePanelWidget(uint w, uint h, const QString& settingsSection, ImagePanIconWidget *pan,
                     QWidget *parent=0, int separateViewMode=SeparateViewAll);
    ~ImagePanelWidget();

    QRect  getOriginalImageRegion();
    QRect  getOriginalImageRegionToRender();
    DImg   getOriginalRegionImage();
    void   setPreviewImage(DImg img);
    void   setCenterImageRegionPosition();

    void   setEnable(bool b);

    void   setPanIconHighLightPoints(const QPointArray& pt);

    void   writeSettings();

    ImageRegionWidget *previewWidget() const;

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

private:

    void updateSelectionInfo(const QRect& rect);
    void readSettings();

private:

    ImagePanelWidgetPriv* d;
};

}  // NameSpace Digikam

#endif /* IMAGEPANNELWIDGET_H */
