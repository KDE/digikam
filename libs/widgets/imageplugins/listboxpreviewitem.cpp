/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-10-05
 * Description : a QListBoxItem which can display an image preview
 *               as a thumbnail and a customized qwhatsthis class 
 *               for listbox items
 *
 * Copyright (C) 2006-2007 by Guillaume Laurent <glaurent@telegraph-road.org>
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

// Local includes.

#include "listboxpreviewitem.h"

namespace Digikam
{

int ListBoxPreviewItem::height(const Q3ListBox *lb) const
{
    int height = Q3ListBoxPixmap::height(lb);
    return qMax(height, pixmap()->height() + 5);
}

int ListBoxPreviewItem::width(const Q3ListBox *lb) const
{
    int width = Q3ListBoxPixmap::width(lb);
    return qMax(width, pixmap()->width() + 5);
}

// -------------------------------------------------------------------

QString ListBoxWhatsThis::text(const QPoint &p)
{
    Q3ListBoxItem* item = m_listBox->itemAt(p);

    if (item != 0) 
        return m_itemWhatsThisMap[item];

    return QString();
}

void ListBoxWhatsThis::add(Q3ListBoxItem* item, const QString& text)
{
    m_itemWhatsThisMap[item] = text;
}

} // namespace Digikam
