/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-01
 * Description : image histogram adjust curves.
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


#include "adjustcurvestool.h"
#include "adjustcurvestool.moc"

// C++ includes

#include <cmath>

// Qt includes

#include <QButtonGroup>
#include <QColor>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>
#include <QToolButton>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <khelpmenu.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <kselector.h>
#include <kstandarddirs.h>

// Local includes

#include "colorgradientwidget.h"
#include "curveswidget.h"
#include "daboutdata.h"
#include "dimgimagefilters.h"
#include "editortoolsettings.h"
#include "histogramwidget.h"
#include "histogrambox.h"
#include "imagecurves.h"
#include "imagehistogram.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "version.h"

using namespace Digikam;

namespace DigikamAdjustCurvesImagesPlugin
{

AdjustCurvesTool::AdjustCurvesTool(QObject* parent)
                : EditorTool(parent)
{
    setObjectName("adjustcurves");
    setToolName(i18n("Adjust Curves"));
    setToolIcon(SmallIcon("adjustcurves"));

    m_destinationPreviewData = 0;

    ImageIface iface(0, 0);
    m_originalImage = iface.getOriginalImg();

    m_histoSegments = m_originalImage->sixteenBit() ? 65535 : 255;

    // -------------------------------------------------------------

    m_previewWidget = new ImageWidget("adjustcurves Tool", 0,
                                      i18n("This is the image's curve-adjustments preview. "
                                           "You can pick a spot on the image "
                                           "to see the corresponding level in the histogram."));
    setToolView(m_previewWidget);

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Load|
                                            EditorToolSettings::SaveAs|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel,
                                            EditorToolSettings::Histogram,
                                            HistogramBox::LRGBA);

    m_gboxSettings->histogramBox()->histogram()->setWhatsThis(i18n("Here you can see the target preview "
                                                   "image histogram drawing of the selected image "
                                                   "channel. This one is re-computed at any curves "
                                                   "settings changes."));

    // we don't need to use the Gradient widget in this tool
    m_gboxSettings->histogramBox()->setGradientVisible(false);

    QGridLayout* grid = new QGridLayout(m_gboxSettings->plainPage());

    // -------------------------------------------------------------

    QWidget *curveBox = new QWidget(m_gboxSettings->plainPage());
    QGridLayout* gl   = new QGridLayout(curveBox);

    m_vGradient = new ColorGradientWidget(Qt::Vertical, 10, curveBox);
    m_vGradient->setColors(QColor("white"), QColor("black"));

    m_curvesWidget = new CurvesWidget(256, 256, m_originalImage->bits(), m_originalImage->width(),
                                                m_originalImage->height(), m_originalImage->sixteenBit(),
                                                curveBox);
    m_curvesWidget->setWhatsThis(i18n("This is the curve drawing of the selected channel from "
                                      "original image"));

    QLabel *spaceh = new QLabel(curveBox);
    spaceh->setFixedHeight(1);

    m_hGradient = new ColorGradientWidget(Qt::Horizontal, 10, curveBox);
    m_hGradient->setColors(QColor("black"), QColor("white"));

    gl->addWidget(m_vGradient,          0, 0, 1, 1);
    gl->addWidget(m_curvesWidget,       0, 2, 1, 1);
    gl->addWidget(spaceh,               1, 2, 1, 1);
    gl->addWidget(m_hGradient,          2, 2, 1, 1);
    gl->setRowMinimumHeight(1, m_gboxSettings->spacingHint());
    gl->setMargin(0);
    gl->setSpacing(0);

    // -------------------------------------------------------------

    QWidget *typeBox   = new QWidget(m_gboxSettings->plainPage());
    QHBoxLayout *hlay2 = new QHBoxLayout(typeBox);
    m_curveType        = new QButtonGroup(typeBox);

    m_curveFree = new QToolButton(typeBox);
    m_curveFree->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/curvefree.png")));
    m_curveFree->setCheckable(true);
    m_curveFree->setToolTip(i18n("Curve free mode"));
    m_curveFree->setWhatsThis( i18n("With this button, you can draw your curve free-hand "
                                    "with the mouse."));
    m_curveType->addButton(m_curveFree, FreeDrawing);

