/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-05-20
 * Description : a tool to export images to Onedrive web service
 *
 * Copyright (C) 2018      by Tarek Talaat <tarektalaat93 at gmail dot com>
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

#include "odwindow.h"

// Qt includes

#include <QPointer>
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
#include "ditemslist.h"
#include "digikam_version.h"
#include "odtalker.h"
#include "oditem.h"
#include "odnewalbumdlg.h"
#include "odwidget.h"

namespace DigikamGenericOneDrivePlugin
{

class Q_DECL_HIDDEN ODWindow::Private
{
public:

    explicit Private()
    {
        imagesCount = 0;
        imagesTotal = 0;
        widget      = nullptr;
        albumDlg    = nullptr;
        talker      = nullptr;
    }

    unsigned int   imagesCount;
    unsigned int   imagesTotal;

    ODWidget*      widget;
    ODNewAlbumDlg* albumDlg;
    ODTalker*      talker;

    QString        currentAlbumName;
    QList<QUrl>    transferQueue;
};

ODWindow::ODWindow(DInfoInterface* const iface,
                   QWidget* const /*parent*/)
    : WSToolDialog(nullptr, QLatin1String("Onedrive Export Dialog")),
      d(new Private)
{
    d->widget = new ODWidget(this, iface, QLatin1String("Onedrive"));

    d->widget->imagesList()->setIface(iface);

    setMainWidget(d->widget);
    setModal(false);
    setWindowTitle(i18n("Export to Onedrive"));

    startButton()->setText(i18n("Start Upload"));
    startButton()->setToolTip(i18n("Start upload to Onedrive"));

    d->widget->setMinimumSize(700, 500);

    connect(d->widget->imagesList(), SIGNAL(signalImageListChanged()),
            this, SLOT(slotImageListChanged()));

    connect(d->widget->getChangeUserBtn(), SIGNAL(clicked()),
            this, SLOT(slotUserChangeRequest()));

    connect(d->widget->getNewAlbmBtn(), SIGNAL(clicked()),
            this, SLOT(slotNewAlbumRequest()));

    connect(d->widget->getReloadBtn(), SIGNAL(clicked()),
            this, SLOT(slotReloadAlbumsRequest()));

    connect(startButton(), SIGNAL(clicked()),
            this, SLOT(slotStartTransfer()));

    d->albumDlg = new ODNewAlbumDlg(this, QLatin1String("Onedrive"));
    d->talker   = new ODTalker(this);

    connect(d->talker,SIGNAL(signalBusy(bool)),
            this,SLOT(slotBusy(bool)));

    connect(d->talker,SIGNAL(signalLinkingFailed()),
            this,SLOT(slotSignalLinkingFailed()));

    connect(d->talker,SIGNAL(signalLinkingSucceeded()),
            this,SLOT(slotSignalLinkingSucceeded()));

    connect(d->talker,SIGNAL(signalSetUserName(QString)),
            this,SLOT(slotSetUserName(QString)));

    connect(d->talker,SIGNAL(signalListAlbumsFailed(QString)),
            this,SLOT(slotListAlbumsFailed(QString)));

    connect(d->talker,SIGNAL(signalListAlbumsDone(QList<QPair<QString,QString> >)), // krazy:exclude=normalize
            this,SLOT(slotListAlbumsDone(QList<QPair<QString,QString> >)));         // krazy:exclude=normalize

    connect(d->talker,SIGNAL(signalCreateFolderFailed(QString)),
            this,SLOT(slotCreateFolderFailed(QString)));

    connect(d->talker,SIGNAL(signalCreateFolderSucceeded()),
            this,SLOT(slotCreateFolderSucceeded()));

    connect(d->talker,SIGNAL(signalAddPhotoFailed(QString)),
            this,SLOT(slotAddPhotoFailed(QString)));

    connect(d->talker,SIGNAL(signalAddPhotoSucceeded()),
            this,SLOT(slotAddPhotoSucceeded()));

    connect(this, SIGNAL(finished(int)),
            this, SLOT(slotFinished()));

    readSettings();
    buttonStateChange(false);

    d->talker->readSettings();
}

ODWindow::~ODWindow()
{
    delete d->widget;
    delete d->albumDlg;
    delete d->talker;
    delete d;
}

void ODWindow::readSettings()
{
    KConfig config;
    KConfigGroup grp   = config.group("Onedrive Settings");
    d->currentAlbumName = grp.readEntry("Current Album",QString());

    if (grp.readEntry("Resize", false))
    {
        d->widget->getResizeCheckBox()->setChecked(true);
        d->widget->getDimensionSpB()->setEnabled(true);
    }
    else
    {
        d->widget->getResizeCheckBox()->setChecked(false);
        d->widget->getDimensionSpB()->setEnabled(false);
    }

    d->widget->getDimensionSpB()->setValue(grp.readEntry("Maximum Width",  1600));
    d->widget->getImgQualitySpB()->setValue(grp.readEntry("Image Quality", 90));

    KConfigGroup dialogGroup = config.group("Onedrive Export Dialog");

    winId();
    KWindowConfig::restoreWindowSize(windowHandle(), dialogGroup);
    resize(windowHandle()->size());
}

void ODWindow::writeSettings()
{
    KConfig config;
    KConfigGroup grp = config.group("Onedrive Settings");

    grp.writeEntry("Current Album", d->currentAlbumName);
    grp.writeEntry("Resize",        d->widget->getResizeCheckBox()->isChecked());
    grp.writeEntry("Maximum Width", d->widget->getDimensionSpB()->value());
    grp.writeEntry("Image Quality", d->widget->getImgQualitySpB()->value());

    KConfigGroup dialogGroup = config.group("Onedrive Export Dialog");
    KWindowConfig::saveWindowSize(windowHandle(), dialogGroup);

    config.sync();
}

void ODWindow::reactivate()
{
    d->widget->imagesList()->loadImagesFromCurrentSelection();
    d->widget->progressBar()->hide();

    show();
}

void ODWindow::setItemsList(const QList<QUrl>& urls)
{
    d->widget->imagesList()->slotAddImages(urls);
}

void ODWindow::slotBusy(bool val)
{
    if (val)
    {
        setCursor(Qt::WaitCursor);
        d->widget->getChangeUserBtn()->setEnabled(false);
        buttonStateChange(false);
    }
    else
    {
        setCursor(Qt::ArrowCursor);
        d->widget->getChangeUserBtn()->setEnabled(true);
        buttonStateChange(true);
    }
}

void ODWindow::slotSetUserName(const QString& msg)
{
    d->widget->updateLabels(msg, QLatin1String(""));
}

void ODWindow::slotListAlbumsDone(const QList<QPair<QString,QString> >& list)
{
    d->widget->getAlbumsCoB()->clear();

    for (int i = 0 ; i < list.size() ; ++i)
    {
        d->widget->getAlbumsCoB()->addItem(
        QIcon::fromTheme(QLatin1String("system-users")),
        list.value(i).second, list.value(i).first);

        if (d->currentAlbumName == list.value(i).first)
        {
            d->widget->getAlbumsCoB()->setCurrentIndex(i);
        }
    }

    buttonStateChange(true);
    d->talker->getUserName();
}

void ODWindow::slotStartTransfer()
{
    d->widget->imagesList()->clearProcessedStatus();

    if (d->widget->imagesList()->imageUrls().isEmpty())
    {
        QMessageBox::critical(this, i18nc("@title:window", "Error"),
                              i18n("No image selected. Please select which images should be uploaded."));
        return;
    }

    if (!(d->talker->authenticated()))
    {
        QPointer<QMessageBox> warn = new QMessageBox(QMessageBox::Warning,
                         i18n("Warning"),
                         i18n("Authentication failed. Click \"Continue\" to authenticate."),
                         QMessageBox::Yes | QMessageBox::No);

        (warn->button(QMessageBox::Yes))->setText(i18n("Continue"));
        (warn->button(QMessageBox::No))->setText(i18n("Cancel"));

        if (warn->exec() == QMessageBox::Yes)
        {
            d->talker->link();
            delete warn;
            return;
        }
        else
        {
            delete warn;
            return;
        }
    }

    d->transferQueue = d->widget->imagesList()->imageUrls();

    if (d->transferQueue.isEmpty())
    {
        return;
    }

    d->currentAlbumName = d->widget->getAlbumsCoB()->itemData(d->widget->getAlbumsCoB()->currentIndex()).toString();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "StartTransfer:" << d->currentAlbumName
                                     << "INDEX: " << d->widget->getAlbumsCoB()->currentIndex();
    d->imagesTotal = d->transferQueue.count();
    d->imagesCount = 0;

    d->widget->progressBar()->setFormat(i18n("%v / %m"));
    d->widget->progressBar()->setMaximum(d->imagesTotal);
    d->widget->progressBar()->setValue(0);
    d->widget->progressBar()->show();
    d->widget->progressBar()->progressScheduled(i18n("Onedrive export"), true, true);
    d->widget->progressBar()->progressThumbnailChanged(
        QIcon::fromTheme(QLatin1String("dk-onedrive")).pixmap(22, 22));

    uploadNextPhoto();
}

void ODWindow::uploadNextPhoto()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "uploadNextPhoto:" << d->transferQueue.count();

