/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-06
 * Description : Red eyes correction tool for image editor
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2004-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "redeyetool.moc"

// Qt includes

#include <QColor>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QToolButton>

// KDE includes

#include <kapplication.h>
#include <kcolordialog.h>
#include <kcolorvalueselector.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kcursor.h>
#include <kglobal.h>
#include <khuesaturationselect.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kvbox.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "colorgradientwidget.h"
#include "dimg.h"
#include "blurfilter.h"
#include "editortoolsettings.h"
#include "histogramwidget.h"
#include "histogrambox.h"
#include "imageiface.h"
#include "imageguidewidget.h"

using namespace KDcrawIface;

namespace DigikamEnhanceImagePlugin
{

class RedEyeTool::Private
{

public:

    Private() :
        thresholdLabel(0),
        smoothLabel(0),
        HSSelector(0),
        VSelector(0),
        tintLevel(0),
        redThreshold(0),
        smoothLevel(0),
        previewWidget(0),
        gboxSettings(0)
    {}

    static const QString    configGroupName;
    static const QString    configHistogramChannelEntry;
    static const QString    configHistogramScaleEntry;
    static const QString    configRedThresholdEntry;
    static const QString    configSmoothLevelEntry;
    static const QString    configHueColoringTintEntry;
    static const QString    configSatColoringTintEntry;
    static const QString    configValColoringTintEntry;
    static const QString    configTintLevelEntry;

    QColor                  selColor;

    QLabel*                 thresholdLabel;
    QLabel*                 smoothLabel;

    KHueSaturationSelector* HSSelector;
    KColorValueSelector*    VSelector;

    RIntNumInput*           tintLevel;
    RIntNumInput*           redThreshold;
    RIntNumInput*           smoothLevel;

    ImageGuideWidget*       previewWidget;
    EditorToolSettings*     gboxSettings;
};

const QString RedEyeTool::Private::configGroupName("redeye Tool");
const QString RedEyeTool::Private::configHistogramChannelEntry("Histogram Channel");
const QString RedEyeTool::Private::configHistogramScaleEntry("Histogram Scale");
const QString RedEyeTool::Private::configRedThresholdEntry("RedThreshold");
const QString RedEyeTool::Private::configSmoothLevelEntry("SmoothLevel");
const QString RedEyeTool::Private::configHueColoringTintEntry("HueColoringTint");
const QString RedEyeTool::Private::configSatColoringTintEntry("SatColoringTint");
const QString RedEyeTool::Private::configValColoringTintEntry("ValColoringTint");
const QString RedEyeTool::Private::configTintLevelEntry("TintLevel");

// --------------------------------------------------------

RedEyeTool::RedEyeTool(QObject* const parent)
    : EditorTool(parent),
      d(new Private)
{
    setObjectName("redeye");
    setToolName(i18n("Red Eye"));
    setToolIcon(SmallIcon("redeyes"));
    setToolHelp("redeyecorrectiontool.anchor");

    d->previewWidget = new ImageGuideWidget(0, true, ImageGuideWidget::PickColorMode, Qt::red, 1, false, ImageIface::ImageSelection);
    d->previewWidget->setToolTip(i18n("Here you can see the image selection preview with "
                                      "red eye reduction applied."));
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setTools(EditorToolSettings::Histogram);

    // -------------------------------------------------------------

    d->thresholdLabel = new QLabel(i18n("Sensitivity:"));
    d->redThreshold   = new RIntNumInput();
    d->redThreshold->setRange(10, 90, 1);
    d->redThreshold->setSliderEnabled(true);
    d->redThreshold->setDefaultValue(20);
    d->redThreshold->setWhatsThis(i18n("<p>Control the red pixel selection threshold.</p>"
                                       "<p>Low values will select more red pixels "
                                       "(aggressive correction), high values will select fewer (mild correction). "
                                       "Use a low value if an eye has been selected exactly. "
                                       "Use a high value if other parts of the face have been selected too.</p>"));

    d->smoothLabel = new QLabel(i18nc("Smoothness when blurring border of changed pixels", "Smooth:"));
    d->smoothLevel = new RIntNumInput();
    d->smoothLevel->setRange(0, 5, 1);
    d->smoothLevel->setSliderEnabled(true);
    d->smoothLevel->setDefaultValue(1);
    d->smoothLevel->setWhatsThis(i18n("Sets the smoothness value when blurring the border "
                                      "of the changed pixels. "
                                      "This leads to a more naturally looking pupil."));

    QLabel* label3 = new QLabel(i18n("Coloring Tint:"));

    d->HSSelector  = new KHueSaturationSelector();
    d->HSSelector->setWhatsThis(i18n("Sets a custom color when re-colorizing the eyes."));
    d->HSSelector->setMinimumSize(200, 142);
    d->HSSelector->setChooserMode(ChooserValue);
    d->HSSelector->setColorValue(255);

    d->VSelector   = new KColorValueSelector();
    d->VSelector->setChooserMode(ChooserValue);
    d->VSelector->setMinimumSize(26, 142);
    d->VSelector->setIndent(false);

    QLabel* label4 = new QLabel(i18n("Tint Level:"));
    d->tintLevel   = new RIntNumInput();
    d->tintLevel->setRange(1, 200, 1);
    d->tintLevel->setSliderEnabled(true);
    d->tintLevel->setDefaultValue(128);
    d->tintLevel->setWhatsThis(i18n("Set the tint level to adjust the luminosity of "
                                    "the new color of the pupil."));

    // -------------------------------------------------------------

    QGridLayout* mainLayout = new QGridLayout();
    mainLayout->addWidget(d->thresholdLabel, 0, 0, 1, 5);
    mainLayout->addWidget(d->redThreshold,   1, 0, 1, 5);
    mainLayout->addWidget(d->smoothLabel,    2, 0, 1, 5);
    mainLayout->addWidget(d->smoothLevel,    3, 0, 1, 5);
    mainLayout->addWidget(label3,            4, 0, 1, 5);
    mainLayout->addWidget(d->HSSelector,     5, 0, 1, 4);
    mainLayout->addWidget(d->VSelector,      5, 4, 1, 1);
    mainLayout->addWidget(label4,            6, 0, 1, 5);
    mainLayout->addWidget(d->tintLevel,      7, 0, 1, 5);
    mainLayout->setRowStretch(8, 10);
    mainLayout->setColumnStretch(3, 10);
    mainLayout->setMargin(d->gboxSettings->spacingHint());
    mainLayout->setSpacing(d->gboxSettings->spacingHint());
    d->gboxSettings->plainPage()->setLayout(mainLayout);

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);

