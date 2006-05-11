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

#ifndef LISTBOXWHATSTHIS_H
#define LISTBOXWHATSTHIS_H

 // Qt includes.

#include <qwhatsthis.h>
#include <qmap.h>

class QListBox;
class QListBoxItem;


namespace Digikam
{

/**
 * A qwhatthis class which can be pointed to a specific item
 * in a QListBox rather than the QListBox itself
 *
 */
class ListBoxWhatsThis : public QWhatsThis 
{

public:

    ListBoxWhatsThis(QListBox* w) : QWhatsThis(w), m_listBox(w) {}
    virtual QString text (const QPoint &);
    void add(QListBoxItem*, const QString& text);

protected:

    QMap<QListBoxItem*, QString>  m_itemWhatsThisMap;
    QListBox                     *m_listBox;
};

}

#endif
