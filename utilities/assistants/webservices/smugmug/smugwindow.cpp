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
#include "wstoolutils.h"
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

class SmugWindow::Private
{
public:

    explicit Private()
    {
        import            = false;
        imagesCount       = 0;
        imagesTotal       = 0;
        anonymousImport   = false;
        currentAlbumID    = 0;
        currentTmplID     = 0;
        currentCategoryID = 0;
        loginDlg          = 0;
        talker            = 0;
        widget            = 0;
        albumDlg          = 0;
        iface             = 0;
    }

    bool             import;
    unsigned int     imagesCount;
    unsigned int     imagesTotal;
    QString          tmpDir;
    QString          tmpPath;

    bool             anonymousImport;
    QString          anonymousNick;
    QString          email;
    QString          password;
    qint64           currentAlbumID;
    QString          currentAlbumKey;
    qint64           currentTmplID;
    qint64           currentCategoryID;

    WSLoginDialog*   loginDlg;

    QList<QUrl>      transferQueue;

    SmugTalker*      talker;
    SmugWidget*      widget;
    SmugNewAlbumDlg* albumDlg;

    DInfoInterface*  iface;
};

SmugWindow::SmugWindow(DInfoInterface* const iface,
                       QWidget* const /*parent*/,
                       bool import)
    : WSToolDialog(0),
      d(new Private)
{
    d->tmpPath.clear();
    d->tmpDir = WSToolUtils::makeTemporaryDir("smug").absolutePath() + QLatin1Char('/');;
    d->import = import;
    d->iface  = iface;
    d->widget = new SmugWidget(this, iface, import);

    setMainWidget(d->widget);
    setModal(false);

    if (import)
    {
        setWindowTitle(i18n("Import from SmugMug Web Service"));

        startButton()->setText(i18n("Start Download"));
        startButton()->setToolTip(i18n("Start download from SmugMug web service"));

        d->widget->setMinimumSize(300, 400);
    }
    else
    {
        setWindowTitle(i18n("Export to SmugMug Web Service"));

        startButton()->setText(i18n("Start Upload"));
        startButton()->setToolTip(i18n("Start upload to SmugMug web service"));

        d->widget->setMinimumSize(700, 500);
    }

    connect(d->widget, SIGNAL(signalUserChangeRequest(bool)),
            this, SLOT(slotUserChangeRequest(bool)) );

    connect(d->widget->m_imgList, SIGNAL(signalImageListChanged()),
            this, SLOT(slotImageListChanged()) );

    connect(d->widget->m_reloadAlbumsBtn, SIGNAL(clicked()),
            this, SLOT(slotReloadAlbumsRequest()) );

    connect(d->widget->m_newAlbumBtn, SIGNAL(clicked()),
            this, SLOT(slotNewAlbumRequest()) );

    connect(startButton(), &QPushButton::clicked,
            this, &SmugWindow::slotStartTransfer);

    connect(this, &WSToolDialog::cancelClicked,
            this, &SmugWindow::slotCancelClicked);

    connect(this, &QDialog::finished,
            this, &SmugWindow::slotDialogFinished);

    // ------------------------------------------------------------------------

    d->loginDlg  = new WSLoginDialog(this,
                                     i18n("<qt>Enter the <b>email address</b> and <b>password</b> for your "
                                          "<a href=\"http://www.smugmug.com\">SmugMug</a> account</qt>"));

    // ------------------------------------------------------------------------

    d->albumDlg  = new SmugNewAlbumDlg(this);

    connect(d->albumDlg->categoryCombo(), SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotCategorySelectionChanged(int)) );

    connect(d->albumDlg->templateCombo(), SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotTemplateSelectionChanged(int)) );

    // ------------------------------------------------------------------------

    d->talker = new SmugTalker(d->iface, this);

    connect(d->talker, SIGNAL(signalBusy(bool)),
            this, SLOT(slotBusy(bool)));

    connect(d->talker, SIGNAL(signalLoginProgress(int,int,QString)),
            this, SLOT(slotLoginProgress(int,int,QString)));

    connect(d->talker, SIGNAL(signalLoginDone(int,QString)),
            this, SLOT(slotLoginDone(int,QString)));

    connect(d->talker, SIGNAL(signalAddPhotoDone(int,QString)),
            this, SLOT(slotAddPhotoDone(int,QString)));

    connect(d->talker, SIGNAL(signalGetPhotoDone(int,QString,QByteArray)),
            this, SLOT(slotGetPhotoDone(int,QString,QByteArray)));

    connect(d->talker, SIGNAL(signalCreateAlbumDone(int,QString,qint64,QString)),
            this, SLOT(slotCreateAlbumDone(int,QString,qint64,QString)));

    connect(d->talker, SIGNAL(signalListAlbumsDone(int,QString,QList<SmugAlbum>)),
            this, SLOT(slotListAlbumsDone(int,QString,QList<SmugAlbum>)));

    connect(d->talker, SIGNAL(signalListPhotosDone(int,QString,QList<SmugPhoto>)),
            this, SLOT(slotListPhotosDone(int,QString,QList<SmugPhoto>)));

    connect(d->talker, SIGNAL(signalListAlbumTmplDone(int,QString,QList<SmugAlbumTmpl>)),
            this, SLOT(slotListAlbumTmplDone(int,QString,QList<SmugAlbumTmpl>)));

    connect(d->talker, SIGNAL(signalListCategoriesDone(int,QString,QList<SmugCategory>)),
            this, SLOT(slotListCategoriesDone(int,QString,QList<SmugCategory>)));

    connect(d->talker, SIGNAL(signalListSubCategoriesDone(int,QString,QList<SmugCategory>)),
            this, SLOT(slotListSubCategoriesDone(int,QString,QList<SmugCategory>)));

    connect(d->widget->progressBar(), SIGNAL(signalProgressCanceled()),
            this, SLOT(slotStopAndCloseProgressBar()));

    // ------------------------------------------------------------------------

    readSettings();

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Calling Login method";
    buttonStateChange(d->talker->loggedIn());

    if (d->import)
    {
        // if no e-mail, switch to anonymous login
        if (d->anonymousImport || d->email.isEmpty())
        {
            d->anonymousImport = true;
            authenticate();
        }
        else
            authenticate(d->email, d->password);
        d->widget->setAnonymous(d->anonymousImport);
    }
    else
    {
        // export cannot login anonymously: pop-up login window`
        if (d->email.isEmpty())
            slotUserChangeRequest(false);
        else
            authenticate(d->email, d->password);
    }
}

