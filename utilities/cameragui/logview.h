/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-26
 * Description : Camera log view.
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

#ifndef LOG_VIEW_H
#define LOG_VIEW_H

// Qt includes

#include <QTreeWidget>
#include <QWidget>
#include <QString>

// Local includes

#include "batchtool.h"
#include "batchtoolsmanager.h"

namespace Digikam
{

class LogView : public QTreeWidget
{
    Q_OBJECT

public:

    enum LogEntryType
    {
        StartingEntry = 0,
        SuccessEntry,
        WarningEntry,
        ErrorEntry,
        ProgressEntry
    };

public:

    LogView(QWidget *parent);
    virtual ~LogView();

    void addedLogEntry(const QString& folder, const QString& file, const QString& text, LogEntryType type);

Q_SIGNALS:

    void signalEntryClicked(const QString& folder, const QString& file);

private Q_SLOTS:

    void slotItemDoubleClicked(QTreeWidgetItem*);
};

}  // namespace Digikam

#endif // LOG_VIEW_H
