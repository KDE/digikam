/* ============================================================
 * Author: Guillaume Laurent <glaurent@telegraph-road.org>
 * Date  : 2006-10-05
 * Description : a QListBoxItem which can display an image preview
 *               as a thumbnail and a customised qwhatsthis class 
 *               for listbox items
 *
 * Copyright 2006 by Guillaume Laurent
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

#include <qlistbox.h>

// Local includes.

#include "listboxpreviewitem.h"

namespace Digikam
{

int ListBoxPreviewItem::height(const QListBox *lb) const
{
    int height = QListBoxPixmap::height(lb);
    return QMAX(height, pixmap()->height() + 5);
}

int ListBoxPreviewItem::width(const QListBox *lb) const
{
    int width = QListBoxPixmap::width(lb);
    return QMAX(width, pixmap()->width() + 5);
}

// -------------------------------------------------------------------

QString ListBoxWhatsThis::text(const QPoint &p)
{
    QListBoxItem* item = m_listBox->itemAt(p);

    if (item != 0) 
        return m_itemWhatsThisMap[item];

    return QString::null;
}

void ListBoxWhatsThis::add(QListBoxItem* item, const QString& text)
{
    m_itemWhatsThisMap[item] = text;
}

} // namespace Digikam
