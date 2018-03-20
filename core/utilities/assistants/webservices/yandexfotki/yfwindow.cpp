/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-11-15
 * Description : a tool to export items to YandexFotki web service
 *
 * Copyright (C) 2010      by Roman Tsisyk <roman at tsisyk dot com>
 * Copyright (C) 2005-2008 by Vardhman Jain <vardhman at gmail dot com>
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009      by Luka Renko <lure at kubuntu dot org>
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

#include "yfwindow.h"

// Qt includes

#include <QButtonGroup>
#include <QCheckBox>
#include <QCloseEvent>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QMenu>
#include <QComboBox>
#include <QApplication>
#include <QMessageBox>
#include <QStack>
#include <QPointer>

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_version.h"
#include "dimageslist.h"
#include "yftalker.h"
#include "yfnewalbumdlg.h"
#include "digikam_debug.h"
#include "dinfointerface.h"
#include "wstoolutils.h"
#include "wslogindialog.h"
#include "yfwidget.h"
#include "previewloadthread.h"
#include "dprogresswdg.h"
#include "dmetadata.h"

namespace Digikam
{

class YFWindow::Private
{
public:

    explicit Private()
    {
        import               = false;
        widget               = 0;
        loginLabel           = 0;
        headerLabel          = 0;
        changeUserButton     = 0;
        albumsBox            = 0;
        newAlbumButton       = 0;
        reloadAlbumsButton   = 0;
        albumsCombo          = 0;
        accessCombo          = 0;
        hideOriginalCheck    = 0;
        disableCommentsCheck = 0;
        adultCheck           = 0;
        resizeCheck          = 0;
        dimensionSpin        = 0;
        imageQualitySpin     = 0;
        policyGroup          = 0;
        imgList              = 0;
        progressBar          = 0;
        iface                = 0;
    }

    bool                        import;
    YFWidget*                   widget;

    // User interface
    QLabel*                     loginLabel;
    QLabel*                     headerLabel;
    QPushButton*                changeUserButton;

    // albums
    QGroupBox*                  albumsBox;
    QPushButton*                newAlbumButton;
    QPushButton*                reloadAlbumsButton;
    QComboBox*                  albumsCombo;

    // upload settings
    QComboBox*                  accessCombo;
    QCheckBox*                  hideOriginalCheck;
    QCheckBox*                  disableCommentsCheck;
    QCheckBox*                  adultCheck;
    QCheckBox*                  resizeCheck;
    QSpinBox*                   dimensionSpin;
    QSpinBox*                   imageQualitySpin;
    QButtonGroup*               policyGroup;

    DImagesList*                imgList;
    DProgressWdg*               progressBar;
    DInfoInterface*             iface;

    // Backend
    QString                     tmpDir;
    YFTalker                    talker;

    QStack<YFPhoto>             transferQueue;

    DMetadata                   meta;

