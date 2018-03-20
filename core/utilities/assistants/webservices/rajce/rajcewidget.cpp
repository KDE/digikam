/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-04-12
 * Description : A tool to export items to Rajce web service
 *
 * Copyright (C) 2011      by Lukas Krejci <krejci.l at centrum dot cz>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "rajcewidget.h"

// Qt includes

#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QButtonGroup>
#include <QRadioButton>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDialog>
#include <QComboBox>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>
#include <kconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "rajcetalker.h"
#include "rajcesession.h"
#include "rajcenewalbumdlg.h"
#include "rajcesession.h"
#include "dimageslist.h"
#include "wslogindialog.h"
#include "dinfointerface.h"
#include "dprogresswdg.h"

namespace Digikam
{

class RajceWidget::Private
{
public:

    explicit Private()
    {
        headerLbl         = 0;
        userNameLbl       = 0;
        userName          = 0;
        dimensionSpB      = 0;
        imageQualitySpB   = 0;
        albumsCoB         = 0;
        newAlbumBtn       = 0;
        reloadAlbumsBtn   = 0;
        changeUserBtn     = 0;
        iface             = 0;
        imgList           = 0;
        progressBar       = 0;
        talker            = 0;
        uploadingPhotos   = false;
        lastLoggedInState = false;
    }

    QLabel*                  headerLbl;
    QLabel*                  userNameLbl;
    QLabel*                  userName;

    QSpinBox*                dimensionSpB;
    QSpinBox*                imageQualitySpB;

    QComboBox*               albumsCoB;

    QPushButton*             newAlbumBtn;
    QPushButton*             reloadAlbumsBtn;
    QPushButton*             changeUserBtn;

    DInfoInterface*          iface;
    DImagesList*             imgList;
    DProgressWdg*            progressBar;

    RajceTalker*             talker;

    QList<QString>           uploadQueue;
    QList<QString>::Iterator currentUploadImage;

