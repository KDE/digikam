/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-11-18
 * Description : a tool to export items to Google web services
 *
 * Copyright (C) 2013      by Pankaj Kumar <me at panks dot me>
 * Copyright (C) 2015      by Shourya Singh Gupta <shouryasgupta at gmail dot com>
 * Copyright (C) 2013-2018 by Caulier Gilles <caulier dot gilles at gmail dot com>
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

#include "gswindow.h"

// Qt includes

#include <QWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QButtonGroup>
#include <QProgressDialog>
#include <QPixmap>
#include <QCheckBox>
#include <QStringList>
#include <QSpinBox>
#include <QFileInfo>
#include <QPointer>
#include <QDesktopServices>
#include <QUrl>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kwindowconfig.h>

// Local includes

#include "wstoolutils.h"
#include "dimageslist.h"
#include "digikam_version.h"
#include "dprogresswdg.h"
#include "gdtalker.h"
#include "gsitem.h"
#include "gsnewalbumdlg.h"
#include "gswidget.h"
#include "gptalker.h"
#include "gsreplacedlg.h"
#include "digikam_debug.h"

namespace Digikam
{

class GSWindow::Private
{
public:

    explicit Private()
    {
        widget          = 0;
        albumDlg        = 0;
        gphotoAlbumDlg  = 0;
        talker          = 0;
        gphotoTalker    = 0;
        iface           = 0;
        imagesCount     = 0;
        imagesTotal     = 0;
        renamingOpt     = 0;
        service         = GoogleService::GPhotoImport;
    }

    unsigned int                  imagesCount;
    unsigned int                  imagesTotal;
    int                           renamingOpt;

    QString                       serviceName;
    QString                       toolName;
    GoogleService                 service;
    QString                       tmp;
    QString                       refreshToken;

    GSWidget*                     widget;
    GSNewAlbumDlg*                albumDlg;
    GSNewAlbumDlg*                gphotoAlbumDlg;

    GDTalker*                     talker;
    GPTalker*                     gphotoTalker;

    QString                       currentAlbumId;

    QList< QPair<QUrl, GSPhoto> > transferQueue;

