/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-01
 * Description : Content aware resizing tool.
 *
 * Copyright (C) 2009 by Julien Pontabry <julien dot pontabry at ulp dot u-strasbg dot fr>
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

#include "contentawareresizetool.h"
#include "contentawareresizetool.moc"

// C++ includes.

#include <cstdio>

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>
#include <QToolButton>
#include <QButtonGroup>

// KDE includes
#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kseparator.h>
#include <klocale.h>
#include <kstandarddirs.h>

// LibKDraw includes

#include <libkdcraw/rnuminput.h>
#include <libkdcraw/rcombobox.h>

// Local includes

#include "version.h"
#include "daboutdata.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "dimgimagefilters.h"
#include "contentawareresizer.h"

using namespace Digikam;
using namespace KDcrawIface;

namespace DigikamContentAwareResizingImagesPlugin
{

class ContentAwareResizeToolPriv
{
public:

    enum LQRFunc
    {
        Norme=0,
        SumAbs,
        Abs
    };

    enum LQRResizeOrder
    {
        Horizontally=0,
        Vertically
    };

    enum MaskTool
    {
        redMask=0,
        greenMask
    };

public:

    ContentAwareResizeToolPriv()
    {
        previewWidget    = 0;
        gboxSettings     = 0;
        preserveRatioBox = 0;
        prevW            = 0;
        prevH            = 0;
        orgWidth         = 0;
        orgHeight        = 0;
        wInput           = 0;
        hInput           = 0;
        wpInput          = 0;
        hpInput          = 0;
        stepInput        = 0;
        rigidityInput    = 0;
    }

    int                 orgWidth;
    int                 orgHeight;
    int                 prevW;
    int                 prevH;

    double              prevWP;
    double              prevHP;

    QCheckBox          *preserveRatioBox;
    QCheckBox          *weightMaskBox;

    ImageWidget        *previewWidget;

    EditorToolSettings *gboxSettings;

    RIntNumInput       *wInput;
    RIntNumInput       *hInput;
    RIntNumInput       *stepInput;

    RDoubleNumInput    *wpInput;
    RDoubleNumInput    *hpInput;
    RDoubleNumInput    *mixedRescaleInput;
    RDoubleNumInput    *rigidityInput;

    RComboBox          *funcInput;
    RComboBox          *resizeOrderInput;

    QToolButton        *redMaskTool;
    QToolButton        *greenMaskTool;