    m_curveSmooth = new QToolButton(typeBox);
    m_curveSmooth->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/curvemooth.png")));
    m_curveSmooth->setCheckable(true);
    m_curveSmooth->setToolTip(i18n("Curve smooth mode"));
    m_curveSmooth->setWhatsThis( i18n("With this button, the curve type is constrained to "
                                      "be a smooth line with tension."));
    m_curveType->addButton(m_curveSmooth, SmoothDrawing);

    hlay2->setMargin(0);
    hlay2->setSpacing(0);
    hlay2->addWidget(m_curveFree);
    hlay2->addWidget(m_curveSmooth);

    m_curveType->setExclusive(true);
    m_curveSmooth->setChecked(true);

    // -------------------------------------------------------------

    m_pickerBox              = new QWidget(m_gboxSettings->plainPage());
    QHBoxLayout *hlay3       = new QHBoxLayout(m_pickerBox);
    m_pickerColorButtonGroup = new QButtonGroup(m_pickerBox);

    m_pickBlack = new QToolButton(m_pickerBox);
    m_pickerColorButtonGroup->addButton(m_pickBlack, BlackTonal);
    m_pickBlack->setIcon(KIcon("color-picker-black"));
    m_pickBlack->setCheckable(true);
    m_pickBlack->setToolTip(i18n("All channels shadow tone color picker"));
    m_pickBlack->setWhatsThis( i18n("With this button, you can pick the color from original "
                                    "image used to set <b>Shadow Tone</b> "
                                    "smooth curves point on Red, Green, Blue, and Luminosity channels."));

    m_pickGray = new QToolButton(m_pickerBox);
    m_pickerColorButtonGroup->addButton(m_pickGray, GrayTonal);
    m_pickGray->setIcon(KIcon("color-picker-grey"));
    m_pickGray->setCheckable(true);
    m_pickGray->setToolTip(i18n("All channels middle tone color picker"));
    m_pickGray->setWhatsThis( i18n("With this button, you can pick the color from original "
                                   "image used to set <b>Middle Tone</b> "
                                   "smooth curves point on Red, Green, Blue, and Luminosity channels."));

    m_pickWhite = new QToolButton(m_pickerBox);
    m_pickerColorButtonGroup->addButton(m_pickWhite, WhiteTonal);
    m_pickWhite->setIcon(KIcon("color-picker-white"));
    m_pickWhite->setCheckable(true);
    m_pickWhite->setToolTip( i18n( "All channels highlight tone color picker" ) );
    m_pickWhite->setWhatsThis( i18n("With this button, you can pick the color from original "
                                    "image used to set <b>Highlight Tone</b> "
                                    "smooth curves point on Red, Green, Blue, and Luminosity channels."));

    hlay3->setMargin(0);
    hlay3->setSpacing(0);
    hlay3->addWidget(m_pickBlack);
    hlay3->addWidget(m_pickGray);
    hlay3->addWidget(m_pickWhite);

    m_pickerColorButtonGroup->setExclusive(true);

    // -------------------------------------------------------------

    m_resetButton = new QPushButton(i18n("&Reset"), m_gboxSettings->plainPage());
    m_resetButton->setIcon(KIconLoader::global()->loadIcon("document-revert", KIconLoader::Toolbar));
    m_resetButton->setToolTip(i18n("Reset current channel curves' values."));
    m_resetButton->setWhatsThis(i18n("If you press this button, all curves' values "
                                     "from the currently selected channel "
                                     "will be reset to the default values."));

    QHBoxLayout* l3 = new QHBoxLayout();
    l3->addWidget(typeBox);
    l3->addWidget(m_pickerBox);
    l3->addWidget(m_resetButton);
    l3->addStretch(10);

    // -------------------------------------------------------------

    grid->addWidget(curveBox,       0, 0, 3, 6);
    grid->addLayout(l3,             3, 1, 1, 4);
    grid->setRowStretch(4, 10);
    grid->setMargin(m_gboxSettings->spacingHint());
    grid->setSpacing(m_gboxSettings->spacingHint());