    DInfoInterface*               iface;
    DMetadata                     meta;
};

GSWindow::GSWindow(DInfoInterface* const iface,
                   QWidget* const /*parent*/,
                   const QString& serviceName)
    : WSToolDialog(0),
      d(new Private)
{
    d->iface       = iface;
    d->serviceName = serviceName;

    if (QString::compare(d->serviceName, QString::fromLatin1("googledriveexport"),
        Qt::CaseInsensitive) == 0)
    {
        d->service  = GoogleService::GDrive;
        d->toolName = QString::fromLatin1("Google Drive");
    }
    else if (QString::compare(d->serviceName, QString::fromLatin1("googlephotoexport"),
             Qt::CaseInsensitive) == 0)
    {
        d->service  = GoogleService::GPhotoExport;
        d->toolName = QString::fromLatin1("Google Photos/PicasaWeb");
    }
    else
    {
        d->service  = GoogleService::GPhotoImport;
        d->toolName = QString::fromLatin1("Google Photos/PicasaWeb");
    }

    d->tmp         = WSToolUtils::makeTemporaryDir("google").absolutePath() + QLatin1Char('/');;
    d->widget      = new GSWidget(this, d->iface, d->service, d->toolName);

    setMainWidget(d->widget);
    setModal(false);

    switch (d->service)
    {
        case GoogleService::GDrive:

            setWindowTitle(i18n("Export to Google Drive"));

            startButton()->setText(i18n("Start Upload"));
            startButton()->setToolTip(i18n("Start upload to Google Drive"));

            d->widget->setMinimumSize(700,500);

            d->albumDlg = new GSNewAlbumDlg(this, d->serviceName, d->toolName);
            d->talker   = new GDTalker(this);

            connect(d->talker,SIGNAL(signalBusy(bool)),
                    this,SLOT(slotBusy(bool)));

            connect(d->talker,SIGNAL(signalTextBoxEmpty()),
                    this,SLOT(slotTextBoxEmpty()));

            connect(d->talker,SIGNAL(signalAccessTokenFailed(int, QString)),
                    this,SLOT(slotAccessTokenFailed(int, QString)));

            connect(d->talker,SIGNAL(signalAccessTokenObtained()),
                    this,SLOT(slotAccessTokenObtained()));

            connect(d->talker,SIGNAL(signalRefreshTokenObtained(QString)),
                    this,SLOT(slotRefreshTokenObtained(QString)));

            connect(d->talker,SIGNAL(signalSetUserName(QString)),
                    this,SLOT(slotSetUserName(QString)));

            connect(d->talker,SIGNAL(signalListAlbumsDone(int, QString, QList<GSFolder>)),
                    this,SLOT(slotListAlbumsDone(int, QString, QList<GSFolder>)));

            connect(d->talker,SIGNAL(signalCreateFolderDone(int, QString)),
                    this,SLOT(slotCreateFolderDone(int, QString)));

            connect(d->talker,SIGNAL(signalAddPhotoDone(int, QString, QString)),
                    this,SLOT(slotAddPhotoDone(int, QString, QString)));

            readSettings();
            buttonStateChange(false);

            if (d->refreshToken.isEmpty())
            {
                d->talker->doOAuth();
            }
            else
            {
                d->talker->getAccessTokenFromRefreshToken(d->refreshToken);
            }

            break;

        case GoogleService::GPhotoImport:
        case GoogleService::GPhotoExport:

            if (d->service == GoogleService::GPhotoExport)
            {
                setWindowTitle(i18n("Export to Google Photos/PicasaWeb Service"));

                startButton()->setText(i18n("Start Upload"));
                startButton()->setToolTip(i18n("Start upload to Google Photos/PicasaWeb Service"));

                d->widget->setMinimumSize(700, 500);
            }
            else
            {
                setWindowTitle(i18n("Import from Google Photos/PicasaWeb Service"));

                startButton()->setText(i18n("Start Download"));
                startButton()->setToolTip(i18n("Start download from Google Photos/PicasaWeb service"));

                d->widget->setMinimumSize(300, 400);
            }

            d->gphotoAlbumDlg = new GSNewAlbumDlg(this, d->serviceName, d->toolName);
            d->gphotoTalker   = new GPTalker(this);

            connect(d->gphotoTalker, SIGNAL(signalBusy(bool)),
                    this, SLOT(slotBusy(bool)));

            connect(d->gphotoTalker, SIGNAL(signalTextBoxEmpty()),
                    this, SLOT(slotTextBoxEmpty()));

            connect(d->gphotoTalker, SIGNAL(signalAccessTokenFailed(int, QString)),
                    this, SLOT(slotAccessTokenFailed(int, QString)));

            connect(d->gphotoTalker, SIGNAL(signalAccessTokenObtained()),
                    this, SLOT(slotAccessTokenObtained()));

            connect(d->gphotoTalker, SIGNAL(signalRefreshTokenObtained(QString)),
                    this, SLOT(slotRefreshTokenObtained(QString)));

            connect(d->gphotoTalker, SIGNAL(signalListAlbumsDone(int, QString, QList<GSFolder>)),
                    this, SLOT(slotListAlbumsDone(int, QString, QList<GSFolder>)));

            connect(d->gphotoTalker, SIGNAL(signalCreateAlbumDone(int, QString, QString)),
                    this, SLOT(slotCreateFolderDone(int, QString, QString)));

            connect(d->gphotoTalker, SIGNAL(signalAddPhotoDone(int, QString, QString)),
                    this, SLOT(slotAddPhotoDone(int,QString,QString)));

            connect(d->gphotoTalker, SIGNAL(signalGetPhotoDone(int, QString, QByteArray)),
                    this, SLOT(slotGetPhotoDone(int, QString, QByteArray)));

            readSettings();
            buttonStateChange(false);

            if (d->refreshToken.isEmpty())
            {
                d->gphotoTalker->doOAuth();
            }
            else
            {
                d->gphotoTalker->getAccessTokenFromRefreshToken(d->refreshToken);
            }

            break;
    }

    connect(d->widget->imagesList(), SIGNAL(signalImageListChanged()),
            this, SLOT(slotImageListChanged()));

    connect(d->widget->getChangeUserBtn(), SIGNAL(clicked()),
            this, SLOT(slotUserChangeRequest()));

    connect(d->widget->getNewAlbmBtn(), SIGNAL(clicked()),
            this,SLOT(slotNewAlbumRequest()));

    connect(d->widget->getReloadBtn(), SIGNAL(clicked()),
            this, SLOT(slotReloadAlbumsRequest()));

    connect(startButton(), SIGNAL(clicked()),
            this, SLOT(slotStartTransfer()));

    connect(this, SIGNAL(finished(int)),
            this, SLOT(slotFinished()));
}

GSWindow::~GSWindow()
{
    delete d->widget;
    delete d->albumDlg;
    delete d->gphotoAlbumDlg;
    delete d->talker;
    delete d->gphotoTalker;
    delete d;
}

void GSWindow::reactivate()
{
    d->widget->imagesList()->loadImagesFromCurrentSelection();
    d->widget->progressBar()->hide();

    show();
}

void GSWindow::readSettings()
{
    KConfig config;
    KConfigGroup grp;

    switch (d->service)
    {
        case GoogleService::GDrive:
            grp = config.group("Google Drive Settings");
            break;
        default:
            grp = config.group("Google Photo Settings");
            break;
    }

    d->currentAlbumId = grp.readEntry("Current Album",QString());
    d->refreshToken  = grp.readEntry("refresh_token");

    if (grp.readEntry("Resize", false))
    {
        d->widget->getResizeCheckBox()->setChecked(true);
        d->widget->getDimensionSpB()->setEnabled(true);
        d->widget->getImgQualitySpB()->setEnabled(true);
    }
    else
    {
        d->widget->getResizeCheckBox()->setChecked(false);
        d->widget->getDimensionSpB()->setEnabled(false);
        d->widget->getImgQualitySpB()->setEnabled(false);
    }

    d->widget->getDimensionSpB()->setValue(grp.readEntry("Maximum Width",  1600));
    d->widget->getImgQualitySpB()->setValue(grp.readEntry("Image Quality", 90));

    if (d->service == GoogleService::GPhotoExport)
        d->widget->m_tagsBGrp->button(grp.readEntry("Tag Paths", 0))->setChecked(true);

    KConfigGroup dialogGroup;

    switch (d->service)
    {
        case GoogleService::GDrive:
            dialogGroup = config.group("Google Drive Export Dialog");
            break;
        case GoogleService::GPhotoExport:
            dialogGroup = config.group("Google Photo Export Dialog");
            break;
        case GoogleService::GPhotoImport:
            dialogGroup = config.group("Google Photo Import Dialog");
            break;
    }

    winId();
    KWindowConfig::restoreWindowSize(windowHandle(), dialogGroup);
    resize(windowHandle()->size());
}

void GSWindow::writeSettings()
{
    KConfig config;
    KConfigGroup grp;

    switch(d->service)
    {
        case GoogleService::GDrive:
            grp = config.group("Google Drive Settings");
            break;
        default:
            grp = config.group("Google Photo Settings");
            break;
    }

    grp.writeEntry("refresh_token", d->refreshToken);
    grp.writeEntry("Current Album", d->currentAlbumId);
    grp.writeEntry("Resize",        d->widget->getResizeCheckBox()->isChecked());
    grp.writeEntry("Maximum Width", d->widget->getDimensionSpB()->value());
    grp.writeEntry("Image Quality", d->widget->getImgQualitySpB()->value());

    if (d->service == GoogleService::GPhotoExport)
        grp.writeEntry("Tag Paths", d->widget->m_tagsBGrp->checkedId());

    KConfigGroup dialogGroup;

    switch (d->service)
    {
        case GoogleService::GDrive:
            dialogGroup = config.group("Google Drive Export Dialog");
            break;
        case GoogleService::GPhotoExport:
            dialogGroup = config.group("Google Photo Export Dialog");
            break;
        case GoogleService::GPhotoImport:
            dialogGroup = config.group("Google Photo Import Dialog");
            break;
    }

    KWindowConfig::saveWindowSize(windowHandle(), dialogGroup);
    config.sync();
}

void GSWindow::slotSetUserName(const QString& msg)
{
    d->widget->updateLabels(msg);
}

void GSWindow::slotListPhotosDoneForDownload(int errCode,
                                             const QString& errMsg,
                                             const QList <GSPhoto>& photosList)
{
    disconnect(d->gphotoTalker, SIGNAL(signalListPhotosDone(int, QString, QList<GSPhoto>)),
               this, SLOT(slotListPhotosDoneForDownload(int, QString, QList<GSPhoto>)));

    if (errCode == 0)
    {
        QMessageBox::critical(this, i18nc("@title:window", "Error"),
                              i18n("Google Photos/PicasaWeb Call Failed: %1\n", errMsg));
        return;
    }

    typedef QPair<QUrl,GSPhoto> Pair;
    d->transferQueue.clear();
    QList<GSPhoto>::const_iterator itPWP;

    for (itPWP = photosList.begin(); itPWP != photosList.end(); ++itPWP)
    {
        d->transferQueue.push_back(Pair((*itPWP).originalURL, (*itPWP)));
    }

    if (d->transferQueue.isEmpty())
        return;

    d->currentAlbumId = d->widget->getAlbumsCoB()->itemData(d->widget->getAlbumsCoB()->currentIndex()).toString();
    d->imagesTotal    = d->transferQueue.count();
    d->imagesCount    = 0;

    d->widget->progressBar()->setFormat(i18n("%v / %m"));
    d->widget->progressBar()->show();

    d->renamingOpt = 0;

    // start download with first photo in queue
    downloadNextPhoto();
}

void GSWindow::slotListPhotosDoneForUpload(int errCode,
                                           const QString& errMsg,
                                           const QList <GSPhoto>& photosList)
{
    qCCritical(DIGIKAM_WEBSERVICES_LOG)<< "err Code is "<< errCode <<" Err Message is "<< errMsg;

    disconnect(d->gphotoTalker, SIGNAL(signalListPhotosDone(int, QString, QList<GSPhoto>)),
               this, SLOT(slotListPhotosDoneForUpload(int, QString, QList<GSPhoto>)));

    if (errCode == 0)
    {
        QMessageBox::critical(this, i18nc("@title:window", "Error"),
                              i18n("Google Photos/PicasaWeb Call Failed: %1\n", errMsg));
        return;
    }

    typedef QPair<QUrl,GSPhoto> Pair;

    d->transferQueue.clear();

    QList<QUrl> urlList = d->widget->imagesList()->imageUrls(true);

    if (urlList.isEmpty())
        return;

    for (QList<QUrl>::ConstIterator it = urlList.constBegin(); it != urlList.constEnd(); ++it)
    {
        DItemInfo info(d->iface->itemInfo((*it).toLocalFile()));
        GSPhoto temp;
        temp.title = info.name();

        // Google Photo doesn't support image titles. Include it in descriptions if needed.
        QStringList descriptions = QStringList() << info.title() << info.comment();
        descriptions.removeAll(QString::fromLatin1(""));
        temp.description         = descriptions.join(QString::fromLatin1("\n\n"));

        // check for existing items
        QString localId;

        if (d->meta.load((*it).toLocalFile()))
        {
            localId = d->meta.getXmpTagString("Xmp.digiKam.picasawebGPhotoId");
        }

        QList<GSPhoto>::const_iterator itPWP;

        for (itPWP = photosList.begin(); itPWP != photosList.end(); ++itPWP)
        {
            if ((*itPWP).id == localId)
            {
                temp.id       = localId;
                temp.editUrl  = (*itPWP).editUrl;
                temp.thumbURL = (*itPWP).thumbURL;
                break;
            }
        }

        // Tags from the database
        temp.gpsLat.setNum(info.latitude());
        temp.gpsLon.setNum(info.longitude());

        temp.tags = info.tagsPath();
        d->transferQueue.append( Pair( (*it), temp) );
    }

    if (d->transferQueue.isEmpty())
        return;

    d->currentAlbumId = d->widget->getAlbumsCoB()->itemData(d->widget->getAlbumsCoB()->currentIndex()).toString();
    d->imagesTotal    = d->transferQueue.count();
    d->imagesCount    = 0;

    d->widget->progressBar()->setFormat(i18n("%v / %m"));
    d->widget->progressBar()->setMaximum(d->imagesTotal);
    d->widget->progressBar()->setValue(0);
    d->widget->progressBar()->show();
    d->widget->progressBar()->progressScheduled(i18n("Google Photo Export"), true, true);
    d->widget->progressBar()->progressThumbnailChanged(QIcon(
        (QLatin1String("googlephoto"))).pixmap(22, 22));

    d->renamingOpt = 0;

    uploadNextPhoto();
}

void GSWindow::slotListAlbumsDone(int code,const QString& errMsg ,const QList <GSFolder>& list)
{
    switch (d->service)
    {
        case GoogleService::GDrive:

            if (code == 0)
            {
                QMessageBox::critical(this, i18nc("@title:window", "Error"),
                                      i18n("Google Drive Call Failed: %1\n", errMsg));
                return;
            }

            d->widget->getAlbumsCoB()->clear();
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "slotListAlbumsDone1:" << list.size();

            for (int i=0;i<list.size();i++)
            {
                d->widget->getAlbumsCoB()->addItem(
                    QIcon::fromTheme(QString::fromLatin1("system-users")),
                    list.value(i).title, list.value(i).id);

                if (d->currentAlbumId == list.value(i).id)
                {
                    d->widget->getAlbumsCoB()->setCurrentIndex(i);
                }
            }

            buttonStateChange(true);
            d->talker->getUserName();
            break;

        default:

            if (code == 0)
            {
                QMessageBox::critical(this, i18nc("@title:window", "Error"),
                                      i18n("Google Photos/PicasaWeb Call Failed: %1\n", errMsg));
                return;
            }

            d->widget->updateLabels(d->gphotoTalker->getLoginName(), d->gphotoTalker->getUserName());
            d->widget->getAlbumsCoB()->clear();

            for (int i = 0; i < list.size(); ++i)
            {
                QString albumIcon;

                if (list.at(i).access == QString::fromLatin1("public"))
                    albumIcon = QString::fromLatin1("folder-image");
                else if (list.at(i).access == QString::fromLatin1("protected"))
                    albumIcon = QString::fromLatin1("folder-locked");
                else
                    albumIcon = QString::fromLatin1("folder");

                d->widget->getAlbumsCoB()->addItem(QIcon::fromTheme(albumIcon), list.at(i).title, list.at(i).id);

                if (d->currentAlbumId == list.at(i).id)
                    d->widget->getAlbumsCoB()->setCurrentIndex(i);

                buttonStateChange(true);
            }
            break;
    }
}

void GSWindow::slotBusy(bool val)
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

void GSWindow::googlePhotoTransferHandler()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Google Photo Transfer invoked";

