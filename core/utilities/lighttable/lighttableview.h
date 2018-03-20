/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-05
 * Description : a widget to display 2 preview image on
 *               lightable to compare pictures.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

namespace Digikam
{

class PreviewSettings;

class LightTableView : public QFrame
{
    Q_OBJECT

public:

    explicit LightTableView(QWidget* const parent = 0);
    ~LightTableView();

    void   setSyncPreview(bool sync);
    void   setNavigateByPair(bool b);

    void   setLeftImageInfo(const ImageInfo& info = ImageInfo());
    void   setRightImageInfo(const ImageInfo& info = ImageInfo());

    ImageInfo leftImageInfo() const;
    ImageInfo rightImageInfo() const;

    void setPreviewSettings(const PreviewSettings& settings);

    void   checkForSelection(const ImageInfo& info);
    void   toggleFullScreen(bool set);

    double leftZoomMax()  const;
    double leftZoomMin()  const;

    double rightZoomMax() const;
    double rightZoomMin() const;

    bool   leftMaxZoom()  const;
    bool   leftMinZoom()  const;

    bool   rightMaxZoom() const;
    bool   rightMinZoom() const;

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

    void signalLeftSlideShowCurrent();
    void signalRightSlideShowCurrent();

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
    void slotLeftZoomFactorChanged(double);
    void slotRightZoomFactorChanged(double);
    void slotLeftPreviewLoaded(bool);
    void slotRightPreviewLoaded(bool);
    void slotDeleteLeftItem();
    void slotDeleteRightItem();

private :

    void checkForSyncPreview();

    /// To not sync right panel during left loading
    bool leftPreviewLoading()  const;

    /// To not sync left panel during right loading.
    bool rightPreviewLoading() const;

private :

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* LIGHTTABLEVIEW_H */
