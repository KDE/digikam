/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-17-06
 * Description : a tool to export images to Smugmug web service
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

#include "smugwindow.h"

// Qt includes

#include <QWindow>
#include <QFileInfo>
#include <QPointer>
#include <QSpinBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QMenu>
#include <QComboBox>
#include <QPushButton>
#include <QMessageBox>
#include <QApplication>

// KDE includes

#include <kconfig.h>
#include <klocalizedstring.h>
#include <kwindowconfig.h>

// Local includes

#include "digikam_debug.h"
#include "dimageslist.h"
#include "exportutils.h"
#include "digikam_version.h"
#include "dprogresswdg.h"
#include "dmetadata.h"
#include "previewloadthread.h"
#include "smugitem.h"
#include "smugtalker.h"
#include "smugwidget.h"
#include "smugnewalbumdlg.h"

namespace Digikam
{

SmugWindow::SmugWindow(DInfoInterface* const iface,
                       QWidget* const /*parent*/,
                       bool import)
    : WSToolDialog(0)
{
    m_tmpPath.clear();
    m_tmpDir      = ExportUtils::makeTemporaryDir("smug").absolutePath() + QLatin1Char('/');;
    m_import      = import;
    m_imagesCount = 0;
    m_imagesTotal = 0;
    m_iface       = iface;
    m_widget      = new SmugWidget(this, iface, import);

    setMainWidget(m_widget);
    setWindowIcon(QIcon::fromTheme(QString::fromLatin1("smugmug")));
    setModal(false);

    if (import)
    {
        setWindowTitle(i18n("Import from SmugMug Web Service"));

        startButton()->setText(i18n("Start Download"));
        startButton()->setToolTip(i18n("Start download from SmugMug web service"));

        m_widget->setMinimumSize(300, 400);
    }
    else
    {
        setWindowTitle(i18n("Export to SmugMug Web Service"));

        startButton()->setText(i18n("Start Upload"));
        startButton()->setToolTip(i18n("Start upload to SmugMug web service"));

        m_widget->setMinimumSize(700, 500);
    }


    connect(m_widget, SIGNAL(signalUserChangeRequest(bool)),
            this, SLOT(slotUserChangeRequest(bool)) );

    connect(m_widget->m_imgList, SIGNAL(signalImageListChanged()),
            this, SLOT(slotImageListChanged()) );

    connect(m_widget->m_reloadAlbumsBtn, SIGNAL(clicked()),
            this, SLOT(slotReloadAlbumsRequest()) );

    connect(m_widget->m_newAlbumBtn, SIGNAL(clicked()),
            this, SLOT(slotNewAlbumRequest()) );

    connect(startButton(), &QPushButton::clicked,
            this, &SmugWindow::slotStartTransfer);

    connect(this, &WSToolDialog::cancelClicked,
            this, &SmugWindow::slotCancelClicked);

    connect(this, &QDialog::finished,
            this, &SmugWindow::slotDialogFinished);

    // ------------------------------------------------------------------------

    m_loginDlg  = new LoginDialog(this,
                                  i18n("<qt>Enter the <b>email address</b> and <b>password</b> for your "
                                       "<a href=\"http://www.smugmug.com\">SmugMug</a> account</qt>"));

    // ------------------------------------------------------------------------

    m_albumDlg  = new SmugNewAlbumDlg(this);

    connect(m_albumDlg->m_categCoB, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotCategorySelectionChanged(int)) );

