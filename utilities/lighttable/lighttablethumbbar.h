/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-11
 * Description : light table thumbs bar
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

#ifndef LIGHTTABLETHUMBBAR_H
#define LIGHTTABLETHUMBBAR_H

// Local includes

#include "imagethumbnailbar.h"
#include "imageinfo.h"
#include "imagelistmodel.h"

namespace Digikam
{

class CollectionImageChangeset;

class LightTableThumbBar : public ImageThumbnailBar
{
    Q_OBJECT

public:

    using Digikam::DCategorizedView::startDrag;

    explicit LightTableThumbBar(QWidget* parent);
    ~LightTableThumbBar();

    int  countItems() const;
    void clear();
    void setItems(const ImageInfoList& list);
    void setSelectedItem(const ImageInfo& info);

    void setOnLeftPanel(const ImageInfo& info);
    void setOnRightPanel(const ImageInfo& info);
    bool isOnLeftPanel(const ImageInfo& info) const;
    bool isOnRightPanel(const ImageInfo& info) const;

    void removeItemByInfo(const ImageInfo& info);
    void removeItemById(qlonglong id);

    void setNavigateByPair(bool b);

    void toggleTag(int tagID);

    QModelIndex findItemByInfo(const ImageInfo& info) const;
    ImageInfo   findItemByIndex(const QModelIndex& index) const;

Q_SIGNALS:

    void signalLightTableThumbBarItemSelected(const ImageInfo&);
    void signalSetItemOnLeftPanel(const ImageInfo&);
    void signalSetItemOnRightPanel(const ImageInfo&);
    void signalEditItem(const ImageInfo&);
    void signalRemoveItem(const ImageInfo&);
    void signalClearAll();
    void signalDroppedItems(const ImageInfoList&);

public Q_SLOTS:

    void slotAssignPickLabel(int);
    void slotAssignColorLabel(int);
    void slotAssignRating(int);
    void slotRatingChanged(const KUrl&, int);

private:

    void drawEmptyMessage(QPixmap& pixmap);
    void contentsMouseReleaseEvent(QMouseEvent*);
    void startDrag();
    void contentsDragEnterEvent(QDragEnterEvent*);
    void contentsDropEvent(QDropEvent*);

    void assignPickLabel(const ImageInfo& info, int pickId);
    void assignColorLabel(const ImageInfo& info, int colorId);
    void assignRating(const ImageInfo& info, int rating);

private Q_SLOTS:

    void slotCollectionImageChange(const CollectionImageChangeset&);

private:

    class LightTableThumbBarPriv;
    LightTableThumbBarPriv* const d;
};

}  // namespace Digikam

#endif /* LIGHTTABLETHUMBBAR_H */
