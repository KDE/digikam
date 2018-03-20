/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : a tool to export items to web services.
 *
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef WS_SETTINGS_PAGE_H
#define WS_SETTINGS_PAGE_H

// Qt includes

#include <QString>

// Local includes

#include "dwizardpage.h"

namespace Digikam
{

class WSSettingsPage : public DWizardPage
{
    Q_OBJECT

public:

    explicit WSSettingsPage(QWizard* const dialog, const QString& title);
    ~WSSettingsPage();

    void initializePage();
    bool validatePage();

private Q_SLOTS:

    void slotImagesFormatChanged(int);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // WS_SETTINGS_PAGE_H