    connect(m_albumDlg->m_templateCoB, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotTemplateSelectionChanged(int)) );

    // ------------------------------------------------------------------------

    m_talker = new SmugTalker(m_iface, this);

    connect(m_talker, SIGNAL(signalBusy(bool)),
            this, SLOT(slotBusy(bool)));

    connect(m_talker, SIGNAL(signalLoginProgress(int,int,QString)),
            this, SLOT(slotLoginProgress(int,int,QString)));

    connect(m_talker, SIGNAL(signalLoginDone(int,QString)),
            this, SLOT(slotLoginDone(int,QString)));

    connect(m_talker, SIGNAL(signalAddPhotoDone(int,QString)),
            this, SLOT(slotAddPhotoDone(int,QString)));

    connect(m_talker, SIGNAL(signalGetPhotoDone(int,QString,QByteArray)),
            this, SLOT(slotGetPhotoDone(int,QString,QByteArray)));

    connect(m_talker, SIGNAL(signalCreateAlbumDone(int,QString,qint64,QString)),
            this, SLOT(slotCreateAlbumDone(int,QString,qint64,QString)));

    connect(m_talker, SIGNAL(signalListAlbumsDone(int,QString,QList<SmugAlbum>)),
            this, SLOT(slotListAlbumsDone(int,QString,QList<SmugAlbum>)));

    connect(m_talker, SIGNAL(signalListPhotosDone(int,QString,QList<SmugPhoto>)),
            this, SLOT(slotListPhotosDone(int,QString,QList<SmugPhoto>)));

    connect(m_talker, SIGNAL(signalListAlbumTmplDone(int,QString,QList<SmugAlbumTmpl>)),
            this, SLOT(slotListAlbumTmplDone(int,QString,QList<SmugAlbumTmpl>)));

    connect(m_talker, SIGNAL(signalListCategoriesDone(int,QString,QList<SmugCategory>)),
            this, SLOT(slotListCategoriesDone(int,QString,QList<SmugCategory>)));

    connect(m_talker, SIGNAL(signalListSubCategoriesDone(int,QString,QList<SmugCategory>)),
            this, SLOT(slotListSubCategoriesDone(int,QString,QList<SmugCategory>)));

    connect(m_widget->progressBar(), SIGNAL(signalProgressCanceled()),
            this, SLOT(slotStopAndCloseProgressBar()));

    // ------------------------------------------------------------------------

    readSettings();

    qCDebug(DIGIKAM_GENERAL_LOG) << "Calling Login method";
    buttonStateChange(m_talker->loggedIn());

    if (m_import)
    {
        // if no e-mail, switch to anonymous login
        if (m_anonymousImport || m_email.isEmpty())
        {
            m_anonymousImport = true;
            authenticate();
        }
        else
            authenticate(m_email, m_password);
        m_widget->setAnonymous(m_anonymousImport);
    }
    else
    {
        // export cannot login anonymously: pop-up login window`
        if (m_email.isEmpty())
            slotUserChangeRequest(false);
        else
            authenticate(m_email, m_password);
    }
}

SmugWindow::~SmugWindow()
{
    delete m_talker;
}

void SmugWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    slotDialogFinished();
    e->accept();
}

void SmugWindow::slotDialogFinished()
{
    slotCancelClicked();

    if (m_talker->loggedIn())
        m_talker->logout();

    writeSettings();
    m_widget->imagesList()->listView()->clear();
}

void SmugWindow::setUiInProgressState(bool inProgress)
{
    setRejectButtonMode(inProgress ? QDialogButtonBox::Cancel : QDialogButtonBox::Close);

    if (inProgress)
    {
        m_widget->progressBar()->show();
    }
    else
    {
        m_widget->progressBar()->hide();
        m_widget->progressBar()->progressCompleted();
    }
}

void SmugWindow::slotCancelClicked()
{
    m_talker->cancel();
    m_transferQueue.clear();
    m_widget->m_imgList->cancelProcess();
    setUiInProgressState(false);
}

void SmugWindow::slotStopAndCloseProgressBar()
{
    slotCancelClicked();

    writeSettings();
    m_widget->imagesList()->listView()->clear();
    reject();
}

void SmugWindow::reactivate()
{
    m_widget->imagesList()->loadImagesFromCurrentSelection();
    show();
}

void SmugWindow::authenticate(const QString& email, const QString& password)
{
    setUiInProgressState(true);
    m_widget->progressBar()->setFormat(QString());

    m_talker->login(email, password);
}

