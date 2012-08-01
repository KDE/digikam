/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-26-07
 * Description : Main view for import tool
 *
 * Copyright (C) 2012 by Islam Wazery <wazery at ubuntu dot com>
 *
 * This program is free software you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef IMPORTVIEW_H
#define IMPORTVIEW_H

// KDE includes

#include <khbox.h>

// Local includes

#include "camiteminfo.h"
#include "sidebarwidget.h"
#include "cameraui.h"
#include "importmodel.h"

namespace Digikam
{

class CameraUI;

class ImportView : public KHBox
{
    Q_OBJECT

public:

    ImportView(CameraUI* const ui, QWidget* const parent);
    ~ImportView();

    void applySettings();
    void refreshView();
    void clearHistory();
    void getForwardHistory(QStringList& titles);
    void getBackwardHistory(QStringList& titles);
    void showSideBars();
    void hideSideBars();
    void setThumbSize(int size);
    void toggleShowBar(bool);
    bool isThumbBarVisible();

    KUrl::List allUrls() const;
    KUrl::List selectedUrls() const;
    bool hasCurrentItem() const;

    double zoomMin();
    double zoomMax();

    ThumbnailSize thumbnailSize();

    QList<SidebarWidget*> leftSidebarWidgets();

Q_SIGNALS:

    void signalImageSelected(const CamItemInfoList& selectedImage, bool hasPrevious, bool hasNext,
                             const CamItemInfoList& allImages);
    void signalNoCurrentItem();
    void signalSelectionChanged(int numberOfSelectedItems);
    void signalThumbSizeChanged(int);
    void signalZoomChanged(double);
    void signalSwitchedToPreview();
    void signalSwitchedToIconView();
    void signalSwitchedToMapView();

public Q_SLOTS:

    void setZoomFactor(double zoom);

    // View Action slots
    void slotZoomIn();
    void slotZoomOut();
    void slotZoomTo100Percents();
    void slotFitToWindow();

    void slotImagePreview();
    //TODO: void slotMapWidgetView();
    void slotIconView();

    void slotSelectAll();
    void slotSelectNone();
    void slotSelectInvert();

    void slotImageRename();
    //void slotImageDelete();
    //void slotImageDeletePermanently();
    //void slotImageDeletePermanentlyDirectly();
    //void slotImageTrashDirectly();

    void slotSortImages(int order);
    void slotSortImagesOrder(int order);
    void slotGroupImages(int mode);

    void slotLeftSideBarActivate(QWidget* widget);
    void slotLeftSideBarActivate(SidebarWidget* widget);

private Q_SLOTS:

    void slotImageSelected();
    void slotTogglePreviewMode(const CamItemInfo& info);
    void slotDispatchImageSelected();

    void slotLeftSidebarChangedTab(QWidget* w);

    void slotFirstItem();
    void slotPrevItem();
    void slotNextItem();
    void slotLastItem();
    void slotSelectItemByUrl(const KUrl&);

    void slotViewModeChanged();
    void slotEscapePreview();

    void slotThumbSizeEffect();
    void slotZoomFactorChanged(double);

    void slotSidebarTabTitleStyleChanged();

    void slotImageChangeFailed(const QString& message, const QStringList& fileNames);

private:

    void toggleZoomActions();
    void setupConnections();
    void loadViewState();
    void saveViewState();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // IMPORTVIEW_H
