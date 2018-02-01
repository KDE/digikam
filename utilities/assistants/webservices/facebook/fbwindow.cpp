/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-17-26
 * Description : a tool to export items to Facebook web service
 *
 * Copyright (C) 2005-2008 by Vardhman Jain <vardhman at gmail dot com>
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008-2009 by Luka Renko <lure at kubuntu dot org>
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

#include "fbwindow.h"

// Qt includes

#include <QWindow>
#include <QComboBox>
#include <QMenu>
#include <QFileInfo>
#include <QCheckBox>
#include <QCloseEvent>
#include <QSpinBox>
#include <QMessageBox>
#include <QPointer>

// KDE includes

#include <kconfig.h>
#include <klocalizedstring.h>
#include <kwindowconfig.h>

// Local includes

#include "digikam_debug.h"
#include "dmetadata.h"
#include "dimageslist.h"
#include "digikam_version.h"
#include "dprogresswdg.h"
#include "wstoolutils.h"
#include "fbitem.h"
#include "fbtalker.h"
#include "fbwidget.h"
#include "fbnewalbumdlg.h"
#include "previewloadthread.h"

namespace Digikam
{

class FbWindow::Private
{
public:

    Private(QWidget* const widget, DInfoInterface* const iface)
    {
        m_widget          = new FbWidget(widget, iface, QString::fromLatin1("Facebook"));
        m_imgList         = m_widget->imagesList();
        m_progressBar     = m_widget->progressBar();
        m_changeUserBtn   = m_widget->getChangeUserBtn();
        m_albumsCoB       = m_widget->getAlbumsCoB();
        m_newAlbumBtn     = m_widget->getNewAlbmBtn();
        m_reloadAlbumsBtn = m_widget->getReloadBtn();
        m_resizeChB       = m_widget->getResizeCheckBox();
        m_dimensionSpB    = m_widget->getDimensionSpB();
        m_imageQualitySpB = m_widget->getImgQualitySpB();
    }

