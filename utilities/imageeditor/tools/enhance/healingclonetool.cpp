/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-06-15
 * Description : a tool to replace part of the image using another
 *
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2017      by Shaza Ismail Kaoud <shaza dot ismail dot k at gmail dot com>
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

#include "healingclonetool.h"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QIcon>
#include <QPoint>

// KDE includes

#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "dexpanderbox.h"
#include "dnuminput.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageguidewidget.h"
#include "imagebrushguidewidget.h"

namespace Digikam
{

class HealingCloneTool::Private
{

public:

    Private() :
        radiusInput(0),
        blurPercent(0),
        previewWidget(0),
        gboxSettings(0),
        srcButton(0)
    {
    }

    static const QString configGroupName;
    static const QString configRadiusAdjustmentEntry;
    static const QString configBlurAdjustmentEntry;

    DIntNumInput*           radiusInput;
    DDoubleNumInput*        blurPercent;
    ImageBrushGuideWidget*  previewWidget;
    EditorToolSettings*     gboxSettings;
    QPoint                  sourcePoint;
    QPoint                  destinationStartPoint;
    QPushButton*            srcButton;
};

const QString HealingCloneTool::Private::configGroupName(QLatin1String("Healing Clone Tool"));
const QString HealingCloneTool::Private::configRadiusAdjustmentEntry(QLatin1String("RadiusAdjustment"));
const QString HealingCloneTool::Private::configBlurAdjustmentEntry(QLatin1String("BlurAdjustment"));
// --------------------------------------------------------

HealingCloneTool::HealingCloneTool(QObject* const parent)
    : EditorTool(parent),
      d(new Private)
{
    setObjectName(QLatin1String("healing clone"));
    setToolName(i18n("Healing Clone Tool"));
    setToolIcon(QIcon::fromTheme(QLatin1String("healimage")));
    setToolHelp(QLatin1String("healingclonetool.anchor"));

    d->gboxSettings      = new EditorToolSettings;
    d->previewWidget     = new ImageBrushGuideWidget(0, true, ImageGuideWidget::PickColorMode, Qt::red);

    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::PreviewTargetImage);

    // --------------------------------------------------------

    QLabel* const label  = new QLabel(i18n("Brush Radius:"));
    d->radiusInput       = new DIntNumInput();
    d->radiusInput->setRange(0, 50, 1);
    d->radiusInput->setDefaultValue(10);
    d->radiusInput->setWhatsThis(i18n("A radius of 0 has no effect, "
                                      "1 and above determine the brush radius "
                                      "that determines the size of parts copied in the image."));

    // --------------------------------------------------------

    QLabel* const label2 = new QLabel(i18n("Radial Blur Percent:"));
    d->blurPercent       = new DDoubleNumInput();
    d->blurPercent->setRange(0, 100, 0.1);
    d->blurPercent->setDefaultValue(0);
    d->blurPercent->setWhatsThis(i18n("A percent of 0 has no effect, values "
                                      "above 0 represent a factor for mixing "
                                      "the destination color with source color "
                                      "this is done radially i.e. the inner part of "
                                      "the brush radius is totally from source and mixing "
                                      "with destination is done gradually till the outer part "
                                      "of the circle."));

    // --------------------------------------------------------

    QLabel* const label3 = new QLabel(i18n("Source:"));
    d->srcButton         = new QPushButton(i18n("Set Source Point"), d->gboxSettings->plainPage());

    // --------------------------------------------------------

    const int spacing = d->gboxSettings->spacingHint();

