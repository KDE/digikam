/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-02-26
 * Description : Upper widget in the search sidebar
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

#ifndef SEARCHTABHEADER_H
#define SEARCHTABHEADER_H

// Qt includes.

#include <QWidget>

// Local includes.

#include "albuminfo.h"

namespace Digikam
{

class SAlbum;
class SearchWindow;
class SearchTabHeaderPriv;

class SearchTabHeader : public QWidget
{
    Q_OBJECT

public:

    SearchTabHeader(QWidget *parent);
    ~SearchTabHeader();

public Q_SLOTS:

    void selectedSearchChanged(SAlbum *album);
    void editSearch(SAlbum *album);
    void newKeywordSearch();
    void newAdvancedSearch();

Q_SIGNALS:

    void searchShallBeSelected(SAlbum *album);

private Q_SLOTS:

    void keywordChanged();
    void editCurrentAdvancedSearch();
    void saveSearch();
    void storedKeywordChanged();
    void editStoredAdvancedSearch();
    void advancedSearchEdited(int id, const QString &query);

private:

    void setCurrentSearch(DatabaseSearch::Type type, const QString &query, bool selectCurrentAlbum = true);
    QString queryFromKeywords(const QString &keywords);
    QString keywordsFromQuery(const QString &query);
    SearchWindow *searchWindow();

    SearchTabHeaderPriv* const d;
};

} // namespace Digikam

#endif // SEARCHTABHEADER_H
