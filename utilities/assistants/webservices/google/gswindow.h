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

#ifndef GS_WINDOW_H
#define GS_WINDOW_H

// Qt includes

#include <QList>
#include <QPair>
#include <QUrl>
#include <QPointer>

// Local includes

#include "digikam_export.h"
#include "tooldialog.h"
#include "gsitem.h"
#include "dinfointerface.h"
#include "dmetadata.h"

class QCloseEvent;


namespace Digikam
{
class GDTalker;
class GPTalker;
class GSWidget;
class GSPhoto;
class GSFolder;
class GSNewAlbumDlg;

class DIGIKAM_EXPORT GSWindow : public ToolDialog
{
    Q_OBJECT

public:

    explicit GSWindow(DInfoInterface* const iface,
                      QWidget* const parent,
                      const QString& serviceName);
    ~GSWindow();

    void reactivate();

private:

    void readSettings();
    void writeSettings();

    void uploadNextPhoto();
    void downloadNextPhoto();

    void buttonStateChange(bool state);
    void closeEvent(QCloseEvent*) Q_DECL_OVERRIDE;
    void googlePhotoTransferHandler();

private Q_SLOTS:

    void slotImageListChanged();
    void slotUserChangeRequest();
    void slotNewAlbumRequest();
    void slotReloadAlbumsRequest();
    void slotStartTransfer();
    void slotFinished();
    //void slotChangeProgressBar();

    void slotBusy(bool);
    void slotTextBoxEmpty();
    void slotAccessTokenFailed(int errCode,const QString& errMsg);
    void slotAccessTokenObtained();
    void slotRefreshTokenObtained(const QString& msg);
    void slotSetUserName(const QString& msg);
    void slotListAlbumsDone(int,const QString&,const QList <GSFolder>&);
    void slotListPhotosDoneForDownload(int errCode, const QString& errMsg, const QList <GSPhoto>& photosList);
    void slotListPhotosDoneForUpload(int errCode, const QString& errMsg, const QList <GSPhoto>& photosList);
    void slotCreateFolderDone(int,const QString& msg, const QString& = QStringLiteral("-1"));
    void slotAddPhotoDone(int,const QString& msg, const QString&);
    void slotGetPhotoDone(int errCode, const QString& errMsg, const QByteArray& photoData);
    void slotTransferCancel();

private:

    unsigned int                  m_imagesCount;
    unsigned int                  m_imagesTotal;
    int                           m_renamingOpt;

    QString                       m_serviceName;
    QString                       m_pluginName;
    PluginName                    m_name;
    QString                       m_tmp;
    QString                       m_refresh_token;

    GSWidget*                     m_widget;
    GSNewAlbumDlg*                m_albumDlg;
    GSNewAlbumDlg*                m_gphoto_albumdlg;

    GDTalker*                     m_talker;
    GPTalker*                     m_gphoto_talker;

    QString                       m_currentAlbumId;

    QList< QPair<QUrl, GSPhoto> > m_transferQueue;

    DInfoInterface*               m_iface;
    DMetadata                     m_meta;
};

} // namespace Digikam

#endif // GS_WINDOW_H