SmugWindow::~SmugWindow()
{
    delete d->talker;
    delete d;
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

    if (d->talker->loggedIn())
        d->talker->logout();

    writeSettings();
    d->widget->imagesList()->listView()->clear();
}

void SmugWindow::setUiInProgressState(bool inProgress)
{
    setRejectButtonMode(inProgress ? QDialogButtonBox::Cancel : QDialogButtonBox::Close);

    if (inProgress)
    {
        d->widget->progressBar()->show();
    }
    else
    {
        d->widget->progressBar()->hide();
        d->widget->progressBar()->progressCompleted();
    }
}

void SmugWindow::slotCancelClicked()
{
    d->talker->cancel();
    d->transferQueue.clear();
    d->widget->m_imgList->cancelProcess();
    setUiInProgressState(false);
}

void SmugWindow::slotStopAndCloseProgressBar()
{
    slotCancelClicked();

    writeSettings();
    d->widget->imagesList()->listView()->clear();
    reject();
}

void SmugWindow::reactivate()
{
    d->widget->imagesList()->loadImagesFromCurrentSelection();
    show();
}

void SmugWindow::authenticate(const QString& email, const QString& password)
{
    setUiInProgressState(true);
    d->widget->progressBar()->setFormat(QString());

    d->talker->login(email, password);
}

