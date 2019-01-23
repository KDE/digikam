/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-05-28
 * Description : Media Server configuration dialog to share a single list of files
 *
 * Copyright (C) 2012-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_DMEDIA_SERVER_DLG_H
#define DIGIKAM_DMEDIA_SERVER_DLG_H

// Qt includes

#include <QUrl>

// Local includes

#include "digikam_export.h"
#include "dinfointerface.h"
#include "dplugindialog.h"

using namespace Digikam;

namespace GenericMediaServerPlugin
{

class DIGIKAM_EXPORT DMediaServerDlg : public DPluginDialog
{
    Q_OBJECT

public:

    explicit DMediaServerDlg(QObject* const parent,
                             DInfoInterface* const iface=0);
    ~DMediaServerDlg();

private:

    void readSettings();
    void saveSettings();
    void updateServerStatus();
    bool setMediaServerContents();
    void startMediaServer();

private Q_SLOTS:

    void accept();
    void slotToggleMediaServer();
    void slotSelectionChanged();

private:

    class Private;
    Private* const d;
};

} // namespace GenericMediaServerPlugin

#endif // DIGIKAM_DMEDIA_SERVER_DLG_H