    if (d->transferQueue.isEmpty())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "empty";
        d->widget->progressBar()->progressCompleted();
        return;
    }

    QString imgPath = d->transferQueue.first().toLocalFile();
    QString temp = d->currentAlbumName + QLatin1Char('/');

    bool result = d->talker->addPhoto(imgPath,
                                      temp,
                                      d->widget->getResizeCheckBox()->isChecked(),
                                      d->widget->getDimensionSpB()->value(),
                                      d->widget->getImgQualitySpB()->value());

    if (!result)
    {
        slotAddPhotoFailed(QLatin1String(""));
        return;
    }
}

void ODWindow::slotAddPhotoFailed(const QString& msg)
{
    if (QMessageBox::question(this, i18n("Uploading Failed"),
                              i18n("Failed to upload photo to OneDrive."
                                   "\n%1\n"
                                   "Do you want to continue?", msg))
        != QMessageBox::Yes)
    {
        d->transferQueue.clear();
        d->widget->progressBar()->hide();
    }
    else
    {
        d->transferQueue.pop_front();
        d->imagesTotal--;
        d->widget->progressBar()->setMaximum(d->imagesTotal);
        d->widget->progressBar()->setValue(d->imagesCount);
        uploadNextPhoto();
    }
}

void ODWindow::slotAddPhotoSucceeded()
{
    // Remove photo uploaded from the list
    d->widget->imagesList()->removeItemByUrl(d->transferQueue.first());
    d->transferQueue.pop_front();
    d->imagesCount++;
    d->widget->progressBar()->setMaximum(d->imagesTotal);
    d->widget->progressBar()->setValue(d->imagesCount);
    uploadNextPhoto();
}

