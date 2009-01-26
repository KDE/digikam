/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-09
 * Description : a tool to sharp an image
 *
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#define MAX_MATRIX_SIZE 25

// C++ includes.

#include <cmath>

// Qt includes.

#include <qlayout.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qwidgetstack.h>

// KDE includes.

#include <kaboutdata.h>
#include <kcursor.h>
#include <klocale.h>
#include <kapplication.h>
#include <kseparator.h>
#include <kconfig.h>
#include <kurl.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <kmessagebox.h>

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>
#include <libkdcraw/rcombobox.h>

// Local includes.

#include "ddebug.h"
#include "imageiface.h"
#include "imagepanelwidget.h"
#include "editortoolsettings.h"
#include "dimgsharpen.h"
#include "unsharp.h"
#include "refocus.h"
#include "sharpentool.h"
#include "sharpentool.moc"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamImagesPluginCore
{

SharpenTool::SharpenTool(QObject* parent)
           : EditorToolThreaded(parent)
{
    setName("sharpen");
    setToolName(i18n("Sharpen"));
    setToolIcon(SmallIcon("sharpenimage"));
    setToolHelp("blursharpentool.anchor");

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel|
                                            EditorToolSettings::Load|
                                            EditorToolSettings::SaveAs|
                                            EditorToolSettings::Try,
                                            EditorToolSettings::PanIcon);
    QGridLayout* grid = new QGridLayout( m_gboxSettings->plainPage(), 3, 1);

    QLabel *label1 = new QLabel(i18n("Method:"), m_gboxSettings->plainPage());

    m_sharpMethod = new RComboBox(m_gboxSettings->plainPage());
    m_sharpMethod->insertItem( i18n("Simple sharp") );
    m_sharpMethod->insertItem( i18n("Unsharp mask") );
    m_sharpMethod->insertItem( i18n("Refocus") );
    m_sharpMethod->setDefaultItem(SimpleSharp);
    QWhatsThis::add( m_sharpMethod, i18n("<p>Select the sharpening method to apply to the image."));

    m_stack = new QWidgetStack(m_gboxSettings->plainPage());

    grid->addMultiCellWidget(label1,                                      0, 0, 0, 0);
    grid->addMultiCellWidget(m_sharpMethod,                               0, 0, 1, 1);
    grid->addMultiCellWidget(new KSeparator(m_gboxSettings->plainPage()), 1, 1, 0, 1);
    grid->addMultiCellWidget(m_stack,                                     2, 2, 0, 1);
    grid->setRowStretch(3, 10);
    grid->setMargin(m_gboxSettings->spacingHint());
    grid->setSpacing(m_gboxSettings->spacingHint());

    // -------------------------------------------------------------

    QWidget *simpleSharpSettings = new QWidget(m_stack);
    QGridLayout* grid1           = new QGridLayout( simpleSharpSettings, 2, 1);

    QLabel *label = new QLabel(i18n("Sharpness:"), simpleSharpSettings);
    m_radiusInput = new RIntNumInput(simpleSharpSettings);
    m_radiusInput->setRange(0, 100, 1);
    m_radiusInput->setDefaultValue(0);
    QWhatsThis::add( m_radiusInput, i18n("<p>A sharpness of 0 has no effect, "
                                         "1 and above determine the sharpen matrix radius "
                                         "that determines how much to sharpen the image."));

    grid1->addMultiCellWidget(label,         0, 0, 0, 1);
    grid1->addMultiCellWidget(m_radiusInput, 1, 1, 0, 1);
    grid1->setRowStretch(2, 10);
    grid1->setMargin(0);
    grid1->setSpacing(0);

    m_stack->addWidget(simpleSharpSettings, SimpleSharp);

    // -------------------------------------------------------------

    QWidget *unsharpMaskSettings = new QWidget(m_stack);
    QGridLayout* grid2           = new QGridLayout( unsharpMaskSettings, 6, 1);

    QLabel *label2 = new QLabel(i18n("Radius:"), unsharpMaskSettings);
    m_radiusInput2 = new RIntNumInput(unsharpMaskSettings);
    m_radiusInput2->setRange(1, 120, 1);
    m_radiusInput2->setDefaultValue(1);
    QWhatsThis::add( m_radiusInput2, i18n("<p>Radius value is the gaussian blur matrix radius value "
                                          "used to determines how much to blur the image.") );

    QLabel *label3 = new QLabel(i18n("Amount:"), unsharpMaskSettings);
    m_amountInput  = new RDoubleNumInput(unsharpMaskSettings);
    m_amountInput->setPrecision(1);
    m_amountInput->setRange(0.0, 5.0, 0.1);
    m_amountInput->setDefaultValue(1.0);
    QWhatsThis::add( m_amountInput, i18n("<p>The value of the difference between the "
                                         "original and the blur image that is added back into the original.") );

    QLabel *label4   = new QLabel(i18n("Threshold:"), unsharpMaskSettings);
    m_thresholdInput = new RDoubleNumInput(unsharpMaskSettings);
    m_thresholdInput->setPrecision(2);
    m_thresholdInput->setRange(0.0, 1.0, 0.01);
    m_thresholdInput->setDefaultValue(0.05);
    QWhatsThis::add( m_thresholdInput, i18n("<p>The threshold, as a fraction of the maximum "
                                            "luminosity value, needed to apply the difference amount.") );

    grid2->addMultiCellWidget(label2,           0, 0, 0, 1);
    grid2->addMultiCellWidget(m_radiusInput2,   1, 1, 0, 1);
    grid2->addMultiCellWidget(label3,           2, 2, 0, 1);
    grid2->addMultiCellWidget(m_amountInput,    3, 3, 0, 1);
    grid2->addMultiCellWidget(label4,           4, 4, 0, 1);
    grid2->addMultiCellWidget(m_thresholdInput, 5, 5, 0, 1);
    grid2->setRowStretch(6, 10);
    grid2->setMargin(0);
    grid2->setSpacing(0);

    m_stack->addWidget(unsharpMaskSettings, UnsharpMask);

    // -------------------------------------------------------------

    QWidget *refocusSettings = new QWidget(m_stack);
    QGridLayout* grid3       = new QGridLayout(refocusSettings, 10, 1);

    QLabel *label5 = new QLabel(i18n("Circular sharpness:"), refocusSettings);
    m_radius       = new RDoubleNumInput(refocusSettings);
    m_radius->setPrecision(2);
    m_radius->setRange(0.0, 5.0, 0.01);
    m_radius->setDefaultValue(1.0);
    QWhatsThis::add( m_radius, i18n("<p>This is the radius of the circular convolution. It is the most important "
                                    "parameter for using this plugin. For most images the default value of 1.0 "
                                    "should give good results. Select a higher value when your image is very blurred."));

    QLabel *label6 = new QLabel(i18n("Correlation:"), refocusSettings);
    m_correlation  = new RDoubleNumInput(refocusSettings);
    m_correlation->setPrecision(2);
    m_correlation->setRange(0.0, 1.0, 0.01);
    m_correlation->setDefaultValue(0.5);
    QWhatsThis::add( m_correlation, i18n("<p>Increasing the correlation may help to reduce artifacts. The correlation can "
                                         "range from 0-1. Useful values are 0.5 and values close to 1, e.g. 0.95 and 0.99. "
                                         "Using a high value for the correlation will reduce the sharpening effect of the "
                                         "plugin."));

    QLabel *label7 = new QLabel(i18n("Noise filter:"), refocusSettings);
    m_noise        = new RDoubleNumInput(refocusSettings);
    m_noise->setPrecision(3);
    m_noise->setRange(0.0, 1.0, 0.001);
    m_noise->setDefaultValue(0.03);
    QWhatsThis::add( m_noise, i18n("<p>Increasing the noise filter parameter may help to reduce artifacts. The noise filter "
                                   "can range from 0-1 but values higher than 0.1 are rarely helpful. When the noise filter "
                                   "value is too low, e.g. 0.0 the image quality will be very poor. A useful value is 0.01. "
                                   "Using a high value for the noise filter will reduce the sharpening "
                                   "effect of the plugin."));

    QLabel *label8 = new QLabel(i18n("Gaussian sharpness:"), refocusSettings);
    m_gauss        = new RDoubleNumInput(refocusSettings);
    m_gauss->setPrecision(2);
    m_gauss->setRange(0.0, 1.0, 0.01);
    m_gauss->setDefaultValue(0.0);
    QWhatsThis::add( m_gauss, i18n("<p>This is the sharpness for the gaussian convolution. Use this parameter when your "
                                   "blurring is of a Gaussian type. In most cases you should set this parameter to 0, because "
                                   "it causes nasty artifacts. When you use non-zero values, you will probably have to "
                                   "increase the correlation and/or noise filter parameters too."));

    QLabel *label9 = new QLabel(i18n("Matrix size:"), refocusSettings);
    m_matrixSize   = new RIntNumInput(refocusSettings);
    m_matrixSize->setRange(0, MAX_MATRIX_SIZE, 1);
    m_matrixSize->setDefaultValue(5);
    QWhatsThis::add( m_matrixSize, i18n("<p>This parameter determines the size of the transformation matrix. "
                                        "Increasing the matrix width may give better results, especially when you have "
                                        "chosen large values for circular or gaussian sharpness."));

    grid3->addMultiCellWidget(label5,        0, 0, 0, 1);
    grid3->addMultiCellWidget(m_radius,      1, 1, 0, 1);
    grid3->addMultiCellWidget(label6,        2, 2, 0, 1);
    grid3->addMultiCellWidget(m_correlation, 3, 3, 0, 1);
    grid3->addMultiCellWidget(label7,        4, 4, 0, 1);
    grid3->addMultiCellWidget(m_noise,       5, 5, 0, 1);
    grid3->addMultiCellWidget(label8,        6, 6, 0, 1);
    grid3->addMultiCellWidget(m_gauss,       7, 7, 0, 1);
    grid3->addMultiCellWidget(label9,        8, 8, 0, 1);
    grid3->addMultiCellWidget(m_matrixSize,  9, 9, 0, 1);
    grid3->setRowStretch(10, 10);
    grid3->setMargin(0);
    grid3->setSpacing(0);

    m_stack->addWidget(refocusSettings, Refocus);

    setToolSettings(m_gboxSettings);

    m_previewWidget = new ImagePanelWidget(470, 350, "sharpen Tool", m_gboxSettings->panIconView());

    setToolView(m_previewWidget);
    init();

    // -------------------------------------------------------------

    connect(m_sharpMethod, SIGNAL(activated(int)),
            this, SLOT(slotSharpMethodActived(int)));

    // -------------------------------------------------------------

    // Image creation with dummy borders (mosaic mode) used by Refocus method. It needs to do
    // it before to apply deconvolution filter on original image border pixels including
    // on matrix size area. This way limit artifacts on image border.

    ImageIface iface(0, 0);

    uchar* data = iface.getOriginalImage();
    int    w    = iface.originalWidth();
    int    h    = iface.originalHeight();
    bool   sb   = iface.originalSixteenBit();
    bool   a    = iface.originalHasAlpha();

    m_img = DImg( w + 4*MAX_MATRIX_SIZE, h + 4*MAX_MATRIX_SIZE, sb, a);

    DImg tmp;
    DImg org(w, h, sb, a, data);

    // Copy original.
    m_img.bitBltImage(&org, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);

    // Create dummy top border
    tmp = org.copy(0, 0, w, 2*MAX_MATRIX_SIZE);
    tmp.flip(DImg::VERTICAL);
    m_img.bitBltImage(&tmp, 2*MAX_MATRIX_SIZE, 0);

    // Create dummy bottom border
    tmp = org.copy(0, h-2*MAX_MATRIX_SIZE, w, 2*MAX_MATRIX_SIZE);
    tmp.flip(DImg::VERTICAL);
    m_img.bitBltImage(&tmp, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE+h);

    // Create dummy left border
    tmp = org.copy(0, 0, 2*MAX_MATRIX_SIZE, h);
    tmp.flip(DImg::HORIZONTAL);
    m_img.bitBltImage(&tmp, 0, 2*MAX_MATRIX_SIZE);

    // Create dummy right border
    tmp = org.copy(w-2*MAX_MATRIX_SIZE, 0, 2*MAX_MATRIX_SIZE, h);
    tmp.flip(DImg::HORIZONTAL);
    m_img.bitBltImage(&tmp, w+2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);

    // Create dummy top/left corner
    tmp = org.copy(0, 0, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);
    tmp.flip(DImg::HORIZONTAL);
    tmp.flip(DImg::VERTICAL);
    m_img.bitBltImage(&tmp, 0, 0);

    // Create dummy top/right corner
    tmp = org.copy(w-2*MAX_MATRIX_SIZE, 0, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);
    tmp.flip(DImg::HORIZONTAL);
    tmp.flip(DImg::VERTICAL);
    m_img.bitBltImage(&tmp, w+2*MAX_MATRIX_SIZE, 0);

    // Create dummy bottom/left corner
    tmp = org.copy(0, h-2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);
    tmp.flip(DImg::HORIZONTAL);
    tmp.flip(DImg::VERTICAL);
    m_img.bitBltImage(&tmp, 0, h+2*MAX_MATRIX_SIZE);

    // Create dummy bottom/right corner
    tmp = org.copy(w-2*MAX_MATRIX_SIZE, h-2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);
    tmp.flip(DImg::HORIZONTAL);
    tmp.flip(DImg::VERTICAL);
    m_img.bitBltImage(&tmp, w+2*MAX_MATRIX_SIZE, h+2*MAX_MATRIX_SIZE);

    delete [] data;
}