    FbWidget*                      m_widget;
    DImagesList*                   m_imgList;
    QPushButton*                   m_changeUserBtn;
    QComboBox*                     m_albumsCoB;
    QPushButton*                   m_newAlbumBtn;
    QPushButton*                   m_reloadAlbumsBtn;
    QCheckBox*                     m_resizeChB;
    QSpinBox*                      m_dimensionSpB;
    QSpinBox*                      m_imageQualitySpB;
    DProgressWdg*                  m_progressBar;
};

FbWindow::FbWindow(DInfoInterface* const iface,
                   QWidget* const /*parent*/)
    : WSToolDialog(0),
      d(new Private(this, iface))
{
    m_tmpPath.clear();
    m_tmpDir      = WSToolUtils::makeTemporaryDir("facebook").absolutePath() + QLatin1Char('/');;
    m_imagesCount = 0;
    m_imagesTotal = 0;
    m_iface       = iface;

    setMainWidget(d->m_widget);
    setWindowIcon(QIcon::fromTheme(QString::fromLatin1("facebook")));
    setModal(false);

    setWindowTitle(i18n("Export to Facebook Web Service"));

    startButton()->setText(i18n("Start Upload"));
    startButton()->setToolTip(i18n("Start upload to Facebook web service"));

    d->m_widget->setMinimumSize(700, 500);
    // ------------------------------------------------------------------------

    connect(d->m_imgList, SIGNAL(signalImageListChanged()),
            this, SLOT(slotImageListChanged()));

    connect(d->m_changeUserBtn, SIGNAL(clicked()),
            this, SLOT(slotUserChangeRequest()));

    connect(d->m_newAlbumBtn, SIGNAL(clicked()),
            this, SLOT(slotNewAlbumRequest()));

    connect(d->m_widget, SIGNAL(reloadAlbums(long long)),
            this, SLOT(slotReloadAlbumsRequest(long long)));

    connect(startButton(), SIGNAL(clicked()),
            this, SLOT(slotStartTransfer()));

    connect(this, SIGNAL(finished(int)),
            this, SLOT(slotFinished()));

    connect(this, SIGNAL(cancelClicked()),
            this, SLOT(slotCancelClicked()));

    m_albumDlg = new FbNewAlbumDlg(this, QString::fromLatin1("Facebook"));

    // ------------------------------------------------------------------------

    m_talker = new FbTalker(this);

    connect(m_talker, SIGNAL(signalBusy(bool)),
            this, SLOT(slotBusy(bool)));

    connect(m_talker, SIGNAL(signalLoginProgress(int,int,QString)),
            this, SLOT(slotLoginProgress(int,int,QString)));

    connect(m_talker, SIGNAL(signalLoginDone(int,QString)),
            this, SLOT(slotLoginDone(int,QString)));

    connect(m_talker, SIGNAL(signalAddPhotoDone(int,QString)),
            this, SLOT(slotAddPhotoDone(int,QString)));

    connect(m_talker, SIGNAL(signalCreateAlbumDone(int,QString,QString)),
            this, SLOT(slotCreateAlbumDone(int,QString,QString)));

    connect(m_talker, SIGNAL(signalListAlbumsDone(int,QString,QList<FbAlbum>)),
            this, SLOT(slotListAlbumsDone(int,QString,QList<FbAlbum>)));

    connect(d->m_progressBar, SIGNAL(signalProgressCanceled()),
            this, SLOT(slotStopAndCloseProgressBar()));

    // ------------------------------------------------------------------------

    readSettings();

    qCDebug(DIGIKAM_GENERAL_LOG) << "Calling Login method";
    buttonStateChange(m_talker->loggedIn());
    authenticate();
}

FbWindow::~FbWindow()
{
    delete m_albumDlg;
    delete m_talker;
    delete d;
}

void FbWindow::slotStopAndCloseProgressBar()
{
    // Cancel the operation
    slotCancelClicked();

    // Write settings and tidy up
    slotFinished();

    // Close the dialog
    reject();
}

void FbWindow::slotFinished()
{
    writeSettings();
    d->m_imgList->listView()->clear();
    d->m_progressBar->progressCompleted();
}

void FbWindow::slotCancelClicked()
{
    setRejectButtonMode(QDialogButtonBox::Close);
    m_talker->cancel();
    m_transferQueue.clear();
    d->m_imgList->cancelProcess();
    d->m_progressBar->hide();
    d->m_progressBar->progressCompleted();
}

void FbWindow::reactivate()
{
    d->m_imgList->loadImagesFromCurrentSelection();
    show();
}

void FbWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    slotFinished();
    e->accept();
}

void FbWindow::readSettings()
{
    KConfig config;
    KConfigGroup grp = config.group("Facebook Settings");
    m_accessToken    = grp.readEntry("Access Token");
    m_sessionExpires = grp.readEntry("Session Expires", 0);

    if (m_accessToken.isEmpty())
    {
        m_sessionKey     = grp.readEntry("Session Key");
        m_sessionSecret  = grp.readEntry("Session Secret");
    }

    m_currentAlbumID = grp.readEntry("Current Album", QString());

    if (grp.readEntry("Resize", false))
    {
        d->m_resizeChB->setChecked(true);
        d->m_dimensionSpB->setEnabled(true);
        d->m_imageQualitySpB->setEnabled(true);
    }
    else
    {
        d->m_resizeChB->setChecked(false);
        d->m_dimensionSpB->setEnabled(false);
        d->m_imageQualitySpB->setEnabled(false);
    }

    d->m_dimensionSpB->setValue(grp.readEntry("Maximum Width", 604));
    d->m_imageQualitySpB->setValue(grp.readEntry("Image Quality", 85));

    winId();
    KConfigGroup dialogGroup = config.group("Facebook Export Dialog");
    KWindowConfig::restoreWindowSize(windowHandle(), dialogGroup);
    resize(windowHandle()->size());
}