    QGridLayout* const grid = new QGridLayout( );
    grid->addWidget(label3,         1, 0, 1, 2);
    grid->addWidget(d->srcButton,   2, 0, 1, 2);
    grid->addWidget(new DLineWidget(Qt::Horizontal, d->gboxSettings->plainPage()), 3, 0, 1, 2);
    grid->addWidget(label,          4, 0, 1, 2);
    grid->addWidget(d->radiusInput, 5, 0, 1, 2);
    grid->addWidget(label2,         6, 0, 1, 2);
    grid->addWidget(d->blurPercent, 7, 0, 1, 2);
    grid->setRowStretch(8, 8);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);
    d->gboxSettings->plainPage()->setLayout(grid);

    // --------------------------------------------------------

    setPreviewModeMask(PreviewToolBar::PreviewTargetImage);
    setToolSettings(d->gboxSettings);
    setToolView(d->previewWidget);

    // --------------------------------------------------------

    connect(d->radiusInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotRadiusChanged(int)));

    connect(d->srcButton, SIGNAL(clicked(bool)),
            d->previewWidget, SLOT(slotSetSourcePoint()));

    connect(d->previewWidget, SIGNAL(signalClone(QPoint,QPoint)),
            this, SLOT(slotReplace(QPoint,QPoint)));

    connect(d->previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotResized()));
}

HealingCloneTool::~HealingCloneTool()
{
    delete d;
}

void HealingCloneTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    d->radiusInput->setValue(group.readEntry(d->configRadiusAdjustmentEntry, d->radiusInput->defaultValue()));
    d->blurPercent->setValue(group.readEntry(d->configBlurAdjustmentEntry, d->blurPercent->defaultValue()));

}

void HealingCloneTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    group.writeEntry(d->configRadiusAdjustmentEntry, d->radiusInput->value());
    group.writeEntry(d->configBlurAdjustmentEntry, d->blurPercent->value());
    config->sync();
}

void HealingCloneTool::finalRendering()
{
    ImageIface iface;
    DImg dest = d->previewWidget->imageIface()->preview();
    FilterAction action(QLatin1String("digikam:healingCloneTool"), 1);
    iface.setOriginal(i18n("healingClone"), action, dest);
}

void HealingCloneTool::slotResetSettings()
{
    d->radiusInput->blockSignals(true);
    d->radiusInput->slotReset();
    d->radiusInput->blockSignals(false);
}

void HealingCloneTool::slotResized()
{
    toolView()->update();
}

void HealingCloneTool::slotReplace(const QPoint& srcPoint, const QPoint& dstPoint)
{
    ImageIface* const iface = d->previewWidget->imageIface();
    DImg* const current     = iface->previewReference();
    clone(current, srcPoint, dstPoint, d->radiusInput->value());
    d->previewWidget->updatePreview();
}

void HealingCloneTool::slotRadiusChanged(int r)
{
    d->previewWidget->setMaskPenSize(r);
}

void HealingCloneTool::clone(DImg* const img, const QPoint& srcPoint, const QPoint& dstPoint, int radius)
{
    double blurPercent = d->blurPercent->value() / 100;

    for (int i = -1 * radius ; i < radius ; i++)
    {
        for (int j = -1 * radius ; j < radius ; j++)
        {
            int rPercent = (i * i) + (j * j);

            if (rPercent < (radius * radius)) // Check for inside the circle
            {
                if (srcPoint.x()+i < 0 || srcPoint.x()+i >= (int)img->width()  ||
                    srcPoint.y()+j < 0 || srcPoint.y()+j >= (int)img->height() ||
                    dstPoint.x()+i < 0 || dstPoint.x()+i >= (int)img->width()  ||
                    dstPoint.y()+j < 0 || dstPoint.y()+j >= (int)img->height())
                {
                    continue;
                }

                double rP   = blurPercent * rPercent / (radius * radius);
                DColor cSrc = img->getPixelColor(srcPoint.x()+i, srcPoint.y()+j);
                DColor cDst = img->getPixelColor(dstPoint.x()+i, dstPoint.y()+j);
                cSrc.multiply(1 - rP);
                cDst.multiply(rP);
                cSrc.blendAdd(cDst);

                img->setPixelColor(dstPoint.x()+i, dstPoint.y()+j, cSrc);
            }
        }
    }
}

} // namespace Digikam