    // -------------------------------------------------------------

    connect(d->previewWidget, SIGNAL(spotPositionChangedFromTarget(Digikam::DColor,QPoint)),
            this, SLOT(slotColorSelectedFromTarget(Digikam::DColor)));

    connect(d->redThreshold, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(d->smoothLevel, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(d->HSSelector, SIGNAL(valueChanged(int,int)),
            this, SLOT(slotHSChanged(int,int)));

    connect(d->VSelector, SIGNAL(valueChanged(int)),
            this, SLOT(slotVChanged(int)));

    connect(d->tintLevel, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));
}

RedEyeTool::~RedEyeTool()
{
    delete d;
}

void RedEyeTool::slotHSChanged(int h, int s)
{
    QColor color;

    int val = d->selColor.value();

    color.setHsv(h, s, val);
    setColor(color);
}

void RedEyeTool::slotVChanged(int v)
{
    QColor color;

    int hue = d->selColor.hue();
    int sat = d->selColor.saturation();

    color.setHsv(hue, sat, v);
    setColor(color);
}

void RedEyeTool::setColor(const QColor& color)
{
    if (color.isValid())
    {
        d->selColor = color;

        // set values
        d->HSSelector->setValues(d->selColor.hue(), d->selColor.saturation());
        d->VSelector->setValue(d->selColor.value());

        // set colors
        d->HSSelector->blockSignals(true);
        d->HSSelector->setHue(d->selColor.hue());
        d->HSSelector->setSaturation(d->selColor.saturation());
        d->HSSelector->setColorValue(d->selColor.value());
        d->HSSelector->updateContents();
        d->HSSelector->blockSignals(false);
        d->HSSelector->repaint();

        d->VSelector->blockSignals(true);
        d->VSelector->setHue(d->selColor.hue());
        d->VSelector->setSaturation(d->selColor.saturation());
        d->VSelector->setColorValue(d->selColor.value());
        d->VSelector->updateContents();
        d->VSelector->blockSignals(false);
        d->VSelector->repaint();

        slotTimer();
    }
}

void RedEyeTool::slotColorSelectedFromTarget(const DColor& color)
{
    d->gboxSettings->histogramBox()->histogram()->setHistogramGuideByColor(color);
}

void RedEyeTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->gboxSettings->histogramBox()->setChannel((ChannelType)group.readEntry(d->configHistogramChannelEntry,
            (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,
            (int)LogScaleHistogram));

    d->redThreshold->setValue(group.readEntry(d->configRedThresholdEntry,       d->redThreshold->defaultValue()));
    d->smoothLevel->setValue(group.readEntry(d->configSmoothLevelEntry,         d->smoothLevel->defaultValue()));
    d->HSSelector->setHue(group.readEntry(d->configHueColoringTintEntry,        0));
    d->HSSelector->setSaturation(group.readEntry(d->configSatColoringTintEntry, 128));
    d->VSelector->setValue(group.readEntry(d->configValColoringTintEntry,       255));
    d->tintLevel->setValue(group.readEntry(d->configTintLevelEntry,             d->tintLevel->defaultValue()));

    QColor col;
    col.setHsv(d->HSSelector->hue(),
               d->HSSelector->saturation(),
               d->VSelector->value());
    setColor(col);
}

void RedEyeTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    group.writeEntry(d->configHistogramChannelEntry, (int)d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,   (int)d->gboxSettings->histogramBox()->scale());
    group.writeEntry(d->configRedThresholdEntry,     d->redThreshold->value());
    group.writeEntry(d->configSmoothLevelEntry,      d->smoothLevel->value());
    group.writeEntry(d->configHueColoringTintEntry,  d->HSSelector->hue());
    group.writeEntry(d->configSatColoringTintEntry,  d->HSSelector->saturation());
    group.writeEntry(d->configValColoringTintEntry,  d->VSelector->value());
    group.writeEntry(d->configTintLevelEntry,        d->tintLevel->value());