void SmugWindow::readSettings()
{
    KConfig config;
    KConfigGroup grp  = config.group("Smug Settings");
    m_anonymousImport = grp.readEntry("AnonymousImport", true);
    m_email           = grp.readEntry("Email");
    m_password        = grp.readEntry("Password");
    m_currentAlbumID  = grp.readEntry("Current Album", -1);
    m_currentAlbumKey = grp.readEntry("Current Key", -1);

    if (grp.readEntry("Resize", false))
    {
        m_widget->m_resizeChB->setChecked(true);
        m_widget->m_dimensionSpB->setEnabled(true);
        m_widget->m_imageQualitySpB->setEnabled(true);
    }
    else
    {
        m_widget->m_resizeChB->setChecked(false);
        m_widget->m_dimensionSpB->setEnabled(false);
        m_widget->m_imageQualitySpB->setEnabled(false);
    }

    m_widget->m_dimensionSpB->setValue(grp.readEntry("Maximum Width", 1600));
    m_widget->m_imageQualitySpB->setValue(grp.readEntry("Image Quality", 85));

    if (m_import)
    {
        winId();
        KConfigGroup dialogGroup = config.group("Smug Import Dialog");
        KWindowConfig::restoreWindowSize(windowHandle(), dialogGroup);
        resize(windowHandle()->size());
    }
    else
    {
        winId();
        KConfigGroup dialogGroup = config.group("Smug Export Dialog");
        KWindowConfig::restoreWindowSize(windowHandle(), dialogGroup);
        resize(windowHandle()->size());
    }
}

void SmugWindow::writeSettings()
{
    KConfig config;
    KConfigGroup grp = config.group("Smug Settings");
    grp.writeEntry("AnonymousImport", m_anonymousImport);
    grp.writeEntry("Email",           m_email);
    grp.writeEntry("Password",        m_password);
    grp.writeEntry("Current Album",   m_currentAlbumID);
    grp.writeEntry("Current Key",     m_currentAlbumKey);
    grp.writeEntry("Resize",          m_widget->m_resizeChB->isChecked());
    grp.writeEntry("Maximum Width",   m_widget->m_dimensionSpB->value());
    grp.writeEntry("Image Quality",   m_widget->m_imageQualitySpB->value());

    if (m_import)
    {
        KConfigGroup dialogGroup = config.group("Smug Import Dialog");
        KWindowConfig::saveWindowSize(windowHandle(), dialogGroup);
    }
    else
    {
        KConfigGroup dialogGroup = config.group("Smug Export Dialog");
        KWindowConfig::saveWindowSize(windowHandle(), dialogGroup);
    }

    config.sync();
}

void SmugWindow::slotLoginProgress(int step, int maxStep, const QString &label)
{
    DProgressWdg* progressBar = m_widget->progressBar();

    if (!label.isEmpty())
        progressBar->setFormat(label);

    if (maxStep > 0)
        progressBar->setMaximum(maxStep);

    progressBar->setValue(step);
}

void SmugWindow::slotLoginDone(int errCode, const QString &errMsg)
{
    setUiInProgressState(false);

    buttonStateChange(m_talker->loggedIn());
    SmugUser user = m_talker->getUser();
    m_widget->updateLabels(user.email, user.displayName, user.nickName);
    m_widget->m_albumsCoB->clear();

    if (errCode == 0 && m_talker->loggedIn())
    {
        if (m_import)
        {
            m_anonymousImport = m_widget->isAnonymous();
            // anonymous: list albums after login only if nick is not empty
            QString nick = m_widget->getNickName();

            if (!nick.isEmpty() || !m_anonymousImport)
            {
                m_talker->listAlbums(nick);
            }
        }
        else
        {
            // get albums from current user
            m_talker->listAlbums();
        }
    }
    else
    {
        QMessageBox::critical(QApplication::activeWindow(), i18n("Error"), i18n("SmugMug Call Failed: %1\n", errMsg));
    }
}