void SmugWindow::readSettings()
{
    KConfig config;
    KConfigGroup grp  = config.group("Smug Settings");
    d->anonymousImport = grp.readEntry("AnonymousImport", true);
    d->email           = grp.readEntry("Email");
    d->password        = grp.readEntry("Password");
    d->currentAlbumID  = grp.readEntry("Current Album", -1);
    d->currentAlbumKey = grp.readEntry("Current Key", -1);

    if (grp.readEntry("Resize", false))
    {
        d->widget->m_resizeChB->setChecked(true);
        d->widget->m_dimensionSpB->setEnabled(true);
        d->widget->m_imageQualitySpB->setEnabled(true);
    }
    else
    {
        d->widget->m_resizeChB->setChecked(false);
        d->widget->m_dimensionSpB->setEnabled(false);
        d->widget->m_imageQualitySpB->setEnabled(false);
    }

    d->widget->m_dimensionSpB->setValue(grp.readEntry("Maximum Width", 1600));
    d->widget->m_imageQualitySpB->setValue(grp.readEntry("Image Quality", 85));

    if (d->import)
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
    grp.writeEntry("AnonymousImport", d->anonymousImport);
    grp.writeEntry("Email",           d->email);
    grp.writeEntry("Password",        d->password);
    grp.writeEntry("Current Album",   d->currentAlbumID);
    grp.writeEntry("Current Key",     d->currentAlbumKey);
    grp.writeEntry("Resize",          d->widget->m_resizeChB->isChecked());
    grp.writeEntry("Maximum Width",   d->widget->m_dimensionSpB->value());
    grp.writeEntry("Image Quality",   d->widget->m_imageQualitySpB->value());

    if (d->import)
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
    DProgressWdg* progressBar = d->widget->progressBar();

    if (!label.isEmpty())
        progressBar->setFormat(label);

    if (maxStep > 0)
        progressBar->setMaximum(maxStep);

    progressBar->setValue(step);
}