SharpenTool::~SharpenTool()
{
}

void SharpenTool::renderingFinished()
{
    switch (m_stack->id(m_stack->visibleWidget()))
    {
        case SimpleSharp:
        {
            m_radiusInput->setEnabled(true);
            m_gboxSettings->enableButton(EditorToolSettings::Load, false);
            m_gboxSettings->enableButton(EditorToolSettings::SaveAs, false);
            break;
        }

        case UnsharpMask:
        {
            m_radiusInput2->setEnabled(true);
            m_amountInput->setEnabled(true);
            m_thresholdInput->setEnabled(true);
            m_gboxSettings->enableButton(EditorToolSettings::Load, false);
            m_gboxSettings->enableButton(EditorToolSettings::SaveAs, false);
            break;
        }

        case Refocus:
        {
            m_matrixSize->setEnabled(true);
            m_radius->setEnabled(true);
            m_gauss->setEnabled(true);
            m_correlation->setEnabled(true);
            m_noise->setEnabled(true);
            break;
        }
    }
}

void SharpenTool::slotSharpMethodActived(int w)
{
    m_stack->raiseWidget(w);
    if (w == Refocus)
    {
        m_gboxSettings->enableButton(EditorToolSettings::Load, true);
        m_gboxSettings->enableButton(EditorToolSettings::SaveAs, true);
    }
    else
    {
        m_gboxSettings->enableButton(EditorToolSettings::Load, false);
        m_gboxSettings->enableButton(EditorToolSettings::SaveAs, false);
    }
}

