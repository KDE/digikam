/* ============================================================
 * Authors: Guillaume Laurent <glaurent@telegraph-road.org>
 * Description : customised qwhatsthis class for listbox items
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

#include "listboxwhatsthis.h"

namespace Digikam
{

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

}