    // XMP id const for images
    static const char*          XMP_SERVICE_ID;
};
    
/*
 * This tag added to our images after uploading to Fotki web service
 */
const char* YFWindow::Private::XMP_SERVICE_ID = "Xmp.digiKam.yandexGPhotoId";


YFWindow::YFWindow(DInfoInterface* const iface, QWidget* const parent, bool import)
    : WSToolDialog(parent),
      d(new Private)
{
    d->iface                = iface;
    d->import               = import;
    d->tmpDir               = WSToolUtils::makeTemporaryDir("yandexfotki").absolutePath() + QLatin1Char('/');
    d->widget               = new YFWidget(this, d->iface, QString::fromLatin1("Yandex.Fotki"));
    d->loginLabel           = d->widget->getUserNameLabel();
    d->headerLabel          = d->widget->getHeaderLbl();
    d->changeUserButton     = d->widget->getChangeUserBtn();
    d->newAlbumButton       = d->widget->getNewAlbmBtn();
    d->reloadAlbumsButton   = d->widget->getReloadBtn();
    d->albumsCombo          = d->widget->getAlbumsCoB();
    d->resizeCheck          = d->widget->getResizeCheckBox();
    d->dimensionSpin        = d->widget->getDimensionSpB();
    d->imageQualitySpin     = d->widget->getImgQualitySpB();
    d->imgList              = d->widget->imagesList();
    d->progressBar          = d->widget->progressBar();
    d->accessCombo          = d->widget->accessCB();
    d->hideOriginalCheck    = d->widget->hideOriginalCB();
    d->disableCommentsCheck = d->widget->disableCommentsCB();
    d->adultCheck           = d->widget->adultCB();
    d->policyGroup          = d->widget->policyGB();
    d->albumsBox            = d->widget->getAlbumBox();

    connect(d->changeUserButton, SIGNAL(clicked()),
            this, SLOT(slotChangeUserClicked()));

    connect(d->newAlbumButton, SIGNAL(clicked()),
            this, SLOT(slotNewAlbumRequest()) );


    connect(d->reloadAlbumsButton, SIGNAL(clicked()),
            this, SLOT(slotReloadAlbumsRequest()) );

    setMainWidget(d->widget);
    d->widget->setMinimumSize(800, 600);

    // -- UI slots -----------------------------------------------------------------------

    connect(startButton(), &QPushButton::clicked,
            this, &YFWindow::slotStartTransfer);

    connect(this, &WSToolDialog::cancelClicked,
            this, &YFWindow::slotCancelClicked);

    connect(this, &QDialog::finished,
            this, &YFWindow::slotFinished);

    // -- Talker slots -------------------------------------------------------------------

    connect(&d->talker, SIGNAL(signalError()),
            this, SLOT(slotError()));

    connect(&d->talker, SIGNAL(signalGetSessionDone()),
            this, SLOT(slotGetSessionDone()));

    connect(&d->talker, SIGNAL(signalGetTokenDone()),
            this, SLOT(slotGetTokenDone()));

    connect(&d->talker, SIGNAL(signalGetServiceDone()),
            this, SLOT(slotGetServiceDone()));

    connect(&d->talker, SIGNAL(signalListAlbumsDone(QList<YandexFotkiAlbum>)),
            this, SLOT(slotListAlbumsDone(QList<YandexFotkiAlbum>)));

    connect(&d->talker, SIGNAL(signalListPhotosDone(QList<YFPhoto>)),
            this, SLOT(slotListPhotosDone(QList<YFPhoto>)));

    connect(&d->talker, SIGNAL(signalUpdatePhotoDone(YFPhoto&)),
            this, SLOT(slotUpdatePhotoDone(YFPhoto&)));

    connect(&d->talker, SIGNAL(signalUpdateAlbumDone()),
            this, SLOT(slotUpdateAlbumDone()));

    // read settings from file
    readSettings();
}

YFWindow::~YFWindow()
{
    reset();
    delete d;
}

void YFWindow::reactivate()
{
    d->imgList->loadImagesFromCurrentSelection();

    reset();
    authenticate(false);
    show();
}

void YFWindow::reset()
{
    d->talker.reset();
    updateControls(true);
    updateLabels();
}

void YFWindow::updateControls(bool val)
{
    if (val)
    {
        if (d->talker.isAuthenticated())
        {
            d->albumsBox->setEnabled(true);
            startButton()->setEnabled(true);
        }
        else
        {
            d->albumsBox->setEnabled(false);
            startButton()->setEnabled(false);
        }

        d->changeUserButton->setEnabled(true);
        setCursor(Qt::ArrowCursor);

        setRejectButtonMode(QDialogButtonBox::Close);
    }
    else
    {
        setCursor(Qt::WaitCursor);
        d->albumsBox->setEnabled(false);
        d->changeUserButton->setEnabled(false);
        startButton()->setEnabled(false);

        setRejectButtonMode(QDialogButtonBox::Cancel);
    }
}

void YFWindow::updateLabels()
{
    QString urltext;
    QString logintext;

    if (d->talker.isAuthenticated())
    {
        logintext = d->talker.login();
        urltext   = YFTalker::USERPAGE_URL.arg(d->talker.login());
        d->albumsBox->setEnabled(true);
    }
    else
    {
        logintext = i18n("Unauthorized");
        urltext   = YFTalker::USERPAGE_DEFAULT_URL;
        d->albumsCombo->clear();
    }

    d->loginLabel->setText(QString::fromLatin1("<b>%1</b>").arg(logintext));
    d->headerLabel->setText(QString::fromLatin1(
        "<b><h2><a href=\"%1\">"
        "<font color=\"#ff000a\">%2</font>"
        "<font color=\"black\">%3</font>"
        "<font color=\"#009d00\">%4</font>"
        "</a></h2></b>")
        .arg(urltext)
        .arg(i18nc("Yandex.Fotki", "Y"))
        .arg(i18nc("Yandex.Fotki", "andex."))
        .arg(i18nc("Yandex.Fotki", "Fotki")));
}

void YFWindow::readSettings()
{
    KConfig config;
    KConfigGroup grp = config.group("YandexFotki Settings");

    d->talker.setLogin(grp.readEntry("login", ""));
    // don't store tokens in plaintext
    //d->talker.setToken(grp.readEntry("token", ""));

    if (grp.readEntry("Resize", false))
    {
        d->resizeCheck->setChecked(true);
        d->dimensionSpin->setEnabled(true);
        d->imageQualitySpin->setEnabled(true);
    }
    else
    {
        d->resizeCheck->setChecked(false);
        d->dimensionSpin->setEnabled(false);
        d->imageQualitySpin->setEnabled(false);
    }

    d->dimensionSpin->setValue(grp.readEntry("Maximum Width", 1600));
    d->imageQualitySpin->setValue(grp.readEntry("Image Quality", 85));
    d->policyGroup->button(grp.readEntry("Sync policy", 0))->setChecked(true);
}

void YFWindow::writeSettings()
{
    KConfig config;
    KConfigGroup grp = config.group("YandexFotki Settings");

    grp.writeEntry("token", d->talker.token());
    // don't store tokens in plaintext
    //grp.writeEntry("login", d->talker.login());

    grp.writeEntry("Resize", d->resizeCheck->isChecked());
    grp.writeEntry("Maximum Width", d->dimensionSpin->value());
    grp.writeEntry("Image Quality", d->imageQualitySpin->value());
    grp.writeEntry("Sync policy", d->policyGroup->checkedId());
}

void YFWindow::slotChangeUserClicked()
{
    // force authenticate window
    authenticate(true);
}

void YFWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    slotFinished();
    e->accept();
}