void SharpenTool::readSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("sharpen Tool");
    m_radiusInput->blockSignals(true);
    m_radiusInput2->blockSignals(true);
    m_amountInput->blockSignals(true);
    m_thresholdInput->blockSignals(true);
    m_matrixSize->blockSignals(true);
    m_radius->blockSignals(true);
    m_gauss->blockSignals(true);
    m_correlation->blockSignals(true);
    m_noise->blockSignals(true);
    m_sharpMethod->blockSignals(true);

    m_radiusInput->setValue(config->readNumEntry("SimpleSharpRadiusAjustment", m_radiusInput->defaultValue()));
    m_radiusInput2->setValue(config->readNumEntry("UnsharpMaskRadiusAjustment", m_radiusInput2->defaultValue()));
    m_amountInput->setValue(config->readDoubleNumEntry("UnsharpMaskAmountAjustment", m_amountInput->defaultValue()));
    m_thresholdInput->setValue(config->readDoubleNumEntry("UnsharpMaskThresholdAjustment", m_thresholdInput->defaultValue()));
    m_matrixSize->setValue(config->readNumEntry("RefocusMatrixSize", m_matrixSize->defaultValue()));
    m_radius->setValue(config->readDoubleNumEntry("RefocusRadiusAjustment", m_radius->defaultValue()));
    m_gauss->setValue(config->readDoubleNumEntry("RefocusGaussAjustment", m_gauss->defaultValue()));
    m_correlation->setValue(config->readDoubleNumEntry("RefocusCorrelationAjustment", m_correlation->defaultValue()));
    m_noise->setValue(config->readDoubleNumEntry("RefocusNoiseAjustment", m_noise->defaultValue()));
    m_sharpMethod->setCurrentItem(config->readNumEntry("SharpenMethod", SimpleSharp));

    m_radiusInput->blockSignals(false);
    m_radiusInput2->blockSignals(false);
    m_amountInput->blockSignals(false);
    m_thresholdInput->blockSignals(false);
    m_matrixSize->blockSignals(false);
    m_radius->blockSignals(false);
    m_gauss->blockSignals(false);
    m_correlation->blockSignals(false);
    m_noise->blockSignals(false);
    m_sharpMethod->blockSignals(false);

    slotSharpMethodActived(m_sharpMethod->currentItem());
}

