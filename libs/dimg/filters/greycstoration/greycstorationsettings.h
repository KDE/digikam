/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-13
 * Description : Greycstoration settings widgets
 *
 * Copyright  (C) 2007-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef GREYCSTORATION_SETTINGS_H
#define GREYCSTORATION_SETTINGS_H

// Qt includes

#include <QtCore/QObject>
#include <QtCore/QFile>
#include <QtCore/QString>

// Local includes

#include "digikam_export.h"
#include "greycstorationfilter.h"

class KTabWidget;

namespace Digikam
{

class DIGIKAM_EXPORT GreycstorationSettings : public QObject
{
    Q_OBJECT

public:

    explicit GreycstorationSettings(KTabWidget* parent);
    ~GreycstorationSettings();

    void setSettings(const GreycstorationContainer& settings);
    GreycstorationContainer settings() const;

    void setDefaultSettings(const GreycstorationContainer& settings);

    bool loadSettings(QFile& file, const QString& header);
    void saveSettings(QFile& file, const QString& header);

    void setEnabled(bool);

private:

    class GreycstorationSettingsPriv;
    GreycstorationSettingsPriv* const d;
};

} // namespace Digikam

#endif /* GREYCSTORATION_SETTINGS_H */
