/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-20
 * Description : search results item.
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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
 * ============================================================ */

#ifndef SEARCHRESULTSITEM_H
#define SEARCHRESULTSITEM_H

// Qt includes.

#include <qiconview.h>

namespace Digikam
{

class SearchResultsItem : public QIconViewItem
{
    friend class SearchResultsView;

public:

    SearchResultsItem(QIconView* view, const QString& path);
    ~SearchResultsItem();

protected:

    void calcRect(const QString& text = QString());
    void paintItem (QPainter * p, const QColorGroup & cg);
    void paintFocus (QPainter * p, const QColorGroup & cg);

private:

    static QPixmap* m_basePixmap;
    bool            m_marked;
    QString         m_path;
};    
    
}  // namespace Digikam

#endif /* SEARCHRESULTSITEM_H */
