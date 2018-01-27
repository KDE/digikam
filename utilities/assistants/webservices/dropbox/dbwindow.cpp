/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-11-18
 * Description : a tool to export images to Dropbox web service
 *
 * Copyright (C) 2013 by Pankaj Kumar <me at panks dot me>
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

#include "dbwindow.h"

// Qt includes

#include <QWindow>
#include <QSpinBox>
#include <QCheckBox>
#include <QMessageBox>
#include <QCloseEvent>

// KDE includes

#include <klocalizedstring.h>
#include <kconfig.h>
#include <kwindowconfig.h>

// Local includes

#include "digikam_debug.h"
#include "dimageslist.h"
#include "digikam_version.h"
#include "dbtalker.h"
#include "dbitem.h"
#include "dbalbum.h"
#include "dbwidget.h"

namespace Digikam
{

DBWindow::DBWindow(DInfoInterface* const iface, QWidget* const /*parent*/)
    : ToolDialog(0)
{
    m_imagesCount = 0;
    m_imagesTotal = 0;

    m_widget      = new DropboxWidget(this, QLatin1String("Dropbox"));
    m_widget->imagesList()->setIface(iface);

    setMainWidget(m_widget);
    setWindowIcon(QIcon::fromTheme(QLatin1String("dropbox")));
    setModal(false);
    setWindowTitle(i18n("Export to Dropbox"));

    startButton()->setText(i18n("Start Upload"));
    startButton()->setToolTip(i18n("Start upload to Dropbox"));

    m_widget->setMinimumSize(700, 500);

    connect(m_widget->imagesList(), SIGNAL(signalImageListChanged()),
            this, SLOT(slotImageListChanged()));

    connect(m_widget->getChangeUserBtn(), SIGNAL(clicked()),
            this, SLOT(slotUserChangeRequest()));

    connect(m_widget->getNewAlbmBtn(), SIGNAL(clicked()),
            this, SLOT(slotNewAlbumRequest()));

    connect(m_widget->getReloadBtn(), SIGNAL(clicked()),
            this, SLOT(slotReloadAlbumsRequest()));

    connect(startButton(), SIGNAL(clicked()),
            this, SLOT(slotStartTransfer()));

    m_albumDlg = new DBNewAlbum(this, QLatin1String("Dropbox"));

    m_talker   = new DBTalker(this);

    connect(m_talker,SIGNAL(signalBusy(bool)),
            this,SLOT(slotBusy(bool)));

    connect(m_talker,SIGNAL(signalLinkingFailed()),
            this,SLOT(slotSignalLinkingFailed()));

    connect(m_talker,SIGNAL(signalLinkingSucceeded()),
            this,SLOT(slotSignalLinkingSucceeded()));

    connect(m_talker,SIGNAL(signalSetUserName(QString)),
            this,SLOT(slotSetUserName(QString)));

    connect(m_talker,SIGNAL(signalListAlbumsFailed(QString)),
            this,SLOT(slotListAlbumsFailed(QString)));

    connect(m_talker,SIGNAL(signalListAlbumsDone(QList<QPair<QString,QString> >)),
            this,SLOT(slotListAlbumsDone(QList<QPair<QString,QString> >)));

    connect(m_talker,SIGNAL(signalCreateFolderFailed(QString)),
            this,SLOT(slotCreateFolderFailed(QString)));

    connect(m_talker,SIGNAL(signalCreateFolderSucceeded()),
            this,SLOT(slotCreateFolderSucceeded()));

    connect(m_talker,SIGNAL(signalAddPhotoFailed(QString)),
            this,SLOT(slotAddPhotoFailed(QString)));

    connect(m_talker,SIGNAL(signalAddPhotoSucceeded()),
            this,SLOT(slotAddPhotoSucceeded()));

    connect(this, SIGNAL(finished(int)),
            this, SLOT(slotFinished()));

    readSettings();
    buttonStateChange(false);

    m_talker->link();
}

DBWindow::~DBWindow()
{
    delete m_widget;
    delete m_albumDlg;
    delete m_talker;
}

void DBWindow::setItemsList(const QList<QUrl>& urls)
{
    m_widget->imagesList()->slotAddImages(urls);
}

void DBWindow::reactivate()
{
    m_widget->imagesList()->loadImagesFromCurrentSelection();
    m_widget->progressBar()->hide();

    show();
}

void DBWindow::readSettings()
{
    KConfig config;
    KConfigGroup grp   = config.group("Dropbox Settings");

    m_currentAlbumName = grp.readEntry("Current Album",QString());

    if (grp.readEntry("Resize", false))
    {
        m_widget->getResizeCheckBox()->setChecked(true);
        m_widget->getDimensionSpB()->setEnabled(true);
        m_widget->getImgQualitySpB()->setEnabled(true);
    }
    else
    {
        m_widget->getResizeCheckBox()->setChecked(false);
        m_widget->getDimensionSpB()->setEnabled(false);
        m_widget->getImgQualitySpB()->setEnabled(false);
    }

    m_widget->getDimensionSpB()->setValue(grp.readEntry("Maximum Width",  1600));
    m_widget->getImgQualitySpB()->setValue(grp.readEntry("Image Quality", 90));

    winId();
    KConfigGroup dialogGroup = config.group("Dropbox Export Dialog");
    KWindowConfig::restoreWindowSize(windowHandle(), dialogGroup);
    resize(windowHandle()->size());
}

void DBWindow::writeSettings()
{
    KConfig config;
    KConfigGroup grp = config.group("Dropbox Settings");

    grp.writeEntry("Current Album", m_currentAlbumName);
    grp.writeEntry("Resize",        m_widget->getResizeCheckBox()->isChecked());
    grp.writeEntry("Maximum Width", m_widget->getDimensionSpB()->value());
    grp.writeEntry("Image Quality", m_widget->getImgQualitySpB()->value());

    KConfigGroup dialogGroup = config.group("Dropbox Export Dialog");
    KWindowConfig::saveWindowSize(windowHandle(), dialogGroup);

    config.sync();
}

void DBWindow::slotSetUserName(const QString& msg)
{
    m_widget->updateLabels(msg, QLatin1String(""));
}

void DBWindow::slotListAlbumsDone(const QList<QPair<QString,QString> >& list)
{
    m_widget->getAlbumsCoB()->clear();
    qCDebug(DIGIKAM_GENERAL_LOG) << "slotListAlbumsDone:" << list.size();

    for (int i = 0 ; i < list.size() ; i++)
    {
        m_widget->getAlbumsCoB()->addItem(
        QIcon::fromTheme(QLatin1String("system-users")),
        list.value(i).second, list.value(i).first);

        if (m_currentAlbumName == list.value(i).first)
        {
            m_widget->getAlbumsCoB()->setCurrentIndex(i);
        }
    }

    buttonStateChange(true);
    m_talker->getUserName();
}

void DBWindow::slotBusy(bool val)
{
    if (val)
    {
        setCursor(Qt::WaitCursor);
        m_widget->getChangeUserBtn()->setEnabled(false);
        buttonStateChange(false);
    }
    else
    {
        setCursor(Qt::ArrowCursor);
        m_widget->getChangeUserBtn()->setEnabled(true);
        buttonStateChange(true);
    }
}

void DBWindow::slotStartTransfer()
{
    m_widget->imagesList()->clearProcessedStatus();

    if (m_widget->imagesList()->imageUrls().isEmpty())
    {
        QMessageBox::critical(this, i18nc("@title:window", "Error"),
                              i18n("No image selected. Please select which images should be uploaded."));
        return;
    }

    if (!(m_talker->authenticated()))
    {
        if (QMessageBox::question(this, i18n("Login Failed"),
                                  i18n("Authentication failed. Do you want to try again?"))
            == QMessageBox::Yes)
        {
            m_talker->link();
            return;
        }
        else
        {
            return;
        }
    }

    m_transferQueue = m_widget->imagesList()->imageUrls();

    if (m_transferQueue.isEmpty())
    {
        return;
    }

    m_currentAlbumName = m_widget->getAlbumsCoB()->itemData(m_widget->getAlbumsCoB()->currentIndex()).toString();

    m_imagesTotal = m_transferQueue.count();
    m_imagesCount = 0;

    m_widget->progressBar()->setFormat(i18n("%v / %m"));
    m_widget->progressBar()->setMaximum(m_imagesTotal);
    m_widget->progressBar()->setValue(0);
    m_widget->progressBar()->show();
    m_widget->progressBar()->progressScheduled(i18n("Dropbox export"), true, true);
    m_widget->progressBar()->progressThumbnailChanged(
        QIcon(QLatin1String(":/icons/kipi-icon.svg")).pixmap(22, 22));

    uploadNextPhoto();
}

void DBWindow::uploadNextPhoto()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "uploadNextPhoto:" << m_transferQueue.count();

