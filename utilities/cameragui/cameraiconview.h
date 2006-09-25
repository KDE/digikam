/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date   : 2004-09-18
 * Description : camera icon view
 * 
 * Copyright 2004-2005 by Renchi Raju
 * Copyright 2006 by Gilles Caulier
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

#ifndef CAMERAICONVIEW_H
#define CAMERAICONVIEW_H

// Qt includes.

#include <qdict.h>
#include <qrect.h>

// KDE includes.

#include <kurl.h>

// Local includes.

#include "iconview.h"
#include "renamecustomizer.h"

class QPixmap;

namespace Digikam
{

class ThumbnailSize;
class GPItemInfo;
class RenameCustomizer;
class CameraUI;
class CameraIconViewItem;
class CameraIconViewPriv;

class CameraIconView : public IconView
{
    Q_OBJECT
    
public:

    CameraIconView(CameraUI* ui, QWidget* parent);
    ~CameraIconView();

    void setRenameCustomizer(RenameCustomizer* renamer);
    
    void addItem(const GPItemInfo& itemInfo);
    void removeItem(const QString& folder, const QString& file);
    void setThumbnail(const QString& folder, const QString& filename, const QImage& image);

    void ensureItemVisible(CameraIconViewItem *item);
    void ensureItemVisible(const GPItemInfo& itemInfo);
    void ensureItemVisible(const QString& folder, const QString& file);

    void setThumbnailSize(const ThumbnailSize& thumbSize);
    ThumbnailSize thumbnailSize() const;
        
    CameraIconViewItem* findItem(const QString& folder, const QString& file);

    int countItemsByFolder(QString folder);
    int itemsDownloaded();

    QPixmap* itemBaseRegPixmap() const;
    QPixmap* itemBaseSelPixmap() const;

    virtual QRect itemRect() const;

    QString defaultDownloadName(CameraIconViewItem *item);
    
signals:

    void signalSelected(CameraIconViewItem*, bool);
    void signalFileView(CameraIconViewItem*);

    void signalUpload(const KURL::List&);
    void signalDownload();
    void signalDelete();
    void signalToggleLock();
    void signalNewSelection(bool);
    
public slots:

    void slotDownloadNameChanged();
    void slotSelectionChanged();
    void slotSelectAll();
    void slotSelectNone();
    void slotSelectInvert();
    void slotSelectNew();

private slots:

    void slotRightButtonClicked(const QPoint& pos);
    void slotContextMenu(IconItem* item, const QPoint& pos);
    void slotDoubleClicked(IconItem* item);
    void slotThemeChanged();
    void slotUpdateDownloadNames(bool hasSelection);

protected:

    void startDrag();
    void contentsDropEvent(QDropEvent *event);
    void updateItemRectsPixmap();
    
private:

    QString getTemplatedName(const GPItemInfo* itemInfo, int position);
    QString getCasedName(const RenameCustomizer::Case ccase, const GPItemInfo* itemInfo);
    void    uploadItemPopupMenu(const KURL::List& srcURLs);

private:

    CameraIconViewPriv* d;
};

}  // namespace Digikam

#endif /* CAMERAICONVIEW_H */
