/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-04
 * Description : metadata edit batch tool.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "metadata.h"
#include "metadata.moc"

// Qt includes.

#include <QWidget>
#include <QLabel>

// KDE includes.

#include <kvbox.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kcombobox.h>

// Local includes.

#include "dimg.h"

namespace Digikam
{

Metadata::Metadata(QObject* parent)
        : BatchTool("Metadata", BaseTool, parent)
{
    setToolTitle(i18n("Edit Metadata"));
    setToolDescription(i18n("A tool to edit metadata"));
    setToolIcon(KIcon(SmallIcon("application-xml")));

    KVBox *vbox   = new KVBox;

    setSettingsWidget(vbox);
}

Metadata::~Metadata()
{
}

BatchToolSettings Metadata::defaultSettings()
{
    BatchToolSettings settings;
//    settings.insert("AutoCorrectionFilter", AutoLevelsCorrection);
    return settings;
}

void Metadata::assignSettings2Widget()
{
//    m_comboBox->setCurrentIndex(settings()["AutoCorrectionFilter"].toInt());
}

void Metadata::slotSettingsChanged()
{
    BatchToolSettings settings;
//    settings.insert("AutoCorrectionFilter", (int)m_comboBox->currentIndex());
    setSettings(settings);
}

bool Metadata::toolOperations()
{
    if (!loadToDImg()) return false;

//    int type = settings()["AutoCorrectionFilter"].toInt();


    return (savefromDImg());
}

}  // namespace Digikam
