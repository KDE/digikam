/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-05-20
 * Description : a tool to export images to Pinterest web service
 *
 * Copyright (C) 2018      by Tarek Talaat <tarektalaat93 at gmail dot com>
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

#ifndef P_WINDOW_H
#define P_WINDOW_H

// Qt includes

#include <QList>
#include <QUrl>
#include <QPair>

// Local includes

#include "wstooldialog.h"
#include "digikam_export.h"
#include "dinfointerface.h"

class QCloseEvent;
class QUrl;

namespace Digikam
{

class DIGIKAM_EXPORT PWindow : public WSToolDialog
{
    Q_OBJECT

public:

    explicit PWindow(DInfoInterface* const iface, QWidget* const parent);
    ~PWindow();

    void reactivate();

    void setItemsList(const QList<QUrl>& urls);

private:

    void readSettings();
    void writeSettings();

    void uploadNextPhoto();

    void buttonStateChange(bool state);
    void closeEvent(QCloseEvent*) Q_DECL_OVERRIDE;

private Q_SLOTS:

    void slotImageListChanged();
    void slotUserChangeRequest();
    void slotNewBoardRequest();
    void slotReloadBoardsRequest();
    void slotStartTransfer();

    void slotBusy(bool);
    void slotSignalLinkingFailed();
    void slotSignalLinkingSucceeded();
    void slotSetUserName(const QString& msg);
    void slotListBoardsFailed(const QString& msg);
    void slotListBoardsDone(const QList<QPair<QString, QString> >& list);
    void slotCreateBoardFailed(const QString& msg);
    void slotCreateBoardSucceeded();
    void slotAddPinFailed(const QString& msg);
    void slotAddPinSucceeded();
    void slotTransferCancel();

    void slotFinished();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // P_WINDOW_H