void FbWindow::writeSettings()
{
    KConfig config;
    KConfigGroup grp = config.group("Facebook Settings");
    grp.writeEntry("Access Token",    m_accessToken);

    /* If we have both access token and session key, then we have just converted one into the other. */
    if (! m_accessToken.isEmpty())
    {
        if (! m_sessionKey.isEmpty())
        {
            grp.deleteEntry("Session Key");
        }

        if (! m_sessionSecret.isEmpty())
        {
            grp.deleteEntry("Session Secret");
        }
    }

    grp.writeEntry("Session Expires", m_sessionExpires);
    grp.writeEntry("Current Album",   m_currentAlbumID);
    grp.writeEntry("Resize",          d->m_resizeChB->isChecked());
    grp.writeEntry("Maximum Width",   d->m_dimensionSpB->value());
    grp.writeEntry("Image Quality",   d->m_imageQualitySpB->value());

    KConfigGroup dialogGroup = config.group("Facebook Export Dialog");
    KWindowConfig::saveWindowSize(windowHandle(), dialogGroup);

    config.sync();
}

void FbWindow::authenticate()
{
    setRejectButtonMode(QDialogButtonBox::Cancel);
    d->m_progressBar->show();
    d->m_progressBar->setFormat(QString::fromLatin1(""));

    // Converting old world session keys into OAuth2 tokens
    if (! m_sessionKey.isEmpty() && m_accessToken.isEmpty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Exchanging session tokens to OAuth";
        m_talker->exchangeSession(m_sessionKey);
    }
    else
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Calling Login method";
        m_talker->authenticate(m_accessToken, m_sessionExpires);
    }
}

void FbWindow::slotLoginProgress(int step, int maxStep, const QString& label)
{
    DProgressWdg* const progressBar = d->m_progressBar;

    if (!label.isEmpty())
    {
        progressBar->setFormat(label);
    }

    if (maxStep > 0)
    {
        progressBar->setMaximum(maxStep);
    }

    progressBar->setValue(step);
}

void FbWindow::slotLoginDone(int errCode, const QString& errMsg)
{
    setRejectButtonMode(QDialogButtonBox::Close);
    d->m_progressBar->hide();

    buttonStateChange(m_talker->loggedIn());
    FbUser user = m_talker->getUser();
    setProfileAID(user.id);
    d->m_widget->updateLabels(user.name, user.profileURL);
    d->m_albumsCoB->clear();

    d->m_albumsCoB->addItem(i18n("<auto create>"), QString());

    m_accessToken    = m_talker->getAccessToken();
    m_sessionExpires = m_talker->getSessionExpires();

    if (errCode == 0 && m_talker->loggedIn())
    {
        m_talker->listAlbums();    // get albums to fill combo box
    }
    else
    {
        QMessageBox::critical(this, QString(), i18n("Facebook Call Failed: %1\n", errMsg));
    }
}

