/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-24
 * Description : a plugin to reduce CCD noise.
 *
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "noisereductiontool.moc"

// Qt includes

#include <QFile>
#include <QImage>
#include <QString>
#include <QTextStream>

// KDE includes

#include <kaboutdata.h>
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

// Local includes

#include "daboutdata.h"
#include "dimg.h"
#include "waveletsnr.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imagepanelwidget.h"
#include "noisereductionsettings.h"
#include "version.h"

using namespace Digikam;

namespace DigikamImagesPluginCore
{

class NoiseReductionToolPriv
{
public:

    NoiseReductionToolPriv() :
        configGroupName("noisereduction Tool"),
        nrSettings(0),
        previewWidget(0),
        gboxSettings(0)
        {}

    const QString           configGroupName;

    NoiseReductionSettings* nrSettings;
    ImagePanelWidget*       previewWidget;
    EditorToolSettings*     gboxSettings;
};

NoiseReductionTool::NoiseReductionTool(QObject* parent)
                  : EditorToolThreaded(parent),
                    d(new NoiseReductionToolPriv)
{
    setObjectName("noisereduction");
    setToolName(i18n("Noise Reduction"));
    setToolIcon(SmallIcon("noisereduction"));

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel|
                                EditorToolSettings::Load|
                                EditorToolSettings::SaveAs|
                                EditorToolSettings::Try);

    d->nrSettings = new NoiseReductionSettings(d->gboxSettings->plainPage());

    setToolSettings(d->gboxSettings);

    d->previewWidget = new ImagePanelWidget(470, 350, "noisereduction Tool");
    setToolView(d->previewWidget);

    init();
}

NoiseReductionTool::~NoiseReductionTool()
{
    delete d;
}

void NoiseReductionTool::renderingFinished()
{
    d->nrSettings->setEnabled(true);
}

void NoiseReductionTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    d->nrSettings->readSettings(group);
}

void NoiseReductionTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->nrSettings->writeSettings(group);
    d->previewWidget->writeSettings();
    group.sync();
}

void NoiseReductionTool::slotResetSettings()
{
    d->nrSettings->resetToDefault();
}

void NoiseReductionTool::prepareEffect()
{
    d->nrSettings->setEnabled(false);

    DImg image              = d->previewWidget->getOriginalRegionImage();
    WaveletsNRContainer prm = d->nrSettings->settings();

    setFilter(dynamic_cast<DImgThreadedFilter*>(new WaveletsNR(&image, this, prm)));
}

void NoiseReductionTool::prepareFinal()
{
    d->nrSettings->setEnabled(false);

    WaveletsNRContainer prm = d->nrSettings->settings();

    ImageIface iface(0, 0);
    setFilter(dynamic_cast<DImgThreadedFilter*>(new WaveletsNR(iface.getOriginalImg(), this, prm)));
}

void NoiseReductionTool::putPreviewData()
{
    d->previewWidget->setPreviewImage(filter()->getTargetImage());
}

void NoiseReductionTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Noise Reduction"), filter()->getTargetImage().bits());
}

void NoiseReductionTool::slotLoadSettings()
{
    d->nrSettings->loadSettings();
}

void NoiseReductionTool::slotSaveAsSettings()
{
    d->nrSettings->saveAsSettings();
}

}  // namespace DigikamImagesPluginCore
