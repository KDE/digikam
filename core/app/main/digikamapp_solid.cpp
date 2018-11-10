/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : main digiKam interface implementation - Solid methods
 *
 * Copyright (C) 2002-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "digikamapp.h"
#include "digikamapp_p.h"

// Solid includes

#include <solid/camera.h>
#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/devicenotifier.h>
#include <solid/predicate.h>
#include <solid/storageaccess.h>
#include <solid/storagedrive.h>
#include <solid/storagevolume.h>

namespace Digikam
{

void DigikamApp::fillSolidMenus()
{
    QHash<QString, QDateTime> newAppearanceTimes;
    d->usbMediaMenu->clear();
    d->cardReaderMenu->clear();

    // delete the actionGroups to avoid duplicate menu entries
    delete d->solidUsmActionGroup;
    delete d->solidCameraActionGroup;

    d->solidCameraActionGroup = new QActionGroup(this);

    connect(d->solidCameraActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(slotOpenSolidCamera(QAction*)));

    d->solidUsmActionGroup = new QActionGroup(this);

    connect(d->solidUsmActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(slotOpenSolidUsmDevice(QAction*)));

    // --------------------------------------------------------

    QList<Solid::Device> cameraDevices = Solid::Device::listFromType(Solid::DeviceInterface::Camera);

    foreach(const Solid::Device& cameraDevice, cameraDevices)
    {
        // USM camera: will be handled below
        if (cameraDevice.is<Solid::StorageAccess>())
        {
            continue;
        }

        if (!checkSolidCamera(cameraDevice))
        {
            continue;
        }

        // --------------------------------------------------------

        QString l     = labelForSolidCamera(cameraDevice);
        QString label = CameraNameHelper::cameraNameAutoDetected(l.trimmed());

        // --------------------------------------------------------

        QString iconName = cameraDevice.icon();

        if (iconName.isEmpty())
        {
            iconName = QLatin1String("camera-photo");
        }

        QAction* const action = new QAction(label, d->solidCameraActionGroup);

        action->setIcon(QIcon::fromTheme(iconName));
        // set data to identify device in action slot slotSolidSetupDevice
        action->setData(cameraDevice.udi());
        newAppearanceTimes[cameraDevice.udi()] = d->cameraAppearanceTimes.contains(cameraDevice.udi()) ?
                                                 d->cameraAppearanceTimes.value(cameraDevice.udi())    :
                                                 QDateTime::currentDateTime();

        d->cameraMenu->addAction(action);
    }

    QList<Solid::Device> storageDevices = Solid::Device::listFromType(Solid::DeviceInterface::StorageAccess);

    foreach(const Solid::Device& accessDevice, storageDevices)
    {
        // check for StorageAccess
        if (!accessDevice.is<Solid::StorageAccess>())
        {
            continue;
        }

        // check for StorageDrive
        Solid::Device driveDevice;

        for (Solid::Device currentDevice = accessDevice ;
             currentDevice.isValid() ;
             currentDevice = currentDevice.parent())
        {
            if (currentDevice.is<Solid::StorageDrive>())
            {
                driveDevice = currentDevice;
                break;
            }
        }

        if (!driveDevice.isValid())
        {
            continue;
        }

        const Solid::StorageDrive* const drive = driveDevice.as<Solid::StorageDrive>();

        QString driveType;

        bool isHarddisk = false;

        switch (drive->driveType())
        {
                // skip these
            case Solid::StorageDrive::CdromDrive:
            case Solid::StorageDrive::Floppy:
            case Solid::StorageDrive::Tape:
            default:
                continue;
                // accept card readers
            case Solid::StorageDrive::CompactFlash:
                driveType = i18n("CompactFlash Card Reader");
                break;
            case Solid::StorageDrive::MemoryStick:
                driveType = i18n("Memory Stick Reader");
                break;
            case Solid::StorageDrive::SmartMedia:
                driveType = i18n("SmartMedia Card Reader");
                break;
            case Solid::StorageDrive::SdMmc:
                driveType = i18n("SD / MMC Card Reader");
                break;
            case Solid::StorageDrive::Xd:
                driveType = i18n("xD Card Reader");
                break;
            case Solid::StorageDrive::HardDisk:

                // We don't want to list HardDisk partitions, but USB Mass Storage devices.
                // Don't know what is the exact difference between removable and hotpluggable.
                if (drive->isRemovable() || drive->isHotpluggable())
                {
                    isHarddisk = true;

                    if (drive->bus() == Solid::StorageDrive::Usb)
                    {
                        driveType = i18n("USB Disk");
                    }
                    else
                    {
                        driveType = i18nc("non-USB removable storage device", "Disk");
                    }

                    break;
                }
                else
                {
                    continue;
                }
        }

        // check for StorageVolume
        Solid::Device volumeDevice;

        for (Solid::Device currentDevice = accessDevice ;
             currentDevice.isValid() ;
             currentDevice = currentDevice.parent())
        {
            if (currentDevice.is<Solid::StorageVolume>())
            {
                volumeDevice = currentDevice;
                break;
            }
        }

        if (!volumeDevice.isValid())
        {
            continue;
        }

        bool isCamera                            = accessDevice.is<Solid::Camera>();
        const Solid::StorageAccess* const access = accessDevice.as<Solid::StorageAccess>();
        const Solid::StorageVolume* const volume = volumeDevice.as<Solid::StorageVolume>();

        if (volume->isIgnored())
        {
            continue;
        }

        QString label;

        if (isCamera)
        {
            label = accessDevice.vendor() + QLatin1Char(' ') + accessDevice.product();
        }
        else
        {
            QString labelOrProduct;

            if (!volume->label().isEmpty())
            {
                labelOrProduct = volume->label();
            }
            else if (!volumeDevice.product().isEmpty())
            {
                labelOrProduct = volumeDevice.product();
            }
            else if (!volumeDevice.vendor().isEmpty())
            {
                labelOrProduct = volumeDevice.vendor();
            }
            else if (!driveDevice.product().isEmpty())
            {
                labelOrProduct = driveDevice.product();
            }

            if (!labelOrProduct.isNull())
            {
                if (!access->filePath().isEmpty())
                {
                    label += i18nc("<drive type> \"<device name or label>\" at <mount path>",
                                   "%1 \"%2\" at %3", driveType, labelOrProduct,
                                   QDir::toNativeSeparators(access->filePath()));
                }
                else
                {
                    label += i18nc("<drive type> \"<device name or label>\"",
                                   "%1 \"%2\"", driveType, labelOrProduct);
                }
            }
            else
            {
                if (!access->filePath().isEmpty())
                {
                    label += i18nc("<drive type> at <mount path>",
                                   "%1 at %2", driveType,
                                   QDir::toNativeSeparators(access->filePath()));
                }
                else
                {
                    label += driveType;
                }
            }

            if (volume->size())
            {
                label += i18nc("device label etc... (<formatted byte size>)",
                               " (%1)", ItemPropertiesTab::humanReadableBytesCount(volume->size()));
            }
        }

        QString iconName;

        if (!driveDevice.icon().isEmpty())
        {
            iconName = driveDevice.icon();
        }
        else if (!accessDevice.icon().isEmpty())
        {
            iconName = accessDevice.icon();
        }
        else if (!volumeDevice.icon().isEmpty())
        {
            iconName = volumeDevice.icon();
        }

        QAction* const action = new QAction(label, d->solidUsmActionGroup);

        if (!iconName.isEmpty())
        {
            action->setIcon(QIcon::fromTheme(iconName));
        }

        // set data to identify device in action slot slotSolidSetupDevice
        action->setData(accessDevice.udi());
        newAppearanceTimes[accessDevice.udi()] = d->cameraAppearanceTimes.contains(accessDevice.udi()) ?
                                                 d->cameraAppearanceTimes.value(accessDevice.udi())    :
                                                 QDateTime::currentDateTime();

        if (isCamera)
        {
            d->cameraMenu->addAction(action);
        }

        if (isHarddisk)
        {
            d->usbMediaMenu->addAction(action);
        }
        else
        {
            d->cardReaderMenu->addAction(action);
        }
    }

/*
    //TODO: Find best usable solution when no devices are connected: One entry, hide, or disable?

    // Add one entry telling that no device is available
    if (d->cameraSolidMenu->isEmpty())
    {
        QAction* const action = d->cameraSolidMenu->addAction(i18n("No Camera Connected"));
        action->setEnabled(false);
    }

    if (d->usbMediaMenu->isEmpty())
    {
        QAction* const action = d->usbMediaMenu->addAction(i18n("No Storage Devices Found"));
        action->setEnabled(false);
    }

    if (d->cardReaderMenu->isEmpty())
    {
        QAction* const action = d->cardReaderMenu->addAction(i18n("No Card Readers Available"));
        action->setEnabled(false);
    }

    // hide empty menus
    d->cameraSolidMenu->menuAction()->setVisible(!d->cameraSolidMenu->isEmpty());
    d->usbMediaMenu->menuAction()->setVisible(!d->usbMediaMenu->isEmpty());
    d->cardReaderMenu->menuAction()->setVisible(!d->cardReaderMenu->isEmpty());
*/

    d->cameraAppearanceTimes = newAppearanceTimes;

    // disable empty menus
    d->usbMediaMenu->setEnabled(!d->usbMediaMenu->isEmpty());
    d->cardReaderMenu->setEnabled(!d->cardReaderMenu->isEmpty());

    updateCameraMenu();
    updateQuickImportAction();

    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(QString)),
            this, SLOT(slotSolidDeviceChanged(QString)));

    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(QString)),
            this, SLOT(slotSolidDeviceChanged(QString)));
}

