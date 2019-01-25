/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-02-12
 * Description : a tool to export images to IPFS web service
 *
 * Copyright (C) 2018 by Amar Lakshya <amar dot lakshya  at xaviers dot edu dot in>
 * Copyright (C) 2018 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DIGIKAM_IPFS_WINDOW_H
#define DIGIKAM_IPFS_WINDOW_H

// Qt includes

#include <QObject>
#include <QLabel>

// Local includes

#include "ipfsimageslist.h"
#include "ipfstalker.h"
#include "wstooldialog.h"
#include "digikam_export.h"
#include "dinfointerface.h"

using namespace Digikam;

namespace GenericDigikamIpfsPlugin
{

class DIGIKAM_EXPORT IpfsWindow : public WSToolDialog
{
    Q_OBJECT

public:

    explicit IpfsWindow(DInfoInterface* const iface, QWidget* const parent = 0);
    ~IpfsWindow();

    void reactivate();

public Q_SLOTS:

    // UI callbacks

    void slotUpload();
    void slotFinished();
    void slotCancel();

    // IpfsTalker callbacks

/*
     void apiAuthorized(bool success, const QString& username);
     void apiAuthError(const QString& msg);
*/
    void apiProgress(unsigned int percent, const IpfsTalkerAction& action);
    void apiRequestPin(const QUrl& url);
    void apiSuccess(const IpfsTalkerResult& result);
    void apiError(const QString &msg, const IpfsTalkerAction& action);
    void apiBusy(bool busy);

private:

    void closeEvent(QCloseEvent* e) Q_DECL_OVERRIDE;
    void setContinueUpload(bool state);
    void readSettings();
    void saveSettings();

private:

    class Private;
    Private* const d;
};

} // namespace GenericDigikamIpfsPlugin

#endif // DIGIKAM_IPFS_WINDOW_H
