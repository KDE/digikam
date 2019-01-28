/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : intro page to export tool where user can choose web service to export,
 *               existent accounts and function mode (export/import).
 *
 * Copyright (C) 2017-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2018 by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
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

#ifndef DIGIKAM_WS_INTRO_PAGE_H
#define DIGIKAM_WS_INTRO_PAGE_H

// Qt includes

#include <QString>

// Local includes

#include "dwizardpage.h"

using namespace Digikam;

namespace DigikamGenericUnifiedPlugin
{

class WSIntroPage : public DWizardPage
{
    Q_OBJECT

public:

    explicit WSIntroPage(QWizard* const dialog, const QString& title);
    ~WSIntroPage();

    void initializePage();
    bool validatePage();

private Q_SLOTS:

    void slotImageGetOptionChanged(int index);
    void slotWebServiceOptionChanged(const QString& serviceName);

private:

    class Private;
    Private* const d;
};

} // namespace DigikamGenericUnifiedPlugin

#endif // DIGIKAM_WS_INTRO_PAGE_H