void SmugWindow::slotLoginDone(int errCode, const QString &errMsg)
{
    setUiInProgressState(false);

    buttonStateChange(d->talker->loggedIn());
    SmugUser user = d->talker->getUser();
    d->widget->updateLabels(user.email, user.displayName, user.nickName);
    d->widget->m_albumsCoB->clear();

    if (errCode == 0 && d->talker->loggedIn())
    {
        if (d->import)
        {
            d->anonymousImport = d->widget->isAnonymous();
            // anonymous: list albums after login only if nick is not empty
            QString nick = d->widget->getNickName();

            if (!nick.isEmpty() || !d->anonymousImport)
            {
                d->talker->listAlbums(nick);
            }
        }
        else
        {
            // get albums from current user
            d->talker->listAlbums();
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

    d->widget->m_albumsCoB->clear();

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
        d->widget->m_albumsCoB->addItem(QIcon::fromTheme(albumIcon), albumsList.at(i).title, data);

        if (d->currentAlbumID == albumsList.at(i).id)
            d->widget->m_albumsCoB->setCurrentIndex(i);
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

    d->transferQueue.clear();

    for (int i = 0; i < photosList.size(); ++i)
    {
        d->transferQueue.push_back(QUrl::fromLocalFile(photosList.at(i).originalURL));
    }

    if (d->transferQueue.isEmpty())
        return;

    d->imagesTotal = d->transferQueue.count();
    d->imagesCount = 0;

    d->widget->progressBar()->setMaximum(d->imagesTotal);
    d->widget->progressBar()->setValue(0);

    // start download with first photo in queue
    downloadNextPhoto();
}

void SmugWindow::slotListAlbumTmplDone(int errCode, const QString &errMsg,
                                       const QList <SmugAlbumTmpl>& albumTList)
{
    // always put at least default <none> subcategory
    d->albumDlg->templateCombo()->clear();
    d->albumDlg->templateCombo()->addItem(i18n("&lt;none&gt;"), 0);

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

        d->albumDlg->templateCombo()->addItem(QIcon::fromTheme(albumIcon), albumTList.at(i).name, albumTList.at(i).id);

        if (d->currentTmplID == albumTList.at(i).id)
            d->albumDlg->templateCombo()->setCurrentIndex(i+1);
    }

    d->currentTmplID = d->albumDlg->templateCombo()->itemData(d->albumDlg->templateCombo()->currentIndex()).toLongLong();

    // now fill in categories
    d->talker->listCategories();
}

void SmugWindow::slotListCategoriesDone(int errCode,
                                        const QString& errMsg,
                                        const QList <SmugCategory>& categoriesList)
{
    if (errCode != 0)
    {
        QMessageBox::critical(QApplication::activeWindow(), i18n("Error"), i18n("SmugMug Call Failed: %1\n", errMsg));
        return;
    }

    d->albumDlg->categoryCombo()->clear();

    for (int i = 0; i < categoriesList.size(); ++i)
    {
        d->albumDlg->categoryCombo()->addItem(
            categoriesList.at(i).name,
            categoriesList.at(i).id);

        if (d->currentCategoryID == categoriesList.at(i).id)
            d->albumDlg->categoryCombo()->setCurrentIndex(i);
    }

    d->currentCategoryID = d->albumDlg->categoryCombo()->itemData(
                          d->albumDlg->categoryCombo()->currentIndex()).toLongLong();
    d->talker->listSubCategories(d->currentCategoryID);
}

void SmugWindow::slotListSubCategoriesDone(int errCode,
                                           const QString &errMsg,
                                           const QList <SmugCategory>& categoriesList)
{
    // always put at least default <none> subcategory
    d->albumDlg->subCategoryCombo()->clear();
    d->albumDlg->subCategoryCombo()->addItem(i18n("&lt;none&gt;"), 0);

    if (errCode != 0)
    {
        QMessageBox::critical(QApplication::activeWindow(), i18n("Error"),
                              i18n("SmugMug Call Failed: %1\n", errMsg));
        return;
    }

    for (int i = 0; i < categoriesList.size(); ++i)
    {
        d->albumDlg->subCategoryCombo()->addItem(
            categoriesList.at(i).name,
            categoriesList.at(i).id);
    }
}

void SmugWindow::slotTemplateSelectionChanged(int index)
{
    if (index < 0)
        return;

    d->currentTmplID = d->albumDlg->templateCombo()->itemData(index).toLongLong();

    // if template is selected, then disable Security & Privacy
    d->albumDlg->privateGroupBox()->setEnabled(d->currentTmplID == 0);
}

void SmugWindow::slotCategorySelectionChanged(int index)
{
    if (index < 0)
        return;

    // subcategories are per category -> reload
    d->currentCategoryID = d->albumDlg->categoryCombo()->itemData(index).toLongLong();
    d->talker->listSubCategories(d->currentCategoryID);
}

void SmugWindow::buttonStateChange(bool state)
{
    d->widget->m_newAlbumBtn->setEnabled(state);
    d->widget->m_reloadAlbumsBtn->setEnabled(state);
    startButton()->setEnabled(state);
}

void SmugWindow::slotBusy(bool val)
{
    if (val)
    {
        setCursor(Qt::WaitCursor);
        d->widget->m_changeUserBtn->setEnabled(false);
        buttonStateChange(false);
    }
    else
    {
        setCursor(Qt::ArrowCursor);
        d->widget->m_changeUserBtn->setEnabled(!d->widget->isAnonymous());
        buttonStateChange(d->talker->loggedIn());
    }
}

void SmugWindow::slotUserChangeRequest(bool anonymous)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Slot Change User Request";

    if (d->talker->loggedIn())
        d->talker->logout();

    if (anonymous)
    {
        authenticate();
    }
    else
    {
        // fill in current email and password
        d->loginDlg->setLogin(d->email);
        d->loginDlg->setPassword(d->password);

        if (d->loginDlg->exec())
        {
            d->email    = d->loginDlg->login();
            d->password = d->loginDlg->password();
            authenticate(d->email, d->password);
        }
    }
}

