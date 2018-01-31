/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-20
 * Description : a digiKam image tool to add a border
 *               around an image.
 *
 * Copyright 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "bordertool.h"

// Qt includes

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "borderfilter.h"
#include "bordersettings.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageguidewidget.h"

namespace Digikam
{

class BorderTool::Private
{
public:

    Private() :
        configGroupName(QLatin1String("border Tool")),

        gboxSettings(0),
        previewWidget(0),
        settingsView(0)
    {
    }

    const QString       configGroupName;

    EditorToolSettings* gboxSettings;
    ImageGuideWidget*   previewWidget;
    BorderSettings*     settingsView;
};

BorderTool::BorderTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d(new Private)
{
    setObjectName(QLatin1String("border"));
    setToolName(i18n("Add Border"));
    setToolIcon(QIcon::fromTheme(QLatin1String("bordertool")));

    d->previewWidget = new ImageGuideWidget(0, false, ImageGuideWidget::HVGuideMode);
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::UnSplitPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->settingsView = new BorderSettings(d->gboxSettings->plainPage());
    setToolSettings(d->gboxSettings);

    // -------------------------------------------------------------

    connect(d->settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotTimer()));
}

BorderTool::~BorderTool()
{
    delete d;
}

void BorderTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->settingsView->readSettings(group);
}

void BorderTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->settingsView->writeSettings(group);

    group.sync();
}

void BorderTool::slotResetSettings()
{
    d->settingsView->resetToDefault();
}

void BorderTool::preparePreview()
{
    ImageIface* iface        = d->previewWidget->imageIface();
    DImg preview             = iface->preview();
    int w                    = iface->previewSize().width();
    float ratio              = (float)w/(float)iface->originalSize().width();
    BorderContainer settings = d->settingsView->settings();
    settings.orgWidth        = iface->originalSize().width();
    settings.orgHeight       = iface->originalSize().height();
    settings.borderWidth1    = (int)((float)settings.borderWidth1*ratio);
    settings.borderWidth2    = (int)(20.0*ratio);
    settings.borderWidth3    = (int)(20.0*ratio);
    settings.borderWidth4    = 3;

    setFilter(new BorderFilter(&preview, this, settings));
}

void BorderTool::prepareFinal()
{
    ImageIface iface;
    DImg* orgImage           = iface.original();
    BorderContainer settings = d->settingsView->settings();
    settings.orgWidth        = iface.originalSize().width();
    settings.orgHeight       = iface.originalSize().height();

    setFilter(new BorderFilter(orgImage, this, settings));
}

void BorderTool::setPreviewImage()
{
    ImageIface* iface = d->previewWidget->imageIface();
    int w             = iface->previewSize().width();
    int h             = iface->previewSize().height();
    DImg imTemp       = filter()->getTargetImage().smoothScale(w, h, Qt::KeepAspectRatio);
    DImg imDest(w, h, filter()->getTargetImage().sixteenBit(), filter()->getTargetImage().hasAlpha());

    imDest.fill(DColor(d->previewWidget->palette().color(QPalette::Background).rgb(),
                       filter()->getTargetImage().sixteenBit()) );

    imDest.bitBltImage(&imTemp, (w-imTemp.width())/2, (h-imTemp.height())/2);

    iface->setPreview(imDest);
    d->previewWidget->updatePreview();
}

void BorderTool::setFinalImage()
{
    ImageIface iface;
    DImg targetImage = filter()->getTargetImage();
    iface.setOriginal(i18n("Add Border"), filter()->filterAction(), targetImage);
}

}  // namespace Digikam
