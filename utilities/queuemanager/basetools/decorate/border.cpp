/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-17
 * Description : batch tool to add border.
 *
 * Copyright (C) 2010-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "border.moc"

// Qt includes

#include <QWidget>

// KDE includes

#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>

// Local includes

#include "dimg.h"
#include "borderfilter.h"
#include "bordersettings.h"

namespace Digikam
{

Border::Border(QObject* const parent)
    : BatchTool("Border", DecorateTool, parent)
{
    m_settingsView = 0;

    setToolTitle(i18n("Add Border"));
    setToolDescription(i18n("Add a border around images"));
    setToolIconName("bordertool");
}

Border::~Border()
{
}

void Border::registerSettingsWidget()
{
    m_settingsWidget = new QWidget;
    m_settingsView   = new BorderSettings(m_settingsWidget);
    m_settingsView->resetToDefault();

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings Border::defaultSettings()
{
    BatchToolSettings prm;
    BorderContainer defaultPrm = m_settingsView->defaultSettings();

    prm.insert("preserveAspectRatio",   defaultPrm.preserveAspectRatio);
    prm.insert("orgWidth",              defaultPrm.orgWidth);
    prm.insert("orgHeight",             defaultPrm.orgHeight);
    prm.insert("borderType",            defaultPrm.borderType);
    prm.insert("borderWidth1",          defaultPrm.borderWidth1);
    prm.insert("borderWidth2",          defaultPrm.borderWidth2);
    prm.insert("borderWidth3",          defaultPrm.borderWidth3);
    prm.insert("borderWidth4",          defaultPrm.borderWidth4);
    prm.insert("borderPercent",         defaultPrm.borderPercent);
    prm.insert("borderPath",            defaultPrm.borderPath);
    prm.insert("solidColor",            defaultPrm.solidColor);
    prm.insert("niepceBorderColor",     defaultPrm.niepceBorderColor);
    prm.insert("niepceLineColor",       defaultPrm.niepceLineColor);
    prm.insert("bevelUpperLeftColor",   defaultPrm.bevelUpperLeftColor);
    prm.insert("bevelLowerRightColor",  defaultPrm.bevelLowerRightColor);
    prm.insert("decorativeFirstColor",  defaultPrm.decorativeFirstColor);
    prm.insert("decorativeSecondColor", defaultPrm.decorativeSecondColor);

    return prm;
}

void Border::slotAssignSettings2Widget()
{
    BorderContainer prm;

    prm.preserveAspectRatio   = settings()["preserveAspectRatio"].toBool();
    prm.borderType            = settings()["borderType"].toInt();
    prm.borderWidth1          = settings()["borderWidth1"].toInt();
    prm.borderWidth2          = settings()["borderWidth2"].toInt();
    prm.borderWidth3          = settings()["borderWidth3"].toInt();
    prm.borderWidth4          = settings()["borderWidth4"].toInt();
    prm.borderPercent         = settings()["borderPercent"].toDouble();
    prm.borderPath            = settings()["borderPath"].toString();
    prm.solidColor            = settings()["solidColor"].value<QColor>();
    prm.niepceBorderColor     = settings()["niepceBorderColor"].value<QColor>();
    prm.niepceLineColor       = settings()["niepceLineColor"].value<QColor>();
    prm.bevelUpperLeftColor   = settings()["bevelUpperLeftColor"].value<QColor>();
    prm.bevelLowerRightColor  = settings()["bevelLowerRightColor"].value<QColor>();
    prm.decorativeFirstColor  = settings()["decorativeFirstColor"].value<QColor>();
    prm.decorativeSecondColor = settings()["decorativeSecondColor"].value<QColor>();

    m_settingsView->setSettings(prm);
}

void Border::slotSettingsChanged()
{
    BatchToolSettings prm;
    BorderContainer currentPrm = m_settingsView->settings();

    prm.insert("preserveAspectRatio",   currentPrm.preserveAspectRatio);
    prm.insert("borderType",            currentPrm.borderType);
    prm.insert("borderWidth1",          currentPrm.borderWidth1);
    prm.insert("borderWidth2",          currentPrm.borderWidth2);
    prm.insert("borderWidth3",          currentPrm.borderWidth3);
    prm.insert("borderWidth4",          currentPrm.borderWidth4);
    prm.insert("borderPercent",         currentPrm.borderPercent);
    prm.insert("borderPath",            currentPrm.borderPath);
    prm.insert("solidColor",            currentPrm.solidColor);
    prm.insert("niepceBorderColor",     currentPrm.niepceBorderColor);
    prm.insert("niepceLineColor",       currentPrm.niepceLineColor);
    prm.insert("bevelUpperLeftColor",   currentPrm.bevelUpperLeftColor);
    prm.insert("bevelLowerRightColor",  currentPrm.bevelLowerRightColor);
    prm.insert("decorativeFirstColor",  currentPrm.decorativeFirstColor);
    prm.insert("decorativeSecondColor", currentPrm.decorativeSecondColor);

    BatchTool::slotSettingsChanged(prm);
}

bool Border::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    BorderContainer prm;
    prm.preserveAspectRatio   = settings()["preserveAspectRatio"].toBool();
    prm.borderType            = settings()["borderType"].toInt();
    prm.borderWidth1          = settings()["borderWidth1"].toInt();
    prm.borderWidth2          = settings()["borderWidth2"].toInt();
    prm.borderWidth3          = settings()["borderWidth3"].toInt();
    prm.borderWidth4          = settings()["borderWidth4"].toInt();
    prm.borderPercent         = settings()["borderPercent"].toDouble();
    prm.borderPath            = settings()["borderPath"].toString();
    prm.solidColor            = settings()["solidColor"].value<QColor>();
    prm.niepceBorderColor     = settings()["niepceBorderColor"].value<QColor>();
    prm.niepceLineColor       = settings()["niepceLineColor"].value<QColor>();
    prm.bevelUpperLeftColor   = settings()["bevelUpperLeftColor"].value<QColor>();
    prm.bevelLowerRightColor  = settings()["bevelLowerRightColor"].value<QColor>();
    prm.decorativeFirstColor  = settings()["decorativeFirstColor"].value<QColor>();
    prm.decorativeSecondColor = settings()["decorativeSecondColor"].value<QColor>();
    prm.orgWidth              = image().width();
    prm.orgHeight             = image().height();

    BorderFilter bd(&image(), 0L, prm);
    applyFilterChangedProperties(&bd);

    return (savefromDImg());
}

}  // namespace Digikam