    QButtonGroup       *maskGroup;
};

ContentAwareResizeTool::ContentAwareResizeTool(QObject *parent) 
                      : EditorToolThreaded(parent), d(new ContentAwareResizeToolPriv)
{
    setObjectName("liquidrescale");
    setToolName(i18n("Liquid Rescale"));
    setToolIcon(SmallIcon("transform-scale"));

    d->previewWidget = new ImageWidget("liquidrescale Tool", 0, QString(),
                                       false, ImageGuideWidget::HVGuideMode, false);

    setToolView(d->previewWidget);

    // -------------------------------------------------------------

    d->gboxSettings   = new EditorToolSettings(EditorToolSettings::Default|
                                               EditorToolSettings::Try|
                                               EditorToolSettings::Ok|
                                               EditorToolSettings::Cancel);

    QGridLayout* grid = new QGridLayout(d->gboxSettings->plainPage());

    // Initialize data
    ImageIface iface(0, 0);
    d->orgWidth  = iface.originalWidth();
    d->orgHeight = iface.originalHeight();
    d->prevW     = d->orgWidth;
    d->prevH     = d->orgHeight;
    d->prevWP    = 100.0;
    d->prevHP    = 100.0;

    // -------------------------------------------------------------

    d->preserveRatioBox = new QCheckBox(i18n("Maintain aspect ratio"), d->gboxSettings->plainPage());
    d->preserveRatioBox->setWhatsThis(i18n("Enable this option to maintain aspect ratio with new image sizes."));
    d->preserveRatioBox->setChecked(true);

    QLabel *labelWidth = new QLabel(i18n("Width (px):"), d->gboxSettings->plainPage());
    d->wInput          = new RIntNumInput(d->gboxSettings->plainPage());
    d->wInput->setRange(1, 2*d->orgWidth, 1);
    d->wInput->setDefaultValue(d->orgWidth);
    d->wInput->setSliderEnabled(true);
    d->wInput->setObjectName("wInput");
    d->wInput->setWhatsThis(i18n("Set here the new image width in pixels."));

    QLabel *labelHeight = new QLabel(i18n("Height (px):"), d->gboxSettings->plainPage());
    d->hInput           = new RIntNumInput(d->gboxSettings->plainPage());
    d->hInput->setRange(1, 2*d->orgHeight, 1);
    d->hInput->setDefaultValue(d->orgHeight);
    d->hInput->setSliderEnabled(true);
    d->hInput->setObjectName("hInput");
    d->hInput->setWhatsThis(i18n("Set here the new image height in pixels."));

    QLabel *labelWidthP = new QLabel(i18n("Width (%):"), d->gboxSettings->plainPage());
    d->wpInput          = new RDoubleNumInput(d->gboxSettings->plainPage());
    d->wpInput->input()->setRange(1.0, 200.0, 1.0, true);
    d->wpInput->setDefaultValue(100.0);
    d->wpInput->setObjectName("wpInput");
    d->wpInput->setWhatsThis(i18n("New image width in percent (%)."));

    QLabel *labelHeightP = new QLabel(i18n("Height (%):"), d->gboxSettings->plainPage());
    d->hpInput           = new RDoubleNumInput(d->gboxSettings->plainPage());
    d->hpInput->input()->setRange(1.0, 200.0, 1.0, true);
    d->hpInput->setDefaultValue(100.0);
    d->hpInput->setObjectName("hpInput");
    d->hpInput->setWhatsThis(i18n("New image height in percent (%)."));

    // -------------------------------------------------------------

    KSeparator *line3  = new KSeparator(Qt::Horizontal, d->gboxSettings->plainPage());

    QLabel *labelMixedPercent = new QLabel(i18n("Percent of content aware rescale:"), d->gboxSettings->plainPage());
    d->mixedRescaleInput      = new RDoubleNumInput(d->gboxSettings->plainPage());
    d->mixedRescaleInput->input()->setRange(0.0, 100.0, 1.0, true);
    d->mixedRescaleInput->setDefaultValue(100.0);
    d->mixedRescaleInput->setObjectName("mixedRescaleInput");
    d->mixedRescaleInput->setWhatsThis(i18n("Set there how many percent of content aware rescale you want."));
    d->mixedRescaleInput->setEnabled(true);

    // -------------------------------------------------------------

    KSeparator *line4  = new KSeparator(Qt::Horizontal, d->gboxSettings->plainPage());

    d->weightMaskBox = new QCheckBox(i18n("Add weight masks"), d->gboxSettings->plainPage());
    d->weightMaskBox->setWhatsThis(i18n("Enable this option to add suppression and preservation masks."));
    d->weightMaskBox->setChecked(false);
    d->maskGroup     = new QButtonGroup(d->gboxSettings->plainPage());
    d->maskGroup->setExclusive(true);

    QLabel *labeRedMaskTool = new QLabel(i18n("Suppression weight mask:"), d->gboxSettings->plainPage());
    d->redMaskTool          = new QToolButton(d->gboxSettings->plainPage());
    d->redMaskTool->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/indicator-red.png")));
    d->redMaskTool->setCheckable(true);
    d->redMaskTool->setChecked(true);
    d->redMaskTool->setToolTip(i18n("Draw a suppression mask"));
    d->redMaskTool->setWhatsThis(i18n("Click on this button to draw image's zones you want to suppress."));
    d->redMaskTool->setEnabled(false);
    d->maskGroup->addButton(d->redMaskTool, ContentAwareResizeToolPriv::redMask);

    QLabel *labeGreenMaskTool = new QLabel(i18n("Preservation weight mask:"), d->gboxSettings->plainPage());
    d->greenMaskTool          = new QToolButton(d->gboxSettings->plainPage());
    d->greenMaskTool->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/indicator-green.png")));
    d->greenMaskTool->setCheckable(true);
    d->greenMaskTool->setToolTip(i18n("Draw a preservation mask"));
    d->greenMaskTool->setWhatsThis(i18n("Click on this button to draw image's zones you want to preserve."));
    d->greenMaskTool->setEnabled(false);
    d->maskGroup->addButton(d->greenMaskTool, ContentAwareResizeToolPriv::greenMask);

    // -------------------------------------------------------------

    KSeparator *line      = new KSeparator(Qt::Horizontal, d->gboxSettings->plainPage());

    QLabel *labelFunction = new QLabel(i18n("Energy function:"), d->gboxSettings->plainPage());
    d->funcInput          = new RComboBox(d->gboxSettings->plainPage());
    d->funcInput->addItem(i18n("Gradient norm"));
    d->funcInput->addItem(i18n("Sum of absolute values"));
    d->funcInput->addItem(i18n("Absolute value"));
    d->funcInput->setDefaultIndex(ContentAwareResizeToolPriv::Abs);
    d->funcInput->setWhatsThis(i18n("This option allows you to choose a gradient function. This function is used "
                                    "to determine which pixels should be removed."));

    // -------------------------------------------------------------

    KSeparator *line2        = new KSeparator(Qt::Horizontal, d->gboxSettings->plainPage());

    QLabel *labelAdvSettings = new QLabel(i18n("Advanced Settings:"), d->gboxSettings->plainPage());

    QLabel *labelRigidity = new QLabel(i18n("Overall rigidity:"), d->gboxSettings->plainPage());
    d->rigidityInput      = new RDoubleNumInput(d->gboxSettings->plainPage());
    d->rigidityInput->input()->setRange(0.0, 10.0, 1.0, true);
    d->rigidityInput->setDefaultValue(0.0);
    d->rigidityInput->setWhatsThis(i18n("Use this value to give a negative bias to the seams which "
                                        "are not straight. May be useful to prevent distortions in "
                                        "some situations, or to avoid artifacts from pixel skipping "
                                        "(it is better to use low values in such case). This setting "
                                        "applies to the whole selected layer if no rigidity mask is used. "
                                        "Note: the bias is proportional to the difference in the transversal "
                                        "coordinate between each two successive points, elevated to the power "
                                        "of 1.5, and summed up for the whole seam."));

    QLabel *labelStep = new QLabel(i18n("Steps:"), d->gboxSettings->plainPage());
    d->stepInput      = new RIntNumInput(d->gboxSettings->plainPage());
    d->stepInput->setRange(1, 5, 1);
    d->stepInput->setDefaultValue(1);
    d->stepInput->setSliderEnabled(true);
    d->stepInput->setWhatsThis(i18n("This option lets you choose the maximum transversal step "
                                    "that the pixels in the seams can take. In the standard "
                                    "algorithm, corresponding to the default value step = 1, "
                                    "each pixel in a seam can be shifted by at most one pixel "
                                    "with respect to its neighbors. This implies that the seams "
                                    "can form an angle of at most 45 degrees with respect to their "
                                    "base line. Increasing the step value lets you overcome this "
                                    "limit, but may lead to the introduction of artifacts. In order "
                                    "to balance the situation, you can use the rigidity setting."));

    QLabel *labelResizeOrder = new QLabel(i18n("Resize Order:"), d->gboxSettings->plainPage());
    d->resizeOrderInput      = new RComboBox(d->gboxSettings->plainPage());
    d->resizeOrderInput->addItem(i18n("Horizontally first"));
    d->resizeOrderInput->addItem(i18n("Vertically first"));
    d->resizeOrderInput->setDefaultIndex(ContentAwareResizeToolPriv::Horizontally);
    d->resizeOrderInput->setWhatsThis(i18n("You can set there if you want to resize horizonally first or "
                                           "vertically first."));

    // -------------------------------------------------------------

    grid->addWidget(d->preserveRatioBox,  0, 0, 1, 3);
    grid->addWidget(labelWidth,           1, 0, 1, 1);
    grid->addWidget(d->wInput,            1, 1, 1, 4);
    grid->addWidget(labelHeight,          2, 0, 1, 1);
    grid->addWidget(d->hInput,            2, 1, 1, 4);
    grid->addWidget(labelWidthP,          3, 0, 1, 1);
    grid->addWidget(d->wpInput,           3, 1, 1, 4);
    grid->addWidget(labelHeightP,         4, 0, 1, 1);
    grid->addWidget(d->hpInput,           4, 1, 1, 4);
    grid->addWidget(line3,                5, 0, 1, -1);
    grid->addWidget(labelMixedPercent,    6, 0, 1, 3);
    grid->addWidget(d->mixedRescaleInput, 7, 0, 1, -1);
    grid->addWidget(line4,                8, 0, 1, -1);
    grid->addWidget(d->weightMaskBox,     9, 0, 1, -1);
    grid->addWidget(labeRedMaskTool,      10, 0, 1, 3);
    grid->addWidget(d->redMaskTool,       10, 2, 1, 1);
    grid->addWidget(labeGreenMaskTool,    11, 0, 1, 3);
    grid->addWidget(d->greenMaskTool,     11, 2, 1, 1);
    grid->addWidget(line,                 12, 0, 1, -1);
    grid->addWidget(labelFunction,        13, 0, 1, 1);
    grid->addWidget(d->funcInput,         13, 1, 1, 4);
    grid->addWidget(line2,                14, 0, 1, -1);
    grid->addWidget(labelAdvSettings,     15, 0, 1, 3);
    grid->addWidget(labelRigidity,        16, 0, 1, 1);
    grid->addWidget(d->rigidityInput,     16, 1, 1, 4);
    grid->addWidget(labelStep,            17, 0, 1, 1);
    grid->addWidget(d->stepInput,         17, 1, 1, 4);
    grid->addWidget(labelResizeOrder,     18, 0, 1, 1);
    grid->addWidget(d->resizeOrderInput,  18, 1, 1, 4);
    grid->setRowStretch(19, 10);
    grid->setMargin(d->gboxSettings->spacingHint());
    grid->setSpacing(d->gboxSettings->spacingHint());

    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(d->wInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotValuesChanged()));

