/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-11-03
 * Description : a tool to create calendar.
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006      by Tom Albers <tomalbers at kde dot nl>
 * Copyright (C) 2007-2008 by Orgad Shaneh <orgads at gmail dot com>
 * Copyright (C) 2012      by Angelo Naselli <anaselli at linux dot it>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CAL_WIZARD_H
#define CAL_WIZARD_H

// Qt includes

#include <QMap>

// Local includes

#include "dwizarddlg.h"
#include "dwizardpage.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT CalWizard : public DWizardDlg
{
    Q_OBJECT

public:

    explicit CalWizard(const QList<QUrl>& urlList, QWidget* const parent=0);
    ~CalWizard();

private:

    void print();

private Q_SLOTS:

    void slotPageSelected(int current);
    void printComplete();
    void updatePage(int page);

private:

    class Private;
    Private* const d;
};

} // Namespace Digikam

#endif // CAL_WIZARD_H