void ODWindow::slotImageListChanged()
{
    startButton()->setEnabled(!(d->widget->imagesList()->imageUrls().isEmpty()));
}

void ODWindow::slotNewAlbumRequest()
{
    if (d->albumDlg->exec() == QDialog::Accepted)
    {
        ODFolder newFolder;
        d->albumDlg->getFolderTitle(newFolder);
        d->currentAlbumName = d->widget->getAlbumsCoB()->itemData(d->widget->getAlbumsCoB()->currentIndex()).toString();
        QString temp        = d->currentAlbumName + newFolder.title;
        d->talker->createFolder(temp);
    }
}

void ODWindow::slotReloadAlbumsRequest()
{
    d->talker->listFolders();
}

void ODWindow::slotSignalLinkingFailed()
{
    slotSetUserName(QLatin1String(""));
    d->widget->getAlbumsCoB()->clear();

    if (QMessageBox::question(this, i18n("Login Failed"),
                              i18n("Authentication failed. Do you want to try again?"))
        == QMessageBox::Yes)
    {
        d->talker->link();
    }
}

void ODWindow::slotSignalLinkingSucceeded()
{
    d->talker->listFolders();
}

void ODWindow::slotListAlbumsFailed(const QString& msg)
{
    QMessageBox::critical(this, QString(), i18n("Onedrive call failed:\n%1", msg));
}

void ODWindow::slotCreateFolderFailed(const QString& msg)
{
    QMessageBox::critical(this, QString(), i18n("Onedrive call failed:\n%1", msg));
}

void ODWindow::slotCreateFolderSucceeded()
{
    d->talker->listFolders();
}

void ODWindow::slotTransferCancel()
{
    d->transferQueue.clear();
    d->widget->progressBar()->hide();
    d->talker->cancel();
}

void ODWindow::slotUserChangeRequest()
{
    slotSetUserName(QLatin1String(""));
    d->widget->getAlbumsCoB()->clear();
    d->talker->unLink();
    d->talker->link();
}

void ODWindow::buttonStateChange(bool state)
{
    d->widget->getNewAlbmBtn()->setEnabled(state);
    d->widget->getReloadBtn()->setEnabled(state);
    startButton()->setEnabled(state);
}

void ODWindow::slotFinished()
{
    writeSettings();
    d->widget->imagesList()->listView()->clear();
}

void ODWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    slotFinished();
    e->accept();
}

} // namespace DigikamGenericOneDrivePlugin
