/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-18-03
 * Description : image preview thumbs bar
 *
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEPREVIEWBAR_H
#define IMAGEPREVIEWBAR_H

// Local includes.

#include "thumbbar.h"
#include "imageinfo.h"
#include "digikam_export.h"

class QDragMoveEvent;
class QDropEvent;
class QMouseEvent;
class QPaintEvent;

namespace Digikam
{

class ImagePreviewBarItem;
class ImagePreviewBarPriv;

class ImagePreviewBar : public ThumbBarView
{
    Q_OBJECT

public:

    ImagePreviewBar(QWidget* parent, int orientation=Qt::Vertical, 
                    bool exifRotate=true);
    virtual ~ImagePreviewBar();


    ThumbBarItem* ratingItem() const;

    void setSelectedItem(ImagePreviewBarItem* ltItem);

    ImageInfo     currentItemImageInfo() const;
    ImageInfoList itemsImageInfoList();

    ImagePreviewBarItem* findItemByInfo(const ImageInfo &info) const;
    ImagePreviewBarItem* findItemByPos(const QPoint& pos) const;

    /** Read tool tip settings from Album Settings instance */
    void readToolTipSettings();

    void applySettings();

    void ensureItemVisible(ThumbBarItem* item);
    void clear(bool updateView=true);
    void takeItem(ThumbBarItem* item);
    void removeItem(ThumbBarItem* item);

protected:

    QRect clickToRateRect(ImagePreviewBarItem* item);
    QPixmap ratingPixmap() const;
    void startDrag();

    virtual void rearrangeItems();
    virtual void contentsMouseMoveEvent(QMouseEvent*);
    virtual void viewportPaintEvent(QPaintEvent*);
    virtual void leaveEvent(QEvent*);
    virtual void focusOutEvent(QFocusEvent*);
    virtual void contentsWheelEvent(QWheelEvent*);
    virtual bool eventFilter(QObject *obj, QEvent *ev);

private slots:

    void slotImageRatingChanged(qlonglong);
    void slotEditRatingFromItem(int);

    void slotThemeChanged();

private:

    ImagePreviewBarPriv* const d;

    friend class ImagePreviewBarItem;
};

// -------------------------------------------------------------------------

class ImagePreviewBarItem : public ThumbBarItem
{
public:

    ImagePreviewBarItem(ImagePreviewBar *view, const ImageInfo &info);
    ~ImagePreviewBarItem();

    ImageInfo info();

    QRect clickToRateRect();

protected:

    ImageInfo        m_info;

private:

    friend class ImagePreviewBar;
};

// -------------------------------------------------------------------------

class ImagePreviewBarToolTip : public ThumbBarToolTip
{

public:

    ImagePreviewBarToolTip(ThumbBarView *parent);
    ~ImagePreviewBarToolTip();

protected:

    virtual QString tipContents();
};

}  // namespace Digikam

#endif /* IMAGEPREVIEWBAR_H */
