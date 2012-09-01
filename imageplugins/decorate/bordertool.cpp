/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-20
 * Description : a digiKam image plugin to add a border
 *               around an image.
 *
 * Copyright 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "bordertool.moc"

// Qt includes

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <knuminput.h>

// Local includes

#include "borderfilter.h"
#include "bordersettings.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageguidewidget.h"

namespace DigikamDecorateImagePlugin
{

class BorderTool::BorderToolPriv
{
public:

    BorderToolPriv() :
        configGroupName("border Tool"),

        gboxSettings(0),
        previewWidget(0),
        settingsView(0)
    {}

    const QString       configGroupName;

    EditorToolSettings* gboxSettings;
    ImageGuideWidget*   previewWidget;
    BorderSettings*     settingsView;
};

BorderTool::BorderTool(QObject* parent)
    : EditorToolThreaded(parent),
      d(new BorderToolPriv)
{
    setObjectName("border");
    setToolName(i18n("Add Border"));
    setToolIcon(SmallIcon("bordertool"));

    d->previewWidget = new ImageGuideWidget(0, false, ImageGuideWidget::HVGuideMode);
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::UnSplitPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->settingsView = new BorderSettings(d->gboxSettings->plainPage());
    setToolSettings(d->gboxSettings);
    init();

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
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->settingsView->readSettings(group);
}

void BorderTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->settingsView->writeSettings(group);

    group.sync();
}

void BorderTool::slotResetSettings()
{
    d->settingsView->resetToDefault();
}

void BorderTool::prepareEffect()
{
    ImageIface* iface        = d->previewWidget->imageIface();
    DImg preview             = iface->getPreview();
    int w                    = iface->previewWidth();
    float ratio              = (float)w/(float)iface->originalWidth();
    BorderContainer settings = d->settingsView->settings();
    settings.orgWidth        = iface->originalWidth();
    settings.orgHeight       = iface->originalHeight();
    settings.borderWidth1    = (int)((float)settings.borderWidth1*ratio);
    settings.borderWidth2    = (int)(20.0*ratio);
    settings.borderWidth3    = (int)(20.0*ratio);
    settings.borderWidth4    = 3;

    setFilter(new BorderFilter(&preview, this, settings));
}

void BorderTool::prepareFinal()
{
    ImageIface iface(0, 0);
    DImg* orgImage           = iface.getOriginal();
    BorderContainer settings = d->settingsView->settings();
    settings.orgWidth        = iface.originalWidth();
    settings.orgHeight       = iface.originalHeight();

    setFilter(new BorderFilter(orgImage, this, settings));
}

void BorderTool::putPreviewData()
{
    ImageIface* iface = d->previewWidget->imageIface();
    int w             = iface->previewWidth();
    int h             = iface->previewHeight();
    DImg imTemp       = filter()->getTargetImage().smoothScale(w, h, Qt::KeepAspectRatio);
    DImg imDest(w, h, filter()->getTargetImage().sixteenBit(), filter()->getTargetImage().hasAlpha());

    imDest.fill(DColor(d->previewWidget->palette().color(QPalette::Background).rgb(),
                       filter()->getTargetImage().sixteenBit()) );

    imDest.bitBltImage(&imTemp, (w-imTemp.width())/2, (h-imTemp.height())/2);

    iface->putPreview(imDest);
    d->previewWidget->updatePreview();
}

void BorderTool::putFinalData()
{
    ImageIface iface(0, 0);
    DImg targetImage = filter()->getTargetImage();
    iface.putOriginal(i18n("Add Border"), filter()->filterAction(), targetImage.bits(), targetImage.width(), targetImage.height());
}

}  // namespace DigikamDecorateImagePlugin