    switch (d->service)
    {
        case GoogleService::GPhotoImport:
            // list photos of the album, then start download
            connect(d->gphotoTalker, SIGNAL(signalListPhotosDone(int, QString, QList<GSPhoto>)),
                    this, SLOT(slotListPhotosDoneForDownload(int, QString, QList<GSPhoto>)));

            d->gphotoTalker->listPhotos(
                d->widget->getAlbumsCoB()->itemData(d->widget->getAlbumsCoB()->currentIndex()).toString(),
                d->widget->getDimensionCoB()->itemData(d->widget->getDimensionCoB()->currentIndex()).toString());
            break;

        default:
            // list photos of the album, then start upload with add/update items
            connect(d->gphotoTalker, SIGNAL(signalListPhotosDone(int, QString, QList<GSPhoto>)),
                    this, SLOT(slotListPhotosDoneForUpload(int, QString, QList<GSPhoto>)));

            d->gphotoTalker->listPhotos(
                d->widget->getAlbumsCoB()->itemData(d->widget->getAlbumsCoB()->currentIndex()).toString());
            break;
    }
}

void GSWindow::slotTextBoxEmpty()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "in slotTextBoxEmpty";
    QMessageBox::critical(this, i18nc("@title:window", "Error"),
                          i18n("The textbox is empty, please enter the code from the browser in the textbox. "
                               "To complete the authentication click \"Change Account\", "
                               "or \"Start Upload\" to authenticate again."));
}

