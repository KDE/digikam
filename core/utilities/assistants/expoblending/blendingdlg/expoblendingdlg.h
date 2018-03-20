/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-12-13
 * Description : a tool to blend bracketed images.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2015      by Benjamin Girault, <benjamin dot girault at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef EXPOBLENDINGDLG_H
#define EXPOBLENDINGDLG_H

// Qt includes

#include <QString>
#include <QPixmap>
#include <QDialog>
#include <QDialogButtonBox>

// Local includes

#include "expoblendingactions.h"

class QCloseEvent;

namespace Digikam
{

class ExpoBlendingManager;
class ExpoBlendingActionData;

class ExpoBlendingDlg : public QDialog
{
    Q_OBJECT

public:

    explicit ExpoBlendingDlg(ExpoBlendingManager* const mngr, QWidget* const parent=0);
    ~ExpoBlendingDlg();

    void loadItems(const QList<QUrl>& urls);

Q_SIGNALS:

    void cancelClicked();

private:

    void closeEvent(QCloseEvent*);

    void setRejectButtonMode(QDialogButtonBox::StandardButton button);

    void readSettings();
    void saveSettings();

    void busy(bool busy);
    void saveItem(const QUrl& temp, const EnfuseSettings& settings);

    void setIdentity(const QUrl& url, const QString& identity);

private Q_SLOTS:

    void slotCloseClicked();
    void slotDefault();
    void slotPreview();
    void slotProcess();
    void slotCancelClicked();
    void slotFinished();

    void slotLoadProcessed(const QUrl&);
    void slotExpoBlendingAction(const Digikam::ExpoBlendingActionData&);
    void slotAddItems(const QList<QUrl>& urls);

    void slotPreviewButtonClicked();
    void slotFileFormatChanged();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // EXPOBLENDINGDLG_H
