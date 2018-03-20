/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a tool to fix automatically camera lens aberrations
 *
 * Copyright (C) 2008      by Adrian Schroeter <adrian at suse dot de>
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "lensautofixtool.h"

// Qt includes

#include <QBitmap>
#include <QBrush>
#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QTimer>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "dmetadata.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageguidewidget.h"
#include "lensfuniface.h"
#include "lensfunfilter.h"
#include "lensfunsettings.h"
#include "lensfuncameraselector.h"
#include "dexpanderbox.h"

namespace Digikam
{

class LensAutoFixTool::Private
{
public:

    Private() :
        configGroupName(QLatin1String("Lens Auto-Correction Tool")),
        configShowGrid(QLatin1String("Show Grid")),
        maskPreviewLabel(0),
        showGrid(0),
        settingsView(0),
        cameraSelector(0),
        previewWidget(0),
        gboxSettings(0)
    {
    }

    const QString          configGroupName;
    const QString          configShowGrid;

    QLabel*                maskPreviewLabel;

    QCheckBox*             showGrid;

    LensFunSettings*       settingsView;
    LensFunCameraSelector* cameraSelector;

    ImageGuideWidget*      previewWidget;
    EditorToolSettings*    gboxSettings;
};

LensAutoFixTool::LensAutoFixTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d(new Private)
{
    setObjectName(QLatin1String("lensautocorrection"));
    setToolName(i18n("Lens Auto-Correction"));
    setToolIcon(QIcon::fromTheme(QLatin1String("lensautofix")));

    d->previewWidget = new ImageGuideWidget(0, true, ImageGuideWidget::HVGuideMode);
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings         = new EditorToolSettings;
    QGridLayout* const grid = new QGridLayout(d->gboxSettings->plainPage());

    d->showGrid             = new QCheckBox(i18n("Show grid"), d->gboxSettings->plainPage());
    d->showGrid->setWhatsThis(i18n("Set this option to visualize the correction grid to be applied."));

    // -------------------------------------------------------------

    d->cameraSelector      = new LensFunCameraSelector(d->gboxSettings->plainPage());
    DImg* const img        = d->previewWidget->imageIface()->original();
    DMetadata meta(img->getMetadata());
    d->cameraSelector->setMetadata(meta);
    DLineWidget* const line = new DLineWidget(Qt::Horizontal, d->gboxSettings->plainPage());

    // -------------------------------------------------------------

    d->settingsView   = new LensFunSettings(d->gboxSettings->plainPage());

    // -------------------------------------------------------------

    const int spacing = d->gboxSettings->spacingHint();

    grid->addWidget(d->showGrid,       0, 0, 1, 2);
    grid->addWidget(d->cameraSelector, 1, 0, 1, 2);
    grid->addWidget(line,              2, 0, 1, 2);
    grid->addWidget(d->settingsView,   3, 0, 1, 2);
    grid->setRowStretch(4, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    setToolSettings(d->gboxSettings);

    // -------------------------------------------------------------

    connect(d->settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotTimer()));

    connect(d->cameraSelector, SIGNAL(signalLensSettingsChanged()),
            this, SLOT(slotLensChanged()));

    connect(d->showGrid, SIGNAL(toggled(bool)),
            this, SLOT(slotTimer()));
}

LensAutoFixTool::~LensAutoFixTool()
{
    delete d;
}

void LensAutoFixTool::slotLensChanged()
{
    d->settingsView->setEnabledCCA(d->cameraSelector->iface()->supportsCCA());
    d->settingsView->setEnabledVig(d->cameraSelector->iface()->supportsVig());
    d->settingsView->setEnabledDist(d->cameraSelector->iface()->supportsDistortion());
    d->settingsView->setEnabledGeom(d->cameraSelector->iface()->supportsDistortion());
    slotTimer();
}

void LensAutoFixTool::readSettings()
{
    d->gboxSettings->blockSignals(true);
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->showGrid->setChecked(group.readEntry(d->configShowGrid, false));
    d->cameraSelector->readSettings(group);
    d->settingsView->readSettings(group);
    d->gboxSettings->blockSignals(false);
    slotTimer();
}

void LensAutoFixTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configShowGrid, d->showGrid->isChecked());
    d->cameraSelector->writeSettings(group);
    d->settingsView->writeSettings(group);
    group.sync();
}

void LensAutoFixTool::slotResetSettings()
{
    d->gboxSettings->blockSignals(true);
    d->showGrid->setChecked(false);
    d->cameraSelector->resetToDefault();
    d->gboxSettings->blockSignals(false);
}

void LensAutoFixTool::preparePreview()
{
    // Settings information must be get before to disable settings view.
    LensFunContainer settings = d->cameraSelector->settings();
    d->settingsView->assignFilterSettings(settings);
    ImageIface* const iface   = d->previewWidget->imageIface();
    DImg preview              = iface->preview();

    if (d->showGrid->isChecked())
    {
        QBitmap pattern(9, 9);
        pattern.clear();
        QPainter p1(&pattern);
        p1.setPen(QPen(Qt::black, 1));
        p1.drawLine(5, 0, 5, 9);
        p1.drawLine(0, 5, 9, 5);
        p1.end();

        QPixmap pix(preview.size());
        pix.fill(Qt::transparent);
        QPainter p2(&pix);
        p2.setPen(QPen(Qt::gray, 1));
        p2.fillRect(0, 0, pix.width(), pix.height(), QBrush(pattern));
        p2.end();
        DImg grid(pix.toImage());

        DColorComposer* const composer            = DColorComposer::getComposer(DColorComposer::PorterDuffNone);
        DColorComposer::MultiplicationFlags flags = DColorComposer::NoMultiplication;

        // Do alpha blending of template on dest image
        preview.bitBlendImage(composer, &grid, 0, 0, preview.width(), preview.height(), 0, 0, flags);
    }

    setFilter(new LensFunFilter(&preview, this, settings));
}

void LensAutoFixTool::prepareFinal()
{
    // Settings information must be handle before to disable settings view.
    LensFunContainer settings = d->cameraSelector->settings();
    d->settingsView->assignFilterSettings(settings);
    ImageIface iface;
    setFilter(new LensFunFilter(iface.original(), this, settings));
}

void LensAutoFixTool::setPreviewImage()
{
    d->previewWidget->imageIface()->setPreview(filter()->getTargetImage());
    d->previewWidget->updatePreview();
}

void LensAutoFixTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Lens Auto-Correction"), filter()->filterAction(), filter()->getTargetImage());

    MetaEngineData data  = iface.originalMetadata();

    // Note: there are certain situations in the editor where these metadata changes may be undone.
    // Ignore for now, as they are not critical.
    LensFunFilter* const fltr = dynamic_cast<LensFunFilter*>(filter());

    if (fltr)
        fltr->registerSettingsToXmp(data);

    iface.setOriginalMetadata(data);
}

}  // namespace Digikam
