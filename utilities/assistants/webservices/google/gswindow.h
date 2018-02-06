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

#ifndef GS_WINDOW_H
#define GS_WINDOW_H

// Qt includes

#include <QList>
#include <QPair>
#include <QUrl>
#include <QPointer>

// Local includes

#include "digikam_export.h"
#include "wstooldialog.h"
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

class DIGIKAM_EXPORT GSWindow : public WSToolDialog
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

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // GS_WINDOW_H
