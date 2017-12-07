/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-05-07
 * Description : QStackedWidget to handle different types of views
 *               (icon view, items preview, media view)
 *
 * Copyright (C) 2012      by Islam Wazery <wazery at ubuntu dot com>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "digikam_config.h"
#include "importthumbnailbar.h"
#include "importpreviewview.h"
#include "thumbbardock.h"
#include "camiteminfo.h"
#include "importiconview.h"
#include "digikam_export.h"

#ifdef HAVE_MEDIAPLAYER
#include "mediaplayerview.h"
#endif //HAVE_MEDIAPLAYER

#ifdef HAVE_MARBLE
#include "mapwidgetview.h"
#endif // HAVE_MARBLE

namespace Digikam
{

class DIGIKAM_EXPORT ImportStackedView : public QStackedWidget
{
    Q_OBJECT

public:

    enum StackedViewMode
    {
        PreviewCameraMode = 0, // previewing the set of items on the camera
        PreviewImageMode,
#ifdef HAVE_MARBLE
        MapWidgetMode,
        MediaPlayerMode
#else
        MediaPlayerMode,
        MapWidgetMode
#endif // HAVE_MARBLE
    };

public:

    explicit ImportStackedView(QWidget*const parent = 0);
    ~ImportStackedView();

    void setDockArea(QMainWindow*);

    ThumbBarDock*       thumbBarDock()      const;
    ImportThumbnailBar* thumbBar()          const;
    ImportIconView*     importIconView()    const;
    ImportPreviewView*  importPreviewView() const;

#ifdef HAVE_MARBLE
    MapWidgetView*      mapWidgetView()     const;
#endif // HAVE_MARBLE

#ifdef HAVE_MEDIAPLAYER
    MediaPlayerView*    mediaPlayerView()   const;
#endif //HAVE_MEDIAPLAYER

    bool isInSingleFileMode()   const;
    bool isInMultipleFileMode() const;
    //FIXME: bool isInAbstractMode() const;

    void setPreviewItem(const CamItemInfo& info = CamItemInfo(),
                        const CamItemInfo& previous = CamItemInfo(),
                        const CamItemInfo& next = CamItemInfo());

    StackedViewMode  viewMode() const;
    void setViewMode(const StackedViewMode mode);
    void previewLoaded();

    void   increaseZoom();
    void   decreaseZoom();
    void   fitToWindow();
    void   toggleFitToWindowOr100();
    void   zoomTo100Percents();
    void   setZoomFactor(double z);
    void   setZoomFactorSnapped(double z);

    bool   maxZoom()    const;
    bool   minZoom()    const;
    double zoomFactor() const;
    double zoomMin()    const;
    double zoomMax()    const;

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
