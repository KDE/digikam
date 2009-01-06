/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-11
 * Description : light table thumbs bar
 *
 * Copyright (C) 2007-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef LIGHTTABLEBAR_H
#define LIGHTTABLEBAR_H

// Local includes.

#include "imagepreviewbar.h"

namespace Digikam
{

class LightTableBarItem;
class LightTableBarItemPriv;
class LightTableBarPriv;

class LightTableBar : public ImagePreviewBar
{
    Q_OBJECT

public:

    LightTableBar(QWidget* parent, int orientation=Qt::Vertical, 
                  bool exifRotate=true);
    ~LightTableBar();

    void setOnLeftPanel(const ImageInfo &info);
    void setOnRightPanel(const ImageInfo &info);

    void removeItem(const ImageInfo &info);

    void setNavigateByPair(bool b);

signals:

    void signalLightTableBarItemSelected(const ImageInfo &);
    void signalSetItemOnLeftPanel(const ImageInfo &);
    void signalSetItemOnRightPanel(const ImageInfo &);
    void signalEditItem(const ImageInfo &);
    void signalRemoveItem(const ImageInfo &);
    void signalClearAll();
    void signalDroppedItems(const ImageInfoList&);

private:

    void viewportPaintEvent(QPaintEvent*);
    void contentsMouseReleaseEvent(QMouseEvent*);
    void startDrag();
    void contentsDragEnterEvent(QDragEnterEvent*);
    void contentsDropEvent(QDropEvent*);

private slots:

    void slotItemSelected(ThumbBarItem*);

    void slotAssignRatingNoStar();
    void slotAssignRatingOneStar();
    void slotAssignRatingTwoStar();
    void slotAssignRatingThreeStar();
    void slotAssignRatingFourStar();
    void slotAssignRatingFiveStar();
    void slotAssignRating(int);

private:

    LightTableBarPriv* const d;

    friend class LightTableBarItem;
};

// -------------------------------------------------------------------------

class LightTableBarItem : public ImagePreviewBarItem
{
public:

    LightTableBarItem(LightTableBar *view, const ImageInfo &info);
    ~LightTableBarItem();

    void setOnLeftPanel(bool on);
    void setOnRightPanel(bool on);
    bool isOnLeftPanel() const;
    bool isOnRightPanel() const;

private:

    LightTableBarItemPriv* const d;

    friend class LightTableBar;
};

}  // namespace Digikam

#endif /* LIGHTTABLEBAR_H */
