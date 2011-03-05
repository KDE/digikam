/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-24
 * Description : icon item.
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

#ifndef ICONITEM_H
#define ICONITEM_H

// Qt includes

#include <QObject>
#include <QRect>
#include <QString>

// Local includes

#include "digikam_export.h"

class QPainter;

namespace Digikam
{

class IconGroupItem;
class IconView;
class IconItemPriv;

class IconItem : public QObject
{
    Q_OBJECT

    friend class IconView;
    friend class IconGroupItem;

public:

    IconItem(IconGroupItem* parent);
    virtual ~IconItem();

    IconItem* nextItem() const;
    IconItem* prevItem() const;

    int   x() const;
    int   y() const;
    QRect rect() const;
    QRect toggleSelectRect() const;
    QRect clickToToggleSelectRect() const;

    bool  move(int x, int y);

    void  setSelected(bool val, bool cb=true);
    bool  isSelected() const;

    void  setHighlighted(bool val);
    bool  isHighlighted() const;

    void  setEditRating(bool val);
    bool  editRating() const;

    void  repaint();
    void  update();

    IconView* iconView() const;

    virtual int compare(IconItem* item);
    virtual QRect clickToOpenRect();
    virtual QRect clickToRateRect();

    virtual void  setRating(int rating);
    virtual int   rating() const;

protected:

    void paintToggleSelectButton(QPainter* p);
    virtual void paintItem(QPainter* p);

private:

    IconItem*    m_next;
    IconItem*    m_prev;

    IconItemPriv* const d;
};

}  // namespace Digikam

#endif /* ICONITEM_H */
