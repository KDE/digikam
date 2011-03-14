/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-10-09
 * Description : Dialog to choose options for face scanning
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef FACESCANDIALOG_H
#define FACESCANDIALOG_H

// Qt includes

#include <QList>

// KDE includes

#include <kdialog.h>

// Local includes

#include "statesavingobject.h"

namespace Digikam
{

class Album;

class FaceScanSettings
{
public:

    enum ScanTask
    {
        DetectAndRecognize,
        RecognizeMarkedFaces,
        RetrainAll,
        Benchmark
    };

    ScanTask      task;

    // for detect and recognize
    enum AlreadyScannedHandling
    {
        Skip,
        Merge,
        Rescan
    };
    AlreadyScannedHandling alreadyScannedHandling;

    // detection
    double        accuracy;

    // albums to scan
    QList<Album*> albums;

    // processing power
    bool          useFullCpu;
};

// -------------------------------------------------------------------------------

class FaceScanDialog : public KDialog, public StateSavingObject
{
    Q_OBJECT

public:

    FaceScanDialog(QWidget* parent = 0);
    ~FaceScanDialog();

    FaceScanSettings settings() const;

protected:

    void doLoadState();
    void doSaveState();
    void accept();

protected Q_SLOTS:

    void setDetectionDefaultParameters();
    void updateClearButtons();
    void retrainAllButtonToggled(bool on);
    void benchmarkButtonToggled(bool on);

private:

    void setupUi();
    void setupConnections();

private:

    class FaceScanDialogPriv;
    FaceScanDialogPriv* const d;
};

} // namespace Digikam

#endif // FACESCANDIALOG_H