    config->sync();
}

void RedEyeTool::slotResetSettings()
{
    d->redThreshold->blockSignals(true);
    d->HSSelector->blockSignals(true);
    d->VSelector->blockSignals(true);
    d->tintLevel->blockSignals(true);

    d->redThreshold->slotReset();
    d->smoothLevel->slotReset();
    d->tintLevel->slotReset();

    // Black color by default
    QColor col;
    col.setHsv(0, 0, 0);
    setColor(col);

    d->redThreshold->blockSignals(false);
    d->HSSelector->blockSignals(false);
    d->VSelector->blockSignals(false);
    d->tintLevel->blockSignals(false);

    slotPreview();
}

void RedEyeTool::slotPreview()
{
    kapp->setOverrideCursor( Qt::WaitCursor );

    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    // Here, we need to use the real selection image data because we will apply
    // a Gaussian blur filter on pixels and we cannot use directly the preview scaled image
    // else the blur radius will not give the same result between preview and final rendering.
    ImageIface* const iface = d->previewWidget->imageIface();
    DImg selection          = iface->selection();

    redEyeFilter(selection);

    DImg preview = selection.smoothScale(iface->previewSize());

    iface->setPreview(preview);
    d->previewWidget->updatePreview();

    // Update histogram.

    d->gboxSettings->histogramBox()->histogram()->updateData(selection.copy(), DImg(), false);

    kapp->restoreOverrideCursor();
}

void RedEyeTool::finalRendering()
{
    kapp->setOverrideCursor( Qt::WaitCursor );

    ImageIface* const iface = d->previewWidget->imageIface();
    DImg selection          = iface->selection();

    redEyeFilter(selection);

    FilterAction action("digikam:redEyeFilter", 1);
    action.setDisplayableName(i18n("Red Eye Filter"));

    iface->setSelection(i18n("Red Eyes Correction"), action, selection);

    kapp->restoreOverrideCursor();
}

