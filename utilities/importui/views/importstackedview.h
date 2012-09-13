/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-05-07
 * Description : QStackedWidget to handle different types of views
 *               (icon view, items preview, media view)
 *
 * Copyright (C) 2012 by Islam Wazery <wazery at ubuntu dot com>
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

#ifndef IMPORTSTACKEDVIEW_H
#define IMPORTSTACKEDVIEW_H

// Qt inclueds

#include <QStackedWidget>

// Local includes

#include "importthumbnailbar.h"
#include "importpreviewview.h"
#include "thumbbardock.h"
#include "camiteminfo.h"
#include "importiconview.h"
#include "mediaplayerview.h"
#include "mapwidgetview.h"

namespace Digikam
{

class ImportStackedView : public QStackedWidget
{
    Q_OBJECT

public:

    enum StackedViewMode
    {
        PreviewCameraMode = 0, // previewing the set of items on the camera
        PreviewImageMode,
        MediaPlayerMode,
        MapWidgetMode
    };

public:

    ImportStackedView(CameraController* const controller, QWidget* const parent = 0);
    ~ImportStackedView();

    void setDockArea(QMainWindow*);

    ThumbBarDock*       thumbBarDock()      const;
    ImportThumbnailBar* thumbBar()          const;
    ImportIconView*     importIconView()    const;
    ImportPreviewView*  importPreviewView() const;
    MapWidgetView*      mapWidgetView()     const;
    MediaPlayerView*    mediaPlayerView()   const;

    bool isInSingleFileMode() const;
    bool isInMultipleFileMode() const;
    //FIXME: bool isInAbstractMode() const;

    void setPreviewItem(const CamItemInfo& info = CamItemInfo(),
                        const CamItemInfo& previous = CamItemInfo(),
                        const CamItemInfo& next = CamItemInfo());

    int  previewMode();
    void setPreviewMode(const int mode);
    void previewLoaded();

    void   increaseZoom();
    void   decreaseZoom();
    void   fitToWindow();
    void   toggleFitToWindowOr100();
    void   zoomTo100Percents();
    bool   maxZoom();
    bool   minZoom();
    void   setZoomFactor(double z);
    void   setZoomFactorSnapped(double z);
    double zoomFactor();
    double zoomMin();
    double zoomMax();

Q_SIGNALS:

    void signalNextItem();
    void signalPrevItem();
    //FIXME: void signalEditItem();
    void signalViewModeChanged();
    void signalEscapePreview();
    //FIXME: void signalSlideShow();
    void signalZoomFactorChanged(double);

    //FIXME: void signalGotoAlbumAndItem(const CamItemInfo&);
    //FIXME: void signalGotoDateAndItem(const CamItemInfo&);
    //FIXME: void signalGotoTagAndItem(int);

public Q_SLOTS:

    void slotEscapePreview();

private Q_SLOTS:

    void slotPreviewLoaded(bool);
    void slotZoomFactorChanged(double);
    void slotThumbBarSelectionChanged();
    void slotIconViewSelectionChanged();

private:

    void readSettings();
    void syncSelection(ImportCategorizedView* const from, ImportCategorizedView* const to);

    /// Used to return the category for a specified camera item.
    QString identifyCategoryforMime(const QString& mime) const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // IMPORTSTACKEDVIEW_H
