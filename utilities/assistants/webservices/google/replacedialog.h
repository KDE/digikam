/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-15
 * Description : a kipi plugin to export images to Google Photo web service
 *
 * Copyright (C) 2010 by Jens Mueller <tschenser at gmx dot de>
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

#ifndef REPLACEDIALOG_H
#define REPLACEDIALOG_H

// Qt includes

#include <QDialog>
#include <QString>
#include <QUrl>
#include <QNetworkReply>

// Libkipi includes

#include <KIPI/Interface>

using namespace KIPI;

namespace KIPIGoogleServicesPlugin
{

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
                  Interface* const iface,
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
    void slotThumbnail(const QUrl&, const QPixmap&);
    void slotProgressTimerDone();

private:

    QPixmap setProgressAnimation(const QPixmap& thumb, const QPixmap& pix);

private:

    class Private;
    Private* const d;
};

} // namespace KIPIGoogleServicesPlugin

#endif // REPLACEDIALOG_H
