/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-09-09
 * Description : scanner dialog
 *
 * Copyright (C) 2007-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_SCAN_DIALOG_H
#define DIGIKAM_SCAN_DIALOG_H

// Qt includes

#include <QCloseEvent>
#include <QWidget>

// Local includes

#include "dplugindialog.h"

using namespace Digikam;

namespace KSaneIface
{
    class KSaneWidget;
}

using namespace KSaneIface;

namespace DigikamGenericDScannerPlugin
{

class ScanDialog : public DPluginDialog
{
    Q_OBJECT

public:

    explicit ScanDialog(KSaneWidget* const saneWdg, QWidget* const parent=nullptr);
    ~ScanDialog();

    void setTargetDir(const QString& targetDir);

protected:

    void closeEvent(QCloseEvent*) override;

Q_SIGNALS:

    void signalImportedImage(const QUrl&);

private Q_SLOTS:

    void slotSaveImage(QByteArray&, int, int, int, int);
    void slotThreadProgress(const QUrl&, int);
    void slotThreadDone(const QUrl&, bool);
    void slotDialogFinished();

private:

    class Private;
    Private* const d;
};

} // namespace DigikamGenericDScannerPlugin

#endif // DIGIKAM_SCAN_DIALOG_H
