/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-05
 * Description : a widget to display 2 preview image on 
 *               lightable to compare pictures.
 * 
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qframe.h>
#include <qstring.h>

// Local includes.

#include "imageinfo.h"
#include "digikam_export.h"

namespace Digikam
{

class LightTableViewPriv;

class DIGIKAM_EXPORT LightTableView : public QFrame
{

Q_OBJECT

public:

    LightTableView(QWidget *parent=0);
    ~LightTableView();

    void   setSyncPreview(bool sync);
    void   setNavigateByPair(bool b);

    void   setLeftImageInfo(ImageInfo* info=0);
    void   setRightImageInfo(ImageInfo* info=0);

    ImageInfo* leftImageInfo() const;
    ImageInfo* rightImageInfo() const;

    void   setLoadFullImageSize(bool b);

    void   setLeftZoomFactor(double z);
    void   setRightZoomFactor(double z);

    void   checkForSelection(ImageInfo* info);

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

    void   fitToWindow();
    void   toggleFitToWindowOr100();

signals:

    void signalLeftPreviewLoaded(bool);
    void signalRightPreviewLoaded(bool);

    void signalLeftZoomFactorChanged(double);
    void signalRightZoomFactorChanged(double);

    void signalLeftDroppedItems(const ImageInfoList&);
    void signalRightDroppedItems(const ImageInfoList&);

    void signalLeftPanelLeftButtonClicked();
    void signalRightPanelLeftButtonClicked();

    void signalSlideShow();
    void signalDeleteItem(ImageInfo*);
    void signalEditItem(ImageInfo*);
    void signalToggleOnSyncPreview(bool);

public slots:

    void slotDecreaseZoom();
    void slotIncreaseZoom();
    void slotDecreaseLeftZoom();
    void slotIncreaseLeftZoom();
    void slotLeftZoomSliderChanged(int);

    void slotDecreaseRightZoom();
    void slotIncreaseRightZoom();
    void slotRightZoomSliderChanged(int);

private slots:

    void slotLeftContentsMoved(int, int);
    void slotRightContentsMoved(int, int);
    void slotLeftPreviewLoaded(bool);
    void slotRightPreviewLoaded(bool);

private :

    void checkForSyncPreview();

private :

    LightTableViewPriv* d;
};

}  // namespace Digikam

#endif /* LIGHTTABLEVIEW_H */
