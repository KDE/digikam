/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-05
 * Description : a widget to display 2 preview image on
 *               lightable to compare pictures.
 *
 * Copyright (C) 2007-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef LIGHTTABLEVIEW_H
#define LIGHTTABLEVIEW_H

// Qt includes

#include <QFrame>
#include <QString>

// Local includes

#include "imageinfo.h"
#include "digikam_export.h"

namespace Digikam
{

class LightTableView : public QFrame
{
    Q_OBJECT

public:

    LightTableView(QWidget* parent=0);
    ~LightTableView();

    void   setSyncPreview(bool sync);
    void   setNavigateByPair(bool b);

    void   setLeftImageInfo(const ImageInfo& info = ImageInfo());
    void   setRightImageInfo(const ImageInfo& info = ImageInfo());

    ImageInfo leftImageInfo() const;
    ImageInfo rightImageInfo() const;

    void   setLoadFullImageSize(bool b);

    void   checkForSelection(const ImageInfo& info);

    double leftZoomMax();
    double leftZoomMin();

    double rightZoomMax();
    double rightZoomMin();

    bool   leftMaxZoom();
    bool   leftMinZoom();

    bool   rightMaxZoom();
    bool   rightMinZoom();

    void   leftReload();
    void   rightReload();

Q_SIGNALS:

    void signalLeftPreviewLoaded(bool);
    void signalRightPreviewLoaded(bool);

    void signalLeftZoomFactorChanged(double);
    void signalRightZoomFactorChanged(double);

    void signalLeftDroppedItems(const ImageInfoList&);
    void signalRightDroppedItems(const ImageInfoList&);

    void signalLeftPanelLeftButtonClicked();
    void signalRightPanelLeftButtonClicked();

    void signalLeftPopupTagsView();
    void signalRightPopupTagsView();

    void signalSlideShow();
    void signalDeleteItem(const ImageInfo&);
    void signalEditItem(const ImageInfo&);
    void signalToggleOnSyncPreview(bool);

public Q_SLOTS:

    void slotDecreaseLeftZoom();
    void slotIncreaseLeftZoom();
    void slotLeftZoomSliderChanged(int);
    void setLeftZoomFactor(double z);
    void slotLeftFitToWindow();
    void slotLeftZoomTo100();

    void slotDecreaseRightZoom();
    void slotIncreaseRightZoom();
    void slotRightZoomSliderChanged(int);
    void setRightZoomFactor(double z);
    void slotRightFitToWindow();
    void slotRightZoomTo100();

private Q_SLOTS:

    void slotLeftContentsMoved(int, int);
    void slotRightContentsMoved(int, int);
    void slotLeftPreviewLoaded(bool);
    void slotRightPreviewLoaded(bool);
    void slotDeleteLeftItem();
    void slotDeleteRightItem();

private :

    void checkForSyncPreview();

private :

    class LightTableViewPriv;
    LightTableViewPriv* const d;
};

}  // namespace Digikam

#endif /* LIGHTTABLEVIEW_H */
