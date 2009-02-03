/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-07-11
 * Description : shared libraries list dialog
 *
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef LIBSINFODLG_H
#define LIBSINFODLG_H

// Qt includes.

#include <QtCore/QMap>

// KDE includes.

#include <kdialog.h>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class LibsInfoDlgPriv;

class DIGIKAM_EXPORT LibsInfoDlg : public KDialog
{
    Q_OBJECT

public:

    LibsInfoDlg(QWidget* parent);
    ~LibsInfoDlg();

    void setComponentsInfoMap(const QMap<QString, QString>& list);

private Q_SLOTS:

    void slotCopy2ClipBoard();

private:

    LibsInfoDlgPriv* const d;
};

}  // namespace Digikam

#endif  // LIBSINFODLG_H