void SharpenTool::writeSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("sharpen Tool");
    config->writeEntry("SimpleSharpRadiusAjustment", m_radiusInput->value());
    config->writeEntry("UnsharpMaskRadiusAjustment", m_radiusInput2->value());
    config->writeEntry("UnsharpMaskAmountAjustment", m_amountInput->value());
    config->writeEntry("UnsharpMaskThresholdAjustment", m_thresholdInput->value());
    config->writeEntry("RefocusMatrixSize", m_matrixSize->value());
    config->writeEntry("RefocusRadiusAjustment", m_radius->value());
    config->writeEntry("RefocusGaussAjustment", m_gauss->value());
    config->writeEntry("RefocusCorrelationAjustment", m_correlation->value());
    config->writeEntry("RefocusNoiseAjustment", m_noise->value());
    config->writeEntry("SharpenMethod", m_sharpMethod->currentItem());
    m_previewWidget->writeSettings();
    config->sync();
}

void SharpenTool::slotResetSettings()
{
    switch (m_stack->id(m_stack->visibleWidget()))
    {
        case SimpleSharp:
        {
            m_radiusInput->blockSignals(true);
            m_radiusInput->slotReset();
            m_radiusInput->blockSignals(false);
            break;
        }

        case UnsharpMask:
        {
            m_radiusInput2->blockSignals(true);
            m_amountInput->blockSignals(true);
            m_thresholdInput->blockSignals(true);

            m_radiusInput2->slotReset();
            m_amountInput->slotReset();
            m_thresholdInput->slotReset();

            m_radiusInput2->blockSignals(false);
            m_amountInput->blockSignals(false);
            m_thresholdInput->blockSignals(false);
            break;
        }

        case Refocus:
        {
            m_matrixSize->blockSignals(true);
            m_radius->blockSignals(true);
            m_gauss->blockSignals(true);
            m_correlation->blockSignals(true);
            m_noise->blockSignals(true);

            m_matrixSize->slotReset();
            m_radius->slotReset();
            m_gauss->slotReset();
            m_correlation->slotReset();
            m_noise->slotReset();

            m_matrixSize->blockSignals(false);
            m_radius->blockSignals(false);
            m_gauss->blockSignals(false);
            m_correlation->blockSignals(false);
            m_noise->blockSignals(false);
            break;
        }
    }
}