    setToolSettings(m_gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(m_curvesWidget, SIGNAL(signalCurvesChanged()),
            this, SLOT(slotTimer()));

    connect(m_previewWidget, SIGNAL(spotPositionChangedFromOriginal( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotSpotColorChanged( const Digikam::DColor & )));

    connect(m_previewWidget, SIGNAL(spotPositionChangedFromTarget( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotColorSelectedFromTarget( const Digikam::DColor & )));

    connect(m_previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));

    // -------------------------------------------------------------
    // ComboBox slots.

    connect(m_curveType, SIGNAL(buttonClicked(int)),
            this, SLOT(slotCurveTypeChanged(int)));

    // -------------------------------------------------------------
    // Buttons slots.

    connect(m_resetButton, SIGNAL(clicked()),
            this, SLOT(slotResetCurrentChannel()));

    connect(m_pickerColorButtonGroup, SIGNAL(buttonReleased(int)),
            this, SLOT(slotPickerColorButtonActived()));
}

AdjustCurvesTool::~AdjustCurvesTool()
{
    delete [] m_destinationPreviewData;
}

void AdjustCurvesTool::slotPickerColorButtonActived()
{
    // Save previous rendering mode and toggle to original image.
    m_currentPreviewMode = m_previewWidget->getRenderingPreviewMode();
    m_previewWidget->setRenderingPreviewMode(ImageGuideWidget::PreviewOriginalImage);
}

void AdjustCurvesTool::slotSpotColorChanged(const DColor &color)
{
    DColor sc = color;

    if ( m_pickBlack->isChecked() )
    {
       // Black tonal curves point.
       m_curvesWidget->curves()->setCurvePoint(ImageHistogram::ValueChannel, 1,
                                 QPoint(qMax(qMax(sc.red(), sc.green()), sc.blue()), 42*m_histoSegments/256));
       m_curvesWidget->curves()->setCurvePoint(ImageHistogram::RedChannel, 1, QPoint(sc.red(), 42*m_histoSegments/256));
       m_curvesWidget->curves()->setCurvePoint(ImageHistogram::GreenChannel, 1, QPoint(sc.green(), 42*m_histoSegments/256));
       m_curvesWidget->curves()->setCurvePoint(ImageHistogram::BlueChannel, 1, QPoint(sc.blue(), 42*m_histoSegments/256));
       m_pickBlack->setChecked(false);
    }
    else if ( m_pickGray->isChecked() )
    {
       // Gray tonal curves point.
       m_curvesWidget->curves()->setCurvePoint(ImageHistogram::ValueChannel, 8,
                                 QPoint(qMax(qMax(sc.red(), sc.green()), sc.blue()), 128*m_histoSegments/256));
       m_curvesWidget->curves()->setCurvePoint(ImageHistogram::RedChannel, 8, QPoint(sc.red(), 128*m_histoSegments/256));
       m_curvesWidget->curves()->setCurvePoint(ImageHistogram::GreenChannel, 8, QPoint(sc.green(), 128*m_histoSegments/256));
       m_curvesWidget->curves()->setCurvePoint(ImageHistogram::BlueChannel, 8, QPoint(sc.blue(), 128*m_histoSegments/256));
       m_pickGray->setChecked(false);
    }
    else if ( m_pickWhite->isChecked() )
    {
       // White tonal curves point.
       m_curvesWidget->curves()->setCurvePoint(ImageHistogram::ValueChannel, 15,
                                 QPoint(qMax(qMax(sc.red(), sc.green()), sc.blue()), 213*m_histoSegments/256));
       m_curvesWidget->curves()->setCurvePoint(ImageHistogram::RedChannel, 15, QPoint(sc.red(), 213*m_histoSegments/256));
       m_curvesWidget->curves()->setCurvePoint(ImageHistogram::GreenChannel, 15, QPoint(sc.green(), 213*m_histoSegments/256));
       m_curvesWidget->curves()->setCurvePoint(ImageHistogram::BlueChannel, 15, QPoint(sc.blue(), 213*m_histoSegments/256));
       m_pickWhite->setChecked(false);
    }
    else
    {
       m_curvesWidget->setCurveGuide(color);
       return;
    }

    // Calculate Red, green, blue curves.

    for (int i = ImageHistogram::ValueChannel ; i <= ImageHistogram::BlueChannel ; i++)
       m_curvesWidget->curves()->curvesCalculateCurve(i);

    m_curvesWidget->repaint();

    // restore previous rendering mode.
    m_previewWidget->setRenderingPreviewMode(m_currentPreviewMode);

    slotEffect();
}

void AdjustCurvesTool::slotColorSelectedFromTarget( const DColor &color )
{
    m_gboxSettings->histogramBox()->histogram()->setHistogramGuideByColor(color);
}

void AdjustCurvesTool::slotResetCurrentChannel()
{
    m_curvesWidget->curves()->curvesChannelReset(m_gboxSettings->histogramBox()->channel());

    m_curvesWidget->repaint();
    slotEffect();
    m_gboxSettings->histogramBox()->histogram()->reset();
}

void AdjustCurvesTool::slotEffect()
{
    ImageIface* iface = m_previewWidget->imageIface();
    uchar *orgData    = iface->getPreviewImage();
    int w             = iface->previewWidth();
    int h             = iface->previewHeight();
    bool sb           = iface->previewSixteenBit();

    // Create the new empty destination image data space.
    m_gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    if (m_destinationPreviewData)
       delete [] m_destinationPreviewData;

    m_destinationPreviewData = new uchar[w*h*(sb ? 8 : 4)];

    // Calculate the LUT to apply on the image.
    m_curvesWidget->curves()->curvesLutSetup(ImageHistogram::AlphaChannel);

    // Apply the LUT to the image.
    m_curvesWidget->curves()->curvesLutProcess(orgData, m_destinationPreviewData, w, h);

    iface->putPreviewImage(m_destinationPreviewData);
    m_previewWidget->updatePreview();

    // Update histogram.
    m_gboxSettings->histogramBox()->histogram()->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);
    delete [] orgData;
}

void AdjustCurvesTool::finalRendering()
{
    kapp->setOverrideCursor( Qt::WaitCursor );
    ImageIface* iface = m_previewWidget->imageIface();
    uchar *orgData             = iface->getOriginalImage();
    int w                      = iface->originalWidth();
    int h                      = iface->originalHeight();
    bool sb                    = iface->originalSixteenBit();

    // Create the new empty destination image data space.
    uchar* desData = new uchar[w*h*(sb ? 8 : 4)];

    // Calculate the LUT to apply on the image.
    m_curvesWidget->curves()->curvesLutSetup(ImageHistogram::AlphaChannel);

    // Apply the LUT to the image.
    m_curvesWidget->curves()->curvesLutProcess(orgData, desData, w, h);

    iface->putOriginalImage(i18n("Adjust Curve"), desData);
    kapp->restoreOverrideCursor();

    delete [] orgData;
    delete [] desData;
}

void AdjustCurvesTool::slotChannelChanged()
{
    int channel = m_gboxSettings->histogramBox()->channel();
    switch (channel)
    {
        case EditorToolSettings::LuminosityChannel:
            m_curvesWidget->m_channelType = CurvesWidget::ValueHistogram;
            m_hGradient->setColors(QColor("white"), QColor("black"));
            m_vGradient->setColors(QColor("white"), QColor("black"));
            break;

        case EditorToolSettings::RedChannel:
            m_curvesWidget->m_channelType = CurvesWidget::RedChannelHistogram;
            m_hGradient->setColors(QColor("red"), QColor("black"));
            m_vGradient->setColors(QColor("red"), QColor("black"));
            break;

        case EditorToolSettings::GreenChannel:
            m_curvesWidget->m_channelType = CurvesWidget::GreenChannelHistogram;
            m_hGradient->setColors(QColor("green"), QColor("black"));
            m_vGradient->setColors(QColor("green"), QColor("black"));
            break;

        case EditorToolSettings::BlueChannel:
            m_curvesWidget->m_channelType = CurvesWidget::BlueChannelHistogram;
            m_hGradient->setColors(QColor("blue"), QColor("black"));
            m_vGradient->setColors(QColor("blue"), QColor("black"));
            break;

        case EditorToolSettings::AlphaChannel:
            m_curvesWidget->m_channelType = CurvesWidget::AlphaChannelHistogram;
            m_hGradient->setColors(QColor("white"), QColor("black"));
            m_vGradient->setColors(QColor("white"), QColor("black"));
            break;
    }

    m_curveType->button(m_curvesWidget->curves()->getCurveType(channel))->setChecked(true);
    m_curvesWidget->repaint();
    m_gboxSettings->histogramBox()->slotChannelChanged();
}

void AdjustCurvesTool::slotScaleChanged()
{
    m_curvesWidget->m_scaleType = m_gboxSettings->histogramBox()->scale();
    m_curvesWidget->repaint();
}

void AdjustCurvesTool::slotCurveTypeChanged(int type)
{
    switch(type)
    {
       case SmoothDrawing:
       {
          m_curvesWidget->curves()->setCurveType(m_curvesWidget->m_channelType, ImageCurves::CURVE_SMOOTH);
          m_pickerBox->setEnabled(true);
          break;
       }

       case FreeDrawing:
       {
          m_curvesWidget->curves()->setCurveType(m_curvesWidget->m_channelType, ImageCurves::CURVE_FREE);
          m_pickerBox->setEnabled(false);
          break;
       }
    }

    m_curvesWidget->curveTypeChanged();
}

void AdjustCurvesTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("adjustcurves Tool");

