/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-07-30
 * Description : digiKam about data.
 *
 * Copyright (C) 2008-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DABOUT_DATA_H
#define DABOUT_DATA_H

// Qt includes

#include <QObject>

// KDE includes

#include <kurl.h>
#include <kaboutdata.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_export.h"

class KXmlGuiWindow;

namespace Digikam
{

class DIGIKAM_EXPORT DAboutData : public QObject
{
    Q_OBJECT

public:

    explicit DAboutData(KXmlGuiWindow* const parent);
    ~DAboutData();

    void registerHelpActions();

    static KLocalizedString digiKamSloganFormated();
    static KLocalizedString digiKamSlogan();
    static KLocalizedString copyright();
    static KUrl             webProjectUrl();
    static void             authorsRegistration(KAboutData& aboutData);

private Q_SLOTS:

    void slotRawCameraList();
    void slotDonateMoney();
    void slotContribute();
};

}  // namespace Digikam

#endif // DABOUT_DATA_H
