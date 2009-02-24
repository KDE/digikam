/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-21
 * Description : a view to show Queue Settings.
 *
 * Copyright (C) 2009 Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef QUEUE_SETTINGS_VIEW_H
#define QUEUE_SETTINGS_VIEW_H

// Qt includes.

#include <QScrollArea>
#include <QList>
#include <QMap>

// KDE includes.

#include <kurl.h>
#include <ktabwidget.h>

namespace Digikam
{

class QueueSettings
{
public:

    enum ConflictRule
    {
        OVERWRITE = 0,
        ASKTOUSER
    };

public:

    QueueSettings()
    {
        conflictRule = OVERWRITE;
    };


    KUrl         targetUrl;

    ConflictRule conflictRule;
};

// --------------------------------------------------------

class AssignedBatchTools;
class QueueSettingsViewPriv;

class QueueSettingsView : public KTabWidget
{
    Q_OBJECT

public:

    QueueSettingsView(QWidget *parent=0);
    ~QueueSettingsView();

signals:

    void signalSettingsChanged(const QueueSettings&);

public slots:

    void slotQueueSelected(int, const QueueSettings&, const AssignedBatchTools&);

private slots:

    void slotSettingsChanged();

private:

    QueueSettingsViewPriv* const d;
};

}  // namespace Digikam

#endif /* QUEUE_SETTINGS_VIEW_H */