bool DigikamApp::checkSolidCamera(const Solid::Device& cameraDevice)
{
    const Solid::Camera* const camera = cameraDevice.as<Solid::Camera>();

    if (!camera)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Solid device" << cameraDevice.description() << "is not a camera";
        return false;
    }

    QStringList drivers = camera->supportedDrivers();

    qCDebug(DIGIKAM_GENERAL_LOG) << "checkSolidCamera: Found Camera "
                                 << QString::fromUtf8("%1 %2").arg(cameraDevice.vendor()).arg(cameraDevice.product())
                                 << " protocols " << camera->supportedProtocols()
                                 << " drivers " << camera->supportedDrivers(QLatin1String("ptp"));

    // We handle gphoto2 cameras in this loop
    if (!(camera->supportedDrivers().contains(QLatin1String("gphoto")) ||
        camera->supportedProtocols().contains(QLatin1String("ptp")))
       )
    {
        return false;
    }

    QVariant driverHandle = camera->driverHandle(QLatin1String("gphoto"));

    if (!driverHandle.canConvert(QVariant::List))
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Solid returns unsupported driver handle for gphoto2";
        return false;
    }

    QList<QVariant> driverHandleList = driverHandle.toList();

    if ((driverHandleList.size() < 3)                               ||
        (driverHandleList.at(0).toString() != QLatin1String("usb")) ||
        !driverHandleList.at(1).canConvert(QVariant::Int)           ||
        !driverHandleList.at(2).canConvert(QVariant::Int)
       )
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Solid returns unsupported driver handle for gphoto2";
        return false;
    }

    return true;
}