void SmugWindow::slotListAlbumsDone(int errCode, const QString &errMsg,
                                    const QList <SmugAlbum>& albumsList)
{
    if (errCode != 0)
    {
        QMessageBox::critical(QApplication::activeWindow(), i18n("Error"), i18n("SmugMug Call Failed: %1\n", errMsg));
        return;
    }

    m_widget->m_albumsCoB->clear();

    for (int i = 0; i < albumsList.size(); ++i)
    {
        QString albumIcon;

        if (!albumsList.at(i).password.isEmpty())
            albumIcon = QString::fromLatin1("folder-locked");
        else if (albumsList.at(i).isPublic)
            albumIcon = QString::fromLatin1("folder-image");
        else
            albumIcon = QString::fromLatin1("folder");

        QString data = QString::fromLatin1("%1:%2").arg(albumsList.at(i).id).arg(albumsList.at(i).key);
        m_widget->m_albumsCoB->addItem(QIcon::fromTheme(albumIcon), albumsList.at(i).title, data);

        if (m_currentAlbumID == albumsList.at(i).id)
            m_widget->m_albumsCoB->setCurrentIndex(i);
    }
}

void SmugWindow::slotListPhotosDone(int errCode, const QString &errMsg,
                                    const QList <SmugPhoto>& photosList)
{
    if (errCode != 0)
    {
        QMessageBox::critical(QApplication::activeWindow(), i18n("Error"), i18n("SmugMug Call Failed: %1\n", errMsg));
        return;
    }

    m_transferQueue.clear();

    for (int i = 0; i < photosList.size(); ++i)
    {
        m_transferQueue.push_back(QUrl::fromLocalFile(photosList.at(i).originalURL));
    }

    if (m_transferQueue.isEmpty())
        return;

    m_imagesTotal = m_transferQueue.count();
    m_imagesCount = 0;

    m_widget->progressBar()->setMaximum(m_imagesTotal);
    m_widget->progressBar()->setValue(0);

    // start download with first photo in queue
    downloadNextPhoto();
}

void SmugWindow::slotListAlbumTmplDone(int errCode, const QString &errMsg,
                                       const QList <SmugAlbumTmpl>& albumTList)
{
    // always put at least default <none> subcategory
    m_albumDlg->m_templateCoB->clear();
    m_albumDlg->m_templateCoB->addItem(i18n("&lt;none&gt;"), 0);

    if (errCode != 0)
    {
        QMessageBox::critical(QApplication::activeWindow(), i18n("Error"), i18n("SmugMug Call Failed: %1\n", errMsg));
        return;
    }

    for (int i = 0; i < albumTList.size(); ++i)
    {
        QString albumIcon;

        if (!albumTList.at(i).password.isEmpty())
            albumIcon = QString::fromLatin1("folder-locked");
        else if (albumTList.at(i).isPublic)
            albumIcon = QString::fromLatin1("folder-image");
        else
            albumIcon = QString::fromLatin1("folder");

        m_albumDlg->m_templateCoB->addItem(QIcon::fromTheme(albumIcon), albumTList.at(i).name, albumTList.at(i).id);

        if (m_currentTmplID == albumTList.at(i).id)
            m_albumDlg->m_templateCoB->setCurrentIndex(i+1);
    }

    m_currentTmplID = m_albumDlg->m_templateCoB->itemData(m_albumDlg->m_templateCoB->currentIndex()).toLongLong();

    // now fill in categories
    m_talker->listCategories();
}