void YFWindow::slotFinished()
{
    writeSettings();
    reset();
}

void YFWindow::slotCancelClicked()
{
    d->talker.cancel();
    updateControls(true);
}

/*
void YFWindow::cancelProcessing()
{
    d->talker.cancel();
    d->transferQueue.clear();
    d->imgList->processed(false);
    progressBar()->hide();
}
*/

void YFWindow::authenticate(bool forceAuthWindow)
{
    // update credentials
    if (forceAuthWindow || d->talker.login().isNull() || d->talker.password().isNull())
    {
        WSLoginDialog* const dlg = new WSLoginDialog(this, QString::fromLatin1("Yandex.Fotki"), d->talker.login(), QString());

        if (dlg->exec() == QDialog::Accepted)
        {
            d->talker.setLogin(dlg->login());
            d->talker.setPassword(dlg->password());
        }
        else
        {
            // don't change anything
            return;
        }

        delete dlg;
    }
/*
    else
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Checking old token...";
        d->talker.checkToken();
        return;
    }
*/

    // if new credentials non-empty, authenticate
    if (!d->talker.login().isEmpty() && !d->talker.password().isEmpty())
    {
        // cancel all tasks first
        reset();

        // start authentication chain
        updateControls(false);
        d->talker.getService();
    }
    else
    {
        // we don't have valid credentials, so cancel all transfers and reset
        reset();
    }

/*
    progressBar()->show();
    progressBar()->setFormat("");
*/
}

void YFWindow::slotListPhotosDone(const QList <YFPhoto>& photosList)
{
    if (d->import)
    {
        slotListPhotosDoneForDownload(photosList);
    }
    else
    {
        slotListPhotosDoneForUpload(photosList);
    }
}

void YFWindow::slotListPhotosDoneForDownload(const QList <YFPhoto>& photosList)
{
    Q_UNUSED(photosList);
    updateControls(true);
}