void DigikamApp::openSolidCamera(const QString& udi, const QString& cameraLabel)
{
    // if there is already an open ImportUI for the device, show and raise it, and be done
    if (d->cameraUIMap.contains(udi))
    {
        ImportUI* const ui = d->cameraUIMap.value(udi);

        if (ui && !ui->isClosed())
        {
            if (ui->isMinimized())
            {
                KWindowSystem::unminimizeWindow(ui->winId());
            }

            KWindowSystem::activateWindow(ui->winId());
            return;
        }
    }

    // recreate device from unambiguous UDI
    Solid::Device device(udi);

    if ( device.isValid() )
    {
        if (cameraLabel.isNull())
        {
            QString label = labelForSolidCamera(device);
        }

        Solid::Camera* const camera = device.as<Solid::Camera>();
        QList<QVariant> list        = camera->driverHandle(QLatin1String("gphoto")).toList();

        // all sanity checks have already been done when creating the action
        if (list.size() < 3)
        {
            return;
        }

        // NOTE: See bug #262296: With KDE 4.6, Solid API return device vendor id
        // and product id in hexadecimal strings.
        bool ok;
        int vendorId  = list.at(1).toString().toInt(&ok, 16);
        int productId = list.at(2).toString().toInt(&ok, 16);
        QString model, port;

        if (CameraList::findConnectedCamera(vendorId, productId, model, port))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Found camera from ids " << vendorId << " " << productId
                                         << " camera is: " << model << " at " << port;

            // the ImportUI will delete itself when it has finished
            ImportUI* const cgui = new ImportUI(cameraLabel, model, port, QLatin1String("/"), 1);
            d->cameraUIMap[udi]  = cgui;

            cgui->show();

            connect(cgui, SIGNAL(signalLastDestination(QUrl)),
                    d->view, SLOT(slotSelectAlbum(QUrl)));
        }
        else
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Failed to detect camera with GPhoto2 from Solid information";
        }
    }
}

QString DigikamApp::labelForSolidCamera(const Solid::Device& cameraDevice)
{
    QString vendor  = cameraDevice.vendor();
    QString product = cameraDevice.product();

    if (product == QLatin1String("USB Imaging Interface") ||
        product == QLatin1String("USB Vendor Specific Interface"))
    {
        Solid::Device parentUsbDevice = cameraDevice.parent();

        if (parentUsbDevice.isValid())
        {
            vendor  = parentUsbDevice.vendor();
            product = parentUsbDevice.product();

            if (!vendor.isEmpty() && !product.isEmpty())
            {
                if (vendor == QLatin1String("Canon, Inc."))
                {
                    vendor = QLatin1String("Canon");

                    if (product.startsWith(QLatin1String("Canon ")))
                    {
                        product = product.mid(6);    // cut off another "Canon " from product
                    }

                    if (product.endsWith(QLatin1String(" (ptp)")))
                    {
                        product.chop(6);             // cut off " (ptp)"
                    }
                }
                else if (vendor == QLatin1String("Fuji Photo Film Co., Ltd"))
                {
                    vendor = QLatin1String("Fuji");
                }
                else if (vendor == QLatin1String("Nikon Corp."))
                {
                    vendor = QLatin1String("Nikon");

                    if (product.startsWith(QLatin1String("NIKON ")))
                    {
                        product = product.mid(6);
                    }
                }
            }
        }
    }

    return vendor + QLatin1Char(' ') + product;
}

