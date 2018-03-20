/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-09
 * Description : scanner dialog
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef SCANDIALOG_H
#define SCANDIALOG_H

// Qt includes

#include <QCloseEvent>
#include <QDialog>

class QWidget;

namespace KSaneIface
{
    class KSaneWidget;
}

using namespace KSaneIface;

namespace Digikam
{

class ScanDialog : public QDialog
{
    Q_OBJECT

public:

    ScanDialog(KSaneWidget* const saneWdg, const QString& config, QWidget* const parent=0);
    ~ScanDialog();

    void setTargetDir(const QString& targetDir);

protected:

    void closeEvent(QCloseEvent*);

Q_SIGNALS:

    void signalImportedImage(const QUrl&);

private Q_SLOTS:

    void slotSaveImage(QByteArray&, int, int, int, int);
    void slotThreadProgress(const QUrl&, int);
    void slotThreadDone(const QUrl&, bool);
    void slotDialogFinished();

private:

    void readSettings();
    void saveSettings();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // SCANDIALOG_H
