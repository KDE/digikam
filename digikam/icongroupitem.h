/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-04-24
 * Description : 
 *
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

#ifndef ICONGROUPITEM_H
#define ICONGROUPITEM_H

// Qt includes.

#include <qrect.h>

namespace Digikam
{

class IconView;
class IconItem;
class IconGroupItemPriv;

class IconGroupItem
{
    friend class IconView;

public:

    IconGroupItem(IconView* parent);
    virtual ~IconGroupItem();

    IconView* iconView() const;
    
    IconGroupItem* nextGroup() const;
    IconGroupItem* prevGroup() const;

    QRect  rect() const;
    int    y() const;
    bool   move(int y);

    IconItem* firstItem() const;
    IconItem* lastItem() const;

    int       count() const;
    int       index(IconItem* item) const;

    void      clear(bool update=true);
    void      sort();
    
    void insertItem(IconItem* item);
    void takeItem(IconItem* item);

    virtual int compare(IconGroupItem *group);
    
protected:
    
    virtual void paintBanner();

private:
    
    static int cmpItems(const void *n1, const void *n2);

private:

    IconGroupItemPriv *d;
    IconGroupItem     *m_next;
    IconGroupItem     *m_prev;
};

}  // namespace Digikam

#endif /* ICONGROUPITEM_H */
