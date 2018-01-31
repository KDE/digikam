/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-09-12
 * Description : a tool to export items to YandexFotki web service
 *
 * Copyright (C) 2015 by Shourya Singh Gupta <shouryasgupta at gmail dot com>
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

#ifndef YF_WIDGET_H
#define YF_WIDGET_H

// Local includes

#include "settingswidget.h"
#include "dinfointerface.h"

namespace Digikam
{

class YFWidget : public SettingsWidget
{
    Q_OBJECT

public:

    enum UpdatePolicy
    {
        POLICY_UPDATE_MERGE = 0,
        POLICY_UPDATE_KEEP, // is not used in GUI
        POLICY_SKIP,
        POLICY_ADDNEW
    };

public:

    explicit YFWidget(QWidget* const parent, DInfoInterface* const iface, const QString& pluginName);
    ~YFWidget();

    void updateLabels(const QString& name = QString(), const QString& url = QString()) Q_DECL_OVERRIDE;

private:

    // upload settings
    QComboBox*    m_accessCombo;
    QCheckBox*    m_hideOriginalCheck;
    QCheckBox*    m_disableCommentsCheck;
    QCheckBox*    m_adultCheck;
    QButtonGroup* m_policyGroup;

    friend class YFWindow;
};

}  // namespace Digikam

#endif // YF_WIDGET_H