void SharpenTool::prepareEffect()
{
    switch (m_stack->id(m_stack->visibleWidget()))
    {
        case SimpleSharp:
        {
            m_radiusInput->setEnabled(false);

            DImg img = m_previewWidget->getOriginalRegionImage();

            double radius = m_radiusInput->value()/10.0;
            double sigma;

            if (radius < 1.0) sigma = radius;
            else sigma = sqrt(radius);

            setFilter(dynamic_cast<DImgThreadedFilter*>(new DImgSharpen(&img, this, radius, sigma )));
            break;
        }

        case UnsharpMask:
        {
            m_radiusInput2->setEnabled(false);
            m_amountInput->setEnabled(false);
            m_thresholdInput->setEnabled(false);

            DImg img = m_previewWidget->getOriginalRegionImage();

            int    r  = m_radiusInput2->value();
            double a  = m_amountInput->value();
            double th = m_thresholdInput->value();

            setFilter(dynamic_cast<DImgThreadedFilter*>(new DigikamImagesPluginCore::UnsharpMask(&img, this, r, a, th)));
            break;
        }

        case Refocus:
        {
            m_matrixSize->setEnabled(false);
            m_radius->setEnabled(false);
            m_gauss->setEnabled(false);
            m_correlation->setEnabled(false);
            m_noise->setEnabled(false);

            int    ms     = m_matrixSize->value();
            double r      = m_radius->value();
            double g      = m_gauss->value();
            double c      = m_correlation->value();
            double n      = m_noise->value();

            QRect area    = m_previewWidget->getOriginalImageRegionToRender();
            QRect tmpRect;
            tmpRect.setLeft(area.left()-2*ms);
            tmpRect.setRight(area.right()+2*ms);
            tmpRect.setTop(area.top()-2*ms);
            tmpRect.setBottom(area.bottom()+2*ms);
            tmpRect.moveBy(2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE);
            DImg imTemp = m_img.copy(tmpRect);

            setFilter(dynamic_cast<DImgThreadedFilter*>(new DigikamImagesPluginCore::Refocus(&imTemp, this, ms, r, g, c, n)));
            break;
        }
    }
}

