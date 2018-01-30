/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-11-18
 * Description : a kipi plugin to export images to Google web service
 *
 * Copyright (C) 2013      by Pankaj Kumar <me at panks dot me>
 * Copyright (C) 2015      by Shourya Singh Gupta <shouryasgupta at gmail dot com>
 * Copyright (C) 2008-2016 by Caulier Gilles <caulier dot gilles at gmail dot com>
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

#include "exportutils.h"
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

GSWindow::GSWindow(DInfoInterface* const iface,
                   QWidget* const /*parent*/,
                   const QString& serviceName)
    : ToolDialog(0),
      m_widget(0),
      m_albumDlg(0),
      m_gphoto_albumdlg(0),
      m_talker(0),
      m_gphoto_talker(0),
      m_iface(iface)
{
    m_serviceName = serviceName;

    if (QString::compare(m_serviceName, QString::fromLatin1("googledriveexport"), Qt::CaseInsensitive) == 0)
    {
        m_name       = PluginName::GDrive;
        m_pluginName = QString::fromLatin1("Google Drive");
    }
    else if (QString::compare(m_serviceName, QString::fromLatin1("googlephotoexport"), Qt::CaseInsensitive) == 0)
    {
        m_name       = PluginName::GPhotoExport;
        m_pluginName = QString::fromLatin1("Google Photos/PicasaWeb");
    }
    else
    {
        m_name       = PluginName::GPhotoImport;
        m_pluginName = QString::fromLatin1("Google Photos/PicasaWeb");
    }

    m_tmp         = ExportUtils::makeTemporaryDir("google").absolutePath() + QLatin1Char('/');;
    m_imagesCount = 0;
    m_imagesTotal = 0;
    m_renamingOpt = 0;
    m_widget      = new GSWidget(this, m_iface, m_name, m_pluginName);

    setMainWidget(m_widget);
    setModal(false);

    switch (m_name)
    {
        case PluginName::GDrive:

            setWindowIcon(QIcon::fromTheme(QString::fromLatin1("kipi-googledrive")));
            setWindowTitle(i18n("Export to Google Drive"));

            startButton()->setText(i18n("Start Upload"));
            startButton()->setToolTip(i18n("Start upload to Google Drive"));

            m_widget->setMinimumSize(700,500);

            m_albumDlg = new GSNewAlbumDlg(this, m_serviceName, m_pluginName);
            m_talker   = new GDTalker(this);

            connect(m_talker,SIGNAL(signalBusy(bool)),
                    this,SLOT(slotBusy(bool)));

            connect(m_talker,SIGNAL(signalTextBoxEmpty()),
                    this,SLOT(slotTextBoxEmpty()));

            connect(m_talker,SIGNAL(signalAccessTokenFailed(int, QString)),
                    this,SLOT(slotAccessTokenFailed(int, QString)));

            connect(m_talker,SIGNAL(signalAccessTokenObtained()),
                    this,SLOT(slotAccessTokenObtained()));

            connect(m_talker,SIGNAL(signalRefreshTokenObtained(QString)),
                    this,SLOT(slotRefreshTokenObtained(QString)));

            connect(m_talker,SIGNAL(signalSetUserName(QString)),
                    this,SLOT(slotSetUserName(QString)));

            connect(m_talker,SIGNAL(signalListAlbumsDone(int, QString, QList<GSFolder>)),
                    this,SLOT(slotListAlbumsDone(int, QString, QList<GSFolder>)));

            connect(m_talker,SIGNAL(signalCreateFolderDone(int, QString)),
                    this,SLOT(slotCreateFolderDone(int, QString)));

            connect(m_talker,SIGNAL(signalAddPhotoDone(int, QString, QString)),
                    this,SLOT(slotAddPhotoDone(int, QString, QString)));

            readSettings();
            buttonStateChange(false);

            if (m_refresh_token.isEmpty())
            {
                m_talker->doOAuth();
            }
            else
            {
                m_talker->getAccessTokenFromRefreshToken(m_refresh_token);
            }

            break;

        case PluginName::GPhotoImport:
        case PluginName::GPhotoExport:

            setWindowIcon(QIcon::fromTheme(QString::fromLatin1("kipi-googlephoto")));

            if (m_name == PluginName::GPhotoExport)
            {
                setWindowTitle(i18n("Export to Google Photos/PicasaWeb Service"));

                startButton()->setText(i18n("Start Upload"));
                startButton()->setToolTip(i18n("Start upload to Google Photos/PicasaWeb Service"));

                m_widget->setMinimumSize(700, 500);
            }
            else
            {
                setWindowTitle(i18n("Import from Google Photos/PicasaWeb Service"));

                startButton()->setText(i18n("Start Download"));
                startButton()->setToolTip(i18n("Start download from Google Photos/PicasaWeb service"));

                m_widget->setMinimumSize(300, 400);
            }

            m_gphoto_albumdlg = new GSNewAlbumDlg(this, m_serviceName, m_pluginName);
            m_gphoto_talker   = new GPTalker(this);

            connect(m_gphoto_talker, SIGNAL(signalBusy(bool)),
                    this, SLOT(slotBusy(bool)));

            connect(m_gphoto_talker, SIGNAL(signalTextBoxEmpty()),
                    this, SLOT(slotTextBoxEmpty()));

            connect(m_gphoto_talker, SIGNAL(signalAccessTokenFailed(int, QString)),
                    this, SLOT(slotAccessTokenFailed(int, QString)));

            connect(m_gphoto_talker, SIGNAL(signalAccessTokenObtained()),
                    this, SLOT(slotAccessTokenObtained()));

            connect(m_gphoto_talker, SIGNAL(signalRefreshTokenObtained(QString)),
                    this, SLOT(slotRefreshTokenObtained(QString)));

            connect(m_gphoto_talker, SIGNAL(signalListAlbumsDone(int, QString, QList<GSFolder>)),
                    this, SLOT(slotListAlbumsDone(int, QString, QList<GSFolder>)));

            connect(m_gphoto_talker, SIGNAL(signalCreateAlbumDone(int, QString, QString)),
                    this, SLOT(slotCreateFolderDone(int, QString, QString)));

            connect(m_gphoto_talker, SIGNAL(signalAddPhotoDone(int, QString, QString)),
                    this, SLOT(slotAddPhotoDone(int,QString,QString)));

            connect(m_gphoto_talker, SIGNAL(signalGetPhotoDone(int, QString, QByteArray)),
                    this, SLOT(slotGetPhotoDone(int, QString, QByteArray)));

            readSettings();
            buttonStateChange(false);

            if (m_refresh_token.isEmpty())
            {
                m_gphoto_talker->doOAuth();
            }
            else
            {
                m_gphoto_talker->getAccessTokenFromRefreshToken(m_refresh_token);
            }

            break;
    }

    connect(m_widget->imagesList(), SIGNAL(signalImageListChanged()),
            this, SLOT(slotImageListChanged()));

    connect(m_widget->getChangeUserBtn(), SIGNAL(clicked()),
            this, SLOT(slotUserChangeRequest()));

    connect(m_widget->getNewAlbmBtn(), SIGNAL(clicked()),
            this,SLOT(slotNewAlbumRequest()));

    connect(m_widget->getReloadBtn(), SIGNAL(clicked()),
            this, SLOT(slotReloadAlbumsRequest()));

    connect(startButton(), SIGNAL(clicked()),
            this, SLOT(slotStartTransfer()));

    connect(this, SIGNAL(finished(int)),
            this, SLOT(slotFinished()));
}

