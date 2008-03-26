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

#ifndef LISTBOXPREVIEWITEM_H
#define LISTBOXPREVIEWITEM_H

 // Qt includes.

#include <Q3ListBox>
#include <QMap>
#include <QPixmap>
#include <Q3WhatsThis>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ListBoxPreviewItem : public Q3ListBoxPixmap
{

public:

    ListBoxPreviewItem(Q3ListBox *listbox, const QPixmap &pix, const QString &text)
        : Q3ListBoxPixmap(listbox, pix, text) {};

    ListBoxPreviewItem(const QPixmap &pix, const QString &text)
        : Q3ListBoxPixmap(pix, text) {};

    virtual int height ( const Q3ListBox * lb ) const;
    virtual int width  ( const Q3ListBox * lb ) const;
};

/**
 * A qwhatthis class which can be pointed to a specific item
 * in a QListBox rather than the QListBox itself
 *
 */
class DIGIKAM_EXPORT ListBoxWhatsThis : public Q3WhatsThis 
{

public:

    ListBoxWhatsThis(Q3ListBox* w) : Q3WhatsThis(w), m_listBox(w) {}
    virtual QString text (const QPoint &);
    void add(Q3ListBoxItem*, const QString& text);

protected:

    QMap<Q3ListBoxItem*, QString>  m_itemWhatsThisMap;
    Q3ListBox                     *m_listBox;
};

}  // namespace Digikam

#endif  // LISTBOXPREVIEWITEM_H
