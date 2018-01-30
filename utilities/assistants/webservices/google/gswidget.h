/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-11-18
 * Description : a tool to export items to Google web services
 *
 * Copyright (C) 2013 by Pankaj Kumar <me at panks dot me>
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

#ifndef GS_WIDGET_H
#define GS_WIDGET_H

//Qt includes

#include <QWidget>

// Local includes

#include "settingswidget.h"
#include "gsitem.h"
#include "dinfointerface.h"

class QLabel;
class QSpinBox;
class QCheckBox;
class QButtonGroup;
class QComboBox;
class QPushButton;

enum class PluginName;

namespace Digikam
{

enum GPhotoTagsBehaviour
{
    GPTagLeaf = 0,
    GPTagSplit,
    GPTagCombined
};

class GSWidget : public SettingsWidget
{
    Q_OBJECT

public:

    explicit GSWidget(QWidget* const parent,
                      DInfoInterface* const iface,
                      const PluginName& pluginName,
                      const QString& serviceName);
    ~GSWidget();

    void updateLabels(const QString& name = QString(),
                      const QString& url = QString()) Q_DECL_OVERRIDE;

private:

    PluginName    m_pluginName;
    QButtonGroup* m_tagsBGrp;

    friend class GSWindow;
};

} // namespace Digikam

#endif // GS_WIDGET_H