void SmugWindow::slotReloadAlbumsRequest()
{
    if (d->import)
    {
        d->talker->listAlbums(d->widget->getNickName());
    }
    else
    {
        // get albums for current user
        d->talker->listAlbums();
    }
}

void SmugWindow::slotNewAlbumRequest()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Slot New Album Request";

    // get list of album templates from SmugMug to fill in dialog
    d->talker->listAlbumTmpl();

    if (d->albumDlg->exec() == QDialog::Accepted)
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Calling New Album method";
        d->currentTmplID = d->albumDlg->templateCombo()->itemData(
                        d->albumDlg->templateCombo()->currentIndex()).toLongLong();
        d->currentCategoryID = d->albumDlg->categoryCombo()->itemData(
                        d->albumDlg->categoryCombo()->currentIndex()).toLongLong();

        SmugAlbum newAlbum;
        d->albumDlg->getAlbumProperties(newAlbum);
        d->talker->createAlbum(newAlbum);
    }
}

void SmugWindow::slotStartTransfer()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "slotStartTransfer invoked";

    if (d->import)
    {
        d->widget->progressBar()->setFormat(i18n("%v / %m"));
        d->widget->progressBar()->setMaximum(0);
        d->widget->progressBar()->setValue(0);
        d->widget->progressBar()->progressScheduled(i18n("SmugMug Import"), true, true);
        d->widget->progressBar()->progressThumbnailChanged(
            QIcon(QLatin1String("smugmug")).pixmap(22, 22));
        setUiInProgressState(true);

        // list photos of the album, then start download
        QString dataStr = d->widget->m_albumsCoB->itemData(d->widget->m_albumsCoB->currentIndex()).toString();
        int colonIdx = dataStr.indexOf(QLatin1Char(':'));
        qint64 albumID = dataStr.left(colonIdx).toLongLong();
        QString albumKey = dataStr.right(dataStr.length() - colonIdx - 1);
        d->talker->listPhotos(albumID, albumKey,
                             d->widget->getAlbumPassword(),
                             d->widget->getSitePassword());
    }
    else
    {
        d->widget->m_imgList->clearProcessedStatus();
        d->transferQueue = d->widget->m_imgList->imageUrls();

        if (d->transferQueue.isEmpty())
            return;

        QString data = d->widget->m_albumsCoB->itemData(d->widget->m_albumsCoB->currentIndex()).toString();
        int colonIdx = data.indexOf(QLatin1Char(':'));
        d->currentAlbumID = data.left(colonIdx).toLongLong();
        d->currentAlbumKey = data.right(data.length() - colonIdx - 1);

        d->imagesTotal = d->transferQueue.count();
        d->imagesCount = 0;

        d->widget->progressBar()->setFormat(i18n("%v / %m"));
        d->widget->progressBar()->setMaximum(d->imagesTotal);
        d->widget->progressBar()->setValue(0);
        d->widget->progressBar()->progressScheduled(i18n("SmugMug Export"), true, true);
        d->widget->progressBar()->progressThumbnailChanged(QIcon(QLatin1String("smugmug")).pixmap(22, 22));
        setUiInProgressState(true);

        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "d->currentAlbumID" << d->currentAlbumID;
        uploadNextPhoto();
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "slotStartTransfer done";
    }
}

