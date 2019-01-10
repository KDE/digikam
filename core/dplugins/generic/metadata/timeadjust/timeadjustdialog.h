/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-05-16
 * Description : dialog to set time stamp of picture files.
 *
 * Copyright (C) 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * Copyright (C) 2003-2005 by Jesper Pedersen <blackie at kde dot org>
 * Copyright (C) 2006-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (c) 2018      by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DIGIKAM_TIME_ADJUST_DIALOG_H
#define DIGIKAM_TIME_ADJUST_DIALOG_H

// Qt includes

#include <QUrl>

// Local includes

#include "dplugindialog.h"
#include "digikam_export.h"
#include "timeadjustsettings.h"

namespace Digikam
{

class DInfoInterface;

class DIGIKAM_EXPORT TimeAdjustDialog : public DPluginDialog
{
    Q_OBJECT

public:

    explicit TimeAdjustDialog(QWidget* const parent, DInfoInterface* const iface);
    ~TimeAdjustDialog();

Q_SIGNALS:

    void signalDateTimeForUrl(const QUrl&, const QDateTime&, bool);

private Q_SLOTS:

    void slotApplyClicked();
    void slotDialogFinished();

    void slotThreadFinished();
    void slotCancelThread();
    void slotProcessStarted(const QUrl&);
    void slotProcessEnded(const QUrl&, int);
    void setBusy(bool);

    /** Read the Used Timestamps for all selected files
     *  (according to the newly selected source timestamp type),
     *  this will also implicitly update listview info.
     */
    void slotReadTimestamps();

private:

    /** Called by readTimestamps() to get host timestamps
     */
    void readApplicationTimestamps();

    /** Called by readTimestamps() to get file timestamps
     */
    void readFileTimestamps();

    /** Called by readTimestamps() to get file metadata timestamps
     */
    void readMetadataTimestamps();

    void readSettings();
    void saveSettings();

    void updateListView();

protected:

    void closeEvent(QCloseEvent*);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // DIGIKAM_TIME_ADJUST_DIALOG_H
