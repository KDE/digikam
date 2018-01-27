/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-12
 * Description : a tool to export images to imgur.com
 *
 * Copyright (C) 2010-2012 by Marius Orcsik <marius at habarnam dot ro>
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
#include "tooldialog.h"
#include "imgurapi3.h"
#include "digikam_export.h"
#include "dinfointerface.h"

namespace Digikam
{

class DIGIKAM_EXPORT ImgurWindow : public ToolDialog
{
    Q_OBJECT

public:

    explicit ImgurWindow(DInfoInterface* const iface, QWidget* const parent = 0);
    ~ImgurWindow();

    void reactivate();

    void setItemsList(const QList<QUrl>& urls);

public Q_SLOTS:

    /* UI callbacks */
    void forgetButtonClicked();
    void slotUpload();
    void slotAnonUpload();
    void slotFinished();
    void slotCancel();

    /* ImgurAPI3 callbacks */
    void apiAuthorized(bool success, const QString& username);
    void apiAuthError(const QString& msg);
    void apiProgress(unsigned int percent, const ImgurAPI3Action& action);
    void apiRequestPin(const QUrl& url);
    void apiSuccess(const ImgurAPI3Result& result);
    void apiError(const QString &msg, const ImgurAPI3Action& action);
    void apiBusy(bool busy);

private:

    void closeEvent(QCloseEvent* e) Q_DECL_OVERRIDE;
    void setContinueUpload(bool state);
    void readSettings();
    void saveSettings();

private:

    ImgurImagesList* list = nullptr;
    ImgurAPI3*       api  = nullptr;
    QPushButton*     forgetButton = nullptr;
    QPushButton*     uploadAnonButton = nullptr;
    QLabel*          userLabel = nullptr;
    /* Contains the imgur username if API authorized.
     * If not, username is null. */
    QString          username;
};

} // namespace Digikam

#endif // IMGUR_WINDOW_H
