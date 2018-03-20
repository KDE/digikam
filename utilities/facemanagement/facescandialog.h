/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-10-09
 * Description : Dialog to choose options for face scanning
 *
 * Copyright (C) 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef FACE_SCAN_DIALOG_H
#define FACE_SCAN_DIALOG_H

// Qt includes

#include <QList>
#include <QDialog>

// Local includes

#include "statesavingobject.h"
#include "facescansettings.h"

namespace Digikam
{

class FaceScanDialog : public QDialog, public StateSavingObject
{
    Q_OBJECT

public:

    explicit FaceScanDialog(QWidget* const parent = 0);
    ~FaceScanDialog();

    FaceScanSettings settings() const;

protected:

    void doLoadState();
    void doSaveState();

protected Q_SLOTS:

    void retrainAllButtonToggled(bool on);

private:

    void setupUi();
    void setupConnections();
    void adjustDetailsButton(bool on);

private Q_SLOTS:

    void slotDetails();
    void slotOk();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // FACE_SCAN_DIALOG_H
