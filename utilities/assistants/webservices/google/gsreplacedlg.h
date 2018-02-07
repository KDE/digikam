/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-15
 * Description : a tool to export items to Google web services
 *
 * Copyright (C) 2010      by Jens Mueller <tschenser at gmx dot de>
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

#ifndef GS_REPLACE_DLG_H
#define GS_REPLACE_DLG_H

// Qt includes

#include <QDialog>
#include <QString>
#include <QUrl>
#include <QNetworkReply>

// Local includes

#include "dinfointerface.h"

namespace Digikam
{

class LoadingDescription;

enum ReplaceDialog_Result
{
    PWR_CANCEL      = 0,
    PWR_ADD         = 1,
    PWR_ADD_ALL     = 2,
    PWR_REPLACE     = 3,
    PWR_REPLACE_ALL = 4
};

class ReplaceDialog : public QDialog
{
    Q_OBJECT

public:

    ReplaceDialog(QWidget* const parent,
                  const QString& caption,
                  DInfoInterface* const iface,
                  const QUrl& src,
                  const QUrl& dest);
    ~ReplaceDialog();

    int getResult();

public Q_SLOTS:

    void cancelPressed();
    void addPressed();
    void addAllPressed();
    void replacePressed();
    void replaceAllPressed();

private Q_SLOTS:

    void slotFinished(QNetworkReply* reply);
    void slotThumbnail(const LoadingDescription&, const QPixmap&);
    void slotProgressTimerDone();

private:

    QPixmap setProgressAnimation(const QPixmap& thumb, const QPixmap& pix);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // GS_REPLACE_DLG_H
