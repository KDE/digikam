/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-05-28
 * Description : Media Server configuration dialog to share a single list of files
 *
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DMEDIA_SERVER_DLG_H
#define DMEDIA_SERVER_DLG_H

// Qt includes

#include <QUrl>
#include <QDialog>

// Local includes

#include "digikam_export.h"
#include "dinfointerface.h"

namespace Digikam
{

class DIGIKAM_EXPORT DMediaServerDlg : public QDialog
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

} // namespace Digikam

#endif // DMEDIA_SERVER_DLG_H
