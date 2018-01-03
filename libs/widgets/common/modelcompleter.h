/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-13
 * Description : A QCompleter for AbstractAlbumModels
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef MODELCOMPLETER_H
#define MODELCOMPLETER_H

// QT includes

#include <QAbstractItemModel>
#include <QStringList>
#include <QCompleter>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ModelCompleter : public QCompleter
{
    Q_OBJECT

public:

    ModelCompleter(QObject* parent = 0);
    ~ModelCompleter();

    /**
     * If the given model is != null, the model is used to populate the
     * completion for this text field.
     *
     * @param model to fill from or null for manual mode
     * @param uniqueIdRole a role for which the model will return a unique integer for each entry
     * @param displayRole the role to retrieve the text for completion, default is Qt::DisplayRole.
     */
    void setItemModel(QAbstractItemModel* const model, int uniqueIdRole, int displayRole = Qt::DisplayRole);
    QAbstractItemModel* itemModel() const;

    void addItem(const QString& item);
    QStringList items() const;

private Q_SLOTS:

    void slotDelayedModelTimer();
    void slotRowsInserted(const QModelIndex& parent, int start, int end);
    void slotRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
    void slotDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
    void slotModelReset();

private:

    void sync(QAbstractItemModel* const model);
    void sync(QAbstractItemModel* const model, const QModelIndex& index);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // MODELCOMPLETER_H