void SmugWindow::slotListCategoriesDone(int errCode, const QString& errMsg,
                                        const QList <SmugCategory>& categoriesList)
{
    if (errCode != 0)
    {
        QMessageBox::critical(QApplication::activeWindow(), i18n("Error"), i18n("SmugMug Call Failed: %1\n", errMsg));
        return;
    }

    m_albumDlg->m_categCoB->clear();

    for (int i = 0; i < categoriesList.size(); ++i)
    {
        m_albumDlg->m_categCoB->addItem(
            categoriesList.at(i).name,
            categoriesList.at(i).id);

        if (m_currentCategoryID == categoriesList.at(i).id)
            m_albumDlg->m_categCoB->setCurrentIndex(i);
    }

    m_currentCategoryID = m_albumDlg->m_categCoB->itemData(
                          m_albumDlg->m_categCoB->currentIndex()).toLongLong();
    m_talker->listSubCategories(m_currentCategoryID);
}

void SmugWindow::slotListSubCategoriesDone(int errCode, const QString &errMsg,
                                              const QList <SmugCategory>& categoriesList)
{
    // always put at least default <none> subcategory
    m_albumDlg->m_subCategCoB->clear();
    m_albumDlg->m_subCategCoB->addItem(i18n("&lt;none&gt;"), 0);

    if (errCode != 0)
    {
        QMessageBox::critical(QApplication::activeWindow(), i18n("Error"), i18n("SmugMug Call Failed: %1\n", errMsg));
        return;
    }

    for (int i = 0; i < categoriesList.size(); ++i)
    {
        m_albumDlg->m_subCategCoB->addItem(
            categoriesList.at(i).name,
            categoriesList.at(i).id);
    }
}

void SmugWindow::slotTemplateSelectionChanged(int index)
{
    if (index < 0)
        return;

    m_currentTmplID = m_albumDlg->m_templateCoB->itemData(index).toLongLong();

    // if template is selected, then disable Security & Privacy
    m_albumDlg->m_privBox->setEnabled(m_currentTmplID == 0);
}

void SmugWindow::slotCategorySelectionChanged(int index)
{
    if (index < 0)
        return;

    // subcategories are per category -> reload
    m_currentCategoryID = m_albumDlg->m_categCoB->itemData(index).toLongLong();
    m_talker->listSubCategories(m_currentCategoryID);
}

void SmugWindow::buttonStateChange(bool state)
{
    m_widget->m_newAlbumBtn->setEnabled(state);
    m_widget->m_reloadAlbumsBtn->setEnabled(state);
    startButton()->setEnabled(state);
}

void SmugWindow::slotBusy(bool val)
{
    if (val)
    {
        setCursor(Qt::WaitCursor);
        m_widget->m_changeUserBtn->setEnabled(false);
        buttonStateChange(false);
    }
    else
    {
        setCursor(Qt::ArrowCursor);
        m_widget->m_changeUserBtn->setEnabled(!m_widget->isAnonymous());
        buttonStateChange(m_talker->loggedIn());
    }
}

void SmugWindow::slotUserChangeRequest(bool anonymous)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Slot Change User Request";

    if (m_talker->loggedIn())
        m_talker->logout();

    if (anonymous)
    {
        authenticate();
    }
    else
    {
        // fill in current email and password
        m_loginDlg->setLogin(m_email);
        m_loginDlg->setPassword(m_password);

        if (m_loginDlg->exec())
        {
            m_email    = m_loginDlg->login();
            m_password = m_loginDlg->password();
            authenticate(m_email, m_password);
        }
    }
}

void SmugWindow::slotReloadAlbumsRequest()
{
    if (m_import)
    {
        m_talker->listAlbums(m_widget->getNickName());
    }
    else
    {
        // get albums for current user
        m_talker->listAlbums();
    }
}

void SmugWindow::slotNewAlbumRequest()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Slot New Album Request";

    // get list of album templates from SmugMug to fill in dialog
    m_talker->listAlbumTmpl();

    if (m_albumDlg->exec() == QDialog::Accepted)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Calling New Album method";
        m_currentTmplID = m_albumDlg->m_templateCoB->itemData(
                        m_albumDlg->m_templateCoB->currentIndex()).toLongLong();
        m_currentCategoryID = m_albumDlg->m_categCoB->itemData(
                        m_albumDlg->m_categCoB->currentIndex()).toLongLong();

        SmugAlbum newAlbum;
        m_albumDlg->getAlbumProperties(newAlbum);
        m_talker->createAlbum(newAlbum);
    }
}