GSWindow::~GSWindow()
{
    delete m_widget;
    delete m_albumDlg;
    delete m_gphoto_albumdlg;
    delete m_talker;
    delete m_gphoto_talker;
}

void GSWindow::reactivate()
{
    m_widget->imagesList()->loadImagesFromCurrentSelection();
    m_widget->progressBar()->hide();

    show();
}

void GSWindow::readSettings()
{
    KConfig config;
    KConfigGroup grp;

    switch (m_name)
    {
        case PluginName::GDrive:
            grp = config.group("Google Drive Settings");
            break;
        default:
            grp = config.group("Google Photo Settings");
            break;
    }

    m_currentAlbumId = grp.readEntry("Current Album",QString());
    m_refresh_token  = grp.readEntry("refresh_token");

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

    if (m_name == PluginName::GPhotoExport)
        m_widget->m_tagsBGrp->button(grp.readEntry("Tag Paths", 0))->setChecked(true);

    KConfigGroup dialogGroup;

    switch (m_name)
    {
        case PluginName::GDrive:
            dialogGroup = config.group("Google Drive Export Dialog");
            break;
        case PluginName::GPhotoExport:
            dialogGroup = config.group("Google Photo Export Dialog");
            break;
        case PluginName::GPhotoImport:
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

    switch(m_name)
    {
        case PluginName::GDrive:
            grp = config.group("Google Drive Settings");
            break;
        default:
            grp = config.group("Google Photo Settings");
            break;
    }

    grp.writeEntry("refresh_token", m_refresh_token);
    grp.writeEntry("Current Album", m_currentAlbumId);
    grp.writeEntry("Resize",        m_widget->getResizeCheckBox()->isChecked());
    grp.writeEntry("Maximum Width", m_widget->getDimensionSpB()->value());
    grp.writeEntry("Image Quality", m_widget->getImgQualitySpB()->value());

    if (m_name == PluginName::GPhotoExport)
        grp.writeEntry("Tag Paths", m_widget->m_tagsBGrp->checkedId());

    KConfigGroup dialogGroup;

    switch (m_name)
    {
        case PluginName::GDrive:
            dialogGroup = config.group("Google Drive Export Dialog");
            break;
        case PluginName::GPhotoExport:
            dialogGroup = config.group("Google Photo Export Dialog");
            break;
        case PluginName::GPhotoImport:
            dialogGroup = config.group("Google Photo Import Dialog");
            break;
    }

    KWindowConfig::saveWindowSize(windowHandle(), dialogGroup);
    config.sync();
}

void GSWindow::slotSetUserName(const QString& msg)
{
    m_widget->updateLabels(msg);
}

void GSWindow::slotListPhotosDoneForDownload(int errCode,
                                             const QString& errMsg,
                                             const QList <GSPhoto>& photosList)
{
    disconnect(m_gphoto_talker, SIGNAL(signalListPhotosDone(int, QString, QList<GSPhoto>)),
               this, SLOT(slotListPhotosDoneForDownload(int, QString, QList<GSPhoto>)));

    if (errCode == 0)
    {
        QMessageBox::critical(this, i18nc("@title:window", "Error"),
                              i18n("Google Photos/PicasaWeb Call Failed: %1\n", errMsg));
        return;
    }

    typedef QPair<QUrl,GSPhoto> Pair;
    m_transferQueue.clear();
    QList<GSPhoto>::const_iterator itPWP;

    for (itPWP = photosList.begin(); itPWP != photosList.end(); ++itPWP)
    {
        m_transferQueue.push_back(Pair((*itPWP).originalURL, (*itPWP)));
    }

    if (m_transferQueue.isEmpty())
        return;

    m_currentAlbumId = m_widget->getAlbumsCoB()->itemData(m_widget->getAlbumsCoB()->currentIndex()).toString();
    m_imagesTotal    = m_transferQueue.count();
    m_imagesCount    = 0;

    m_widget->progressBar()->setFormat(i18n("%v / %m"));
    m_widget->progressBar()->show();

    m_renamingOpt = 0;

    // start download with first photo in queue
    downloadNextPhoto();
}

void GSWindow::slotListPhotosDoneForUpload(int errCode,
                                           const QString& errMsg,
                                           const QList <GSPhoto>& photosList)
{
    qCCritical(DIGIKAM_GENERAL_LOG)<< "err Code is "<< errCode <<" Err Message is "<< errMsg;

    disconnect(m_gphoto_talker, SIGNAL(signalListPhotosDone(int, QString, QList<GSPhoto>)),
               this, SLOT(slotListPhotosDoneForUpload(int, QString, QList<GSPhoto>)));

    if (errCode == 0)
    {
        QMessageBox::critical(this, i18nc("@title:window", "Error"),
                              i18n("Google Photos/PicasaWeb Call Failed: %1\n", errMsg));
        return;
    }

    typedef QPair<QUrl,GSPhoto> Pair;

    m_transferQueue.clear();

    QList<QUrl> urlList = m_widget->imagesList()->imageUrls(true);

    if (urlList.isEmpty())
        return;

    for (QList<QUrl>::ConstIterator it = urlList.constBegin(); it != urlList.constEnd(); ++it)
    {
        DItemInfo info(m_iface->itemInfo((*it).toLocalFile()));
        GSPhoto temp;
        temp.title = info.name();

        // Google Photo doesn't support image titles. Include it in descriptions if needed.
        QStringList descriptions = QStringList() << info.title() << info.comment();
        descriptions.removeAll(QString::fromLatin1(""));
        temp.description         = descriptions.join(QString::fromLatin1("\n\n"));

        // check for existing items
        QString localId;

        if (m_meta.load((*it).toLocalFile()))
        {
            localId = m_meta.getXmpTagString("Xmp.digiKam.picasawebGPhotoId");
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
        m_transferQueue.append( Pair( (*it), temp) );
    }

    if (m_transferQueue.isEmpty())
        return;

    m_currentAlbumId = m_widget->getAlbumsCoB()->itemData(m_widget->getAlbumsCoB()->currentIndex()).toString();
    m_imagesTotal    = m_transferQueue.count();
    m_imagesCount    = 0;

    m_widget->progressBar()->setFormat(i18n("%v / %m"));
    m_widget->progressBar()->setMaximum(m_imagesTotal);
    m_widget->progressBar()->setValue(0);
    m_widget->progressBar()->show();
    m_widget->progressBar()->progressScheduled(i18n("Google Photo Export"), true, true);
    m_widget->progressBar()->progressThumbnailChanged(QIcon(
        (QLatin1String(":/icons/kipi-icon.svg"))).pixmap(22, 22));

    m_renamingOpt = 0;

    uploadNextPhoto();
}

void GSWindow::slotListAlbumsDone(int code,const QString& errMsg ,const QList <GSFolder>& list)
{
    switch (m_name)
    {
        case PluginName::GDrive:

            if (code == 0)
            {
                QMessageBox::critical(this, i18nc("@title:window", "Error"),
                                      i18n("Google Drive Call Failed: %1\n", errMsg));
                return;
            }

            m_widget->getAlbumsCoB()->clear();
            qCDebug(DIGIKAM_GENERAL_LOG) << "slotListAlbumsDone1:" << list.size();

            for (int i=0;i<list.size();i++)
            {
                m_widget->getAlbumsCoB()->addItem(
                    QIcon::fromTheme(QString::fromLatin1("system-users")),
                    list.value(i).title, list.value(i).id);

                if (m_currentAlbumId == list.value(i).id)
                {
                    m_widget->getAlbumsCoB()->setCurrentIndex(i);
                }
            }

            buttonStateChange(true);
            m_talker->getUserName();
            break;

        default:

            if (code == 0)
            {
                QMessageBox::critical(this, i18nc("@title:window", "Error"),
                                      i18n("Google Photos/PicasaWeb Call Failed: %1\n", errMsg));
                return;
            }

            m_widget->updateLabels(m_gphoto_talker->getLoginName(), m_gphoto_talker->getUserName());
            m_widget->getAlbumsCoB()->clear();

            for (int i = 0; i < list.size(); ++i)
            {
                QString albumIcon;

                if (list.at(i).access == QString::fromLatin1("public"))
                    albumIcon = QString::fromLatin1("folder-image");
                else if (list.at(i).access == QString::fromLatin1("protected"))
                    albumIcon = QString::fromLatin1("folder-locked");
                else
                    albumIcon = QString::fromLatin1("folder");

                m_widget->getAlbumsCoB()->addItem(QIcon::fromTheme(albumIcon), list.at(i).title, list.at(i).id);

                if (m_currentAlbumId == list.at(i).id)
                    m_widget->getAlbumsCoB()->setCurrentIndex(i);

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

void GSWindow::googlePhotoTransferHandler()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Google Photo Transfer invoked";

    switch (m_name)
    {
        case PluginName::GPhotoImport:
            // list photos of the album, then start download
            connect(m_gphoto_talker, SIGNAL(signalListPhotosDone(int, QString, QList<GSPhoto>)),
                    this, SLOT(slotListPhotosDoneForDownload(int, QString, QList<GSPhoto>)));

            m_gphoto_talker->listPhotos(
                m_widget->getAlbumsCoB()->itemData(m_widget->getAlbumsCoB()->currentIndex()).toString(),
                m_widget->getDimensionCoB()->itemData(m_widget->getDimensionCoB()->currentIndex()).toString());
            break;

        default:
            // list photos of the album, then start upload with add/update items
            connect(m_gphoto_talker, SIGNAL(signalListPhotosDone(int, QString, QList<GSPhoto>)),
                    this, SLOT(slotListPhotosDoneForUpload(int, QString, QList<GSPhoto>)));

            m_gphoto_talker->listPhotos(
                m_widget->getAlbumsCoB()->itemData(m_widget->getAlbumsCoB()->currentIndex()).toString());
            break;
    }
}

void GSWindow::slotTextBoxEmpty()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "in slotTextBoxEmpty";
    QMessageBox::critical(this, i18nc("@title:window", "Error"),
                          i18n("The textbox is empty, please enter the code from the browser in the textbox. "
                               "To complete the authentication click \"Change Account\", "
                               "or \"Start Upload\" to authenticate again."));
}

void GSWindow::slotStartTransfer()
{
    m_widget->imagesList()->clearProcessedStatus();

    switch (m_name)
    {
        case PluginName::GDrive:
        case PluginName::GPhotoExport:
            if (m_widget->imagesList()->imageUrls().isEmpty())
            {
                QMessageBox::critical(this, i18nc("@title:window", "Error"),
                                      i18n("No image selected. Please select which images should be uploaded."));
                return;
            }
            break;
        case PluginName::GPhotoImport:
            break;
    }

    switch (m_name)
    {
        case PluginName::GDrive:
            if (!(m_talker->authenticated()))
            {
                QMessageBox warn(QMessageBox::Warning,
                                 i18n("Warning"),
                                 i18n("Authentication failed. Click \"Continue\" to authenticate."),
                                 QMessageBox::Yes | QMessageBox::No);

                (warn.button(QMessageBox::Yes))->setText(i18n("Continue"));
                (warn.button(QMessageBox::No))->setText(i18n("Cancel"));

                if (warn.exec() == QMessageBox::Yes)
                {
                    m_talker->doOAuth();
                    return;
                }
                else
                {
                    return;
                }
            }
            break;

        default:
            if (!(m_gphoto_talker->authenticated()))
            {
                QMessageBox warn(QMessageBox::Warning,
                                 i18n("Warning"),
                                 i18n("Authentication failed. Click \"Continue\" to authenticate."),
                                 QMessageBox::Yes | QMessageBox::No);

                (warn.button(QMessageBox::Yes))->setText(i18n("Continue"));
                (warn.button(QMessageBox::No))->setText(i18n("Cancel"));

                if (warn.exec() == QMessageBox::Yes)
                {
                    m_gphoto_talker->doOAuth();
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

    for (int i = 0 ; i < (m_widget->imagesList()->imageUrls().size()) ; i++)
    {
        DItemInfo info(m_iface->itemInfo(m_widget->imagesList()->imageUrls().value(i).toLocalFile()));
        GSPhoto temp;
        qCDebug(DIGIKAM_GENERAL_LOG) << "in start transfer info " <<info.title() << info.comment();

        switch (m_name)
        {
            case PluginName::GDrive:
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

        m_transferQueue.append(Pair(m_widget->imagesList()->imageUrls().value(i),temp));
    }

    m_currentAlbumId = m_widget->getAlbumsCoB()->itemData(m_widget->getAlbumsCoB()->currentIndex()).toString();
    m_imagesTotal    = m_transferQueue.count();
    m_imagesCount    = 0;

    m_widget->progressBar()->setFormat(i18n("%v / %m"));
    m_widget->progressBar()->setMaximum(m_imagesTotal);
    m_widget->progressBar()->setValue(0);
    m_widget->progressBar()->show();
    m_widget->progressBar()->progressScheduled(i18n("Google Drive export"), true, true);
    m_widget->progressBar()->progressThumbnailChanged(QIcon(QLatin1String(":/icons/kipi-icon.svg")).pixmap(22, 22));

    uploadNextPhoto();
}

void GSWindow::uploadNextPhoto()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "in upload nextphoto " << m_transferQueue.count();

    if (m_transferQueue.isEmpty())
    {
        //m_widget->progressBar()->hide();
        m_widget->progressBar()->progressCompleted();
        return;
    }

    typedef QPair<QUrl,GSPhoto> Pair;
    Pair pathComments = m_transferQueue.first();
    GSPhoto info      = pathComments.second;
    bool res          = true;
    m_widget->imagesList()->processing(pathComments.first);

    switch (m_name)
    {
        case PluginName::GDrive:
        {
            res = m_talker->addPhoto(pathComments.first.toLocalFile(),
                                     info,
                                     m_currentAlbumId,
                                     m_widget->getResizeCheckBox()->isChecked(),
                                     m_widget->getDimensionSpB()->value(),
                                     m_widget->getImgQualitySpB()->value());
            break;
        }

        case PluginName::GPhotoExport:
        {
            bool bCancel = false;
            bool bAdd    = true;

            if (!info.id.isEmpty() && !info.editUrl.isEmpty())
            {
                switch(m_renamingOpt)
                {
                    case PWR_ADD_ALL:
                        bAdd = true;
                        break;
                    case PWR_REPLACE_ALL:
                        bAdd = false;
                        break;
                    default:
                    {
                        ReplaceDialog dlg(this, QString::fromLatin1(""), m_iface, pathComments.first, info.thumbURL);
                        dlg.exec();

                        switch (dlg.getResult())
                        {
                            case PWR_ADD_ALL:
                                m_renamingOpt = PWR_ADD_ALL;
                                break;
                            case PWR_ADD:
                                bAdd = true;
                                break;
                            case PWR_REPLACE_ALL:
                                m_renamingOpt = PWR_REPLACE_ALL;
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

            switch (m_widget->m_tagsBGrp->checkedId())
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
                    res = m_gphoto_talker->addPhoto(pathComments.first.toLocalFile(),
                                                     info,
                                                     m_currentAlbumId,
                                                     m_widget->getResizeCheckBox()->isChecked(),
                                                     m_widget->getDimensionSpB()->value(),
                                                     m_widget->getImgQualitySpB()->value());
                }
                else
                {
                    res = m_gphoto_talker->updatePhoto(pathComments.first.toLocalFile(),
                                                        info,
                                                        m_widget->getResizeCheckBox()->isChecked(),
                                                        m_widget->getDimensionSpB()->value(),
                                                        m_widget->getImgQualitySpB()->value());
                }
            }
            break;
        }

        case PluginName::GPhotoImport:
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
    if (m_transferQueue.isEmpty())
    {
        m_widget->progressBar()->hide();
        m_widget->progressBar()->progressCompleted();
        return;
    }

    m_widget->progressBar()->setMaximum(m_imagesTotal);
    m_widget->progressBar()->setValue(m_imagesCount);

    QString imgPath = m_transferQueue.first().first.url();

    m_gphoto_talker->getPhoto(imgPath);
}

void GSWindow::slotGetPhotoDone(int errCode, const QString& errMsg, const QByteArray& photoData)
{
    GSPhoto item = m_transferQueue.first().second;
    QUrl tmpUrl  = QUrl::fromLocalFile(QString(m_tmp + item.title));

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
            if (m_meta.load(tmpUrl.toLocalFile()))
            {
                if (m_meta.supportXmp() && m_meta.canWriteXmp(tmpUrl.toLocalFile()))
                {
                    m_meta.setXmpTagString("Xmp.digiKam.picasawebGPhotoId", item.id);
                    m_meta.setXmpKeywords(item.tags);
                }

                if (!item.gpsLat.isEmpty() && !item.gpsLon.isEmpty())
                {
                    m_meta.setGPSInfo(0.0, item.gpsLat.toDouble(), item.gpsLon.toDouble());
                }

                m_meta.setMetadataWritingMode((int)DMetadata::WRITETOIMAGEONLY);
                m_meta.save(tmpUrl.toLocalFile());
            }

            m_transferQueue.pop_front();
            m_imagesCount++;
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

    QUrl newUrl = QUrl::fromLocalFile(QString(m_widget->getDestinationPath() + tmpUrl.fileName()));

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
    else
    {
/* FIXME
        DItemInfo info(m_iface->itemInfo(newUrl));
        info.setName(item.title);
        info.setComment(item.description);
        info.setTagsPath(item.tags);

        if (!item.gpsLat.isEmpty() && !item.gpsLon.isEmpty())
        {
            info.setLatitude(item.gpsLat.toDouble());
            info.setLongitude(item.gpsLon.toDouble());
        }
*/
    }

    downloadNextPhoto();
}

void GSWindow::slotAddPhotoDone(int err, const QString& msg, const QString& photoId)
{
    if (err == 0)
    {
        m_widget->imagesList()->processed(m_transferQueue.first().first,false);

        QMessageBox warn(QMessageBox::Warning,
                         i18n("Warning"),
                         i18n("Failed to upload photo to %1.\n%2\nDo you want to continue?",m_pluginName,msg),
                         QMessageBox::Yes | QMessageBox::No);

        (warn.button(QMessageBox::Yes))->setText(i18n("Continue"));
        (warn.button(QMessageBox::No))->setText(i18n("Cancel"));

        if (warn.exec() != QMessageBox::Yes)
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
    else
    {
        QUrl fileUrl = m_transferQueue.first().first;

        if (m_meta.supportXmp()                       &&
            m_meta.canWriteXmp(fileUrl.toLocalFile()) &&
            m_meta.load(fileUrl.toLocalFile())        &&
            !photoId.isEmpty()
           )
        {
            m_meta.setXmpTagString("Xmp.digiKam.picasawebGPhotoId", photoId);
            m_meta.save(fileUrl.toLocalFile());
        }

        // Remove photo uploaded from the list
        m_widget->imagesList()->removeItemByUrl(m_transferQueue.first().first);
        m_transferQueue.pop_front();
        m_imagesCount++;
        qCDebug(DIGIKAM_GENERAL_LOG) << "In slotAddPhotoSucceeded" << m_imagesCount;
        m_widget->progressBar()->setMaximum(m_imagesTotal);
        m_widget->progressBar()->setValue(m_imagesCount);
        uploadNextPhoto();
    }
}

void GSWindow::slotImageListChanged()
{
    startButton()->setEnabled(!(m_widget->imagesList()->imageUrls().isEmpty()));
}

void GSWindow::slotNewAlbumRequest()
{
    switch (m_name)
    {
        case PluginName::GDrive:
            if (m_albumDlg->exec() == QDialog::Accepted)
            {
                GSFolder newFolder;
                m_albumDlg->getAlbumProperties(newFolder);
                m_currentAlbumId = m_widget->getAlbumsCoB()->itemData(m_widget->getAlbumsCoB()->currentIndex()).toString();
                m_talker->createFolder(newFolder.title,m_currentAlbumId);
            }
            break;

        default:
            if (m_gphoto_albumdlg->exec() == QDialog::Accepted)
            {
                GSFolder newFolder;
                m_gphoto_albumdlg->getAlbumProperties(newFolder);
                m_gphoto_talker->createAlbum(newFolder);
            }
            break;
    }
}

void GSWindow::slotReloadAlbumsRequest()
{
    switch (m_name)
    {
        case PluginName::GDrive:
            m_talker->listFolders();
            break;
        case PluginName::GPhotoImport:
        case PluginName::GPhotoExport:
            m_gphoto_talker->listAlbums();
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
    switch (m_name)
    {
        case PluginName::GDrive:
            m_talker->listFolders();
            break;
        case PluginName::GPhotoImport:
        case PluginName::GPhotoExport:
            m_gphoto_talker->listAlbums();
            break;
    }
}

void GSWindow::slotRefreshTokenObtained(const QString& msg)
{
    switch (m_name)
    {
        case PluginName::GDrive:
            m_refresh_token = msg;
            m_talker->listFolders();
            break;
        case PluginName::GPhotoImport:
        case PluginName::GPhotoExport:
            m_refresh_token = msg;
            m_gphoto_talker->listAlbums();
            break;
    }
}

void GSWindow::slotCreateFolderDone(int code, const QString& msg, const QString& albumId)
{
    switch (m_name)
    {
        case PluginName::GDrive:
            if (code == 0)
                QMessageBox::critical(this, i18nc("@title:window", "Error"),
                                      i18n("Google Drive call failed:\n%1", msg));
            else
                m_talker->listFolders();
            break;
        case PluginName::GPhotoImport:
        case PluginName::GPhotoExport:
            if (code == 0)
                QMessageBox::critical(this, i18nc("@title:window", "Error"),
                                      i18n("Google Photos/PicasaWeb call failed:\n%1", msg));
            else
            {
                m_currentAlbumId = albumId;
                m_gphoto_talker->listAlbums();
            }
            break;
    }
}

void GSWindow::slotTransferCancel()
{
    m_transferQueue.clear();
    m_widget->progressBar()->hide();

    switch (m_name)
    {
        case PluginName::GDrive:
            m_talker->cancel();
            break;
        case PluginName::GPhotoImport:
        case PluginName::GPhotoExport:
            m_gphoto_talker->cancel();
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
        m_refresh_token = QString::fromLatin1("");

        switch (m_name)
        {
            case PluginName::GDrive:
                m_talker->doOAuth();
                break;
            case PluginName::GPhotoImport:
            case PluginName::GPhotoExport:
                m_gphoto_talker->doOAuth();
                break;
        }
    }
}

void GSWindow::buttonStateChange(bool state)
{
    m_widget->getNewAlbmBtn()->setEnabled(state);
    m_widget->getReloadBtn()->setEnabled(state);
    startButton()->setEnabled(state);
}

void GSWindow::slotFinished()
{
    writeSettings();
    m_widget->imagesList()->listView()->clear();
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