bool SmugWindow::prepareImageForUpload(const QString& imgPath) const
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
    d->tmpPath  = d->tmpDir + QFileInfo(imgPath).baseName().trimmed() + QString::fromLatin1(".jpg");

    // rescale image if requested
    int maxDim = d->widget->m_dimensionSpB->value();

    if (d->widget->m_resizeChB->isChecked() &&
        (image.width() > maxDim || image.height() > maxDim))
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Resizing to " << maxDim;
        image = image.scaled(maxDim, maxDim, Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation);
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Saving to temp file: " << d->tmpPath;
    image.save(d->tmpPath, "JPEG", d->widget->m_imageQualitySpB->value());

    // copy meta-data to temporary image
    DMetadata meta;

    if (meta.load(imgPath))
    {
        meta.setImageDimensions(image.size());
        meta.setImageOrientation(MetaEngine::ORIENTATION_NORMAL);
        meta.setImageProgramId(QString::fromLatin1("digiKam"), digiKamVersion());
        meta.setMetadataWritingMode((int)DMetadata::WRITETOIMAGEONLY);
        meta.save(d->tmpPath);
    }

    return true;
}

void SmugWindow::uploadNextPhoto()
{
    if (d->transferQueue.isEmpty())
    {
        setUiInProgressState(false);
        return;
    }

    d->widget->m_imgList->processing(d->transferQueue.first());

    QUrl imgPath = d->transferQueue.first();
    DItemInfo info(d->iface->itemInfo(imgPath));

    d->widget->progressBar()->setMaximum(d->imagesTotal);
    d->widget->progressBar()->setValue(d->imagesCount);

    bool res;

    if (d->widget->m_resizeChB->isChecked())
    {
        if (!prepareImageForUpload(imgPath.toLocalFile()))
        {
            slotAddPhotoDone(666, i18n("Cannot open file"));
            return;
        }

        res = d->talker->addPhoto(d->tmpPath, d->currentAlbumID, d->currentAlbumKey, info.comment());
    }
    else
    {
        d->tmpPath.clear();
        res = d->talker->addPhoto(imgPath.toLocalFile(), d->currentAlbumID, d->currentAlbumKey, info.comment());
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
    if (!d->tmpPath.isEmpty())
    {
        QFile::remove(d->tmpPath);
        d->tmpPath.clear();
    }

    d->widget->m_imgList->processed(d->transferQueue.first(), (errCode == 0));

    if (errCode == 0)
    {
        d->transferQueue.pop_front();
        d->imagesCount++;
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
            d->transferQueue.clear();
            return;
        }
    }

    uploadNextPhoto();
}

void SmugWindow::downloadNextPhoto()
{
    if (d->transferQueue.isEmpty())
    {
        setUiInProgressState(false);
        return;
    }

    d->widget->progressBar()->setMaximum(d->imagesTotal);
    d->widget->progressBar()->setValue(d->imagesCount);

    QString imgPath = d->transferQueue.first().url();

    d->talker->getPhoto(imgPath);
}

void SmugWindow::slotGetPhotoDone(int errCode,
                                  const QString& errMsg,
                                  const QByteArray& photoData)
{
    QString imgPath = d->widget->getDestinationPath() + QLatin1Char('/')
                      + d->transferQueue.first().fileName();

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
            d->transferQueue.pop_front();
            d->imagesCount++;
        }
        else
        {
            if (QMessageBox::question(this, i18n("Processing Failed"),
                                      i18n("Failed to save photo: %1\n"
                                           "Do you want to continue?", errText))
                != QMessageBox::Yes)
            {
                d->transferQueue.clear();
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
            d->transferQueue.clear();
            setUiInProgressState(false);
            return;
        }
    }

    downloadNextPhoto();
}

void SmugWindow::slotCreateAlbumDone(int errCode,
                                     const QString& errMsg,
                                     qint64 newAlbumID,
                                     const QString& newAlbumKey)
{
    if (errCode != 0)
    {
        QMessageBox::critical(QApplication::activeWindow(), i18n("Error"), i18n("SmugMug Call Failed: %1\n", errMsg));
        return;
    }

    // reload album list and automatically select new album
    d->currentAlbumID  = newAlbumID;
    d->currentAlbumKey = newAlbumKey;
    d->talker->listAlbums();
}

void SmugWindow::slotImageListChanged()
{
    startButton()->setEnabled(!(d->widget->m_imgList->imageUrls().isEmpty()));
}

} // namespace Digikam