void SmugWindow::slotStartTransfer()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "slotStartTransfer invoked";

    if (m_import)
    {
        m_widget->progressBar()->setFormat(i18n("%v / %m"));
        m_widget->progressBar()->setMaximum(0);
        m_widget->progressBar()->setValue(0);
        m_widget->progressBar()->progressScheduled(i18n("SmugMug Import"), true, true);
        m_widget->progressBar()->progressThumbnailChanged(
            QIcon(QLatin1String("smugmug")).pixmap(22, 22));
        setUiInProgressState(true);

        // list photos of the album, then start download
        QString dataStr = m_widget->m_albumsCoB->itemData(m_widget->m_albumsCoB->currentIndex()).toString();
        int colonIdx = dataStr.indexOf(QLatin1Char(':'));
        qint64 albumID = dataStr.left(colonIdx).toLongLong();
        QString albumKey = dataStr.right(dataStr.length() - colonIdx - 1);
        m_talker->listPhotos(albumID, albumKey,
                             m_widget->getAlbumPassword(),
                             m_widget->getSitePassword());
    }
    else
    {
        m_widget->m_imgList->clearProcessedStatus();
        m_transferQueue = m_widget->m_imgList->imageUrls();

        if (m_transferQueue.isEmpty())
            return;

        QString data = m_widget->m_albumsCoB->itemData(m_widget->m_albumsCoB->currentIndex()).toString();
        int colonIdx = data.indexOf(QLatin1Char(':'));
        m_currentAlbumID = data.left(colonIdx).toLongLong();
        m_currentAlbumKey = data.right(data.length() - colonIdx - 1);

        m_imagesTotal = m_transferQueue.count();
        m_imagesCount = 0;

        m_widget->progressBar()->setFormat(i18n("%v / %m"));
        m_widget->progressBar()->setMaximum(m_imagesTotal);
        m_widget->progressBar()->setValue(0);
        m_widget->progressBar()->progressScheduled(i18n("SmugMug Export"), true, true);
        m_widget->progressBar()->progressThumbnailChanged(QIcon(QLatin1String("smugmug")).pixmap(22, 22));
        setUiInProgressState(true);

        qCDebug(DIGIKAM_GENERAL_LOG) << "m_currentAlbumID" << m_currentAlbumID;
        uploadNextPhoto();
        qCDebug(DIGIKAM_GENERAL_LOG) << "slotStartTransfer done";
    }
}

bool SmugWindow::prepareImageForUpload(const QString& imgPath)
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
    m_tmpPath  = m_tmpDir + QFileInfo(imgPath).baseName().trimmed() + QString::fromLatin1(".jpg");

    // rescale image if requested
    int maxDim = m_widget->m_dimensionSpB->value();

    if (m_widget->m_resizeChB->isChecked() &&
        (image.width() > maxDim || image.height() > maxDim))
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Resizing to " << maxDim;
        image = image.scaled(maxDim, maxDim, Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation);
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Saving to temp file: " << m_tmpPath;
    image.save(m_tmpPath, "JPEG", m_widget->m_imageQualitySpB->value());

    // copy meta-data to temporary image
    DMetadata meta;

    if (meta.load(imgPath))
    {
        meta.setImageDimensions(image.size());
        meta.setImageOrientation(MetaEngine::ORIENTATION_NORMAL);
        meta.setImageProgramId(QString::fromLatin1("digiKam"), digiKamVersion());
        meta.setMetadataWritingMode((int)DMetadata::WRITETOIMAGEONLY);
        meta.save(m_tmpPath);
    }

    return true;
}

