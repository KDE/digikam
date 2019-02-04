/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to acquire images with a digital scanner.
 *
 * Copyright (C) 2018-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_DSCANNER_PLUGIN_H
#define DIGIKAM_DSCANNER_PLUGIN_H

// Local includes

#include "dplugingeneric.h"

#define DPLUGIN_IID "org.kde.digikam.plugin.generic.DigitalScanner"

namespace KSaneIface
{
    class KSaneWidget;
}

using namespace Digikam;

namespace DigikamGenericDScannerPlugin
{

class DigitalScannerPlugin : public DPluginGeneric
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DPLUGIN_IID)
    Q_INTERFACES(Digikam::DPluginGeneric)

public:

    explicit DigitalScannerPlugin(QObject* const parent = 0);
    ~DigitalScannerPlugin();

    QString name()                 const override;
    QString iid()                  const override;
    QIcon   icon()                 const override;
    QString details()              const override;
    QString description()          const override;
    QList<DPluginAuthor> authors() const override;

    void setup(QObject* const) override;

private Q_SLOTS:

    void slotDigitalScanner();

private:

    KSaneIface::KSaneWidget* m_saneWidget;
};

} // namespace DigikamGenericDScannerPlugin

#endif // DIGIKAM_DSCANNER_PLUGIN_H