void GSWindow::slotStartTransfer()
{
    d->widget->imagesList()->clearProcessedStatus();

    switch (d->service)
    {
        case GoogleService::GDrive:
        case GoogleService::GPhotoExport:
            if (d->widget->imagesList()->imageUrls().isEmpty())
            {
                QMessageBox::critical(this, i18nc("@title:window", "Error"),
                                      i18n("No image selected. Please select which images should be uploaded."));
                return;
            }
            break;
        case GoogleService::GPhotoImport:
            break;
    }

    switch (d->service)
    {
        case GoogleService::GDrive:
            if (!(d->talker->authenticated()))
            {
                QMessageBox warn(QMessageBox::Warning,
                                 i18n("Warning"),
                                 i18n("Authentication failed. Click \"Continue\" to authenticate."),
                                 QMessageBox::Yes | QMessageBox::No);

                (warn.button(QMessageBox::Yes))->setText(i18n("Continue"));
                (warn.button(QMessageBox::No))->setText(i18n("Cancel"));

                if (warn.exec() == QMessageBox::Yes)
                {
                    d->talker->doOAuth();
                    return;
                }
                else
                {
                    return;
                }
            }
            break;

        default:
            if (!(d->gphotoTalker->authenticated()))
            {
                QMessageBox warn(QMessageBox::Warning,
                                 i18n("Warning"),
                                 i18n("Authentication failed. Click \"Continue\" to authenticate."),
                                 QMessageBox::Yes | QMessageBox::No);

                (warn.button(QMessageBox::Yes))->setText(i18n("Continue"));
                (warn.button(QMessageBox::No))->setText(i18n("Cancel"));

                if (warn.exec() == QMessageBox::Yes)
                {
                    d->gphotoTalker->doOAuth();
                    return;
                }
                else
                {
                    return;
                }
            }

            googlePhotoTransferHandler();
            return;
    }

    typedef QPair<QUrl, GSPhoto> Pair;

    for (int i = 0 ; i < (d->widget->imagesList()->imageUrls().size()) ; i++)
    {
        DItemInfo info(d->iface->itemInfo(d->widget->imagesList()->imageUrls().value(i).toLocalFile()));
        GSPhoto temp;
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "in start transfer info " <<info.title() << info.comment();

        switch (d->service)
        {
            case GoogleService::GDrive:
                temp.title      = info.title();
                break;
            default:
                temp.title      = info.name();
                break;
        }

        temp.description = info.comment().section(QString::fromLatin1("\n"), 0, 0);
        temp.gpsLat.setNum(info.latitude());
        temp.gpsLon.setNum(info.longitude());
        temp.tags        = info.tagsPath();

        d->transferQueue.append(Pair(d->widget->imagesList()->imageUrls().value(i),temp));
    }

    d->currentAlbumId = d->widget->getAlbumsCoB()->itemData(d->widget->getAlbumsCoB()->currentIndex()).toString();
    d->imagesTotal    = d->transferQueue.count();
    d->imagesCount    = 0;

    d->widget->progressBar()->setFormat(i18n("%v / %m"));
    d->widget->progressBar()->setMaximum(d->imagesTotal);
    d->widget->progressBar()->setValue(0);
    d->widget->progressBar()->show();
    d->widget->progressBar()->progressScheduled(i18n("Google Drive export"), true, true);
    d->widget->progressBar()->progressThumbnailChanged(QIcon(QLatin1String("googledrive")).pixmap(22, 22));

    uploadNextPhoto();
}

