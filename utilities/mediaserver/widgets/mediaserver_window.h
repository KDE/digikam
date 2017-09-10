/*
 *  Copyright (C) 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of an application named HUpnpAvSimpleTestApp
 *  used for demonstrating how to use the Herqq UPnP A/V (HUPnPAv) library.
 *
 *  HUpnpAvSimpleTestApp is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  HUpnpAvSimpleTestApp is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with HUpnpAvSimpleTestApp. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MEDIASERVER_WINDOW_H
#define MEDIASERVER_WINDOW_H

#include <QtGlobal>
#include <QMainWindow>
#include <QPointer>
#include <QMap>

#include "hav_defs.h"
#include "hav_fwd.h"
#include "hav_global.h"
#include "hdevicehost.h"
#include "hrootdir.h"

using namespace Herqq::Upnp;
using namespace Herqq::Upnp::Av;

namespace Ui
{
    class MediaServerWindow;
}

class AddContentDialog;

class DIGIKAM_EXPORT MediaServerWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MediaServerWindow(QWidget* parent = 0);
    virtual ~MediaServerWindow();

    void initRequiredDirectories();

    DIGIKAM_EXPORT static bool deletedFlag;
    static const QString serverDatabasePath;
    static const QString serverDescriptionPath;

Q_SIGNALS:

    void closed();

protected:

    virtual void changeEvent(QEvent*);
    virtual void closeEvent(QCloseEvent*);

private:

    void addRootDirectoriesToServer(const HRootDir& rd);
    void addItemsToServer(const QStringList& fullPaths);
    void saveDirectoriesToDatabase();
    void loadDirectoriesFromDatabase();
    void saveItemsToDatabase();
    void loadItemsFromDatabase();

private Q_SLOTS:

    void on_addContentButton_clicked();
    void addContenDlgtFinished();

    void on_addItemButton_clicked();

    void on_DeleteDirectoriesButton_clicked();
    void on_HideWindowButton_clicked();
    void setDeletedFlag();

private:

    Ui::MediaServerWindow*     m_ui;

    HDeviceHost*               m_deviceHost;
    HFileSystemDataSource*     m_datasource;

    QPointer<AddContentDialog> m_dlg;
    QMap<int, HRootDir>        m_rootDirectoriesMap;
    QMap<int, QString>         m_itemsMap;
};

#endif // MEDIASERVER_WINDOW_H