void SmugWindow::uploadNextPhoto()
{
    if (m_transferQueue.isEmpty())
    {
        setUiInProgressState(false);
        return;
    }

    m_widget->m_imgList->processing(m_transferQueue.first());

    QUrl imgPath = m_transferQueue.first();
    DItemInfo info(m_iface->itemInfo(imgPath));

    m_widget->progressBar()->setMaximum(m_imagesTotal);
    m_widget->progressBar()->setValue(m_imagesCount);

    bool res;

    if (m_widget->m_resizeChB->isChecked())
    {
        if (!prepareImageForUpload(imgPath.toLocalFile()))
        {
            slotAddPhotoDone(666, i18n("Cannot open file"));
            return;
        }

        res = m_talker->addPhoto(m_tmpPath, m_currentAlbumID, m_currentAlbumKey, info.comment());
    }
    else
    {
        m_tmpPath.clear();
        res = m_talker->addPhoto(imgPath.toLocalFile(), m_currentAlbumID, m_currentAlbumKey, info.comment());
    }

    if (!res)
    {
        slotAddPhotoDone(666, i18n("Cannot open file"));
        return;
    }
}

void SmugWindow::slotAddPhotoDone(int errCode, const QString& errMsg)
{
    // Remove temporary file if it was used
    if (!m_tmpPath.isEmpty())
    {
        QFile::remove(m_tmpPath);
        m_tmpPath.clear();
    }

    m_widget->m_imgList->processed(m_transferQueue.first(), (errCode == 0));

    if (errCode == 0)
    {
        m_transferQueue.pop_front();
        m_imagesCount++;
    }
    else
    {
        if (QMessageBox::question(this, i18n("Uploading Failed"),
                              i18n("Failed to upload photo to SmugMug."
                                   "\n%1\n"
                                   "Do you want to continue?", errMsg))
            != QMessageBox::Yes)
        {
            setUiInProgressState(false);
            m_transferQueue.clear();
            return;
        }
    }

    uploadNextPhoto();
}

void SmugWindow::downloadNextPhoto()
{
    if (m_transferQueue.isEmpty())
    {
        setUiInProgressState(false);
        return;
    }

    m_widget->progressBar()->setMaximum(m_imagesTotal);
    m_widget->progressBar()->setValue(m_imagesCount);

    QString imgPath = m_transferQueue.first().url();

    m_talker->getPhoto(imgPath);
}

void SmugWindow::slotGetPhotoDone(int errCode, const QString& errMsg,
                                  const QByteArray& photoData)
{
    QString imgPath = m_widget->getDestinationPath() + QLatin1Char('/')
                      + m_transferQueue.first().fileName();

    if (errCode == 0)
    {
        QString errText;
        QFile imgFile(imgPath);

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
            m_transferQueue.pop_front();
            m_imagesCount++;
        }
        else
        {
            if (QMessageBox::question(this, i18n("Processing Failed"),
                                      i18n("Failed to save photo: %1\n"
                                           "Do you want to continue?", errText))
                != QMessageBox::Yes)
            {
                m_transferQueue.clear();
                setUiInProgressState(false);
                return;
            }
        }
    }
    else
    {
        if (QMessageBox::question(this, i18n("Processing Failed"),
                                  i18n("Failed to download photo: %1\n"
                                       "Do you want to continue?", errMsg))
                != QMessageBox::Yes)
        {
            m_transferQueue.clear();
            setUiInProgressState(false);
            return;
        }
    }

    downloadNextPhoto();
}

void SmugWindow::slotCreateAlbumDone(int errCode, const QString& errMsg,
                                     qint64 newAlbumID, const QString& newAlbumKey)
{
    if (errCode != 0)
    {
        QMessageBox::critical(QApplication::activeWindow(), i18n("Error"), i18n("SmugMug Call Failed: %1\n", errMsg));
        return;
    }

    // reload album list and automatically select new album
    m_currentAlbumID  = newAlbumID;
    m_currentAlbumKey = newAlbumKey;
    m_talker->listAlbums();
}

void SmugWindow::slotImageListChanged()
{
    startButton()->setEnabled(!(m_widget->m_imgList->imageUrls().isEmpty()));
}

} // namespace Digikam