void RedEyeTool::redEyeFilter(DImg& selection)
{
    // To restore selection alpha properties at end.
    bool selHasAlpha = selection.hasAlpha();

    DImg mask(selection.width(), selection.height(), selection.sixteenBit(), true,
              selection.bits(), true);

    selection          = mask.copy();
    float redThreshold = d->redThreshold->value()/10.0f;
    int hue            = d->VSelector->hue();
    int sat            = d->VSelector->saturation();
    int val            = d->VSelector->value();
    QColor coloring    = QColor::fromHsv(hue, sat, val);

    struct channel
    {
        float red_gain;
        float green_gain;
        float blue_gain;
    };

    channel red_chan, green_chan, blue_chan;

    red_chan.red_gain     = 0.1f;
    red_chan.green_gain   = 0.6f;
    red_chan.blue_gain    = 0.3f;

    green_chan.red_gain   = 0.0f;
    green_chan.green_gain = 1.0f;
    green_chan.blue_gain  = 0.0f;

    blue_chan.red_gain    = 0.0f;
    blue_chan.green_gain  = 0.0f;
    blue_chan.blue_gain   = 1.0f;

    float red_norm, green_norm, blue_norm;
    int  level = 201 - d->tintLevel->value();

    red_norm   = 1.0f / (red_chan.red_gain   + red_chan.green_gain   + red_chan.blue_gain);
    green_norm = 1.0f / (green_chan.red_gain + green_chan.green_gain + green_chan.blue_gain);
    blue_norm  = 1.0f / (blue_chan.red_gain  + blue_chan.green_gain  + blue_chan.blue_gain);

    red_norm   *= coloring.red()   / level;
    green_norm *= coloring.green() / level;
    blue_norm  *= coloring.blue()  / level;

    // Perform a red color pixels detection in selection image and create a correction mask using an alpha channel.

    if (!selection.sixteenBit())         // 8 bits image.
    {
        uchar* ptr  = selection.bits();
        uchar* mptr = mask.bits();
        uchar  r, g, b, r1, g1, b1;

        for (uint i = 0 ; i < selection.width() * selection.height() ; ++i)
        {
            b       = ptr[0];
            g       = ptr[1];
            r       = ptr[2];
            mptr[3] = 255;

            if (r >= ( redThreshold * g))
            {
                r1 = qMin(255, (int)(red_norm   * (red_chan.red_gain     * r +
                                                   red_chan.green_gain   * g +
                                                   red_chan.blue_gain    * b)));

                g1 = qMin(255, (int)(green_norm * (green_chan.red_gain   * r +
                                                   green_chan.green_gain * g +
                                                   green_chan.blue_gain  * b)));

                b1 = qMin(255, (int)(blue_norm  * (blue_chan.red_gain    * r +
                                                   blue_chan.green_gain  * g +
                                                   blue_chan.blue_gain   * b)));

                mptr[0] = b1;
                mptr[1] = g1;
                mptr[2] = r1;
                mptr[3] = qMin( (int)((r-g) / 150.0 * 255.0), 255);
            }

            ptr += 4;
            mptr+= 4;
        }
    }
    else                                 // 16 bits image.
    {
        unsigned short* ptr  = reinterpret_cast<unsigned short*>(selection.bits());
        unsigned short* mptr = reinterpret_cast<unsigned short*>(mask.bits());
        unsigned short  r, g, b, r1, g1, b1;

        for (uint i = 0 ; i < selection.width() * selection.height() ; ++i)
        {
            b       = ptr[0];
            g       = ptr[1];
            r       = ptr[2];
            mptr[3] = 65535;

            if (r >= ( redThreshold * g))
            {
                r1 = qMin(65535, (int)(red_norm   * (red_chan.red_gain     * r +
                                                     red_chan.green_gain   * g +
                                                     red_chan.blue_gain    * b)));

                g1 = qMin(65535, (int)(green_norm * (green_chan.red_gain   * r +
                                                     green_chan.green_gain * g +
                                                     green_chan.blue_gain  * b)));

                b1 = qMin(65535, (int)(blue_norm  * (blue_chan.red_gain    * r +
                                                     blue_chan.green_gain  * g +
                                                     blue_chan.blue_gain   * b)));

                mptr[0] = b1;
                mptr[1] = g1;
                mptr[2] = r1;
                mptr[3] = qMin( (int)((r-g) / 38400.0 * 65535.0), 65535);
            }

            ptr += 4;
            mptr+= 4;
        }
    }

    // Now, we will blur only the transparency pixels from the mask.

    DImg mask2 = mask.copy();
    BlurFilter blur(&mask2, 0L, d->smoothLevel->value());
    blur.startFilterDirectly();
    mask2.putImageData(blur.getTargetImage().bits());

    if (!selection.sixteenBit())         // 8 bits image.
    {
        uchar* mptr  = mask.bits();
        uchar* mptr2 = mask2.bits();

        for (uint i = 0 ; i < mask2.width() * mask2.height() ; ++i)
        {
            if (mptr2[3] < 255)
            {
                mptr[0] = mptr2[0];
                mptr[1] = mptr2[1];
                mptr[2] = mptr2[2];
                mptr[3] = mptr2[3];
            }

            mptr += 4;
            mptr2+= 4;
        }
    }
    else                                // 16 bits image.
    {
        unsigned short* mptr  = reinterpret_cast<unsigned short*>(mask.bits());
        unsigned short* mptr2 = reinterpret_cast<unsigned short*>(mask2.bits());

        for (uint i = 0 ; i < mask2.width() * mask2.height() ; ++i)
        {
            if (mptr2[3] < 65535)
            {
                mptr[0] = mptr2[0];
                mptr[1] = mptr2[1];
                mptr[2] = mptr2[2];
                mptr[3] = mptr2[3];
            }

            mptr += 4;
            mptr2+= 4;
        }
    }

    // - Perform pixels blending using alpha channel between the mask and the selection.

    DColorComposer* const composer = DColorComposer::getComposer(DColorComposer::PorterDuffSrcOver);

    // NOTE: 'mask' is the Source image, 'selection' is the Destination image.

    selection.bitBlendImage(composer, &mask,
                            0, 0, mask.width(), mask.height(),
                            0, 0);

    delete composer;

    if (!selHasAlpha)
        selection.removeAlphaChannel();
}

}  // namespace DigikamEnhanceImagePlugin
