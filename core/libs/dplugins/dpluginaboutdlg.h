/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-12-31
 * Description : digiKam plugin about dialog
 *
 * Copyright (C) 2018-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_DPLUGIN_ABOUT_DLG_H
#define DIGIKAM_DPLUGIN_ABOUT_DLG_H

#include <QDialog>

// Local includes

#include "digikam_export.h"
#include "dplugin.h"

namespace Digikam
{

class DIGIKAM_EXPORT DPluginAboutDlg : public QDialog
{
    Q_OBJECT

public:

    explicit DPluginAboutDlg(DPlugin* const tool, QWidget* const parent = 0);
    ~DPluginAboutDlg();

private:

    Q_DISABLE_COPY(DPluginAboutDlg)

};

} // namespace Digikam

#endif // DIGIKAM_DPLUGIN_ABOUT_DLG_H