    connect(d->hInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotValuesChanged()));

    connect(d->wpInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotValuesChanged()));

    connect(d->hpInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotValuesChanged()));

    connect(d->mixedRescaleInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotMixedRescaleValueChanged()));

    connect(d->weightMaskBox, SIGNAL(stateChanged(int)),
            this, SLOT(slotWeightMaskBoxStateChanged(int)));

    connect(d->maskGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(slotMaskColorChanged(int)));
}

ContentAwareResizeTool::~ContentAwareResizeTool()
{
    delete d;
}

void ContentAwareResizeTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("liquidrescale Tool");

    blockWidgetSignals(true);

    // NOTE: size settings are not restored here because they depands of image size.
    d->stepInput->setValue(group.readEntry("Step",                      d->stepInput->defaultValue()));
    d->rigidityInput->setValue(group.readEntry("Rigidity",              d->rigidityInput->defaultValue()));
    d->funcInput->setCurrentIndex(group.readEntry("Function",           d->funcInput->defaultIndex()));
    d->resizeOrderInput->setCurrentIndex(group.readEntry("Order",       d->resizeOrderInput->defaultIndex()));
    d->mixedRescaleInput->setValue(group.readEntry("MixedRescaleValue", d->mixedRescaleInput->defaultValue()));

    blockWidgetSignals(false);
}

void ContentAwareResizeTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("liquidrescale Tool");

    // NOTE: size settings are not saved here because they depands of image size.
    group.writeEntry("Step",              d->stepInput->value());
    group.writeEntry("Rigidity",          d->rigidityInput->value());
    group.writeEntry("Function",          d->funcInput->currentIndex());
    group.writeEntry("Order",             d->resizeOrderInput->currentIndex());
    group.writeEntry("MixedRescaleValue", d->mixedRescaleInput->value());

    d->previewWidget->writeSettings();
    group.sync();
}

void ContentAwareResizeTool::slotResetSettings()
{
    blockWidgetSignals(true);

    d->preserveRatioBox->setChecked(true);
    d->wInput->slotReset();
    d->hInput->slotReset();
    d->wpInput->slotReset();
    d->hpInput->slotReset();
    d->mixedRescaleInput->slotReset();

    blockWidgetSignals(false);
}

void ContentAwareResizeTool::slotValuesChanged()
{
    blockWidgetSignals(true);

    QString s(sender()->objectName());

    if(s == "wInput")
    {
        double val  = d->wInput->value();
        double pval = val / (double)(d->orgWidth) * 100.0;

        d->wpInput->setValue(pval);

        if(d->preserveRatioBox->isChecked())
        {
            int h = (int)(pval * d->orgHeight / 100);

            d->hpInput->setValue(pval);
            d->hInput->setValue(h);
        }
    }
    else if(s == "hInput")
    {
        double val  = d->hInput->value();
        double pval = val / (double)(d->orgHeight) * 100.0;

        d->hpInput->setValue(pval);

        if(d->preserveRatioBox->isChecked())
        {
            int w = (int)(pval * d->orgWidth / 100);

            d->wpInput->setValue(pval);
            d->wInput->setValue(w);
        }
    }
    else if(s == "wpInput")
    {
        double val = d->wpInput->value();
        int w      = (int)(val * d->orgWidth / 100);

        d->wInput->setValue(w);

        if(d->preserveRatioBox->isChecked())
        {
            int h = (int)(val * d->orgHeight / 100);

            d->hpInput->setValue(val);
            d->hInput->setValue(h);
        }
    }
    else if(s == "hpInput")
    {
        double val = d->hpInput->value();
        int h = (int)(val * d->orgHeight / 100);

        d->hInput->setValue(h);

        if(d->preserveRatioBox->isChecked())
        {
            int w = (int)(val * d->orgWidth / 100);

            d->wpInput->setValue(val);
            d->wInput->setValue(w);
        }
    }

    d->prevW  = d->wInput->value();
    d->prevH  = d->hInput->value();
    d->prevWP = d->wpInput->value();
    d->prevHP = d->hpInput->value();

    blockWidgetSignals(false);
}

