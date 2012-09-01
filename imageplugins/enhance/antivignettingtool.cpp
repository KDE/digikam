/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-25
 * Description : a digiKam image plugin to reduce
 *               vignetting on an image.
 *
 * Copyright (C) 2004-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Julien Narboux <julien at narboux dot fr>
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

#include "antivignettingtool.moc"

// Qt includes

#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QPainter>
#include <QPen>
#include <QPixmap>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <knuminput.h>
#include <kstandarddirs.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "antivignettingfilter.h"
#include "antivignettingsettings.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageguidewidget.h"

namespace DigikamEnhanceImagePlugin
{

class AntiVignettingTool::AntiVignettingToolPriv
{
public:

    AntiVignettingToolPriv() :
        configGroupName("antivignetting Tool"),
        settingsView(0),
        previewWidget(0),
        gboxSettings(0)
    {}

    const QString           configGroupName;

    AntiVignettingSettings* settingsView;
    ImageGuideWidget*       previewWidget;
    EditorToolSettings*     gboxSettings;
};

AntiVignettingTool::AntiVignettingTool(QObject* parent)
    : EditorToolThreaded(parent),
      d(new AntiVignettingToolPriv)
{
    setObjectName("antivignetting");
    setToolName(i18n("Vignetting Correction"));
    setToolIcon(SmallIcon("antivignetting"));

    d->previewWidget = new ImageGuideWidget(0, false, ImageGuideWidget::HVGuideMode);
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::UnSplitPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel|
                                EditorToolSettings::Try);

    // -------------------------------------------------------------

    d->settingsView = new AntiVignettingSettings(d->gboxSettings->plainPage());
    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(d->settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotTimer()));
}

AntiVignettingTool::~AntiVignettingTool()
{
    delete d;
}

void AntiVignettingTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->settingsView->readSettings(group);
    slotEffect();
}

void AntiVignettingTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->settingsView->writeSettings(group);
    group.sync();
}

void AntiVignettingTool::slotResetSettings()
{
    d->settingsView->resetToDefault();
    slotEffect();
}

void AntiVignettingTool::prepareEffect()
{
    AntiVignettingContainer settings = d->settingsView->settings();

    ImageIface* iface = d->previewWidget->imageIface();
    int previewWidth  = iface->previewWidth();
    int previewHeight = iface->previewHeight();
    DImg imTemp       = iface->getOriginalImg()->smoothScale(previewWidth, previewHeight, Qt::KeepAspectRatio);

    setFilter(new AntiVignettingFilter(&imTemp, this, settings));
}

void AntiVignettingTool::prepareFinal()
{
    AntiVignettingContainer settings = d->settingsView->settings();

    ImageIface iface(0, 0);
    setFilter(new AntiVignettingFilter(iface.getOriginalImg(), this, settings));
}

void AntiVignettingTool::putPreviewData()
{
    ImageIface* iface = d->previewWidget->imageIface();
    DImg preview      = filter()->getTargetImage().smoothScale(iface->previewWidth(), iface->previewHeight());
    iface->putPreview(preview);
    d->previewWidget->updatePreview();
}

void AntiVignettingTool::putFinalData()
{
    ImageIface* iface = d->previewWidget->imageIface();
    DImg finalImage   = filter()->getTargetImage();

    iface->putOriginalImage(i18n("Vignetting Correction"), filter()->filterAction(), finalImage.bits());
}

}  // namespace DigikamEnhanceImagePlugin