    m_curvesWidget->reset();

    for (int i = 0 ; i < 5 ; i++)
    {
        m_curvesWidget->curves()->curvesChannelReset(i);
        m_curvesWidget->curves()->setCurveType(i,
                  (ImageCurves::CurveType)group.readEntry(QString("CurveTypeChannel%1").arg(i),
                  (int)ImageCurves::CURVE_SMOOTH));

        for (int j = 0 ; j < 17 ; j++)
        {
            QPoint disable(-1, -1);
            QPoint p = group.readEntry(QString("CurveAdjustmentChannel%1Point%2").arg(i).arg(j), disable);

            if (m_originalImage->sixteenBit() && p.x() != -1)
            {
                p.setX(p.x()*255);
                p.setY(p.y()*255);
            }

            m_curvesWidget->curves()->setCurvePoint(i, j, p);
        }

        m_curvesWidget->curves()->curvesCalculateCurve(i);
    }

    // we need to call the set methods here, otherwise the curve will not be updated correctly
    m_gboxSettings->histogramBox()->setChannel(group.readEntry("Histogram Channel",
                    (int)EditorToolSettings::LuminosityChannel));
    m_gboxSettings->histogramBox()->setScale(group.readEntry("Histogram Scale",
                    (int)CurvesWidget::LogScaleHistogram));

