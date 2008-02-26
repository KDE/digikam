/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-26-02
 * Description : Tree folder item.
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef TREEFOLDERITEM_H
#define TREEFOLDERITEM_H

// Qt includes.

#include <Q3ListView>
#include <QStyleOptionQ3ListView>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class TreeFolderView;

class DIGIKAM_EXPORT TreeFolderItem : public Q3ListViewItem
{
public:

    TreeFolderItem(Q3ListView* parent, const QString& text, bool special=false);
    TreeFolderItem(Q3ListViewItem* parent, const QString& text, bool special=false);

    virtual ~TreeFolderItem();

    virtual int id() const;

    void setFocus(bool b);
    bool focus() const;

protected:

    void paintCell(QPainter* p, const QColorGroup & cg, int column, int width, int align);
    void setup();

private:

    bool m_focus;
    bool m_special;
};

// ------------------------------------------------------------------------------------

class TreeFolderCheckListItem : public Q3CheckListItem
{
public:

    TreeFolderCheckListItem(Q3ListView* parent, const QString& text,
                            Q3CheckListItem::Type tt);
    TreeFolderCheckListItem(Q3ListViewItem* parent, const QString& text,
                            Q3CheckListItem::Type tt);
    virtual ~TreeFolderCheckListItem();

protected:

    void paintCell(QPainter* p, const QColorGroup & cg, int column, int width, int align);
    void setup();
    QStyleOptionQ3ListView getStyleOption(const TreeFolderView *fv);
};

}  // namespace Digikam

#endif /* TREEFOLDERITEM_H */