void SharpenTool::prepareFinal()
{
    switch (m_stack->id(m_stack->visibleWidget()))
    {
        case SimpleSharp:
        {
            m_radiusInput->setEnabled(false);

            double radius = m_radiusInput->value()/10.0;
            double sigma;

            if (radius < 1.0) sigma = radius;
            else sigma = sqrt(radius);

            ImageIface iface(0, 0);
            uchar *data     = iface.getOriginalImage();
            int w           = iface.originalWidth();
            int h           = iface.originalHeight();
            bool sixteenBit = iface.originalSixteenBit();
            bool hasAlpha   = iface.originalHasAlpha();
            DImg orgImage = DImg(w, h, sixteenBit, hasAlpha ,data);
            delete [] data;
            setFilter(dynamic_cast<DImgThreadedFilter*>(new DImgSharpen(&orgImage, this, radius, sigma )));
            break;
        }

        case UnsharpMask:
        {
            m_radiusInput2->setEnabled(false);
            m_amountInput->setEnabled(false);
            m_thresholdInput->setEnabled(false);

            int    r  = m_radiusInput2->value();
            double a  = m_amountInput->value();
            double th = m_thresholdInput->value();

            ImageIface iface(0, 0);
            uchar *data     = iface.getOriginalImage();
            int w           = iface.originalWidth();
            int h           = iface.originalHeight();
            bool sixteenBit = iface.originalSixteenBit();
            bool hasAlpha   = iface.originalHasAlpha();
            DImg orgImage = DImg(w, h, sixteenBit, hasAlpha ,data);
            delete [] data;
            setFilter(dynamic_cast<DImgThreadedFilter*>(new DigikamImagesPluginCore::UnsharpMask(&orgImage, this, r, a, th)));
            break;
        }

        case Refocus:
        {

            m_matrixSize->setEnabled(false);
            m_radius->setEnabled(false);
            m_gauss->setEnabled(false);
            m_correlation->setEnabled(false);
            m_noise->setEnabled(false);

            int    ms   = m_matrixSize->value();
            double r    = m_radius->value();
            double g    = m_gauss->value();
            double c    = m_correlation->value();
            double n    = m_noise->value();

            setFilter(dynamic_cast<DImgThreadedFilter*>(new DigikamImagesPluginCore::Refocus(&m_img, this, ms, r, g, c, n)));
            break;
        }
    }
}

