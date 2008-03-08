/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-21
 * Description : a generic list view item widget to 
 *               display metadata key like a title
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

#ifndef MDKEYLISTVIEWITEM_H
#define MDKEYLISTVIEWITEM_H

// Qt includes.

#include <qstring.h>
#include <qlistview.h>

// Local includes.

#include "digikam_export.h"

class QPainter;

namespace Digikam
{

class DIGIKAM_EXPORT MdKeyListViewItem : public QListViewItem
{

public:

    MdKeyListViewItem(QListView *parent, const QString& key);
    ~MdKeyListViewItem();

    QString getMdKey();
    
protected:

    void paintCell(QPainter*, const QColorGroup &, int, int, int);

private:

    QString m_decryptedKey;
};

}  // namespace Digikam

#endif /* MDKEYLISTVIEWITEM_H */
