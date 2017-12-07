/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-11
 * Description : light table thumbs bar
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

#ifndef LIGHTTABLETHUMBBAR_H
#define LIGHTTABLETHUMBBAR_H

// Local includes

#include "imagethumbnailbar.h"
#include "imageinfo.h"
#include "imagelistmodel.h"

namespace Digikam
{

class LightTableThumbBar : public ImageThumbnailBar
{
    Q_OBJECT

public:

    explicit LightTableThumbBar(QWidget* const parent);
    ~LightTableThumbBar();

    void clear();

    void setItems(const ImageInfoList& list);
    void setOnLeftPanel(const ImageInfo& info);
    void setOnRightPanel(const ImageInfo& info);
    void setNavigateByPair(bool b);

    void removeItemByInfo(const ImageInfo& info);
    void toggleTag(int tagID);

    bool isOnLeftPanel(const ImageInfo& info)             const;
    bool isOnRightPanel(const ImageInfo& info)            const;
    int  countItems()                                     const;
    QModelIndex findItemByInfo(const ImageInfo& info)     const;
    ImageInfo   findItemByIndex(const QModelIndex& index) const;

Q_SIGNALS:

    void signalSetItemOnLeftPanel(const ImageInfo&);
    void signalSetItemOnRightPanel(const ImageInfo&);
    void signalEditItem(const ImageInfo&);
    void signalRemoveItem(const ImageInfo&);
    void signalDroppedItems(const QList<ImageInfo>&);
    void signalClearAll();
    void signalContentChanged();

public Q_SLOTS:

    void slotAssignPickLabel(int);
    void slotAssignColorLabel(int);
    void slotAssignRating(int);
    void slotRatingChanged(const QUrl&, int);
    void slotColorLabelChanged(const QUrl&, int);
    void slotPickLabelChanged(const QUrl&, int);
    void slotToggleTag(const QUrl&, int);
    void slotDockLocationChanged(Qt::DockWidgetArea area);

private:

    void paintEvent(QPaintEvent*);
    void showContextMenuOnInfo(QContextMenuEvent* e, const ImageInfo& info);
    void assignPickLabel(const ImageInfo& info, int pickId);
    void assignColorLabel(const ImageInfo& info, int colorId);
    void assignRating(const ImageInfo& info, int rating);
    void toggleTag(const ImageInfo& info, int tagID);

private Q_SLOTS:

    void slotSetupChanged();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* LIGHTTABLETHUMBBAR_H */