void SharpenTool::putPreviewData()
{
    switch (m_stack->id(m_stack->visibleWidget()))
    {
        case SimpleSharp:
        case UnsharpMask:
        {
            DImg imDest = filter()->getTargetImage();
            m_previewWidget->setPreviewImage(imDest);
            break;
        }

        case Refocus:
        {
            int   ms   = m_matrixSize->value();
            QRect area = m_previewWidget->getOriginalImageRegionToRender();

            DImg imDest = filter()->getTargetImage()
                          .copy(2*ms, 2*ms, area.width(), area.height());
            m_previewWidget->setPreviewImage(imDest);
            break;
        }
    }
}

void SharpenTool::putFinalData()
{
    ImageIface iface(0, 0);
    DImg imDest = filter()->getTargetImage();

    switch (m_stack->id(m_stack->visibleWidget()))
    {
        case SimpleSharp:
        {
            iface.putOriginalImage(i18n("Sharpen"), imDest.bits());
            break;
        }

        case UnsharpMask:
        {
            iface.putOriginalImage(i18n("Unsharp Mask"), imDest.bits());
            break;
        }

        case Refocus:
        {
            QRect area = m_previewWidget->getOriginalImageRegionToRender();
            ImageIface iface(0, 0);

            iface.putOriginalImage(i18n("Refocus"), filter()->getTargetImage()
                                .copy(2*MAX_MATRIX_SIZE, 2*MAX_MATRIX_SIZE,
                                        iface.originalWidth(),
                                        iface.originalHeight())
                                .bits());
            break;
        }
    }
}

void SharpenTool::slotLoadSettings()
{
    KURL loadRestorationFile = KFileDialog::getOpenURL(KGlobalSettings::documentPath(),
                               QString( "*" ), kapp->activeWindow(),
                               QString( i18n("Photograph Refocus Settings File to Load")) );
    if ( loadRestorationFile.isEmpty() )
        return;

    QFile file(loadRestorationFile.path());

    if ( file.open(IO_ReadOnly) )
    {
        QTextStream stream( &file );
        if ( stream.readLine() != "# Photograph Refocus Configuration File" )
        {
            KMessageBox::error(kapp->activeWindow(),
                               i18n("\"%1\" is not a Photograph Refocus settings text file.")
                               .arg(loadRestorationFile.fileName()));
            file.close();
            return;
        }

        blockSignals(true);
        m_matrixSize->setValue( stream.readLine().toInt() );
        m_radius->setValue( stream.readLine().toDouble() );
        m_gauss->setValue( stream.readLine().toDouble() );
        m_correlation->setValue( stream.readLine().toDouble() );
        m_noise->setValue( stream.readLine().toDouble() );
        blockSignals(false);
    }
    else
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot load settings from the Photograph Refocus text file."));

    file.close();
}

void SharpenTool::slotSaveAsSettings()
{
    KURL saveRestorationFile = KFileDialog::getSaveURL(KGlobalSettings::documentPath(),
                               QString( "*" ), kapp->activeWindow(),
                               QString( i18n("Photograph Refocus Settings File to Save")) );
    if ( saveRestorationFile.isEmpty() )
        return;

    QFile file(saveRestorationFile.path());

    if ( file.open(IO_WriteOnly) )
    {
        QTextStream stream( &file );
        stream << "# Photograph Refocus Configuration File\n";
        stream << m_matrixSize->value() << "\n";
        stream << m_radius->value() << "\n";
        stream << m_gauss->value() << "\n";
        stream << m_correlation->value() << "\n";
        stream << m_noise->value() << "\n";
    }
    else
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot save settings to the Photograph Refocus text file."));

    file.close();
}

}  // NameSpace DigikamImagesPluginCore
