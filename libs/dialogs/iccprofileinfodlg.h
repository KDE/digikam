/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-16
 * Description : a dialog to display ICC profile information.
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ICCPROFILEINFODLG_H
#define ICCPROFILEINFODLG_H

// Qt includes

#include <QByteArray>
#include <QString>
#include <QDialog>

// Local includes

#include "digikam_export.h"
#include "icctransform.h"

class QWidget;

namespace Digikam
{

class DIGIKAM_EXPORT ICCProfileInfoDlg : public QDialog
{
    Q_OBJECT

public:

    ICCProfileInfoDlg(QWidget* const parent, const QString& profilePath, const IccProfile& profileData = IccProfile());
    ~ICCProfileInfoDlg();

private Q_SLOTS:

    void slotHelp();
};

}  // namespace Digikam

#endif /* ICCPROFILEINFODLG_H */