void YFWindow::slotListPhotosDoneForUpload(const QList <YFPhoto>& photosList)
{
    updateControls(true);

    QMap<QString, int> dups;
    int i = 0;

    foreach(const YFPhoto& photo, photosList)
    {
        dups.insert(photo.urn(), i);
        i++;
    }

    YFWidget::UpdatePolicy policy = static_cast<YFWidget::UpdatePolicy>(d->policyGroup->checkedId());
    const YFPhoto::Access access  = static_cast<YFPhoto::Access>(
                                            d->accessCombo->itemData(d->accessCombo->currentIndex()).toInt());

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "";
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "----";
    d->transferQueue.clear();

    foreach(const QUrl& url, d->imgList->imageUrls(true))
    {
        DItemInfo info(d->iface->itemInfo(url.toLocalFile()));

        // check if photo alredy uploaded

        int oldPhotoId = -1;

        if (d->meta.load(url.toLocalFile()))
        {
            QString localId = d->meta.getXmpTagString(d->XMP_SERVICE_ID);
            oldPhotoId      = dups.value(localId, -1);
        }

        // get tags
        QStringList tags = info.tagsPath();
        bool updateFile  = true;

        QSet<QString> oldtags;

        if (oldPhotoId != -1)
        {
            if (policy == YFWidget::UpdatePolicy::POLICY_SKIP)
            {
                qCDebug(DIGIKAM_WEBSERVICES_LOG) << "SKIP: " << url;
                continue;
            }

            // old photo copy
            d->transferQueue.push(photosList[oldPhotoId]);

            if (policy == YFWidget::UpdatePolicy::POLICY_UPDATE_MERGE)
            {
                foreach(const QString& t, d->transferQueue.top().tags)
                {
                    oldtags.insert(t);
                }
            }

            if (policy != YFWidget::UpdatePolicy::POLICY_ADDNEW)
            {
                updateFile = false;
            }
        }
        else
        {
            // empty photo
            d->transferQueue.push(YFPhoto());
        }

        YFPhoto& photo = d->transferQueue.top();
        // TODO: updateFile is not used
        photo.setOriginalUrl(url.toLocalFile());
        photo.setTitle(info.name());
        photo.setSummary(info.comment());
        photo.setAccess(access);
        photo.setHideOriginal(d->hideOriginalCheck->isChecked());
        photo.setDisableComments(d->disableCommentsCheck->isChecked());

        // adult flag can't be removed, API restrictions
        if (!photo.isAdult())
            photo.setAdult(d->adultCheck->isChecked());

        foreach(const QString& t, tags)
        {
            if (!oldtags.contains(t))
            {
                photo.tags.append(t);
            }
        }

        if (updateFile)
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "METADATA + IMAGE: " << url;
        }
        else
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "METADATA: " << url;
        }
    }

    if (d->transferQueue.isEmpty())
    {
        return;    // nothing to do
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "----";
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "";

    updateControls(false);
    updateNextPhoto();
}

void YFWindow::updateNextPhoto()
{
    // select only one image from stack
    while (!d->transferQueue.isEmpty())
    {
        YFPhoto& photo = d->transferQueue.top();

        if (!photo.originalUrl().isNull())
        {
            QImage image = PreviewLoadThread::loadHighQualitySynchronously(photo.originalUrl()).copyQImage();

            if (image.isNull())
            {
                image.load(photo.originalUrl());
            }

            photo.setLocalUrl(d->tmpDir + QFileInfo(photo.originalUrl())
                              .baseName()
                              .trimmed() + QString::fromLatin1(".jpg"));

            bool prepared = false;

            if (!image.isNull())
            {
                // get temporary file name

                // rescale image if requested
                int maxDim = d->dimensionSpin->value();

                if (d->resizeCheck->isChecked() && (image.width() > maxDim || image.height() > maxDim))
                {
                    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Resizing to " << maxDim;
                    image = image.scaled(maxDim, maxDim, Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);
                }

                // copy meta data to temporary image

                if (image.save(photo.localUrl(), "JPEG", d->imageQualitySpin->value()))
                {
                    if (d->meta.load(photo.originalUrl()))
                    {
                        d->meta.setImageDimensions(image.size());
                        d->meta.setImageOrientation(MetaEngine::ORIENTATION_NORMAL);
                        d->meta.setImageProgramId(QString::fromLatin1("digiKam"), digiKamVersion());
                        d->meta.setMetadataWritingMode((int)DMetadata::WRITETOIMAGEONLY);
                        d->meta.save(photo.localUrl());
                        prepared = true;
                    }
                }
            }

            if (!prepared)
            {
                if (QMessageBox::question(this, i18n("Processing Failed"),
                                  i18n("Failed to prepare image %1\n"
                                       "Do you want to continue?", photo.originalUrl()))
                    != QMessageBox::Yes)
                {
                    // stop uploading
                    d->transferQueue.clear();
                    continue;
                }
                else
                {
                    d->transferQueue.pop();
                    continue;
                }
            }
        }

        const YandexFotkiAlbum& album = d->talker.albums().at(d->albumsCombo->currentIndex());

        qCDebug(DIGIKAM_WEBSERVICES_LOG) << photo.originalUrl();

        d->talker.updatePhoto(photo, album);

        return;
    }

    updateControls(true);

    QMessageBox::information(this, QString(), i18n("Images has been uploaded"));
    return;
}

void YFWindow::slotNewAlbumRequest()
{
    YandexFotkiAlbum album;
    QPointer<YFNewAlbumDlg> dlg = new YFNewAlbumDlg(this, album);

    if (dlg->exec() == QDialog::Accepted)
    {
        updateControls(false);
        d->talker.updateAlbum(album);
    }

    delete dlg;
}

void YFWindow::slotReloadAlbumsRequest()
{
    updateControls(false);
    d->talker.listAlbums();
}

