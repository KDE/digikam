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

#include "mediaserver_window.h"
#include "ui_mediaserver_window.h"

// Qt includes

#include <QDir>
#include <QFileDialog>
#include <QDataStream>
#include <QDebug>
#include <QStandardPaths>

// Hupnp includes

#include "hupnp_global.h"
#include "hdeviceinfo.h"
#include "hdevicehost_configuration.h"
#include "hitem.h"
#include "hrootdir.h"
#include "hcontainer.h"
#include "hav_devicemodel_creator.h"
#include "hmediaserver_deviceconfiguration.h"
#include "hfsys_datasource.h"
#include "hcontentdirectory_serviceconfiguration.h"

const QString MediaServerWindow::serverDescriptionPath(QStandardPaths::locate(QStandardPaths::GenericDataLocation,
              QString::fromLatin1("digikam/mediaserver/descriptions/herqq_mediaserver_description.xml")));
const QString MediaServerWindow::serverDatabasePath(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
              + QLatin1String("/digikam/mediaserver/database"));

bool MediaServerWindow:: deletedFlag = false ;

MediaServerWindow::MediaServerWindow(QWidget* const parent)
    : QMainWindow(parent),
      m_ui(new Ui::MediaServerWindow),
      m_deviceHost(0),
      m_datasource(0)
{
    m_ui->setupUi(this);

    Herqq::Upnp::SetLoggingLevel(Herqq::Upnp::Debug);

    // 1) Configure a data source
    HFileSystemDataSourceConfiguration datasourceConfig;
    // Here you could configure the data source in more detail if needed. For example,
    // you could add "root directories" to the configuration and the data source
    // would scan those directories for media content upon initialization.
    m_datasource = new HFileSystemDataSource(datasourceConfig);

    // 2) Configure ContentDirectoryService by providing it access to the desired data source.
    HContentDirectoryServiceConfiguration cdsConfig;
    cdsConfig.setDataSource(m_datasource, false);

    // 3) Configure MediaServer by giving it the ContentDirectoryService configuration.
    HMediaServerDeviceConfiguration mediaServerConfig;
    mediaServerConfig.setContentDirectoryConfiguration(cdsConfig);

    // 4) Setup the "Device Model Cretor" that HUPnP will use to create
    // appropriate UPnP A/V device and service instances. Here you provide the
    // MediaServer configuration HUPnP will pass to the MediaServer device instance.
    HAvDeviceModelCreator creator;
    creator.setMediaServerConfiguration(mediaServerConfig);

    // 5) Setup the HDeviceHost with desired configuration info.
    HDeviceConfiguration config;

    QString deviceDescriptionPath = serverDescriptionPath;
    qDebug() << "APP PATH" << deviceDescriptionPath;
    qDebug() << "server database path " << serverDatabasePath ;
    config.setPathToDeviceDescription(deviceDescriptionPath);
    config.setCacheControlMaxAge(180);

    HDeviceHostConfiguration hostConfiguration;
    hostConfiguration.setDeviceModelCreator(creator);
    hostConfiguration.add(config);

    // 6) Initialize the HDeviceHost.
    m_deviceHost = new HDeviceHost(this);

    if (!m_deviceHost->init(hostConfiguration))
    {
        Q_ASSERT_X(false, "",  m_deviceHost->errorDescription().toLocal8Bit().constData());
    }

    // init database storage path
    initRequiredDirectories();

    // load previously saved data
    loadDirectoriesFromDatabase();
    loadItemsFromDatabase();
}

MediaServerWindow::~MediaServerWindow()
{
    saveDirectoriesToDatabase();
    saveItemsToDatabase();

    delete m_ui;
    delete m_datasource;
}

void MediaServerWindow::initRequiredDirectories()
{
    if (!QDir(serverDatabasePath).exists())
    {
        if (QDir().mkpath(serverDatabasePath))
        {
            QFile f(serverDatabasePath);
            f.setPermissions(QFile::ReadUser | QFile::WriteUser | QFile::ExeUser); // 0700
        }
    }

    if (!QDir(serverDescriptionPath).exists())
    {
        if (QDir().mkpath(serverDescriptionPath))
        {
            QFile f(serverDescriptionPath);
            f.setPermissions(QFile::ReadUser | QFile::WriteUser | QFile::ExeUser); // 0700
        }
    }
}

void MediaServerWindow::addRootDirectoriesToServer(const HRootDir& rd)
{
    HRootDir::ScanMode smode = rd.scanMode();

    if (m_datasource->add(rd) >= 0)
    {
        int rc = m_ui->sharedItemsTable->rowCount();
        m_rootDirectoriesMap.insert(rc, rd);
        m_ui->sharedItemsTable->insertRow(rc);

        QTableWidgetItem* const newItemScanType = new QTableWidgetItem(
            (smode == HRootDir::RecursiveScan) ? QLatin1String("Yes") : QLatin1String("No"));

        newItemScanType->setFlags(Qt::ItemIsEnabled);

        m_ui->sharedItemsTable->setItem(rc, 0, newItemScanType);

        QTableWidgetItem* const newItemWatchType = new QTableWidgetItem(QLatin1String("No"));
        newItemWatchType->setFlags(Qt::ItemIsEnabled);

        m_ui->sharedItemsTable->setItem(rc, 1, newItemWatchType);

        QTableWidgetItem* const newItem = new QTableWidgetItem(rd.dir().absolutePath());

        newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        m_ui->sharedItemsTable->setItem(rc, 2, newItem);
    }
}