    slotEffect();
}

void AdjustCurvesTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("adjustcurves Tool");
    group.writeEntry("Histogram Channel", m_gboxSettings->histogramBox()->channel());
    group.writeEntry("Histogram Scale", m_gboxSettings->histogramBox()->scale());

    for (int i = 0 ; i < 5 ; i++)
    {
        group.writeEntry(QString("CurveTypeChannel%1").arg(i), m_curvesWidget->curves()->getCurveType(i));

        for (int j = 0 ; j < 17 ; j++)
        {
            QPoint p = m_curvesWidget->curves()->getCurvePoint(i, j);

            if (m_originalImage->sixteenBit() && p.x() != -1)
            {
                p.setX(p.x()/255);
                p.setY(p.y()/255);
            }

            group.writeEntry(QString("CurveAdjustmentChannel%1Point%2").arg(i).arg(j), p);
        }
    }

    m_previewWidget->writeSettings();

    config->sync();
}

void AdjustCurvesTool::slotResetSettings()
{
    for (int channel = 0 ; channel < 5 ; channel++)
       m_curvesWidget->curves()->curvesChannelReset(channel);

    m_curvesWidget->reset();
    m_gboxSettings->histogramBox()->histogram()->reset();

    slotEffect();
}

void AdjustCurvesTool::slotLoadSettings()
{
    KUrl loadCurvesFile;

    loadCurvesFile = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                                             QString( "*" ), kapp->activeWindow(),
                                             QString( i18n("Select Gimp Curves File to Load")) );
    if( loadCurvesFile.isEmpty() )
       return;

    if ( m_curvesWidget->curves()->loadCurvesFromGimpCurvesFile( loadCurvesFile ) == false )
    {
       KMessageBox::error(kapp->activeWindow(),
                          i18n("Cannot load from the Gimp curves text file."));
       return;
    }

    // Refresh the current curves config.
    slotChannelChanged();
    slotEffect();
}

void AdjustCurvesTool::slotSaveAsSettings()
{
    KUrl saveCurvesFile;

    saveCurvesFile = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
                                             QString( "*" ), kapp->activeWindow(),
                                             QString( i18n("Gimp Curves File to Save")) );
    if( saveCurvesFile.isEmpty() )
       return;

    if ( m_curvesWidget->curves()->saveCurvesToGimpCurvesFile( saveCurvesFile ) == false )
    {
       KMessageBox::error(kapp->activeWindow(),
                          i18n("Cannot save to the Gimp curves text file."));
       return;
    }

    // Refresh the current curves config.
    slotChannelChanged();
}

}  // namespace DigikamAdjustCurvesImagesPlugin