void ContentAwareResizeTool::enableContentAwareSettings(bool b)
{
    d->stepInput->setEnabled(b);
    d->rigidityInput->setEnabled(b);
    d->funcInput->setEnabled(b);
    d->resizeOrderInput->setEnabled(b);
    d->weightMaskBox->setEnabled(b);
    d->redMaskTool->setEnabled(b);
    d->greenMaskTool->setEnabled(b);
}

void ContentAwareResizeTool::slotMixedRescaleValueChanged()
{
    blockWidgetSignals(true);
    enableContentAwareSettings(d->mixedRescaleInput->value()>0.0);
    blockWidgetSignals(false);
}

void ContentAwareResizeTool::disableSettings()
{
    d->preserveRatioBox->setEnabled(false);
    d->wInput->setEnabled(false);
    d->hInput->setEnabled(false);
    d->wpInput->setEnabled(false);
    d->hpInput->setEnabled(false);
    d->mixedRescaleInput->setEnabled(false);
    enableContentAwareSettings(false);
}

void ContentAwareResizeTool::contentAwareResizeCore(DImg *image, int target_width, int target_height, QImage mask)
{
    setFilter(dynamic_cast<DImgThreadedFilter*>(
              new ContentAwareResizer(image, target_width, target_height,
                                      d->stepInput->value(), d->rigidityInput->value(),
                                      (LqrGradFuncType)d->funcInput->currentIndex(),
                                      (LqrResizeOrder)d->resizeOrderInput->currentIndex(),
                                      mask, this)));
}

void ContentAwareResizeTool::prepareEffect()
{
    if (d->prevW  != d->wInput->value()  || d->prevH  != d->hInput->value() ||
        d->prevWP != d->wpInput->value() || d->prevHP != d->hpInput->value())
        slotValuesChanged();

    disableSettings();

    ImageIface* iface = d->previewWidget->imageIface();
    int w             = iface->previewWidth();
    int h             = iface->previewHeight();
    DImg imTemp       = iface->getOriginalImg()->smoothScale(w, h, Qt::ScaleMin);
    int new_w         = w*d->wpInput->value()/100.0;
    int new_h         = h*d->hpInput->value()/100.0;

    if(d->mixedRescaleInput->value()<100.0) // mixed rescale
    {
        double stdRescaleP = (100.0 - d->mixedRescaleInput->value()) / 100.0;
        int diff_w         = stdRescaleP * (w - new_w);
        int diff_h         = stdRescaleP * (h - new_h);

        imTemp.resize(imTemp.width() - diff_w, imTemp.height() - diff_h);
    }

    QImage mask;
    if(d->weightMaskBox->isChecked())
        mask = d->previewWidget->getMask();

    contentAwareResizeCore( &imTemp, new_w, new_h, mask );
}

