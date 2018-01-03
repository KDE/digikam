/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
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

#ifndef HTML_IMAGE_SETTINGS_PAGE_H
#define HTML_IMAGE_SETTINGS_PAGE_H

// Qt includes

#include <QString>

// Local includes

#include "dwizardpage.h"

namespace Digikam
{

class HTMLImageSettingsPage : public DWizardPage
{
public:

    explicit HTMLImageSettingsPage(QWizard* const dialog, const QString& title);
    ~HTMLImageSettingsPage();

    void initializePage();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // HTML_IMAGE_SETTINGS_PAGE_H
