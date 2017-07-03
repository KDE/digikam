/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : a tool to create calendar.
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CAL_INTRO_PAGE_H
#define CAL_INTRO_PAGE_H

// Qt includes

#include <QString>

// Local includes

#include "dwizardpage.h"

namespace Digikam
{

class CalIntroPage : public DWizardPage
{

public:

    explicit CalIntroPage(QWizard* const dialog, const QString& title);
    ~CalIntroPage();
};

} // namespace Digikam

#endif // CAL_INTRO_PAGE_H
