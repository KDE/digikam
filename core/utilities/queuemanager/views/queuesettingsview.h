/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-21
 * Description : a view to show Queue Settings.
 *
 * Copyright (C) 2009-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_BQM_QUEUE_SETTINGS_VIEW_H
#define DIGIKAM_BQM_QUEUE_SETTINGS_VIEW_H

// Qt includes

#include <QScrollArea>
#include <QList>
#include <QMap>
#include <QTabWidget>

namespace Digikam
{

class AssignedBatchTools;
class QueueSettings;

class QueueSettingsView : public QTabWidget
{
    Q_OBJECT

public:

    explicit QueueSettingsView(QWidget* const parent = nullptr);
    ~QueueSettingsView();

    void setBusy(bool b);

Q_SIGNALS:

    void signalSettingsChanged(const QueueSettings&);

public Q_SLOTS:

    void slotQueueSelected(int, const QueueSettings&, const AssignedBatchTools&);

private Q_SLOTS:

    void slotUseOrgAlbum();
    void slotResetSettings();
    void slotSettingsChanged();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_BQM_QUEUE_SETTINGS_VIEW_H