void GSWindow::uploadNextPhoto()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "in upload nextphoto " << d->transferQueue.count();

    if (d->transferQueue.isEmpty())
    {
        //d->widget->progressBar()->hide();
        d->widget->progressBar()->progressCompleted();
        return;
    }

    typedef QPair<QUrl,GSPhoto> Pair;
    Pair pathComments = d->transferQueue.first();
    GSPhoto info      = pathComments.second;
    bool res          = true;
    d->widget->imagesList()->processing(pathComments.first);

    switch (d->service)
    {
        case GoogleService::GDrive:
        {
            res = d->talker->addPhoto(pathComments.first.toLocalFile(),
                                     info,
                                     d->currentAlbumId,
                                     d->widget->getResizeCheckBox()->isChecked(),
                                     d->widget->getDimensionSpB()->value(),
                                     d->widget->getImgQualitySpB()->value());
            break;
        }

        case GoogleService::GPhotoExport:
        {
            bool bCancel = false;
            bool bAdd    = true;

            if (!info.id.isEmpty() && !info.editUrl.isEmpty())
            {
                switch(d->renamingOpt)
                {
                    case PWR_ADD_ALL:
                        bAdd = true;
                        break;
                    case PWR_REPLACE_ALL:
                        bAdd = false;
                        break;
                    default:
                    {
                        ReplaceDialog dlg(this, QString::fromLatin1(""), d->iface, pathComments.first, info.thumbURL);
                        dlg.exec();

                        switch (dlg.getResult())
                        {
                            case PWR_ADD_ALL:
                                d->renamingOpt = PWR_ADD_ALL;
                                break;
                            case PWR_ADD:
                                bAdd = true;
                                break;
                            case PWR_REPLACE_ALL:
                                d->renamingOpt = PWR_REPLACE_ALL;
                                break;
                            case PWR_REPLACE:
                                bAdd = false;
                                break;
                            case PWR_CANCEL:
                            default:
                                bCancel = true;
                                break;
                        }

                        break;
                    }
                }
            }

            //adjust tags according to radio button clicked

            switch (d->widget->m_tagsBGrp->checkedId())
            {
                case GPTagLeaf:
                {
                    QStringList newTags;
                    QStringList::const_iterator itT;

                    for (itT = info.tags.constBegin(); itT != info.tags.constEnd(); ++itT)
                    {
                        QString strTmp = *itT;
                        int idx        = strTmp.lastIndexOf(QString::fromLatin1("/"));

                        if (idx > 0)
                        {
                            strTmp.remove(0, idx + 1);
                        }

                        newTags.append(strTmp);
                    }

                    info.tags = newTags;
                    break;
                }

                case GPTagSplit:
                {
                    QSet<QString> newTagsSet;
                    QStringList::const_iterator itT;

                    for (itT = info.tags.constBegin(); itT != info.tags.constEnd(); ++itT)
                    {
                        QStringList strListTmp = itT->split(QLatin1Char('/'));
                        QStringList::const_iterator itT2;

                        for (itT2 = strListTmp.constBegin(); itT2 != strListTmp.constEnd(); ++itT2)
                        {
                            if (!newTagsSet.contains(*itT2))
                            {
                                newTagsSet.insert(*itT2);
                            }
                        }
                    }

                    info.tags.clear();
                    QSet<QString>::const_iterator itT3;

                    for (itT3 = newTagsSet.begin(); itT3 != newTagsSet.end(); ++itT3)
                    {
                        info.tags.append(*itT3);
                    }

                    break;
                }

                case GPTagCombined:
                default:
                    break;
            }

            if (bCancel)
            {
                slotTransferCancel();
                res = true;
            }
            else
            {
                if (bAdd)
                {
                    res = d->gphotoTalker->addPhoto(pathComments.first.toLocalFile(),
                                                     info,
                                                     d->currentAlbumId,
                                                     d->widget->getResizeCheckBox()->isChecked(),
                                                     d->widget->getDimensionSpB()->value(),
                                                     d->widget->getImgQualitySpB()->value());
                }
                else
                {
                    res = d->gphotoTalker->updatePhoto(pathComments.first.toLocalFile(),
                                                        info,
                                                        d->widget->getResizeCheckBox()->isChecked(),
                                                        d->widget->getDimensionSpB()->value(),
                                                        d->widget->getImgQualitySpB()->value());
                }
            }
            break;
        }

        case GoogleService::GPhotoImport:
            break;
    }

    if (!res)
    {
        slotAddPhotoDone(0, QString::fromLatin1(""), QString::fromLatin1("-1"));
        return;
    }
}

