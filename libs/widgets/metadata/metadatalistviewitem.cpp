/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-21
 * Description : a generic list view item widget to 
 *               display metadata
 * 
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qpalette.h>
#include <qfont.h>
#include <qpainter.h>

// Local includes.

#include "metadatalistviewitem.h"

namespace Digikam
{

MetadataListViewItem::MetadataListViewItem(QListViewItem *parent, const QString& key,
                                           const QString& title, const QString& value)
                    : QListViewItem(parent)
{
    m_key = key;
    
    setSelectable(true);
    setText(0, title);
 
    QString tagVal = value.simplifyWhiteSpace();   
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

}  // namespace Digikam
