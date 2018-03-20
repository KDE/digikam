/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-12
 * Description : a tool to export images to Imgur web service
 *
 * Copyright (C) 2010-2012 by Marius Orcsik <marius at habarnam dot ro>
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

#ifndef IMGUR_WINDOW_H
#define IMGUR_WINDOW_H

// Qt includes

#include <QObject>
#include <QLabel>
#include <QList>
#include <QUrl>

// Local includes

#include "imgurimageslist.h"
#include "wstooldialog.h"
#include "imgurtalker.h"
#include "digikam_export.h"
#include "dinfointerface.h"

namespace Digikam
{

class DIGIKAM_EXPORT ImgurWindow : public WSToolDialog
{
    Q_OBJECT

public:

    explicit ImgurWindow(DInfoInterface* const iface, QWidget* const parent = 0);
    ~ImgurWindow();

public:

    void reactivate();

    void setItemsList(const QList<QUrl>& urls);

public Q_SLOTS:

    // UI callbacks
    void slotForgetButtonClicked();
    void slotUpload();
    void slotAnonUpload();
    void slotFinished();
    void slotCancel();

    // ImgurTalker callbacks
    void slotApiAuthorized(bool success, const QString& username);
    void slotApiAuthError(const QString& msg);
    void slotApiProgress(unsigned int percent, const ImgurTalkerAction& action);
    void slotApiRequestPin(const QUrl& url);
    void slotApiSuccess(const ImgurTalkerResult& result);
    void slotApiError(const QString &msg, const ImgurTalkerAction& action);
    void slotApiBusy(bool busy);

private:

    void closeEvent(QCloseEvent* e) Q_DECL_OVERRIDE;
    void setContinueUpload(bool state);
    void readSettings();
    void saveSettings();

private:
   
    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // IMGUR_WINDOW_H