void GSWindow::downloadNextPhoto()
{
    if (d->transferQueue.isEmpty())
    {
        d->widget->progressBar()->hide();
        d->widget->progressBar()->progressCompleted();
        return;
    }

    d->widget->progressBar()->setMaximum(d->imagesTotal);
    d->widget->progressBar()->setValue(d->imagesCount);

    QString imgPath = d->transferQueue.first().first.url();

    d->gphotoTalker->getPhoto(imgPath);
}

void GSWindow::slotGetPhotoDone(int errCode, const QString& errMsg, const QByteArray& photoData)
{
    GSPhoto item = d->transferQueue.first().second;
    QUrl tmpUrl  = QUrl::fromLocalFile(QString(d->tmp + item.title));

    if (item.mimeType == QString::fromLatin1("video/mpeg4"))
    {
        tmpUrl = tmpUrl.adjusted(QUrl::RemoveFilename);
        tmpUrl.setPath(tmpUrl.path() + item.title + QString::fromLatin1(".mp4"));
    }

    if (errCode == 1)
    {
        QString errText;
        QFile imgFile(tmpUrl.toLocalFile());

        if (!imgFile.open(QIODevice::WriteOnly))
        {
            errText = imgFile.errorString();
        }
        else if (imgFile.write(photoData) != photoData.size())
        {
            errText = imgFile.errorString();
        }
        else
        {
            imgFile.close();
        }

        if (errText.isEmpty())
        {
            if (d->meta.load(tmpUrl.toLocalFile()))
            {
                if (d->meta.supportXmp() && d->meta.canWriteXmp(tmpUrl.toLocalFile()))
                {
                    d->meta.setXmpTagString("Xmp.digiKam.picasawebGPhotoId", item.id);
                    d->meta.setXmpKeywords(item.tags);
                }

                if (!item.gpsLat.isEmpty() && !item.gpsLon.isEmpty())
                {
                    d->meta.setGPSInfo(0.0, item.gpsLat.toDouble(), item.gpsLon.toDouble());
                }

                d->meta.setMetadataWritingMode((int)DMetadata::WRITETOIMAGEONLY);
                d->meta.save(tmpUrl.toLocalFile());
            }

            d->transferQueue.pop_front();
            d->imagesCount++;
        }
        else
        {
            QMessageBox warn(QMessageBox::Warning,
                             i18n("Warning"),
                             i18n("Failed to save photo: %1\n"
                                  "Do you want to continue?", errText),
                             QMessageBox::Yes | QMessageBox::No);

            (warn.button(QMessageBox::Yes))->setText(i18n("Continue"));
            (warn.button(QMessageBox::No))->setText(i18n("Cancel"));

            if (warn.exec() != QMessageBox::Yes)
            {
                slotTransferCancel();
                return;
            }
        }
    }
    else
    {
        QMessageBox warn(QMessageBox::Warning,
                         i18n("Warning"),
                         i18n("Failed to download photo: %1\n"
                              "Do you want to continue?", errMsg),
                         QMessageBox::Yes | QMessageBox::No);

        (warn.button(QMessageBox::Yes))->setText(i18n("Continue"));
        (warn.button(QMessageBox::No))->setText(i18n("Cancel"));

        if (warn.exec() != QMessageBox::Yes)
        {
            slotTransferCancel();
            return;
        }
    }

    QUrl newUrl = QUrl::fromLocalFile(QString(d->widget->getDestinationPath() + tmpUrl.fileName()));

    QFileInfo targetInfo(newUrl.toLocalFile());

    if (targetInfo.exists())
    {
        int i          = 0;
        bool fileFound = false;

        do
        {
            QFileInfo newTargetInfo(newUrl.toLocalFile());

            if (!newTargetInfo.exists())
            {
                fileFound = false;
            }
            else
            {
                newUrl = newUrl.adjusted(QUrl::RemoveFilename);
                newUrl.setPath(newUrl.path() + targetInfo.completeBaseName() +
                                               QString::fromUtf8("_%1.").arg(++i) +
                                               targetInfo.completeSuffix());
                fileFound = true;
            }
        }
        while (fileFound);
    }

    if (!QFile::rename(tmpUrl.toLocalFile(), newUrl.toLocalFile()))
    {
        QMessageBox::critical(this, i18nc("@title:window", "Error"),
                              i18n("Failed to save image to %1", newUrl.toLocalFile()));
    }
/* TODO
    else
    {
        KPImageInfo info(newUrl);
        info.setName(item.title);
        info.setDescription(item.description);
        info.setTagsPath(item.tags);

        if (!item.gpsLat.isEmpty() && !item.gpsLon.isEmpty())
        {
            info.setLatitude(item.gpsLat.toDouble());
            info.setLongitude(item.gpsLon.toDouble());
        }
    }
*/
    downloadNextPhoto();
}