    if (m_transferQueue.isEmpty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "empty";
        m_widget->progressBar()->progressCompleted();
        return;
    }

    QString imgPath = m_transferQueue.first().toLocalFile();
    QString temp = m_currentAlbumName + QLatin1String("/");

    bool res = m_talker->addPhoto(imgPath,temp,m_widget->getResizeCheckBox()->isChecked(),m_widget->getDimensionSpB()->value(),
                                  m_widget->getImgQualitySpB()->value());

    if (!res)
    {
        slotAddPhotoFailed(QLatin1String(""));
        return;
    }
}

void DBWindow::slotAddPhotoFailed(const QString& msg)
{
    if (QMessageBox::question(this, i18n("Uploading Failed"),
                              i18n("Failed to upload photo to Dropbox."
                                   "\n%1\n"
                                   "Do you want to continue?", msg))
        != QMessageBox::Yes)
    {
        m_transferQueue.clear();
        m_widget->progressBar()->hide();
    }
    else
    {
        m_transferQueue.pop_front();
        m_imagesTotal--;
        m_widget->progressBar()->setMaximum(m_imagesTotal);
        m_widget->progressBar()->setValue(m_imagesCount);
        uploadNextPhoto();
    }
}

void DBWindow::slotAddPhotoSucceeded()
{
    // Remove photo uploaded from the list
    m_widget->imagesList()->removeItemByUrl(m_transferQueue.first());
    m_transferQueue.pop_front();
    m_imagesCount++;
    m_widget->progressBar()->setMaximum(m_imagesTotal);
    m_widget->progressBar()->setValue(m_imagesCount);
    uploadNextPhoto();
}

