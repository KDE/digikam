/* ============================================================
 * 
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 
 * Description :
 *
 * Copyright (C) 2013-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2013-2014 by Gowtham Ashok <gwty93 at gmail dot com>
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

#ifndef QUALITYSCANDIALOG_H
#define QUALITYSCANDIALOG_H

// Qt includes

#include <QList>

// KDE includes

#include <kdialog.h>

// Local includes

#include "statesavingobject.h"
#include "qualityscansettings.h"

namespace Digikam
{

class QualityScanDialog : public KDialog, public StateSavingObject
{
{
    Q_OBJECT

public:

    explicit QualityScanDialog(QWidget* const parent = 0);
    ~QualityScanDialog();

    QualityScanSettings settings() const;

protected:

    void doLoadState();
    void doSaveState();
    void accept();

protected Q_SLOTS:

    void setDetectionDefaultParameters();
    void updateClearButtons();
    void benchmarkButtonToggled(bool on);

private:

    void setupUi();
    void setupConnections();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // FACESCANDIALOG_H
