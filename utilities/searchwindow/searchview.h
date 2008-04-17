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
#include <QCache>

// KDE includes

// Local includes


class QVBoxLayout;
class KDialogButtonBox;
class KPushButton;

namespace Digikam
{

class SearchGroup;
class SearchViewBottomBar;
class SearchClickLabel;

class SearchViewThemedPartsCache
{
public:

    virtual ~SearchViewThemedPartsCache() {}
    virtual QPixmap groupLabelPixmap(int w, int h) = 0;
    virtual QPixmap bottomBarPixmap(int w, int h) = 0;
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

signals:

    void searchOk();
    void searchTryout();
    void searchCancel();

protected slots:

    void setTheme();
    void slotAddGroupButton();

public:

    QPixmap groupLabelPixmap(int w, int h);
    QPixmap bottomBarPixmap(int w, int h);

protected:

    QPixmap cachedBannerPixmap(int w, int h);

    QVBoxLayout         *m_layout;
    QList<SearchGroup *> m_groups;
    SearchViewBottomBar *m_bar;
    QCache<QString, QPixmap> m_pixmapCache;
};

class SearchViewBottomBar : public QWidget
{
    Q_OBJECT

public:

    SearchViewBottomBar(SearchViewThemedPartsCache * cache, QWidget *parent = 0);

signals:

    void okPressed();
    void cancelPressed();
    void tryoutPressed();
    void addGroupPressed();

protected:

    virtual void paintEvent(QPaintEvent *);

    SearchViewThemedPartsCache *m_themeCache;
    QHBoxLayout                *m_mainLayout;
    KDialogButtonBox           *m_buttonBox;
    KPushButton                *m_addGroupsButton;
};

}

#endif


