/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-06-13
 * Description : A widget stack to embedded album content view
 *               or the current image preview.
 *
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef STACKEDVIEW_H
#define STACKEDVIEW_H

// Qt includes

#include <QByteArray>
#include <QMainWindow>
#include <QStackedWidget>
#include <QString>

// KDE includes

#include <kurl.h>

// Local includes

#include "digikam_export.h"
#include "imageinfo.h"
#include "thumbbardock.h"
//#include "mapwidgetview.h"

namespace Digikam
{

class DigikamImageView;
class ImageCategorizedView;
class ImagePreviewView;
class MapWidgetView;
class MediaPlayerView;
class ImageThumbnailBar;

class StackedView : public QStackedWidget
{
    Q_OBJECT

public:

    enum StackedViewMode
    {
        PreviewAlbumMode=0,
        PreviewImageMode,
        WelcomePageMode,
        MediaPlayerMode,
        MapWidgetMode
    };

public:

    StackedView(QWidget* parent=0);
    ~StackedView();

    /* Attach the thumbnail dock widget to the specified QMainWindow. */
    void setDockArea(QMainWindow*);

    ThumbBarDock*     thumbBarDock()     const;
    ImageThumbnailBar*thumbBar()     const;
    DigikamImageView* imageIconView()    const;
    ImagePreviewView* imagePreviewView() const;
    MapWidgetView*    mapWidgetView()    const;
    MediaPlayerView*  mediaPlayerView()  const;

    /**
     * Single-file mode is image preview or media player,
     * multi-file is icon view or map,
     * abstract modes do not handle files (welcome page)
     */
    bool isInSingleFileMode() const;
    bool isInMultipleFileMode() const;
    bool isInAbstractMode() const;

    void setPreviewItem(const ImageInfo& info = ImageInfo(),
                        const ImageInfo& previous = ImageInfo(),
                        const ImageInfo& next = ImageInfo());
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
    void signalEditItem();
    void signalDeleteItem();
    void signalViewModeChanged();
    void signalBack2Album();
    void signalSlideShow();
    void signalZoomFactorChanged(double);
    void signalInsert2LightTable();
    void signalInsert2QueueMgr();
    void signalFindSimilar();
    void signalAddToExistingQueue(int);

    void signalGotoAlbumAndItem(const ImageInfo&);
    void signalGotoDateAndItem(const ImageInfo&);
    void signalGotoTagAndItem(int);

public Q_SLOTS:

    void slotEscapePreview();

private Q_SLOTS:

    void slotPreviewLoaded(bool);
    void slotZoomFactorChanged(double);
    void slotThumbBarSelectionChanged();
    void slotIconViewSelectionChanged();

private:

    void readSettings();
    void syncSelection(ImageCategorizedView* from, ImageCategorizedView* to);

private:

    class StackedViewPriv;
    StackedViewPriv* const d;
};

}  // namespace Digikam

#endif /* STACKEDVIEW_H */
