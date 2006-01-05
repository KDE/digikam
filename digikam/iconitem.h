/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-04-24
 * Copyright 2005 by Renchi Raju
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

// Qt includes.

#include <qrect.h>
#include <qstring.h>

namespace Digikam
{

class IconGroupItem;
class IconView;

class IconItem
{
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

    bool  move(int x, int y);

    void  setSelected(bool val, bool cb=true);
    bool  isSelected() const;

    void  repaint(bool force=true);

    IconView* iconView() const;

    virtual int compare(IconItem *item);
    virtual QRect clickToOpenRect();

protected:

    virtual void paintItem();

private:

    IconGroupItem   *m_group;
    IconItem        *m_next;
    IconItem        *m_prev;
    int              m_x;
    int              m_y;
    bool             m_selected;
};
    
}  // namespace Digikam

#endif /* ICONITEM_H */
