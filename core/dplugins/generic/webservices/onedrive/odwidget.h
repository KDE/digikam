/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-05-20
 * Description : a tool to export images to Onedrive web service
 *
 * Copyright (C) 2018      by Tarek Talaat <tarektalaat93 at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DIGIKAM_OD_WIDGET_H
#define DIGIKAM_OD_WIDGET_H

// Qt includes

#include <QWidget>

// Local includes

#include "wssettingswidget.h"
#include "oditem.h"
#include "dinfointerface.h"

using namespace Digikam;

class QButtonGroup;

namespace DigikamGenericOneDrivePlugin
{

class ODWidget : public WSSettingsWidget
{
    Q_OBJECT

public:

    explicit ODWidget(QWidget* const parent,
                      DInfoInterface* const iface,
                      const QString& toolName);
    ~ODWidget();

    void updateLabels(const QString& name = QString(),
                      const QString& url = QString()) Q_DECL_OVERRIDE;

private:

    friend class ODWindow;
};

} // namespace DigikamGenericOneDrivePlugin

#endif // DIGIKAM_OD_WIDGET_H
