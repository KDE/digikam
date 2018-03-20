/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-26
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

#ifndef FB_WINDOW_H
#define FB_WINDOW_H

// Qt includes

#include <QList>

// Local includes

#include "digikam_export.h"
#include "dinfointerface.h"
#include "wstooldialog.h"

class QCloseEvent;
class QUrl;

namespace Digikam
{

class FbAlbum;

class DIGIKAM_EXPORT FbWindow : public WSToolDialog
{
    Q_OBJECT

public:

    explicit FbWindow(DInfoInterface* const iface, QWidget* const parent);
    ~FbWindow();

    /**
     * Use this method to (re-)activate the dialog after it has been created
     * to display it. This also loads the currently selected images.
     */
    void reactivate();

private Q_SLOTS:

    void slotBusy(bool val);
    void slotLoginProgress(int step, int maxStep, const QString& label);
    void slotLoginDone(int errCode, const QString& errMsg);
    void slotAddPhotoDone(int errCode, const QString& errMsg);
    void slotCreateAlbumDone(int errCode, const QString& errMsg,
                             const QString &newAlbumID);
    void slotListAlbumsDone(int errCode, const QString& errMsg,
                            const QList<FbAlbum>& albumsList);

    void slotUserChangeRequest();
    void slotReloadAlbumsRequest(long long userID);
    void slotNewAlbumRequest();
    void slotStartTransfer();
    void slotImageListChanged();
    void slotStopAndCloseProgressBar();

    void slotFinished();
    void slotCancelClicked();

private:

    void    setProfileAID(long long userID);
    QString getImageCaption(const QString& fileName);
    bool    prepareImageForUpload(const QString& imgPath, QString& caption);

    void    uploadNextPhoto();

    void    readSettings();
    void    writeSettings();

    void    authenticate();

    void    buttonStateChange(bool state);

    void    closeEvent(QCloseEvent*) Q_DECL_OVERRIDE;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // FB_WINDOW_H
