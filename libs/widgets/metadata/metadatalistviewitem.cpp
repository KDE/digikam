/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-21
 * Description : a generic list view item widget to 
 *               display metadata
 * 
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

// Qt include.

#include <qpalette.h>
#include <qfont.h>
#include <qpainter.h>

// Local includes.

#include "metadatalistviewitem.h"

namespace Digikam
{

MetadataListViewItem::MetadataListViewItem(KListViewItem *parent, const QString& key,
                                           const QString& title, const QString& value)
                    : KListViewItem(parent)
{
    m_key = key;
    
    setText(0, title);
    setText(1, value);
    setSelectable(true);
}

MetadataListViewItem::~MetadataListViewItem()
{
} 

QString MetadataListViewItem::getKey()
{
    return m_key;
}

QString MetadataListViewItem::getTitle()
{
    return text(0);
}

QString MetadataListViewItem::getValue()
{
    return text(1);
}

void MetadataListViewItem::paintCell(QPainter* p, const QColorGroup& cg,
                                     int column, int width, int align)
{
    if (column == 0)
    {
        p->save();
        QFont fn(p->font());
        fn.setBold(true);
        p->setFont(fn);
    }
    
    KListViewItem::paintCell(p, cg, column, width, align);

    if (column == 0)
        p->restore();
}

}  // namespace Digikam