void GSWindow::slotAddPhotoDone(int err, const QString& msg, const QString& photoId)
{
    if (err == 0)
    {
        d->widget->imagesList()->processed(d->transferQueue.first().first,false);

        QMessageBox warn(QMessageBox::Warning,
                         i18n("Warning"),
                         i18n("Failed to upload photo to %1.\n%2\nDo you want to continue?", d->toolName,msg),
                         QMessageBox::Yes | QMessageBox::No);

        (warn.button(QMessageBox::Yes))->setText(i18n("Continue"));
        (warn.button(QMessageBox::No))->setText(i18n("Cancel"));

        if (warn.exec() != QMessageBox::Yes)
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
    else
    {
        QUrl fileUrl = d->transferQueue.first().first;

        if (d->meta.supportXmp()                       &&
            d->meta.canWriteXmp(fileUrl.toLocalFile()) &&
            d->meta.load(fileUrl.toLocalFile())        &&
            !photoId.isEmpty()
           )
        {
            d->meta.setXmpTagString("Xmp.digiKam.picasawebGPhotoId", photoId);
            d->meta.save(fileUrl.toLocalFile());
        }

        // Remove photo uploaded from the list
        d->widget->imagesList()->removeItemByUrl(d->transferQueue.first().first);
        d->transferQueue.pop_front();
        d->imagesCount++;
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In slotAddPhotoSucceeded" << d->imagesCount;
        d->widget->progressBar()->setMaximum(d->imagesTotal);
        d->widget->progressBar()->setValue(d->imagesCount);
        uploadNextPhoto();
    }
}

void GSWindow::slotImageListChanged()
{
    startButton()->setEnabled(!(d->widget->imagesList()->imageUrls().isEmpty()));
}

void GSWindow::slotNewAlbumRequest()
{
    switch (d->service)
    {
        case GoogleService::GDrive:
            if (d->albumDlg->exec() == QDialog::Accepted)
            {
                GSFolder newFolder;
                d->albumDlg->getAlbumProperties(newFolder);
                d->currentAlbumId = d->widget->getAlbumsCoB()->itemData(d->widget->getAlbumsCoB()->currentIndex()).toString();
                d->talker->createFolder(newFolder.title,d->currentAlbumId);
            }
            break;

        default:
            if (d->gphotoAlbumDlg->exec() == QDialog::Accepted)
            {
                GSFolder newFolder;
                d->gphotoAlbumDlg->getAlbumProperties(newFolder);
                d->gphotoTalker->createAlbum(newFolder);
            }
            break;
    }
}

void GSWindow::slotReloadAlbumsRequest()
{
    switch (d->service)
    {
        case GoogleService::GDrive:
            d->talker->listFolders();
            break;
        case GoogleService::GPhotoImport:
        case GoogleService::GPhotoExport:
            d->gphotoTalker->listAlbums();
            break;
    }
}

void GSWindow::slotAccessTokenFailed(int errCode,const QString& errMsg)
{
    QMessageBox::critical(this, i18nc("@title:window", "Error"),
                          i18nc("%1 is the error string, %2 is the error code",
                                "An authentication error occurred: %1 (%2)",
                                errMsg, errCode));
    return;
}

void GSWindow::slotAccessTokenObtained()
{
    switch (d->service)
    {
        case GoogleService::GDrive:
            d->talker->listFolders();
            break;
        case GoogleService::GPhotoImport:
        case GoogleService::GPhotoExport:
            d->gphotoTalker->listAlbums();
            break;
    }
}

void GSWindow::slotRefreshTokenObtained(const QString& msg)
{
    switch (d->service)
    {
        case GoogleService::GDrive:
            d->refreshToken = msg;
            d->talker->listFolders();
            break;
        case GoogleService::GPhotoImport:
        case GoogleService::GPhotoExport:
            d->refreshToken = msg;
            d->gphotoTalker->listAlbums();
            break;
    }
}

void GSWindow::slotCreateFolderDone(int code, const QString& msg, const QString& albumId)
{
    switch (d->service)
    {
        case GoogleService::GDrive:
            if (code == 0)
                QMessageBox::critical(this, i18nc("@title:window", "Error"),
                                      i18n("Google Drive call failed:\n%1", msg));
            else
                d->talker->listFolders();
            break;
        case GoogleService::GPhotoImport:
        case GoogleService::GPhotoExport:
            if (code == 0)
                QMessageBox::critical(this, i18nc("@title:window", "Error"),
                                      i18n("Google Photos/PicasaWeb call failed:\n%1", msg));
            else
            {
                d->currentAlbumId = albumId;
                d->gphotoTalker->listAlbums();
            }
            break;
    }
}

void GSWindow::slotTransferCancel()
{
    d->transferQueue.clear();
    d->widget->progressBar()->hide();

    switch (d->service)
    {
        case GoogleService::GDrive:
            d->talker->cancel();
            break;
        case GoogleService::GPhotoImport:
        case GoogleService::GPhotoExport:
            d->gphotoTalker->cancel();
            break;
    }
}

void GSWindow::slotUserChangeRequest()
{
    QUrl url(QString::fromLatin1("https://accounts.google.com/logout"));
    QDesktopServices::openUrl(url);

    QMessageBox warn(QMessageBox::Warning,
                     i18nc("@title:window", "Warning"),
                     i18n("After you have been logged out in the browser, "
                          "click \"Continue\" to authenticate for another account"),
                     QMessageBox::Yes | QMessageBox::No);

    (warn.button(QMessageBox::Yes))->setText(i18n("Continue"));
    (warn.button(QMessageBox::No))->setText(i18n("Cancel"));

    if (warn.exec() == QMessageBox::Yes)
    {
        d->refreshToken = QString::fromLatin1("");

        switch (d->service)
        {
            case GoogleService::GDrive:
                d->talker->doOAuth();
                break;
            case GoogleService::GPhotoImport:
            case GoogleService::GPhotoExport:
                d->gphotoTalker->doOAuth();
                break;
        }
    }
}

void GSWindow::buttonStateChange(bool state)
{
    d->widget->getNewAlbmBtn()->setEnabled(state);
    d->widget->getReloadBtn()->setEnabled(state);
    startButton()->setEnabled(state);
}

void GSWindow::slotFinished()
{
    writeSettings();
    d->widget->imagesList()->listView()->clear();
}

void GSWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    slotFinished();
    e->accept();
}

} // namespace Digikam
