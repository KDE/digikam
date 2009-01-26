/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-01
 * Description : a widget to draw a control panel image tool.
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

#ifndef IMAGEPANELWIDGET_H
#define IMAGEPANELWIDGET_H

// Qt includes.

#include <QtGui/QPolygon>
#include <QtGui/QImage>
#include <QtCore/QRect>
#include <QtCore/QString>
#include <QtGui/QResizeEvent>
#include <QtGui/QWidget>

// Local includes.

#include "dimg.h"
#include "digikam_export.h"

class QProgressBar;

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

    void   setPanIconHighLightPoints(const QPolygon& pt);

    void   writeSettings();

    ImageRegionWidget *previewWidget() const;

Q_SIGNALS:

    void signalOriginalClipFocusChanged();
    void signalResized();

public Q_SLOTS:

    // Set the top/Left corner clip position.
    void slotSetImageRegionPosition(const QRect& rect, bool targetDone);

    // Slot used when the original image clip focus is changed by the user.
    void slotOriginalImageRegionChanged(bool target);

protected:

    void resizeEvent(QResizeEvent *e);

private Q_SLOTS:

    void slotPanIconTakeFocus();
    void slotInitGui();
    void slotZoomSliderChanged(int);

private:

    void updateSelectionInfo(const QRect& rect);
    void readSettings();

private:

    ImagePanelWidgetPriv* const d;
};

}  // namespace Digikam

#endif /* IMAGEPANELWIDGET_H */
