/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-01-20
 * Description : User interface for searches
 *
 * Copyright (C) 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include <QCache>
#include <QList>
#include <QRect>
#include <QWidget>

class QHBoxLayout;
class QDialogButtonBox;
class QPushButton;

namespace Digikam
{

class SearchGroup;
class SearchViewBottomBar;
class SearchXmlCachingReader;
class SearchXmlWriter;

class SearchViewThemedPartsCache
{
public:

    virtual ~SearchViewThemedPartsCache()
    {
    }

    virtual QPixmap groupLabelPixmap(int w, int h) = 0;
    virtual QPixmap bottomBarPixmap(int w, int h) = 0;
};

class AbstractSearchGroupContainer : public QWidget
{
    Q_OBJECT

public:

    /// Abstract base class for classes that contain SearchGroups
    // To contain common code of SearchView and SearchGroup,
    // as SearchGroups can have subgroups.

    explicit AbstractSearchGroupContainer(QWidget* const parent = 0);

public Q_SLOTS:

    SearchGroup* addSearchGroup();
    void removeSearchGroup(SearchGroup* group);

protected:

    /// Call before reading the XML part that could contain group elements
    void startReadingGroups(SearchXmlCachingReader& reader);

    /// Call when a group element is the current element
    void readGroup(SearchXmlCachingReader& reader);

    /// Call when the XML part is finished
    void finishReadingGroups();

    /// Write contained groups to writer
    void writeGroups(SearchXmlWriter& writer) const;

    /// Collects the data from the same method of all contained groups (position relative to this widget)
    QList<QRect> startupAnimationAreaOfGroups() const;

    /// Re-implement: create and setup a search group
    virtual SearchGroup* createSearchGroup() = 0;

    /// Re-implement: Adds a newly created group to the layout structures
    virtual void addGroupToLayout(SearchGroup* group) = 0;

protected Q_SLOTS:

    void removeSendingSearchGroup();

protected:

    int                 m_groupIndex;
    QList<SearchGroup*> m_groups;
};

// -------------------------------------------------------------------------

class SearchView : public AbstractSearchGroupContainer, public SearchViewThemedPartsCache
{
    Q_OBJECT

public:

    SearchView();
    ~SearchView();

    void setup();
    void setBottomBar(SearchViewBottomBar* const bar);

    void read(const QString& search);
    QString write() const;

    QPixmap groupLabelPixmap(int w, int h);
    QPixmap bottomBarPixmap(int w, int h);

Q_SIGNALS:

    void searchOk();
    void searchTryout();
    void searchCancel();

protected Q_SLOTS:

    void setTheme();
    void slotAddGroupButton();
    void slotResetButton();
    void startAnimation();
    void animationFrame(int);
    void timeLineFinished();

protected:

    QPixmap cachedBannerPixmap(int w, int h) const;

    virtual void paintEvent(QPaintEvent* e);
    virtual void showEvent(QShowEvent* event);

    virtual SearchGroup* createSearchGroup();
    virtual void addGroupToLayout(SearchGroup* group);

private:

    class Private;
    Private* const d;
};

// -------------------------------------------------------------------------

class SearchViewBottomBar : public QWidget
{
    Q_OBJECT

public:

    explicit SearchViewBottomBar(SearchViewThemedPartsCache* const cache, QWidget* const parent = 0);

Q_SIGNALS:

    void okPressed();
    void cancelPressed();
    void tryoutPressed();
    void addGroupPressed();
    void resetPressed();

protected:

    virtual void paintEvent(QPaintEvent*);

protected:

    QHBoxLayout*                m_mainLayout;

    QDialogButtonBox*           m_buttonBox;
    QPushButton*                m_addGroupsButton;
    QPushButton*                m_resetButton;

    SearchViewThemedPartsCache* m_themeCache;
};

} // namespace Digikam

#endif // SEARCHVIEW_H
