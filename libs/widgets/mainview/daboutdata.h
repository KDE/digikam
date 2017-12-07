/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-07-30
 * Description : digiKam about data.
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QUrl>
#include <QString>

// Local includes

#include "digikam_export.h"

class KAboutData;

namespace Digikam
{
class DXmlGuiWindow;

class DIGIKAM_EXPORT DAboutData : public QObject
{
public:

    explicit DAboutData(DXmlGuiWindow* const parent);
    ~DAboutData();

    static const QString digiKamSloganFormated();
    static const QString digiKamSlogan();
    static const QString copyright();
    static const QUrl    webProjectUrl();
    static void          authorsRegistration(KAboutData& aboutData);
};

} // namespace Digikam

#endif // DABOUT_DATA_H