void DBWindow::slotImageListChanged()
{
    startButton()->setEnabled(!(m_widget->imagesList()->imageUrls().isEmpty()));
}

void DBWindow::slotNewAlbumRequest()
{
    if (m_albumDlg->exec() == QDialog::Accepted)
    {
        DBFolder newFolder;
        m_albumDlg->getFolderTitle(newFolder);
        qCDebug(DIGIKAM_GENERAL_LOG) << "slotNewAlbumRequest:" << newFolder.title;
        m_currentAlbumName = m_widget->getAlbumsCoB()->itemData(m_widget->getAlbumsCoB()->currentIndex()).toString();
        QString temp = m_currentAlbumName + newFolder.title;
        m_talker->createFolder(temp);
    }
}

void DBWindow::slotReloadAlbumsRequest()
{
    m_talker->listFolders();
}

void DBWindow::slotSignalLinkingFailed()
{
    slotSetUserName(QLatin1String(""));
    m_widget->getAlbumsCoB()->clear();

    if (QMessageBox::question(this, i18n("Login Failed"),
                              i18n("Authentication failed. Do you want to try again?"))
        == QMessageBox::Yes)
    {
        m_talker->link();
    }
}

void DBWindow::slotSignalLinkingSucceeded()
{
    m_talker->listFolders();
}

void DBWindow::slotListAlbumsFailed(const QString& msg)
{
    QMessageBox::critical(this, QString(), i18n("Dropbox call failed:\n%1", msg));
}

void DBWindow::slotCreateFolderFailed(const QString& msg)
{
    QMessageBox::critical(this, QString(), i18n("Dropbox call failed:\n%1", msg));
}

void DBWindow::slotCreateFolderSucceeded()
{
    m_talker->listFolders();
}

void DBWindow::slotTransferCancel()
{
    m_transferQueue.clear();
    m_widget->progressBar()->hide();
    m_talker->cancel();
}

void DBWindow::slotUserChangeRequest()
{
    slotSetUserName(QLatin1String(""));
    m_widget->getAlbumsCoB()->clear();
    m_talker->unLink();
    m_talker->link();
}

void DBWindow::buttonStateChange(bool state)
{
    m_widget->getNewAlbmBtn()->setEnabled(state);
    m_widget->getReloadBtn()->setEnabled(state);
    startButton()->setEnabled(state);
}

void DBWindow::slotFinished()
{
    writeSettings();
    m_widget->imagesList()->listView()->clear();
}

void DBWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    slotFinished();
    e->accept();
}

} // namespace Digikam
