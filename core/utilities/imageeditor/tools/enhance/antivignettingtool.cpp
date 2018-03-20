/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-25
 * Description : a digiKam image tool to reduce
 *               vignetting on an image.
 *
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "antivignettingtool.h"

// Qt includes

#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "dnuminput.h"
#include "antivignettingfilter.h"
#include "antivignettingsettings.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageguidewidget.h"

namespace Digikam
{

class AntiVignettingTool::Private
{
public:

    Private() :
        configGroupName(QLatin1String("antivignetting Tool")),
        settingsView(0),
        previewWidget(0),
        gboxSettings(0)
    {}

    const QString           configGroupName;

    AntiVignettingSettings* settingsView;
    ImageGuideWidget*       previewWidget;
    EditorToolSettings*     gboxSettings;
};

AntiVignettingTool::AntiVignettingTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d(new Private)
{
    setObjectName(QLatin1String("antivignetting"));
    setToolName(i18n("Vignetting Correction"));
    setToolIcon(QIcon::fromTheme(QLatin1String("antivignetting")));

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
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->settingsView->readSettings(group);
    slotPreview();
}

void AntiVignettingTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->settingsView->writeSettings(group);
    group.sync();
}

void AntiVignettingTool::slotResetSettings()
{
    d->settingsView->resetToDefault();
    slotPreview();
}

void AntiVignettingTool::preparePreview()
{
    AntiVignettingContainer settings = d->settingsView->settings();

    ImageIface* const iface = d->previewWidget->imageIface();
    int previewWidth        = iface->previewSize().width();
    int previewHeight       = iface->previewSize().height();
    DImg imTemp             = iface->original()->smoothScale(previewWidth, previewHeight, Qt::KeepAspectRatio);

    setFilter(new AntiVignettingFilter(&imTemp, this, settings));
}

void AntiVignettingTool::prepareFinal()
{
    AntiVignettingContainer settings = d->settingsView->settings();

    ImageIface iface;
    setFilter(new AntiVignettingFilter(iface.original(), this, settings));
}

void AntiVignettingTool::setPreviewImage()
{
    ImageIface* const iface = d->previewWidget->imageIface();
    DImg preview            = filter()->getTargetImage().smoothScale(iface->previewSize());
    iface->setPreview(preview);
    d->previewWidget->updatePreview();
}

void AntiVignettingTool::setFinalImage()
{
    ImageIface* const iface = d->previewWidget->imageIface();
    DImg finalImage         = filter()->getTargetImage();

    iface->setOriginal(i18n("Vignetting Correction"), filter()->filterAction(), finalImage);
}

}  // namespace Digikam
