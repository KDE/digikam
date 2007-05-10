/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-11
 * Description : light table thumbs bar
 *
 * Copyright (C) 2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qpixmap.h>

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
class LightTableBarToolTip;

class DIGIKAM_EXPORT LightTableBar : public ThumbBarView
{
    Q_OBJECT

public:

    LightTableBar(QWidget* parent, int orientation=Vertical, bool exifRotate=false);
    ~LightTableBar();

    ImageInfo*    currentItemImageInfo() const;
    ImageInfoList itemsImageInfoList();

    void setSelected(LightTableBarItem* ltItem);

    LightTableBarItem* findItemByInfo(const ImageInfo* info) const;
    LightTableBarItem* findItemByPos(const QPoint& pos) const;

    /** Read tool tip settings from Album Settings instance */
    void readToolTipSettings();

    void setOnLeftPanel(const ImageInfo* info);
    void setOnRightPanel(const ImageInfo* info);

signals:

    void signalLightTableBarItemSelected(ImageInfo*);
    void signalSetItemOnLeftPanel(ImageInfo*);
    void signalSetItemOnRightPanel(ImageInfo*);
    void signalRemoveItem(const KURL&);

private:

    void viewportPaintEvent(QPaintEvent*);    
    void contentsMouseReleaseEvent(QMouseEvent*);
    void startDrag();
    void contentsDragMoveEvent(QDragMoveEvent*);
    void contentsDropEvent(QDropEvent*);

private slots:

    void slotImageRatingChanged(Q_LLONG);
    void slotItemSelected(ThumbBarItem*);
    void slotAssignRating(int);

private:

    QPixmap               m_ratingPixmap;

    LightTableBarToolTip *m_toolTip;

private:

    friend class LightTableBarItem;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT LightTableBarItem : public ThumbBarItem
{
public:

    LightTableBarItem(LightTableBar* view, ImageInfo* item);
    ~LightTableBarItem();

    ImageInfo* info();

    void setOnLeftPanel(bool on);
    void setOnRightPanel(bool on);
    bool getOnLeftPanel() const;
    bool getOnRightPanel() const;
    
private:

    bool       m_onLeftPanel;
    bool       m_onRightPanel;
 
    ImageInfo *m_info;

    friend class LightTableBar;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT LightTableBarToolTip : public ThumbBarToolTip
{
public:

    LightTableBarToolTip(ThumbBarView *parent);

private:

    QString tipContentExtraData(ThumbBarItem* item);
};

}  // NameSpace Digikam

#endif /* LIGHTTABLEBAR_H */
