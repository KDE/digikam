/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-11
 * Description : light table thumbs bar
 *
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "thumbbar.h"
#include "imageinfo.h"
#include "digikam_export.h"

class QDragMoveEvent;
class QDropEvent;
class QMouseEvent;
class QPaintEvent;

class KURL;

namespace Digikam
{

class LightTableBarItem;
class LightTableBarItemPriv;
class LightTableBarPriv;

class DIGIKAM_EXPORT LightTableBar : public ThumbBarView
{
    Q_OBJECT

public:

    LightTableBar(QWidget* parent, int orientation=Vertical, bool exifRotate=true);
    ~LightTableBar();

    ImageInfo*    currentItemImageInfo() const;
    ImageInfoList itemsImageInfoList();

    void setSelectedItem(LightTableBarItem* ltItem);

    LightTableBarItem* findItemByInfo(const ImageInfo* info) const;
    LightTableBarItem* findItemByPos(const QPoint& pos) const;

    /** Read tool tip settings from Album Settings instance */
    void readToolTipSettings();

    void setOnLeftPanel(const ImageInfo* info);
    void setOnRightPanel(const ImageInfo* info);

    void removeItem(const ImageInfo* info);

    void setNavigateByPair(bool b);

signals:

    void signalLightTableBarItemSelected(ImageInfo*);
    void signalSetItemOnLeftPanel(ImageInfo*);
    void signalSetItemOnRightPanel(ImageInfo*);
    void signalEditItem(ImageInfo*);
    void signalRemoveItem(ImageInfo*);
    void signalClearAll();
    void signalDroppedItems(const ImageInfoList&);

private:

    void viewportPaintEvent(QPaintEvent*);    
    void contentsMouseReleaseEvent(QMouseEvent*);
    void startDrag();
    void contentsDragMoveEvent(QDragMoveEvent*);
    void contentsDropEvent(QDropEvent*);

private slots:

    void slotImageRatingChanged(Q_LLONG);
    void slotItemSelected(ThumbBarItem*);

    void slotAssignRatingNoStar();
    void slotAssignRatingOneStar();
    void slotAssignRatingTwoStar();
    void slotAssignRatingThreeStar();
    void slotAssignRatingFourStar();
    void slotAssignRatingFiveStar();
    void slotAssignRating(int);

    void slotThemeChanged();

private:

    LightTableBarPriv *d;

    friend class LightTableBarItem;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT LightTableBarItem : public ThumbBarItem
{
public:

    LightTableBarItem(LightTableBar *view, ImageInfo *info);
    ~LightTableBarItem();

    ImageInfo* info() const;

    void setOnLeftPanel(bool on);
    void setOnRightPanel(bool on);
    bool isOnLeftPanel() const;
    bool isOnRightPanel() const;
    
private:

    LightTableBarItemPriv *d;

    friend class LightTableBar;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT LightTableBarToolTip : public ThumbBarToolTip
{
public:

    LightTableBarToolTip(ThumbBarView *parent);

private:

    QString tipContentExtraData(ThumbBarItem *item);
};

}  // NameSpace Digikam

#endif /* LIGHTTABLEBAR_H */
