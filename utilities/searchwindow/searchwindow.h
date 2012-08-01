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

#ifndef SEARCHWINDOW_H
#define SEARCHWINDOW_H

// Qt includes

#include <QWidget>

namespace Digikam
{

class SearchWindow : public QWidget
{
    Q_OBJECT

public:

    /**
     * Create a new SearchWindow with an empty advanced search
     */
    SearchWindow();

    ~SearchWindow();

    /**
     * Read the given search into the search widgets.
     * The id will be emitted with the searchEdited signal.
     */
    void readSearch(int id, const QString& query);

    /**
     * Reset the search widget to an empty search.
     * Current id is -1.
     */
    void reset();

    /**
     * Returns the currently produced search string
     */
    QString search() const;

Q_SIGNALS:

    /**
     * Signals that the user has finished editing the search.
     * The given query is the same as search().
     */
    void searchEdited(int id, const QString& query);

protected Q_SLOTS:

    void searchOk();
    void searchCancel();
    void searchTryout();

protected:

    virtual void keyPressEvent(QKeyEvent*);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // SEARCHWINDOW_H