void ContentAwareResizeTool::prepareFinal()
{
    if (d->prevW  != d->wInput->value()  || d->prevH  != d->hInput->value() ||
        d->prevWP != d->wpInput->value() || d->prevHP != d->hpInput->value())
        slotValuesChanged();

    disableSettings();

    ImageIface iface(0, 0);
    QImage mask;

    if(d->mixedRescaleInput->value() < 100.0) // mixed rescale
    {
        double stdRescaleP = (100.0 - d->mixedRescaleInput->value()) / 100.0;
        int diff_w         = stdRescaleP * (iface.originalWidth() - d->wInput->value());
        int diff_h         = stdRescaleP * (iface.originalHeight() - d->hInput->value());
        DImg image         = iface.getOriginalImg()->smoothScale(iface.originalWidth() - diff_w,
                                                                 iface.originalHeight() - diff_h,
                                                                 Qt::IgnoreAspectRatio);

        if(d->weightMaskBox->isChecked())
        {
            mask = d->previewWidget->getMask().scaled(iface.originalWidth()  - diff_w,
                                                      iface.originalHeight() - diff_h);
        }
        contentAwareResizeCore( &image, d->wInput->value(), d->hInput->value(), mask);
    }
    else
    {
        if(d->weightMaskBox->isChecked())
            mask = d->previewWidget->getMask().scaled(iface.originalWidth(), iface.originalHeight());

        contentAwareResizeCore( iface.getOriginalImg(), d->wInput->value(), d->hInput->value(), mask);

    }
}

void ContentAwareResizeTool::putPreviewData()
{
    ImageIface* iface = d->previewWidget->imageIface();
    int w             = iface->previewWidth();
    int h             = iface->previewHeight();
    DImg imTemp       = filter()->getTargetImage().smoothScale(w, h, Qt::ScaleMin);
    DImg imDest(w, h, filter()->getTargetImage().sixteenBit(),
                filter()->getTargetImage().hasAlpha());

    QColor background = toolView()->backgroundRole();
    imDest.fill(DColor(background, filter()->getTargetImage().sixteenBit()));
    imDest.bitBltImage(&imTemp, (w-imTemp.width())/2, (h-imTemp.height())/2);

    iface->putPreviewImage((imDest.smoothScale(iface->previewWidth(), iface->previewHeight())).bits());
    d->previewWidget->updatePreview();
}

void ContentAwareResizeTool::renderingFinished()
{
    d->preserveRatioBox->setEnabled(true);
    d->wInput->setEnabled(true);
    d->hInput->setEnabled(true);
    d->wpInput->setEnabled(true);
    d->hpInput->setEnabled(true);
    d->stepInput->setEnabled(true);
    d->rigidityInput->setEnabled(true);
    d->funcInput->setEnabled(true);
    d->resizeOrderInput->setEnabled(true);
    d->mixedRescaleInput->setEnabled(true);
    d->weightMaskBox->setEnabled(true);
    d->redMaskTool->setEnabled(true);
    d->greenMaskTool->setEnabled(true);
}

void ContentAwareResizeTool::putFinalData()
{
    ImageIface iface(0, 0);
    DImg targetImage = filter()->getTargetImage();
    iface.putOriginalImage(i18n("Liquid Rescale"),
                           targetImage.bits(),
                           targetImage.width(), targetImage.height());

}

void ContentAwareResizeTool::blockWidgetSignals(bool b)
{
    d->preserveRatioBox->blockSignals(b);
    d->wInput->blockSignals(b);
    d->hInput->blockSignals(b);
    d->wpInput->blockSignals(b);
    d->hpInput->blockSignals(b);
    d->mixedRescaleInput->blockSignals(b);
    d->weightMaskBox->blockSignals(b);
    d->redMaskTool->blockSignals(b);
    d->greenMaskTool->blockSignals(b);
}

void ContentAwareResizeTool::slotMaskColorChanged(int type)
{
    if (type == ContentAwareResizeToolPriv::redMask)
        d->previewWidget->setPaintColor(QColor(255, 0, 0, 255));
    else // green mask
        d->previewWidget->setPaintColor(QColor(0, 255, 0, 255));
}

void ContentAwareResizeTool::slotWeightMaskBoxStateChanged(int state)
{
    if (state == Qt::Unchecked)
    {
        d->redMaskTool->setEnabled(false);
        d->greenMaskTool->setEnabled(false);
        d->previewWidget->setMaskEnabled(false);
    }
    else    // Checked
    {
        d->redMaskTool->setEnabled(true);
        d->greenMaskTool->setEnabled(true);
        d->previewWidget->setMaskEnabled(true);

        if (d->redMaskTool->isChecked())
            d->previewWidget->setPaintColor(QColor(255, 0, 0, 255));
        else
            d->previewWidget->setPaintColor(QColor(0, 255, 0, 255));
    }
}

} // namespace DigikamContentAwareResizingImagesPlugin
