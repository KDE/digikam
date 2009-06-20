/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-20
 * Description : identity list view.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef INDENTITYLIST_H
#define INDENTITYLIST_H

// Qt includes

#include <QTreeWidget>

namespace Digikam
{

class Photographer;

class IndentityListItem : public QTreeWidgetItem
{

public:

    IndentityListItem(QTreeWidget *parent, Photographer *photographer);
    ~IndentityListItem();

    void setPhotographer(Photographer *photographer);

    Photographer* photographer() const;

private:

    Photographer* m_photographer;
};

// -------------------------------------------------------------------

class IndentityList : public QTreeWidget
{
    Q_OBJECT

public:

    IndentityList(QWidget* parent=0);
    ~IndentityList();

    void readSettings();
    void applySettings();
};

}  // namespace Digikam

#endif // INDENTITYLIST_H