void MediaServerWindow::saveDirectoriesToDatabase()
{
    QFile file(serverDatabasePath + (QLatin1String("/serverDirectories.dat")));
    file.open(QFile::WriteOnly);
    QDataStream out(&file);

    auto end = m_rootDirectoriesMap.cend();

    for (auto it = m_rootDirectoriesMap.cbegin() ; it != end ; ++it)
    {
        out << it.value();
    }
}

void MediaServerWindow::loadDirectoriesFromDatabase()
{
    QFile file(serverDatabasePath + (QLatin1String("/serverDirectories.dat")));
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);
    HRootDir dir ;

    while (in.atEnd() == false)
    {
        in >> dir;
        addRootDirectoriesToServer(dir);
    }
}

void MediaServerWindow::saveItemsToDatabase()
{
    QFile file(serverDatabasePath + (QLatin1String("/serverItems.dat")));
    file.open(QFile::WriteOnly);
    QDataStream out(&file);

    auto end = m_itemsMap.cend();

    for (auto it = m_itemsMap.cbegin() ; it != end ; ++it)
    {
        out << it.value();
    }
}

void MediaServerWindow::loadItemsFromDatabase()
{
    QFile file(serverDatabasePath + (QLatin1String("/serverItems.dat")));
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);
    QString     dir;
    QStringList list;

    while (in.atEnd() == false)
    {
        in >> dir;
        list.append(dir);
    }

    addItemsToServer(list);
}

void MediaServerWindow::on_addContentButton_clicked()
{
    QString dirName = QFileDialog::getExistingDirectory(this, QLatin1String("Open Directory"));

    if (!dirName.isEmpty())
    {
        HRootDir::ScanMode smode = (m_ui->scanRecursivelyCheckbox->checkState() == Qt::Checked) ?
                                   HRootDir::RecursiveScan : HRootDir::SingleDirectoryScan;

        HRootDir rd(dirName, smode);

        addRootDirectoriesToServer(rd);
    }
}

void MediaServerWindow::setDeletedFlag()
{
    MediaServerWindow::deletedFlag = true;
}

void MediaServerWindow::addItemsToServer(const QStringList& fullPaths)
{
    if (!fullPaths.isEmpty())
    {
        foreach (const QString& fullPath, fullPaths)
        {
            QStringList parts = fullPath.split(QDir::separator(), QString::SkipEmptyParts);

            if (parts.isEmpty())
            {
                m_datasource->add(fullPath, QLatin1String("0"));
            }
            else
            {
                QString lastParentContainerId = QLatin1String("0");

                for (int i = 0 ; i < parts.count() - 1 ; ++i)
                {
                    HContainer* container = m_datasource->findContainerWithTitle(parts[i]);

                    if (!container)
                    {
                        container = new HContainer(parts[i], lastParentContainerId);
                        m_datasource->add(container);
                    }

                    lastParentContainerId = container->id();
                }

                m_datasource->add(parts.last(), lastParentContainerId);
            }

            int rc = m_ui->sharedItemsTable->rowCount();
            m_itemsMap.insert(rc,fullPath);
            m_ui->sharedItemsTable->insertRow(rc);

            QTableWidgetItem* const newItemScanType = new QTableWidgetItem(QLatin1String("No"));

            newItemScanType->setFlags(Qt::ItemIsEnabled);

            m_ui->sharedItemsTable->setItem(rc, 0, newItemScanType);

            QTableWidgetItem* const newItemWatchType = new QTableWidgetItem(QLatin1String("No"));
            newItemWatchType->setFlags(Qt::ItemIsEnabled);

            m_ui->sharedItemsTable->setItem(rc, 1, newItemWatchType);

            QTableWidgetItem* const newItem = new QTableWidgetItem(fullPath);

            newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            m_ui->sharedItemsTable->setItem(rc, 2, newItem);
        }
    }
}

void MediaServerWindow::on_DeleteDirectoriesButton_clicked()
{
    QModelIndexList indexes = m_ui->sharedItemsTable->selectionModel()->selectedRows();
    int countRow            = indexes.count();

    for (int i = countRow ; i > 0 ; i--)
    {
        if ((m_rootDirectoriesMap).contains(indexes[i-1].row()))
            m_rootDirectoriesMap.remove(indexes[i-1].row());
        else
            m_itemsMap.remove(indexes[i-1].row());

        m_ui->sharedItemsTable->removeRow(indexes[i-1].row());
    }
}

// -- Pure GUI implementations ----------------------------------------------------------

void MediaServerWindow::on_addItemButton_clicked()
{
    QStringList fullPaths = QFileDialog::getOpenFileNames(this, QLatin1String("Open File(s)"));
    addItemsToServer(fullPaths);
}

void MediaServerWindow::on_HideWindowButton_clicked()
{
    this->hide();
}

void MediaServerWindow::changeEvent(QEvent* e)
{
    QMainWindow::changeEvent(e);

    switch (e->type())
    {
        case QEvent::LanguageChange:
            m_ui->retranslateUi(this);
            break;
        default:
            break;
    }
}

void MediaServerWindow::addContenDlgtFinished()
{
    m_ui->addContentButton->setEnabled(true);
}

void MediaServerWindow::closeEvent(QCloseEvent*)
{
    emit closed();
}
