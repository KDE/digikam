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

#include <qmap.h>
#include <qlistbox.h>
#include <qwhatsthis.h>
#include <qpixmap.h>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ListBoxPreviewItem : public QListBoxPixmap
{

public:

    ListBoxPreviewItem(QListBox *listbox, const QPixmap &pix, const QString &text)
        : QListBoxPixmap(listbox, pix, text) {};

    ListBoxPreviewItem(const QPixmap &pix, const QString &text)
        : QListBoxPixmap(pix, text) {};

    virtual int height ( const QListBox * lb ) const;
    virtual int width  ( const QListBox * lb ) const;
};

/**
 * A qwhatthis class which can be pointed to a specific item
 * in a QListBox rather than the QListBox itself
 *
 */
class DIGIKAM_EXPORT ListBoxWhatsThis : public QWhatsThis 
{

public:

    ListBoxWhatsThis(QListBox* w) : QWhatsThis(w), m_listBox(w) {}
    virtual QString text (const QPoint &);
    void add(QListBoxItem*, const QString& text);

protected:

    QMap<QListBoxItem*, QString>  m_itemWhatsThisMap;
    QListBox                     *m_listBox;
};

}  // namespace Digikam

#endif  // LISTBOXPREVIEWITEM_H
