/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-01-20
 * Description : User interface for searches
 * 
 * Copyright (C) 2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef SEARCHVIEW_H
#define SEARCHVIEW_H

// Qt includes

#include <QWidget>
#include <QList>

// KDE includes

// Local includes


class QVBoxLayout;

namespace Digikam
{

class SearchGroup;

class SearchViewThemedPartsCache
{
public:

    virtual ~SearchViewThemedPartsCache() {}
    virtual QPixmap groupLabelPixmap(int w, int h) = 0;
};

class SearchView : public QWidget, public SearchViewThemedPartsCache
{

    Q_OBJECT

public:

    SearchView();

    void setup();

    void read(const QString &search);
    QString write();

public slots:

    SearchGroup *addSearchGroup();

protected slots:

    void setTheme();

public:

    QPixmap groupLabelPixmap(int w, int h);

protected:

    QVBoxLayout         *m_layout;
    QList<SearchGroup *> m_groups;
    QPixmap              m_cachedGroupLabelPixmap;
};

}

#endif