void DigikamApp::openSolidUsmDevice(const QString& udi, const QString& givenLabel)
{
    QString mediaLabel = givenLabel;

    // if there is already an open ImportUI for the device, show and raise it
    if (d->cameraUIMap.contains(udi))
    {
        ImportUI* const ui = d->cameraUIMap.value(udi);

        if (ui && !ui->isClosed())
        {
            if (ui->isMinimized())
            {
                KWindowSystem::unminimizeWindow(ui->winId());
            }

            KWindowSystem::activateWindow(ui->winId());
            return;
        }
    }

    // recreate device from unambiguous UDI
    Solid::Device device(udi);

    if ( device.isValid() )
    {
        Solid::StorageAccess* const access = device.as<Solid::StorageAccess>();

        if (!access)
        {
            return;
        }

        if (!access->isAccessible())
        {
            QApplication::setOverrideCursor(Qt::WaitCursor);

            if (!access->setup())
            {
                return;
            }

            d->eventLoop = new QEventLoop(this);

            connect(access, SIGNAL(setupDone(Solid::ErrorType,QVariant,QString)),
                    this, SLOT(slotSolidSetupDone(Solid::ErrorType,QVariant,QString)));

            int returnCode = d->eventLoop->exec(QEventLoop::ExcludeUserInputEvents);

            delete d->eventLoop;
            d->eventLoop = 0;
            QApplication::restoreOverrideCursor();

            if (returnCode == 1)
            {
                QMessageBox::critical(this, qApp->applicationName(), d->solidErrorMessage);
                return;
            }
        }

        // Create Camera UI

        QString path = QDir::fromNativeSeparators(access->filePath());

        if (mediaLabel.isNull())
        {
            mediaLabel = path;
        }

        // the ImportUI will delete itself when it has finished
        ImportUI* const cgui = new ImportUI(i18n("Images on %1", mediaLabel),
                                            QLatin1String("directory browse"),
                                            QLatin1String("Fixed"), path, 1);
        d->cameraUIMap[udi]  = cgui;

        cgui->show();

        connect(cgui, SIGNAL(signalLastDestination(QUrl)),
                d->view, SLOT(slotSelectAlbum(QUrl)));
    }
}

void DigikamApp::slotOpenSolidCamera(QAction* action)
{
    QString udi = action->data().toString();
    openSolidCamera(udi, action->iconText());
}

void DigikamApp::slotOpenSolidUsmDevice(QAction* action)
{
    QString udi = action->data().toString();
    openSolidUsmDevice(udi, action->iconText());
}

void DigikamApp::slotOpenSolidDevice(const QString& udi)
{
    // Identifies device as either Camera or StorageAccess and calls methods accordingly

    Solid::Device device(udi);

    if (!device.isValid())
    {
        QMessageBox::critical(this, qApp->applicationName(),
                              i18n("The specified device (\"%1\") is not valid.", udi));
        return;
    }

    if (device.is<Solid::StorageAccess>())
    {
        openSolidUsmDevice(udi);
    }
    else if (device.is<Solid::Camera>())
    {
        if (!checkSolidCamera(device))
        {
            QMessageBox::critical(this, qApp->applicationName(),
                                  i18n("The specified camera (\"%1\") is not supported.", udi));
            return;
        }

        openSolidCamera(udi);
    }
}

void DigikamApp::slotSolidSetupDone(Solid::ErrorType errorType, QVariant errorData, const QString& /*udi*/)
{
    if (!d->eventLoop)
    {
        return;
    }

    if (errorType == Solid::NoError)
    {
        d->eventLoop->exit(0);
    }
    else
    {
        d->solidErrorMessage  = i18n("Cannot access the storage device.\n");
        d->solidErrorMessage += errorData.toString();
        d->eventLoop->exit(1);
    }
}

void DigikamApp::slotSolidDeviceChanged(const QString& udi)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "slotSolidDeviceChanged:" << udi;
    fillSolidMenus();
}

} // namespace Digikam