void FbWindow::slotListAlbumsDone(int errCode,
                                  const QString& errMsg,
                                  const QList<FbAlbum>& albumsList)
{

    QString albumDebug = QString::fromLatin1("");
    foreach(const FbAlbum &album, albumsList)
    {
        albumDebug.append(QString::fromLatin1("%1: %2\n").arg(album.id).arg(album.title));
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Received albums (errCode = " << errCode << ", errMsg = "
             << errMsg << "): " << albumDebug;

    if (errCode != 0)
    {
        QMessageBox::critical(this, QString(), i18n("Facebook Call Failed: %1\n", errMsg));
        return;
    }

    d->m_albumsCoB->clear();
    d->m_albumsCoB->addItem(i18n("<auto create>"), QString());

    for (int i = 0; i < albumsList.size(); ++i)
    {
        QString albumIcon;

        switch (albumsList.at(i).privacy)
        {
            case FB_ME:
                albumIcon = QString::fromLatin1("secure-card");
                break;

            case FB_FRIENDS:
                albumIcon = QString::fromLatin1("user-identity");
                break;

            case FB_FRIENDS_OF_FRIENDS:
                albumIcon = QString::fromLatin1("system-users");
                break;

            case FB_NETWORKS:
                albumIcon = QString::fromLatin1("network-workgroup");
                break;

            case FB_EVERYONE:
                albumIcon = QString::fromLatin1("folder-html");
                break;

            case FB_CUSTOM:
                albumIcon = QString::fromLatin1("configure");
                break;
        }

        d->m_albumsCoB->addItem(
            QIcon::fromTheme(albumIcon),
            albumsList.at(i).title,
            albumsList.at(i).id);

        if (m_currentAlbumID == albumsList.at(i).id)
        {
            d->m_albumsCoB->setCurrentIndex(i + 1);
        }
    }
}

void FbWindow::buttonStateChange(bool state)
{
    d->m_newAlbumBtn->setEnabled(state);
    d->m_reloadAlbumsBtn->setEnabled(state);
    startButton()->setEnabled(state);
}

void FbWindow::slotBusy(bool val)
{
    if (val)
    {
        setCursor(Qt::WaitCursor);
        d->m_changeUserBtn->setEnabled(false);
        buttonStateChange(false);
    }
    else
    {
        setCursor(Qt::ArrowCursor);
        d->m_changeUserBtn->setEnabled(true);
        buttonStateChange(m_talker->loggedIn());
    }
}

void FbWindow::slotUserChangeRequest()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Slot Change User Request";

    if (m_talker->loggedIn())
    {
        m_talker->logout();
        QMessageBox warn(QMessageBox::Warning,
                     i18n("Warning"),
                     i18n("After you have been logged out in the browser, "
                          "click \"Continue\" to authenticate for another account"),
                     QMessageBox::Yes | QMessageBox::No);

        (warn.button(QMessageBox::Yes))->setText(i18n("Continue"));
        (warn.button(QMessageBox::No))->setText(i18n("Cancel"));

        if (warn.exec() == QMessageBox::Yes)
        {
            m_accessToken.clear();
            m_sessionExpires = 0;
        }
        else
            return;
    }

    authenticate();
}

void FbWindow::slotReloadAlbumsRequest(long long userID)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Reload Albums Request for UID:" << userID;

    if (userID == 0)
    {
        FbUser user = m_talker->getUser();
        setProfileAID(user.id);
        m_talker->listAlbums(); // re-get albums from current user
    }
    else
    {
        setProfileAID(userID);
        m_talker->listAlbums(userID); // re-get albums for friend
    }
}

void FbWindow::slotNewAlbumRequest()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Slot New Album Request";

    if (m_albumDlg->exec() == QDialog::Accepted)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Calling New Album method";
        FbAlbum newAlbum;
        m_albumDlg->getAlbumProperties(newAlbum);
        m_talker->createAlbum(newAlbum);
    }
}

void FbWindow::slotStartTransfer()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "slotStartTransfer invoked";

    d->m_imgList->clearProcessedStatus();
    m_transferQueue  = d->m_imgList->imageUrls();

    if (m_transferQueue.isEmpty())
    {
        return;
    }

    m_currentAlbumID = d->m_albumsCoB->itemData(d->m_albumsCoB->currentIndex()).toString();
    qCDebug(DIGIKAM_GENERAL_LOG) << "upload request got album id from widget: " << m_currentAlbumID;
    m_imagesTotal    = m_transferQueue.count();
    m_imagesCount    = 0;

    setRejectButtonMode(QDialogButtonBox::Cancel);
    d->m_progressBar->setFormat(i18n("%v / %m"));
    d->m_progressBar->setMaximum(m_imagesTotal);
    d->m_progressBar->setValue(0);
    d->m_progressBar->show();
    d->m_progressBar->progressScheduled(i18n("Facebook export"), true, true);
    d->m_progressBar->progressThumbnailChanged(QIcon(QLatin1String("facebook")).pixmap(22, 22));

    uploadNextPhoto();
}

void FbWindow::setProfileAID(long long userID)
{
    // store AID of Profile Photos album
    // http://wiki.developers.facebook.com/index.php/Profile_archive_album
    m_profileAID = QString::number((userID << 32)
                                   + (-3 & 0xFFFFFFFF));
}

QString FbWindow::getImageCaption(const QString& fileName)
{
    DItemInfo info(m_iface->itemInfo(QUrl::fromLocalFile(fileName)));

    // Facebook doesn't support image titles. Include it in descriptions if needed.
    QStringList descriptions = QStringList() << info.title() << info.comment();
    descriptions.removeAll(QString::fromLatin1(""));
    return descriptions.join(QString::fromLatin1("\n\n"));
}