void YFWindow::slotStartTransfer()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "slotStartTransfer invoked";

    if (d->albumsCombo->currentIndex() == -1 || d->albumsCombo->count() == 0)
    {
        QMessageBox::information(this, QString(), i18n("Please select album first"));
        return;
    }

    // TODO: import support
    if (!d->import)
    {
        // list photos of the album, then start upload
        const YandexFotkiAlbum& album = d->talker.albums().at(d->albumsCombo->currentIndex());

        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Album selected" << album;

        updateControls(false);
        d->talker.listPhotos(album);
    }
}

void YFWindow::slotError()
{
    switch (d->talker.state())
    {
        case YFTalker::STATE_GETSESSION_ERROR:
            QMessageBox::critical(this, QString(), i18n("Session error"));
            break;
        case YFTalker::STATE_GETTOKEN_ERROR:
            QMessageBox::critical(this, QString(), i18n("Token error"));
            break;
        case YFTalker::STATE_INVALID_CREDENTIALS:
            QMessageBox::critical(this, QString(), i18n("Invalid credentials"));
//            authenticate(true);
            break;
        case YFTalker::STATE_GETSERVICE_ERROR:
            QMessageBox::critical(this, QString(), i18n("Cannot get service document"));
            break;
/*
        case YFTalker::STATE_CHECKTOKEN_INVALID:
            // remove old expired token
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "CheckToken invalid";
            d->talker.setToken(QString());
            // don't say anything, simple show new auth window
            authenticate(true);
            break;
*/
        case YFTalker::STATE_LISTALBUMS_ERROR:
            d->albumsCombo->clear();
            QMessageBox::critical(this, QString(), i18n("Cannot list albums"));
            break;
        case YFTalker::STATE_LISTPHOTOS_ERROR:
            QMessageBox::critical(this, QString(), i18n("Cannot list photos"));
            break;
        case YFTalker::STATE_UPDATEALBUM_ERROR:
            QMessageBox::critical(this, QString(), i18n("Cannot update album info"));
            break;
        case YFTalker::STATE_UPDATEPHOTO_FILE_ERROR:
        case YFTalker::STATE_UPDATEPHOTO_INFO_ERROR:
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "UpdatePhotoError";

            if (QMessageBox::question(this, i18n("Uploading Failed"),
                                      i18n("Failed to upload image %1\n"
                                           "Do you want to continue?",
                                      d->transferQueue.top().originalUrl()))
                != QMessageBox::Yes)
            {
                // clear upload stack
                d->transferQueue.clear();
            }
            else
            {
                // cancel current operation
                d->talker.cancel();
                // remove only bad image
                d->transferQueue.pop();
                // and try next
                updateNextPhoto();
                return;
            }
            break;
        default:
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Unhandled error" << d->talker.state();
            QMessageBox::critical(this, QString(), i18n("Unknown error"));
    }

    // cancel current operation
    d->talker.cancel();
    updateControls(true);
}

void YFWindow::slotGetServiceDone()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "GetService Done";
    d->talker.getSession();
}

void YFWindow::slotGetSessionDone()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "GetSession Done";
    d->talker.getToken();
}

void YFWindow::slotGetTokenDone()
{
    updateLabels();
    slotReloadAlbumsRequest();
}

void YFWindow::slotListAlbumsDone(const QList<YandexFotkiAlbum>& albumsList)
{
    d->albumsCombo->clear();

    foreach(const YandexFotkiAlbum& album, albumsList)
    {
        QString albumIcon;

        if (album.isProtected())
        {
            albumIcon = QString::fromLatin1("folder-locked");
        }
        else
        {
            albumIcon = QString::fromLatin1("folder-image");
        }

        d->albumsCombo->addItem(QIcon::fromTheme(albumIcon), album.toString());
    }

    d->albumsCombo->setEnabled(true);
    updateControls(true);
}

void YFWindow::slotUpdatePhotoDone(YFPhoto& photo)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "photoUploaded" << photo;

    if (d->meta.supportXmp() && d->meta.canWriteXmp(photo.originalUrl()) &&
        d->meta.load(photo.originalUrl()))
    {
        // ignore errors here
        if (d->meta.setXmpTagString(d->XMP_SERVICE_ID, photo.urn()) &&
            d->meta.save(photo.originalUrl()))
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "MARK: " << photo.originalUrl();
        }
    }

    d->transferQueue.pop();
    updateNextPhoto();
}

void YFWindow::slotUpdateAlbumDone()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Album created";
    d->albumsCombo->clear();
    d->talker.listAlbums();
}

} // namespace Digikam
