/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-21
 * Description : a generic list view item widget to 
 *               display metadata
 * 
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QPalette>
#include <QFont>
#include <QPainter>

// Local includes.

#include "metadatalistviewitem.h"

namespace Digikam
{

MetadataListViewItem::MetadataListViewItem(K3ListViewItem *parent, const QString& key,
                                           const QString& title, const QString& value)
                    : K3ListViewItem(parent)
{
    m_key = key;
    
    setSelectable(true);
    setText(0, title);
 
    QString tagVal = value.simplified();   
    if (tagVal.length() > 128)
    {
        tagVal.truncate(128);
        tagVal.append("...");
    }
    setText(1, tagVal);
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
    
    K3ListViewItem::paintCell(p, cg, column, width, align);

    if (column == 0)
        p->restore();
}

}  // namespace Digikam
