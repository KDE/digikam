/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-21-12
 * Description : digiKam light table preview item.
 * 
 * Copyright (C) 2006-2008 Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef LIGHTTABLEPREVIEW_H
#define LIGHTTABLEPREVIEW_H

// Qt includes.

#include <qstring.h>
#include <qimage.h>
#include <qsize.h>

// Local includes.

#include "imageinfo.h"
#include "previewwidget.h"
#include "digikam_export.h"

class QPixmap;

namespace Digikam
{

class DImg;
class LoadingDescription;
class LightTablePreviewPriv;

class DIGIKAM_EXPORT LightTablePreview : public PreviewWidget
{

Q_OBJECT

public:

    LightTablePreview(QWidget *parent=0);
    ~LightTablePreview();

    void setLoadFullImageSize(bool b);

    void setImage(const DImg& image);
    DImg& getImage() const;

    QSize getImageSize();

    void setImageInfo(ImageInfo* info=0, ImageInfo *previous=0, ImageInfo *next=0);
    ImageInfo* getImageInfo() const;

    void reload();
    void setImagePath(const QString& path=QString());
    void setPreviousNextPaths(const QString& previous, const QString &next);

    void setSelected(bool sel);
    bool isSelected();

    void setDragAndDropEnabled(bool b); 
    void setDragAndDropMessage();

signals:

    void signalDroppedItems(const ImageInfoList&);
    void signalDeleteItem(ImageInfo*);
    void signalEditItem(ImageInfo*);
    void signalPreviewLoaded(bool success);
    void signalSlideShow();

protected:

    void resizeEvent(QResizeEvent* e);
    void drawFrame(QPainter *p);

private slots:

    void slotGotImagePreview(const LoadingDescription &loadingDescription, const DImg &image);
    void slotNextPreload();
    void slotContextMenu();
    void slotAssignTag(int tagID);
    void slotRemoveTag(int tagID);
    void slotAssignRating(int rating);
    void slotThemeChanged();
    void slotCornerButtonPressed();
    void slotPanIconSelectionMoved(const QRect&, bool);
    void slotPanIconHiden();

private:
    
    int  previewWidth();
    int  previewHeight();
    bool previewIsNull();
    void resetPreview();
    void zoomFactorChanged(double zoom);
    void updateZoomAndSize(bool alwaysFitToWindow);
    inline void paintPreview(QPixmap *pix, int sx, int sy, int sw, int sh);

    void contentsDragMoveEvent(QDragMoveEvent*);
    void contentsDropEvent(QDropEvent*);

private:

    LightTablePreviewPriv* d;
};

}  // NameSpace Digikam

#endif /* LIGHTTABLEPREVIEW_H */
