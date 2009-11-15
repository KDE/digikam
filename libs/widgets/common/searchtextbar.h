/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-25
 * Description : a bar used to search a string.
 *
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SEARCH_TEXT_BAR_H
#define SEARCH_TEXT_BAR_H

// QT includes
#include <qabstractitemmodel.h>

// KDE includes

#include <klineedit.h>
#include <klocale.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class SearchTextBarPriv;

class DIGIKAM_EXPORT SearchTextSettings
{

public:

    SearchTextSettings()
    {
        caseSensitive = Qt::CaseInsensitive;
    }

    Qt::CaseSensitivity caseSensitive;

    QString             text;
};

class DIGIKAM_EXPORT SearchTextBar : public KLineEdit
{
    Q_OBJECT

public:

    SearchTextBar(QWidget *parent, const char* name, const QString& msg=i18n("Search..."));
    ~SearchTextBar();

    void setTextQueryCompletion(bool b);
    bool hasTextQueryCompletion() const;

    /**
     * If the given model is != null, the model is used to populate the
     * completion for this text field.
     *
     * @param model to fill from or null for manual mode
     */
    void setModel(QAbstractItemModel *model);

    void setCaseSensitive(bool b);
    bool hasCaseSensitive() const;

    void setSearchTextSettings(const SearchTextSettings& settings);
    SearchTextSettings searchTextSettings() const;

Q_SIGNALS:

    void signalSearchTextSettings(const SearchTextSettings& settings);

public Q_SLOTS:

    void slotSearchResult(bool);

private Q_SLOTS:

    void slotTextChanged(const QString&);
    void slotRowsInserted(const QModelIndex &parent, int start, int end);
    void slotRowsRemoved(const QModelIndex &parent, int start, int end);
    void slotDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void slotModelReset();

private:

    void contextMenuEvent(QContextMenuEvent* e);
    void connectToModel(QAbstractItemModel *model);
    void disconnectFromModel(QAbstractItemModel *model);
    void sync(QAbstractItemModel *model);
    void sync(QAbstractItemModel *model, const QModelIndex &index);

private:

    SearchTextBarPriv* const d;
};

}  // namespace Digikam

#endif /* SEARCH_TEXT_BAR_H */
