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

// C++ includes.

#include <cmath>

// Qt includes.

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

// KDE includes.

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

// Local includes.

#include "colorgradientwidget.h"
#include "curveswidget.h"
#include "daboutdata.h"
#include "dimgimagefilters.h"
#include "editortoolsettings.h"
#include "histogramwidget.h"
#include "imagecurves.h"
#include "imagehistogram.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "version.h"
#include "adjustcurvestool.h"
#include "adjustcurvestool.moc"

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
                                      i18n("<p>This is the image's curve-adjustments preview. "
                                           "You can pick a spot on the image "
                                           "to see the corresponding level in the histogram."));
    setToolView(m_previewWidget);

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Load|
                                            EditorToolSettings::SaveAs|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel);

    QGridLayout* grid = new QGridLayout(m_gboxSettings->plainPage());

    QLabel *label1 = new QLabel(i18n("Channel:"), m_gboxSettings->plainPage());
    label1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_channelCB = new KComboBox(m_gboxSettings->plainPage());
    m_channelCB->addItem(i18n("Luminosity"));
    m_channelCB->addItem(i18n("Red"));
    m_channelCB->addItem(i18n("Green"));
    m_channelCB->addItem(i18n("Blue"));
    m_channelCB->addItem(i18n("Alpha"));
    m_channelCB->setCurrentIndex(0);
    m_channelCB->setWhatsThis( i18n("<p>Select the histogram channel to display here:<p>"
                                    "<b>Luminosity</b>: display the image's luminosity values.<p>"
                                    "<b>Red</b>: display the red image-channel values.<p>"
                                    "<b>Green</b>: display the green image-channel values.<p>"
                                    "<b>Blue</b>: display the blue image-channel values.<p>"
                                    "<b>Alpha</b>: display the alpha image-channel values. "
                                    "This channel corresponds to the transparency value and "
                                    "is supported by some image formats, such as PNG or TIF."));

    // -------------------------------------------------------------

    QWidget *scaleBox = new QWidget(m_gboxSettings->plainPage());
    QHBoxLayout *hlay = new QHBoxLayout(scaleBox);
    m_scaleBG         = new QButtonGroup(scaleBox);
    scaleBox->setWhatsThis(i18n("<p>Select the histogram scale here.<p>"
                                "If the image's maximal counts are small, you can use the linear scale.<p>"
                                "Logarithmic scale can be used when the maximal counts are big; "
                                "if it is used, all values (small and large) will be visible on the graph."));

    QToolButton *linHistoButton = new QToolButton( scaleBox );
    linHistoButton->setToolTip( i18n( "<p>Linear" ) );
    linHistoButton->setIcon(KIcon("view-object-histogram-linear"));
    linHistoButton->setCheckable(true);
    m_scaleBG->addButton(linHistoButton, CurvesWidget::LinScaleHistogram);

    QToolButton *logHistoButton = new QToolButton( scaleBox );
    logHistoButton->setToolTip( i18n( "<p>Logarithmic" ) );
    logHistoButton->setIcon(KIcon("view-object-histogram-logarithmic"));
    logHistoButton->setCheckable(true);
    m_scaleBG->addButton(logHistoButton, CurvesWidget::LogScaleHistogram);

    hlay->setMargin(0);
    hlay->setSpacing(0);
    hlay->addWidget(linHistoButton);
    hlay->addWidget(logHistoButton);

    m_scaleBG->setExclusive(true);
    logHistoButton->setChecked(true);

    // -------------------------------------------------------------

    QHBoxLayout* l1 = new QHBoxLayout();
    l1->addStretch(10);
    l1->addWidget(scaleBox);

    // -------------------------------------------------------------

    QWidget *curveBox = new QWidget(m_gboxSettings->plainPage());
    QGridLayout* gl   = new QGridLayout(curveBox);

    m_histogramWidget = new HistogramWidget(256, 140, curveBox, false, true, true);
    m_histogramWidget->setWhatsThis( i18n("<p>Here you can see the target preview image histogram drawing "
                                          "of the selected image channel. This one is re-computed at any "
                                          "curves settings changes."));

    m_vGradient = new ColorGradientWidget( ColorGradientWidget::Vertical, 10, curveBox );
    m_vGradient->setColors( QColor( "white" ), QColor( "black" ) );

    QLabel *spacev = new QLabel(curveBox);
    spacev->setFixedWidth(1);

    m_curvesWidget = new CurvesWidget(256, 256, m_originalImage->bits(), m_originalImage->width(),
                                               m_originalImage->height(), m_originalImage->sixteenBit(),
                                               curveBox);
    m_curvesWidget->setWhatsThis( i18n("<p>This is the curve drawing of the selected channel from "
                                       "original image"));

    QLabel *spaceh = new QLabel(curveBox);
    spaceh->setFixedHeight(1);

    m_hGradient = new ColorGradientWidget( ColorGradientWidget::Horizontal, 10, curveBox );
    m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );

    gl->addWidget(m_histogramWidget,    0, 2, 1, 1);
    gl->addWidget(m_vGradient,          2, 0, 1, 1);
    gl->addWidget(spacev,               2, 1, 1, 1);
    gl->addWidget(m_curvesWidget,       2, 2, 1, 1);
    gl->addWidget(spaceh,               3, 2, 1, 1);
    gl->addWidget(m_hGradient,          4, 2, 1, 1);
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
    m_curveFree->setToolTip( i18n( "Curve free mode" ) );
    m_curveFree->setWhatsThis( i18n("<p>With this button, you can draw your curve free-hand "
                                    "with the mouse."));
    m_curveType->addButton(m_curveFree, FreeDrawing);

    m_curveSmooth = new QToolButton(typeBox);
    m_curveSmooth->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/curvemooth.png")));
    m_curveSmooth->setCheckable(true);
    m_curveSmooth->setToolTip( i18n( "Curve smooth mode" ) );
    m_curveSmooth->setWhatsThis( i18n("<p>With this button, you constrains the curve type "
                                      "to a smooth line with tension."));
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
    m_pickBlack->setToolTip( i18n( "All channels shadow tone color picker" ) );
    m_pickBlack->setWhatsThis( i18n("<p>With this button, you can pick the color from original "
                                    "image used to set <b>Shadow Tone</b> "
                                    "smooth curves point on Red, Green, Blue, and Luminosity channels."));

    m_pickGray  = new QToolButton(m_pickerBox);
    m_pickerColorButtonGroup->addButton(m_pickGray, GrayTonal);
    m_pickGray->setIcon(KIcon("color-picker-grey"));
    m_pickGray->setCheckable(true);
    m_pickGray->setToolTip( i18n( "All channels middle tone color picker" ) );
    m_pickGray->setWhatsThis( i18n("<p>With this button, you can pick the color from original "
                                   "image used to set <b>Middle Tone</b> "
                                   "smooth curves point on Red, Green, Blue, and Luminosity channels."));

    m_pickWhite = new QToolButton(m_pickerBox);
    m_pickerColorButtonGroup->addButton(m_pickWhite, WhiteTonal);
    m_pickWhite->setIcon(KIcon("color-picker-white"));
    m_pickWhite->setCheckable(true);
    m_pickWhite->setToolTip( i18n( "All channels highlight tone color picker" ) );
    m_pickWhite->setWhatsThis( i18n("<p>With this button, you can pick the color from original "
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
    m_resetButton->setIcon( KIconLoader::global()->loadIcon("document-revert", KIconLoader::Toolbar) );
    m_resetButton->setToolTip( i18n( "Reset current channel curves' values." ) );
    m_resetButton->setWhatsThis( i18n("<p>If you press this button, all curves' values "
                                      "from the current selected channel "
                                      "will be reset to the default values."));

    QHBoxLayout* l3 = new QHBoxLayout();
    l3->addWidget(typeBox);
    l3->addWidget(m_pickerBox);
    l3->addWidget(m_resetButton);
    l3->addStretch(10);

    // -------------------------------------------------------------

    grid->addWidget(label1,         0, 1, 1, 1);
    grid->addWidget(m_channelCB,    0, 2, 1, 1);
    grid->addLayout(l1,             0, 4, 1, 5- 4+1);
    grid->addWidget(curveBox,       1, 0, 3, 5+1);
    grid->addLayout(l3,             4, 1, 1, 5);
    grid->setRowStretch(5, 10);
    grid->setMargin(m_gboxSettings->spacingHint());
    grid->setSpacing(m_gboxSettings->spacingHint());

    setToolSettings(m_gboxSettings);

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

    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(m_scaleBG, SIGNAL(buttonReleased(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(m_curveType, SIGNAL(buttonClicked(int)),
            this, SLOT(slotCurveTypeChanged(int)));

    // -------------------------------------------------------------
    // Bouttons slots.

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
    m_histogramWidget->setHistogramGuideByColor(color);
}

void AdjustCurvesTool::slotResetCurrentChannel()
{
    m_curvesWidget->curves()->curvesChannelReset(m_channelCB->currentIndex());

    m_curvesWidget->repaint();
    slotEffect();
    m_histogramWidget->reset();
}

void AdjustCurvesTool::slotEffect()
{
    ImageIface* iface = m_previewWidget->imageIface();
    uchar *orgData             = iface->getPreviewImage();
    int w                      = iface->previewWidth();
    int h                      = iface->previewHeight();
    bool sb                    = iface->previewSixteenBit();

    // Create the new empty destination image data space.
    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData)
       delete [] m_destinationPreviewData;

    m_destinationPreviewData = new uchar[w*h*(sb ? 8 : 4)];

    // Calculate the LUT to apply on the image.
    m_curvesWidget->curves()->curvesLutSetup(ImageHistogram::AlphaChannel);

    // Apply the lut to the image.
    m_curvesWidget->curves()->curvesLutProcess(orgData, m_destinationPreviewData, w, h);

    iface->putPreviewImage(m_destinationPreviewData);
    m_previewWidget->updatePreview();

    // Update histogram.
    m_histogramWidget->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);
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

    // Apply the lut to the image.
    m_curvesWidget->curves()->curvesLutProcess(orgData, desData, w, h);

    iface->putOriginalImage(i18n("Adjust Curve"), desData);
    kapp->restoreOverrideCursor();

    delete [] orgData;
    delete [] desData;
}

void AdjustCurvesTool::slotChannelChanged(int channel)
{
    switch(channel)
    {
       case LuminosityChannel:
          m_histogramWidget->m_channelType = HistogramWidget::ValueHistogram;
          m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
          m_curvesWidget->m_channelType = CurvesWidget::ValueHistogram;
          m_vGradient->setColors( QColor( "white" ), QColor( "black" ) );
          break;

        case RedChannel:
          m_histogramWidget->m_channelType = HistogramWidget::RedChannelHistogram;
          m_hGradient->setColors( QColor( "black" ), QColor( "red" ) );
          m_curvesWidget->m_channelType = CurvesWidget::RedChannelHistogram;
          m_vGradient->setColors( QColor( "red" ), QColor( "black" ) );
          break;

       case GreenChannel:
          m_histogramWidget->m_channelType = HistogramWidget::GreenChannelHistogram;
          m_hGradient->setColors( QColor( "black" ), QColor( "green" ) );
          m_curvesWidget->m_channelType = CurvesWidget::GreenChannelHistogram;
          m_vGradient->setColors( QColor( "green" ), QColor( "black" ) );
          break;

       case BlueChannel:
          m_histogramWidget->m_channelType = HistogramWidget::BlueChannelHistogram;
          m_hGradient->setColors( QColor( "black" ), QColor( "blue" ) );
          m_curvesWidget->m_channelType = CurvesWidget::BlueChannelHistogram;
          m_vGradient->setColors( QColor( "blue" ), QColor( "black" ) );
          break;

       case AlphaChannel:
          m_histogramWidget->m_channelType = HistogramWidget::AlphaChannelHistogram;
          m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
          m_curvesWidget->m_channelType = CurvesWidget::AlphaChannelHistogram;
          m_vGradient->setColors( QColor( "white" ), QColor( "black" ) );
          break;
    }

    m_curveType->button(m_curvesWidget->curves()->getCurveType(channel))->setChecked(true);

    m_curvesWidget->repaint();
    m_histogramWidget->repaint();
}

void AdjustCurvesTool::slotScaleChanged(int scale)
{
    m_curvesWidget->m_scaleType = scale;
    m_histogramWidget->m_scaleType = scale;
    m_histogramWidget->repaint();
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

    m_channelCB->setCurrentIndex(group.readEntry("Histogram Channel", 0));    // Luminosity.
    m_scaleBG->button(group.readEntry("Histogram Scale",
                      (int)CurvesWidget::LogScaleHistogram))->setChecked(true);

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
            QPoint p = group.readEntry(QString("CurveAjustmentChannel%1Point%2").arg(i).arg(j), disable);

            if (m_originalImage->sixteenBit() && p.x() != -1)
            {
                p.setX(p.x()*255);
                p.setY(p.y()*255);
            }

            m_curvesWidget->curves()->setCurvePoint(i, j, p);
        }

        m_curvesWidget->curves()->curvesCalculateCurve(i);
    }

    slotChannelChanged(m_channelCB->currentIndex());
    slotScaleChanged(m_scaleBG->checkedId());

    slotEffect();
}

void AdjustCurvesTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("adjustcurves Tool");
    group.writeEntry("Histogram Channel", m_channelCB->currentIndex());
    group.writeEntry("Histogram Scale", m_scaleBG->checkedId());

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

            group.writeEntry(QString("CurveAjustmentChannel%1Point%2").arg(i).arg(j), p);
        }
    }

    config->sync();
}

void AdjustCurvesTool::slotResetSettings()
{
    for (int channel = 0 ; channel < 5 ; channel++)
       m_curvesWidget->curves()->curvesChannelReset(channel);

    m_curvesWidget->reset();
    m_histogramWidget->reset();

    slotEffect();
}

// Load all settings.
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
    slotChannelChanged(m_channelCB->currentIndex());
    slotEffect();
}

// Save all settings.
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
    slotChannelChanged(m_channelCB->currentIndex());
}

}  // NameSpace DigikamAdjustCurvesImagesPlugin
