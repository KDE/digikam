/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export and import items with Google web-service.
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

#ifndef DIGIKAM_GS_PLUGIN_H
#define DIGIKAM_GS_PLUGIN_H

// Local includes

#include "dplugingeneric.h"

#define DPLUGIN_IID "org.kde.digikam.plugin.generic.GoogleServices"

using namespace Digikam;

namespace DigikamGenericGoogleServicesPlugin
{

class GSWindow;

class GSPlugin : public DPluginGeneric
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DPLUGIN_IID)
    Q_INTERFACES(Digikam::DPluginGeneric)

public:

    explicit GSPlugin(QObject* const parent = nullptr);
    ~GSPlugin();

    QString name()                 const override;
    QString iid()                  const override;
    QIcon   icon()                 const override;
    QString details()              const override;
    QString description()          const override;
    QList<DPluginAuthor> authors() const override;

    void setup(QObject* const) override;

private Q_SLOTS:

    void slotImportGphoto();
    void slotExportGphoto();
    void slotExportGdrive();

private:

    QPointer<GSWindow> m_toolDlgImportGphoto;
    QPointer<GSWindow> m_toolDlgExportGphoto;
    QPointer<GSWindow> m_toolDlgExportGdrive;
};

} // namespace DigikamGenericGoogleServicesPlugin

#endif // DIGIKAM_GS_PLUGIN_H
