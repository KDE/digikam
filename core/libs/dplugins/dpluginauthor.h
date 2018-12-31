/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : author data container for external plugin
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

#ifndef DIGIKAM_DPLUGIN_AUTHOR_H
#define DIGIKAM_DPLUGIN_AUTHOR_H

// Qt includes

#include <QString>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DPluginAuthor
{
public:

    explicit DPluginAuthor(const QString& n,
                           const QString& e,
                           const QString& y,
                           const QString& r = i18n("Developer"));

    ~DPluginAuthor();

    /**
     * Return author details as string.
     * For debug purpose only.
     */
    QString toString() const;

public:

    QString name;    // Author name and surname
    QString email;   // Email anti-spammed
    QString years;   // Copyrights years
    QString roles;   // Author roles, as "Developer", "Designer", "Translator", etc.
};

} // namespace Digikam

#endif // DIGIKAM_DPLUGIN_AUTHOR_H
