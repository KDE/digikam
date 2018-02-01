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
#include "exportutils.h"
#include "logindialog.h"
#include "yfwidget.h"
#include "previewloadthread.h"

namespace Digikam
{

/*
 * This tag added to our images after uploading to Fotki web service
 */
const char* YFWindow::XMP_SERVICE_ID = "Xmp.digiKam.yandexGPhotoId";

YFWindow::YFWindow(DInfoInterface* const iface, QWidget* const parent, bool import)
    : WSToolDialog(parent)
{
    m_iface  = iface;
    m_import = import;
    m_tmpDir = ExportUtils::makeTemporaryDir("yandexfotki").absolutePath() + QLatin1Char('/');
    m_widget = new YFWidget(this, m_iface, QString::fromLatin1("Yandex.Fotki"));

    m_loginLabel           = m_widget->getUserNameLabel();
    m_headerLabel          = m_widget->getHeaderLbl();
    m_changeUserButton     = m_widget->getChangeUserBtn();
    m_newAlbumButton       = m_widget->getNewAlbmBtn();
    m_reloadAlbumsButton   = m_widget->getReloadBtn();
    m_albumsCombo          = m_widget->getAlbumsCoB();
    m_resizeCheck          = m_widget->getResizeCheckBox();
    m_dimensionSpin        = m_widget->getDimensionSpB();
    m_imageQualitySpin     = m_widget->getImgQualitySpB();
    m_imgList              = m_widget->imagesList();
    m_progressBar          = m_widget->progressBar();
    m_accessCombo          = m_widget->m_accessCombo;
    m_hideOriginalCheck    = m_widget->m_hideOriginalCheck;
    m_disableCommentsCheck = m_widget->m_disableCommentsCheck;
    m_adultCheck           = m_widget->m_adultCheck;
    m_policyGroup          = m_widget->m_policyGroup;
    m_albumsBox            = m_widget->getAlbumBox();

    connect(m_changeUserButton, SIGNAL(clicked()),
            this, SLOT(slotChangeUserClicked()));

    connect(m_newAlbumButton, SIGNAL(clicked()),
            this, SLOT(slotNewAlbumRequest()) );


    connect(m_reloadAlbumsButton, SIGNAL(clicked()),
            this, SLOT(slotReloadAlbumsRequest()) );

    setMainWidget(m_widget);
    m_widget->setMinimumSize(800, 600);

    // -- UI slots -----------------------------------------------------------------------

    connect(startButton(), &QPushButton::clicked,
            this, &YFWindow::slotStartTransfer);

    connect(this, &WSToolDialog::cancelClicked,
            this, &YFWindow::slotCancelClicked);

    connect(this, &QDialog::finished,
            this, &YFWindow::slotFinished);

    // -- Talker slots -------------------------------------------------------------------

    connect(&m_talker, SIGNAL(signalError()),
            this, SLOT(slotError()));

    connect(&m_talker, SIGNAL(signalGetSessionDone()),
            this, SLOT(slotGetSessionDone()));

    connect(&m_talker, SIGNAL(signalGetTokenDone()),
            this, SLOT(slotGetTokenDone()));

    connect(&m_talker, SIGNAL(signalGetServiceDone()),
            this, SLOT(slotGetServiceDone()));

    connect(&m_talker, SIGNAL(signalListAlbumsDone(QList<YandexFotkiAlbum>)),
            this, SLOT(slotListAlbumsDone(QList<YandexFotkiAlbum>)));

    connect(&m_talker, SIGNAL(signalListPhotosDone(QList<YFPhoto>)),
            this, SLOT(slotListPhotosDone(QList<YFPhoto>)));

    connect(&m_talker, SIGNAL(signalUpdatePhotoDone(YFPhoto&)),
            this, SLOT(slotUpdatePhotoDone(YFPhoto&)));

    connect(&m_talker, SIGNAL(signalUpdateAlbumDone()),
            this, SLOT(slotUpdateAlbumDone()));

    // read settings from file
    readSettings();
}

YFWindow::~YFWindow()
{
    reset();
}

void YFWindow::reactivate()
{
    m_imgList->loadImagesFromCurrentSelection();

    reset();
    authenticate(false);
    show();
}

void YFWindow::reset()
{
    m_talker.reset();
    updateControls(true);
    updateLabels();
}

void YFWindow::updateControls(bool val)
{
    if (val)
    {
        if (m_talker.isAuthenticated())
        {
            m_albumsBox->setEnabled(true);
            startButton()->setEnabled(true);
        }
        else
        {
            m_albumsBox->setEnabled(false);
            startButton()->setEnabled(false);
        }

        m_changeUserButton->setEnabled(true);
        setCursor(Qt::ArrowCursor);

        setRejectButtonMode(QDialogButtonBox::Close);
    }
    else
    {
        setCursor(Qt::WaitCursor);
        m_albumsBox->setEnabled(false);
        m_changeUserButton->setEnabled(false);
        startButton()->setEnabled(false);

        setRejectButtonMode(QDialogButtonBox::Cancel);
    }
}

void YFWindow::updateLabels()
{
    QString urltext;
    QString logintext;

    if (m_talker.isAuthenticated())
    {
        logintext = m_talker.login();
        urltext = YFTalker::USERPAGE_URL.arg(m_talker.login());
        m_albumsBox->setEnabled(true);
    }
    else
    {
        logintext = i18n("Unauthorized");
        urltext = YFTalker::USERPAGE_DEFAULT_URL;
        m_albumsCombo->clear();
    }

    m_loginLabel->setText(QString::fromLatin1("<b>%1</b>").arg(logintext));
    m_headerLabel->setText(QString::fromLatin1(
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

    m_talker.setLogin(grp.readEntry("login", ""));
    // don't store tokens in plaintext
    //m_talker.setToken(grp.readEntry("token", ""));

    if (grp.readEntry("Resize", false))
    {
        m_resizeCheck->setChecked(true);
        m_dimensionSpin->setEnabled(true);
        m_imageQualitySpin->setEnabled(true);
    }
    else
    {
        m_resizeCheck->setChecked(false);
        m_dimensionSpin->setEnabled(false);
        m_imageQualitySpin->setEnabled(false);
    }

    m_dimensionSpin->setValue(grp.readEntry("Maximum Width", 1600));
    m_imageQualitySpin->setValue(grp.readEntry("Image Quality", 85));
    m_policyGroup->button(grp.readEntry("Sync policy", 0))->setChecked(true);
}

void YFWindow::writeSettings()
{
    KConfig config;
    KConfigGroup grp = config.group("YandexFotki Settings");

    grp.writeEntry("token", m_talker.token());
    // don't store tokens in plaintext
    //grp.writeEntry("login", m_talker.login());

    grp.writeEntry("Resize", m_resizeCheck->isChecked());
    grp.writeEntry("Maximum Width", m_dimensionSpin->value());
    grp.writeEntry("Image Quality", m_imageQualitySpin->value());
    grp.writeEntry("Sync policy", m_policyGroup->checkedId());
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
    m_talker.cancel();
    updateControls(true);
}

/*
void YFWindow::cancelProcessing()
{
    m_talker.cancel();
    m_transferQueue.clear();
    m_imgList->processed(false);
    progressBar()->hide();
}
*/

void YFWindow::authenticate(bool forceAuthWindow)
{
    // update credentials
    if (forceAuthWindow || m_talker.login().isNull() || m_talker.password().isNull())
    {
        LoginDialog* const dlg = new LoginDialog(this, QString::fromLatin1("Yandex.Fotki"), m_talker.login(), QString());

        if (dlg->exec() == QDialog::Accepted)
        {
            m_talker.setLogin(dlg->login());
            m_talker.setPassword(dlg->password());
        }
        else
        {
            // don't change anything
            return;
        }

        delete dlg;
    }

    /*else
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Checking old token...";
        m_talker.checkToken();
        return;
    }
    */

    // if new credentials non-empty, authenticate
    if (!m_talker.login().isEmpty() && !m_talker.password().isEmpty())
    {
        // cancel all tasks first
        reset();

        // start authentication chain
        updateControls(false);
        m_talker.getService();
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
    if (m_import)
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

    YFWidget::UpdatePolicy policy = static_cast<YFWidget::UpdatePolicy>(m_policyGroup->checkedId());
    const YFPhoto::Access access  = static_cast<YFPhoto::Access>(
                                            m_accessCombo->itemData(m_accessCombo->currentIndex()).toInt());

    qCDebug(DIGIKAM_GENERAL_LOG) << "";
    qCDebug(DIGIKAM_GENERAL_LOG) << "----";
    m_transferQueue.clear();

    foreach(const QUrl& url, m_imgList->imageUrls(true))
    {
        DItemInfo info(m_iface->itemInfo(url.toLocalFile()));
    
        // check if photo alredy uploaded

        int oldPhotoId = -1;

        if (m_meta.load(url.toLocalFile()))
        {
            QString localId = m_meta.getXmpTagString(XMP_SERVICE_ID);
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
                qCDebug(DIGIKAM_GENERAL_LOG) << "SKIP: " << url;
                continue;
            }

            // old photo copy
            m_transferQueue.push(photosList[oldPhotoId]);

            if (policy == YFWidget::UpdatePolicy::POLICY_UPDATE_MERGE)
            {
                foreach(const QString& t, m_transferQueue.top().tags)
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
            m_transferQueue.push(YFPhoto());
        }

        YFPhoto& photo = m_transferQueue.top();
        // TODO: updateFile is not used
        photo.setOriginalUrl(url.toLocalFile());
        photo.setTitle(info.name());
        photo.setSummary(info.comment());
        photo.setAccess(access);
        photo.setHideOriginal(m_hideOriginalCheck->isChecked());
        photo.setDisableComments(m_disableCommentsCheck->isChecked());

        // adult flag can't be removed, API restrictions
        if (!photo.isAdult())
            photo.setAdult(m_adultCheck->isChecked());

        foreach(const QString& t, tags)
        {
            if (!oldtags.contains(t))
            {
                photo.tags.append(t);
            }
        }

        if (updateFile)
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "METADATA + IMAGE: " << url;
        }
        else
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "METADATA: " << url;
        }
    }

    if (m_transferQueue.isEmpty())
    {
        return;    // nothing to do
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "----";
    qCDebug(DIGIKAM_GENERAL_LOG) << "";

    updateControls(false);
    updateNextPhoto();
}

void YFWindow::updateNextPhoto()
{
    // select only one image from stack
    while (!m_transferQueue.isEmpty())
    {
        YFPhoto& photo = m_transferQueue.top();

        if (!photo.originalUrl().isNull())
        {
            QImage image = PreviewLoadThread::loadHighQualitySynchronously(photo.originalUrl()).copyQImage();

            if (image.isNull())
            {
                image.load(photo.originalUrl());
            }

            photo.setLocalUrl(m_tmpDir + QFileInfo(photo.originalUrl())
                              .baseName()
                              .trimmed() + QString::fromLatin1(".jpg"));

            bool prepared = false;

            if (!image.isNull())
            {
                // get temporary file name

                // rescale image if requested
                int maxDim = m_dimensionSpin->value();

                if (m_resizeCheck->isChecked() && (image.width() > maxDim || image.height() > maxDim))
                {
                    qCDebug(DIGIKAM_GENERAL_LOG) << "Resizing to " << maxDim;
                    image = image.scaled(maxDim, maxDim, Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);
                }

                // copy meta data to temporary image

                if (image.save(photo.localUrl(), "JPEG", m_imageQualitySpin->value()))
                {
                    if (m_meta.load(photo.originalUrl()))
                    {
                        m_meta.setImageDimensions(image.size());
                        m_meta.setImageOrientation(MetaEngine::ORIENTATION_NORMAL);
                        m_meta.setImageProgramId(QString::fromLatin1("digiKam"), digiKamVersion());
                        m_meta.setMetadataWritingMode((int)DMetadata::WRITETOIMAGEONLY);
                        m_meta.save(photo.localUrl());
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
                    m_transferQueue.clear();
                    continue;
                }
                else
                {
                    m_transferQueue.pop();
                    continue;
                }
            }
        }

        const YandexFotkiAlbum& album = m_talker.albums().at(m_albumsCombo->currentIndex());

        qCDebug(DIGIKAM_GENERAL_LOG) << photo.originalUrl();

        m_talker.updatePhoto(photo, album);

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
        m_talker.updateAlbum(album);
    }

    delete dlg;
}

void YFWindow::slotReloadAlbumsRequest()
{
    updateControls(false);
    m_talker.listAlbums();
}

void YFWindow::slotStartTransfer()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "slotStartTransfer invoked";

    if (m_albumsCombo->currentIndex() == -1 || m_albumsCombo->count() == 0)
    {
        QMessageBox::information(this, QString(), i18n("Please select album first"));
        return;
    }

    // TODO: import support
    if (!m_import)
    {
        // list photos of the album, then start upload
        const YandexFotkiAlbum& album = m_talker.albums().at(m_albumsCombo->currentIndex());

        qCDebug(DIGIKAM_GENERAL_LOG) << "Album selected" << album;

        updateControls(false);
        m_talker.listPhotos(album);
    }
}

void YFWindow::slotError()
{
    switch (m_talker.state())
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
            qCDebug(DIGIKAM_GENERAL_LOG) << "CheckToken invalid";
            m_talker.setToken(QString());
            // don't say anything, simple show new auth window
            authenticate(true);
            break;
*/
        case YFTalker::STATE_LISTALBUMS_ERROR:
            m_albumsCombo->clear();
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
            qCDebug(DIGIKAM_GENERAL_LOG) << "UpdatePhotoError";

            if (QMessageBox::question(this, i18n("Uploading Failed"),
                                      i18n("Failed to upload image %1\n"
                                           "Do you want to continue?",
                                      m_transferQueue.top().originalUrl()))
                != QMessageBox::Yes)
            {
                // clear upload stack
                m_transferQueue.clear();
            }
            else
            {
                // cancel current operation
                m_talker.cancel();
                // remove only bad image
                m_transferQueue.pop();
                // and try next
                updateNextPhoto();
                return;
            }
            break;
        default:
            qCDebug(DIGIKAM_GENERAL_LOG) << "Unhandled error" << m_talker.state();
            QMessageBox::critical(this, QString(), i18n("Unknown error"));
    }

    // cancel current operation
    m_talker.cancel();
    updateControls(true);
}

void YFWindow::slotGetServiceDone()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "GetService Done";
    m_talker.getSession();
}

void YFWindow::slotGetSessionDone()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "GetSession Done";
    m_talker.getToken();
}

void YFWindow::slotGetTokenDone()
{
    updateLabels();
    slotReloadAlbumsRequest();
}

void YFWindow::slotListAlbumsDone(const QList<YandexFotkiAlbum>& albumsList)
{
    m_albumsCombo->clear();

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

        m_albumsCombo->addItem(QIcon::fromTheme(albumIcon), album.toString());
    }

    m_albumsCombo->setEnabled(true);
    updateControls(true);
}

void YFWindow::slotUpdatePhotoDone(YFPhoto& photo)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "photoUploaded" << photo;

    if (m_meta.supportXmp() && m_meta.canWriteXmp(photo.originalUrl()) &&
        m_meta.load(photo.originalUrl()))
    {
        // ignore errors here
        if (m_meta.setXmpTagString(XMP_SERVICE_ID, photo.urn()) &&
            m_meta.save(photo.originalUrl()))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "MARK: " << photo.originalUrl();
        }
    }

    m_transferQueue.pop();
    updateNextPhoto();
}

void YFWindow::slotUpdateAlbumDone()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Album created";
    m_albumsCombo->clear();
    m_talker.listAlbums();
}

} // namespace Digikam
