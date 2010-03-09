/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : a plugin to enhance image with local contrasts (as human eye does).
 *
 * Copyright (C) 2009 by Julien Pontabry <julien dot pontabry at gmail dot com>
 * Copyright (C) 2009-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "localcontrasttool.moc"

// Qt includes

#include <QCheckBox>
#include <QFile>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QString>
#include <QTextStream>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktabwidget.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>
#include <libkdcraw/rcombobox.h>

// Local includes

#include "dimg.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageregionwidget.h"
#include "localcontrastfilter.h"
#include "localcontrastsettings.h"
#include "rexpanderbox.h"

using namespace KDcrawIface;

namespace DigikamImagesPluginCore
{

class LocalContrastToolPriv
{
public:

    LocalContrastToolPriv() :
        configGroupName("localcontrast Tool"),
        settingsView(0),
        previewWidget(0),
        gboxSettings(0)
        {}

    const QString          configGroupName;

    LocalContrastSettings* settingsView;
    ImageRegionWidget*     previewWidget;
    EditorToolSettings*    gboxSettings;
};

LocalContrastTool::LocalContrastTool(QObject* parent)
                 : EditorToolThreaded(parent),
                   d(new LocalContrastToolPriv)
{
    setObjectName("localcontrast");
    setToolName(i18n("Local Contrast"));
    setToolIcon(SmallIcon("contrast"));

    d->previewWidget = new ImageRegionWidget;
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel|
                                EditorToolSettings::Load|
                                EditorToolSettings::SaveAs|
                                EditorToolSettings::Try);

    // -------------------------------------------------------------

    d->settingsView = new LocalContrastSettings(d->gboxSettings->plainPage());
    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------
}

LocalContrastTool::~LocalContrastTool()
{
    delete d;
}

void LocalContrastTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    d->settingsView->readSettings(group);
}

void LocalContrastTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->settingsView->writeSettings(group);
    group.sync();
}

void LocalContrastTool::slotResetSettings()
{
    d->settingsView->resetToDefault();
}

void LocalContrastTool::prepareEffect()
{
    toolSettings()->setEnabled(false);
    toolView()->setEnabled(false);

    DImg image = d->previewWidget->getOriginalRegionImage(true);
    setFilter(new LocalContrastFilter(&image, this, createParams()));
}

void LocalContrastTool::prepareFinal()
{
    toolSettings()->setEnabled(false);
    toolView()->setEnabled(false);

    ImageIface iface(0, 0);
    setFilter(new LocalContrastFilter(iface.getOriginalImg(), this, createParams()));
}

void LocalContrastTool::putPreviewData()
{
    d->previewWidget->setPreviewImage(filter()->getTargetImage());
}

void LocalContrastTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Local Contrast"), filter()->getTargetImage().bits());
}

void LocalContrastTool::renderingFinished()
{
    toolSettings()->setEnabled(true);
    toolView()->setEnabled(true);
}

void LocalContrastTool::slotLoadSettings()
{
    d->settingsView->loadSettings();
//    d->gboxSettings->histogramBox()->histogram()->reset();
    slotEffect();
}

void LocalContrastTool::slotSaveAsSettings()
{
    d->settingsView->saveSettings();
}

} // namespace DigikamImagesPluginCore