bool FbWindow::prepareImageForUpload(const QString& imgPath, QString& caption)
{
    QImage image = PreviewLoadThread::loadHighQualitySynchronously(imgPath).copyQImage();

    if (image.isNull())
    {
        image.load(imgPath);
    }

    if (image.isNull())
    {
        return false;
    }

    // get temporary file name
    m_tmpPath = m_tmpDir + QFileInfo(imgPath).baseName().trimmed() + QString::fromLatin1(".jpg");

    // rescale image if requested
    int maxDim = d->m_dimensionSpB->value();

    if (d->m_resizeChB->isChecked()
        && (image.width() > maxDim || image.height() > maxDim))
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Resizing to " << maxDim;
        image = image.scaled(maxDim, maxDim, Qt::KeepAspectRatio,
                             Qt::SmoothTransformation);
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Saving to temp file: " << m_tmpPath;
    image.save(m_tmpPath, "JPEG", d->m_imageQualitySpB->value());

    // copy meta data to temporary image

    DMetadata meta;

    if (meta.load(imgPath))
    {
        caption = getImageCaption(imgPath);
        meta.setImageDimensions(image.size());
        meta.setImageOrientation(MetaEngine::ORIENTATION_NORMAL);
        meta.setImageProgramId(QString::fromLatin1("digiKam"), digiKamVersion());
        meta.setMetadataWritingMode((int)DMetadata::WRITETOIMAGEONLY);
        meta.save(m_tmpPath);
    }
    else
    {
        caption.clear();
    }

    return true;
}

void FbWindow::uploadNextPhoto()
{
    if (m_transferQueue.isEmpty())
    {
        setRejectButtonMode(QDialogButtonBox::Close);
        d->m_progressBar->hide();
        d->m_progressBar->progressCompleted();
        return;
    }

    d->m_imgList->processing(m_transferQueue.first());
    QString imgPath = m_transferQueue.first().toLocalFile();

    d->m_progressBar->setMaximum(m_imagesTotal);
    d->m_progressBar->setValue(m_imagesCount);

    QString caption;
    bool    res;

    if (d->m_resizeChB->isChecked())
    {
        if (!prepareImageForUpload(imgPath, caption))
        {
            slotAddPhotoDone(666, i18n("Cannot open file"));
            return;
        }

        res = m_talker->addPhoto(m_tmpPath, m_currentAlbumID, caption);
    }
    else
    {
        caption = getImageCaption(imgPath);
        m_tmpPath.clear();
        res     = m_talker->addPhoto(imgPath, m_currentAlbumID, caption);
    }

    if (!res)
    {
        slotAddPhotoDone(666, i18n("Cannot open file"));
        return;
    }
}

void FbWindow::slotAddPhotoDone(int errCode, const QString& errMsg)
{
    // Remove temporary file if it was used
    if (!m_tmpPath.isEmpty())
    {
        QFile::remove(m_tmpPath);
        m_tmpPath.clear();
    }

    d->m_imgList->processed(m_transferQueue.first(), (errCode == 0));

    if (errCode == 0)
    {
        m_transferQueue.pop_front();
        m_imagesCount++;
    }
    else
    {
        if (QMessageBox::question(this, i18n("Uploading Failed"),
                                  i18n("Failed to upload photo into Facebook: %1\n"
                                       "Do you want to continue?", errMsg))
            != QMessageBox::Yes)
        {
            setRejectButtonMode(QDialogButtonBox::Close);
            d->m_progressBar->hide();
            d->m_progressBar->progressCompleted();
            m_transferQueue.clear();
            return;
        }
    }

    uploadNextPhoto();
}

void FbWindow::slotCreateAlbumDone(int errCode, const QString& errMsg, const QString& newAlbumID)
{
    if (errCode != 0)
    {
        QMessageBox::critical(this, QString(), i18n("Facebook Call Failed: %1", errMsg));
        return;
    }

    // reload album list and automatically select new album
    m_currentAlbumID = newAlbumID;
    m_talker->listAlbums();
}

void FbWindow::slotImageListChanged()
{
    startButton()->setEnabled(!(d->m_imgList->imageUrls().isEmpty()));
}

} // namespace Digikam