    bool                     uploadingPhotos;
    bool                     lastLoggedInState;
    QString                  currentAlbumName;
};

RajceWidget::RajceWidget(DInfoInterface* const iface, QWidget* const parent)
    : WSSettingsWidget(parent, iface, QString::fromLatin1("Rajce.net")),
      d(new Private)
{
    d->iface             = iface;
    d->talker            = new RajceTalker(this);
    d->albumsCoB         = getAlbumsCoB();
    d->dimensionSpB      = getDimensionSpB();
    d->imageQualitySpB   = getImgQualitySpB();
    d->newAlbumBtn       = getNewAlbmBtn();
    d->reloadAlbumsBtn   = getReloadBtn();
    d->progressBar       = progressBar();
    d->imgList           = imagesList();
    d->changeUserBtn     = getChangeUserBtn();

    getUploadBox()->hide();
    getSizeBox()->hide();

    updateLabels();

    // ------------------------------------------------------------------------

    connect(d->talker, SIGNAL(signalBusyStarted(uint)),
            this, SLOT(slotProgressStarted(uint)));

    connect(d->talker, SIGNAL(signalBusyFinished(uint)),
            this, SLOT(slotProgressFinished(uint)));

    connect(d->talker, SIGNAL(signalBusyProgress(uint,uint)),
            this, SLOT(slotProgressChanged(uint,uint)));

    connect(d->changeUserBtn, SIGNAL(clicked()),
            this, SLOT(slotChangeUserClicked()));

    connect(d->newAlbumBtn, SIGNAL(clicked()),
            this, SLOT(slotCreateAlbum()));

    connect(d->reloadAlbumsBtn, SIGNAL(clicked()),
            this, SLOT(slotLoadAlbums()));

    connect(d->albumsCoB, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(slotSelectedAlbumChanged(QString)));
}

RajceWidget::~RajceWidget()
{
    delete d;
}

void RajceWidget::updateLabels(const QString&, const QString&)
{
    bool loggedIn = !d->talker->session().sessionToken().isEmpty();

    if (loggedIn != d->lastLoggedInState)
    {
        d->lastLoggedInState = loggedIn;
        emit signalLoginStatusChanged(loggedIn);
    }

    QString username = loggedIn ? d->talker->session().username() : QString::fromLatin1("");
    QString nickname = loggedIn ? d->talker->session().nickname() : i18n("Not logged in");

    getUserNameLabel()->setText(QString::fromLatin1("<b>%2</b> <small>%1</small>").arg(username, nickname));

    QString link = loggedIn
        ? QString::fromLatin1("<b><h2><a href='http://") + d->talker->session().nickname() +
        QString::fromLatin1(".rajce.net'>"
        "<font color=\"#9ACD32\">Rajce.net</font>"
        "</a></h2></b>")
        : QString::fromLatin1("<b><h2><a href='http://www.rajce.net'>"
        "<font color=\"#9ACD32\">Rajce.net</font>"
        "</a></h2></b>");

    getHeaderLbl()->setText(link);

    disconnect(d->albumsCoB, SIGNAL(currentIndexChanged(QString)),
               this, SLOT(slotSelectedAlbumChanged(QString)));

    d->albumsCoB->clear();
    RajceAlbum album;
    int   selIdx = 0;
    int   i      = 0;

    foreach (album, d->talker->session().albums())
    {
        d->albumsCoB->addItem(album.name, QVariant::fromValue(album));

        if (d->currentAlbumName == album.name)
        {
            selIdx = i;
        }

        ++i;
    }

    if (!d->currentAlbumName.isEmpty())
    {
        d->albumsCoB->setCurrentIndex(selIdx);
    }

    connect(d->albumsCoB, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(slotSelectedAlbumChanged(QString)));

    unsigned max = d->talker->session().maxHeight();
    max          = max > d->talker->session().maxWidth() ? max
                                                        : d->talker->session().maxWidth();
    d->dimensionSpB->setMaximum(max);

    if (d->dimensionSpB->value() == 0)
    {
        d->dimensionSpB->setValue(max);
    }

    d->newAlbumBtn->setEnabled(loggedIn);
    d->albumsCoB->setEnabled(loggedIn);
    d->reloadAlbumsBtn->setEnabled(loggedIn);
    d->dimensionSpB->setEnabled(loggedIn);
    d->imageQualitySpB->setEnabled(loggedIn);

    if (d->talker->session().lastErrorCode() != 0)
    {
        d->progressBar->setVisible(true);

        switch (d->talker->session().lastErrorCode())
        {
            case UnknownError:                   d->progressBar->setFormat(i18n("Unknown error"));                  break;
            case InvalidCommand:                 d->progressBar->setFormat(i18n("Invalid command"));                break;
            case InvalidCredentials:             d->progressBar->setFormat(i18n("Invalid login name or password")); break;
            case InvalidSessionToken:            d->progressBar->setFormat(i18n("Session expired"));                break;
            case InvalidOrRepeatedColumnName:                                                                       break;
            case InvalidAlbumId:                 d->progressBar->setFormat(i18n("Unknown album"));                  break;
            case AlbumDoesntExistOrNoPrivileges: d->progressBar->setFormat(i18n("Unknown album"));                  break;
            case InvalidAlbumToken:              d->progressBar->setFormat(i18n("Failed to open album"));           break;
            case AlbumNameEmpty:                 d->progressBar->setFormat(i18n("The album name cannot be empty")); break;
            case FailedToCreateAlbum:            d->progressBar->setFormat(i18n("Failed to create album"));         break;
            case AlbumDoesntExist:               d->progressBar->setFormat(i18n("Album does not exist"));           break;
            case UnknownApplication:                                                                                break;
            case InvalidApplicationKey:                                                                             break;
            case FileNotAttached:                d->progressBar->setFormat(i18n("File upload failed"));             break;
            case NewerVersionExists:                                                                                break;
            case SavingFileFailed:               d->progressBar->setFormat(i18n("File upload failed"));             break;
            case UnsupportedFileExtension:       d->progressBar->setFormat(i18n("Unsupported file extension"));     break;
            case UnknownClientVersion:                                                                              break;
            case NonexistentTarget:                                                                                 break;
            default:                                                                                                break;
        }

        QPalette palette = d->progressBar->palette();
        palette.setColor(QPalette::Active, QPalette::Background, Qt::darkRed);
        d->progressBar->setPalette(palette);
    }
}

void RajceWidget::reactivate()
{
    d->imgList->listView()->clear();
    d->imgList->loadImagesFromCurrentSelection();
    d->talker->clearLastError();
    updateLabels();
}

void RajceWidget::slotProgressChanged(unsigned /*commandType*/, unsigned int percent)
{
    if (d->uploadingPhotos)
    {
        unsigned idx  = d->currentUploadImage - d->uploadQueue.begin() - 1;
        float perc    = (float)idx / d->uploadQueue.size();
        perc         += (float)percent / 100 / d->uploadQueue.size();
        percent       = perc * 100;
    }

    d->progressBar->setValue(percent);
}

void RajceWidget::slotProgressFinished(unsigned)
{
    if (d->uploadingPhotos)
    {
        unsigned idx = d->currentUploadImage - d->uploadQueue.begin();
        float perc   = (float)idx / d->uploadQueue.size();

        d->progressBar->setValue(perc * 100);
    }
    else
    {
        d->progressBar->setVisible(false);
        setEnabledWidgets(true);
        updateLabels();
    }
}

void RajceWidget::slotProgressStarted(unsigned commandType)
{
    QString text;

    switch(commandType)
    {
        case Login:       text = i18n("Logging in %v%");     break;
        case Logout:      text = i18n("Logging out %v%");    break;
        case ListAlbums:  text = i18n("Loading albums %v%"); break;
        case CreateAlbum: text = i18n("Creating album %v%"); break;
        case OpenAlbum:   text = i18n("Opening album %v%");  break;
        case CloseAlbum:  text = i18n("Closing album %v%");  break;
        case AddPhoto:    text = i18n("Adding photos %v%");  break;
    }

    if (!d->uploadingPhotos)
    {
        d->progressBar->setValue(0);
    }

    d->progressBar->setFormat(text);
    d->progressBar->setVisible(true);
    setEnabledWidgets(false);
}

void RajceWidget::slotChangeUserClicked()
{
    WSLoginDialog* const dlg = new WSLoginDialog(this, QString::fromLatin1("Rajce.net"));

    if (dlg->exec() == QDialog::Accepted)
    {
        d->talker->clearLastError();

        connect(d->talker, SIGNAL(signalBusyFinished(uint)),
                this, SLOT(slotLoadAlbums()));

        d->talker->login(dlg->login(), dlg->password());
    }

    delete dlg;
}

void RajceWidget::slotLoadAlbums()
{
    disconnect(d->talker, SIGNAL(signalBusyFinished(uint)),
               this, SLOT(slotLoadAlbums()));

    d->talker->loadAlbums();
}

void RajceWidget::slotCreateAlbum()
{
    RajceNewAlbumDlg* const dlg = new RajceNewAlbumDlg(this);

    if (dlg->exec() == QDialog::Accepted)
    {
        d->talker->clearLastError();

        connect(d->talker, SIGNAL(signalBusyFinished(uint)),
                this, SLOT(slotLoadAlbums()));

        d->talker->createAlbum(dlg->albumName(), dlg->albumDescription(), dlg->albumVisible());
    }

    delete dlg;
}

void RajceWidget::slotStartUpload()
{
    d->talker->clearLastError();
    setEnabledWidgets(false);

    d->uploadQueue.clear();

    foreach (const QUrl& image, d->imgList->imageUrls(true))
    {
        QString imagePath = image.toLocalFile();
        d->uploadQueue.append(imagePath);
    }

    if (d->uploadQueue.isEmpty())
    {
        setEnabledWidgets(true);
        return;
    }

    connect(d->talker, SIGNAL(signalBusyFinished(uint)),
            this, SLOT(slotStartUploadAfterAlbumOpened()));

    QString albumName = d->albumsCoB->currentText();
    RajceAlbum album;

    foreach (RajceAlbum a, d->talker->session().albums())
    {
        if (a.name == albumName)
        {
            album = a;
            break;
        }
    }

    if (album.name == albumName)
    {
        d->talker->openAlbum(album);
    }
}

void RajceWidget::slotStartUploadAfterAlbumOpened()
{
    disconnect(d->talker, SIGNAL(signalBusyFinished(uint)),
               this, SLOT(slotStartUploadAfterAlbumOpened()));

    connect(d->talker, SIGNAL(signalBusyFinished(uint)),
            this, SLOT(slotUploadNext()));

    d->uploadingPhotos    = true;
    d->progressBar->setValue(0);
    slotProgressStarted(AddPhoto);
    d->currentUploadImage = d->uploadQueue.begin();
    slotUploadNext();
}

void RajceWidget::slotCloseAlbum()
{
    setEnabledWidgets(true);

    disconnect(d->talker, SIGNAL(signalBusyFinished(uint)),
               this, SLOT(slotCloseAlbum()));

    d->uploadQueue.clear();
    d->progressBar->setVisible(false);

    d->uploadingPhotos = false;
}

void RajceWidget::slotUploadNext()
{
    QList<QString>::Iterator tmp = d->currentUploadImage;

    if (d->currentUploadImage == d->uploadQueue.end())
    {
        d->imgList->processed(QUrl::fromLocalFile(*(--tmp)), (d->talker->session().lastErrorCode() == 0));
        cancelUpload();
        return;
    }

    if (d->currentUploadImage != d->uploadQueue.begin())
    {
        d->imgList->processed(QUrl::fromLocalFile(*(--tmp)), (d->talker->session().lastErrorCode() == 0));
    }

    d->imgList->processing(QUrl::fromLocalFile(*d->currentUploadImage));

    QString currentPhoto = *d->currentUploadImage;
    ++d->currentUploadImage;

    unsigned dimension   = d->dimensionSpB->value();
    int jpgQuality       = d->imageQualitySpB->value();

    d->talker->uploadPhoto(currentPhoto, dimension, jpgQuality);
}

void RajceWidget::cancelUpload()
{
    if (d->uploadingPhotos && d->currentUploadImage != d->uploadQueue.begin() &&
        d->currentUploadImage != d->uploadQueue.end())
    {
        d->imgList->processed(QUrl::fromLocalFile(*d->currentUploadImage), false);
    }

    disconnect(d->talker, SIGNAL(signalBusyFinished(uint)),
               this, SLOT(slotUploadNext()));

    connect(d->talker, SIGNAL(signalBusyFinished(uint)),
            this, SLOT(slotCloseAlbum()));

    d->talker->cancelCurrentCommand();
    d->talker->closeAlbum();
    d->uploadQueue.clear();
}

void RajceWidget::slotSelectedAlbumChanged(const QString& newName)
{
    d->currentAlbumName = newName;
}

void RajceWidget::setEnabledWidgets(bool enabled)
{
    d->changeUserBtn->setEnabled(enabled);
    d->newAlbumBtn->setEnabled(enabled);
    d->albumsCoB->setEnabled(enabled);
    d->reloadAlbumsBtn->setEnabled(enabled);
    d->dimensionSpB->setEnabled(enabled);
    d->imageQualitySpB->setEnabled(enabled);

    emit signalLoginStatusChanged(enabled);
}

void RajceWidget::readSettings()
{
    KConfig config;
    KConfigGroup grp = config.group("RajceExport Settings");

    RajceSession session;

    session.sessionToken() = grp.readEntry("token");
    session.username()     = grp.readEntry("username");
    session.nickname()     = grp.readEntry("nickname");
    d->currentAlbumName    = grp.readEntry("album");
    session.maxHeight()    = grp.readEntry("maxHeight",    1200);
    session.maxWidth()     = grp.readEntry("maxWidth",     1200);
    session.imageQuality() = grp.readEntry("imageQuality", 85);

    d->talker->init(session);

    if (!d->talker->session().sessionToken().isEmpty())
    {
        d->talker->loadAlbums();
    }
}

void RajceWidget::writeSettings()
{
    KConfig config;
    KConfigGroup grp            = config.group("RajceExport Settings");
    const RajceSession& session = d->talker->session();

    grp.writeEntry("token",        session.sessionToken());
    grp.writeEntry("username",     session.username());
    grp.writeEntry("nickname",     session.nickname());
    grp.writeEntry("album",        d->currentAlbumName);
    grp.writeEntry("maxWidth",     session.maxWidth());
    grp.writeEntry("maxHeight",    session.maxHeight());
    grp.writeEntry("imageQuality", session.imageQuality());
}

} // namespace Digikam
