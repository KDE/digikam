/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-01
 * Description : image histogram adjust curves.
 *
 * Copyright (C) 2004-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QColor>
#include <QGroupBox>
#include <QLabel>
#include <QPainter>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QFrame>
#include <QTimer>
#include <QButtonGroup>
#include <QPixmap>
#include <QHBoxLayout>
#include <QGridLayout>

// KDE includes.

#include <kconfig.h>
#include <kcursor.h>
#include <klocale.h>
#include <knuminput.h>
#include <kmessagebox.h>
#include <kselector.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <kglobal.h>

// Local includes.

#include "version.h"
#include "daboutdata.h"
#include "ddebug.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "imagehistogram.h"
#include "imagecurves.h"
#include "histogramwidget.h"
#include "curveswidget.h"
#include "colorgradientwidget.h"
#include "dimgimagefilters.h"
#include "adjustcurves.h"
#include "adjustcurves.moc"

namespace DigikamAdjustCurvesImagesPlugin
{

AdjustCurveDialog::AdjustCurveDialog(QWidget* parent)
                 : Digikam::ImageDlgBase(parent, i18n("Adjust Color Curves"), "adjustcurves", true, false)
{
    m_destinationPreviewData = 0L;

    Digikam::ImageIface iface(0, 0);
    uchar *data     = iface.getOriginalImage();
    int w           = iface.originalWidth();
    int h           = iface.originalHeight();
    bool sixteenBit = iface.originalSixteenBit();
    bool hasAlpha   = iface.originalHasAlpha();
    m_originalImage = Digikam::DImg(w, h, sixteenBit, hasAlpha ,data);
    delete [] data;

    m_histoSegments = m_originalImage.sixteenBit() ? 65535 : 255;
    m_curves = new Digikam::ImageCurves(m_originalImage.sixteenBit());

    // About data and help button.

    KAboutData* about = new KAboutData("digikam", 0,
                                       ki18n("Adjust Color Curves"),
                                       digiKamVersion().toAscii(),
                                       ki18n("An image-histogram-curves adjustment plugin for digiKam."),
                                       KAboutData::License_GPL,
                                       ki18n("(c) 2004-2008, Gilles Caulier"),
                                       KLocalizedString(),
                                       Digikam::webProjectUrl().url().toUtf8());

    about->addAuthor(ki18n("Gilles Caulier"), ki18n("Author and maintainer"),
                     "caulier dot gilles at gmail dot com");

    setAboutData(about);

    // -------------------------------------------------------------

    m_previewWidget = new Digikam::ImageWidget("adjustcurves Tool Dialog", mainWidget(),
                                               i18n("<p>This is the image's curve-adjustments preview. "
                                                    "You can pick a spot on the image "
                                                    "to see the corresponding level in the histogram."));
    setPreviewAreaWidget(m_previewWidget);

    // -------------------------------------------------------------

    QWidget *gboxSettings = new QWidget(mainWidget());
    QGridLayout* grid     = new QGridLayout( gboxSettings ) ;

    QLabel *label1 = new QLabel(i18n("Channel:"), gboxSettings);
    label1->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    m_channelCB    = new QComboBox(gboxSettings );
    m_channelCB->addItem( i18n("Luminosity") );
    m_channelCB->addItem( i18n("Red") );
    m_channelCB->addItem( i18n("Green") );
    m_channelCB->addItem( i18n("Blue") );
    m_channelCB->addItem( i18n("Alpha") );
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

    QWidget *scaleBox = new QWidget(gboxSettings);
    QHBoxLayout *hlay = new QHBoxLayout(scaleBox);
    m_scaleBG         = new QButtonGroup(scaleBox);
    scaleBox->setWhatsThis(i18n("<p>Select the histogram scale here.<p>"
                                "If the image's maximal counts are small, you can use the linear scale.<p>"
                                "Logarithmic scale can be used when the maximal counts are big; "
                                "if it is used, all values (small and large) will be visible on the graph."));

    QPushButton *linHistoButton = new QPushButton( scaleBox );
    linHistoButton->setToolTip( i18n( "<p>Linear" ) );
    linHistoButton->setIcon(KIcon("view-object-histogram-linear"));
    linHistoButton->setCheckable(true);
    m_scaleBG->addButton(linHistoButton, Digikam::CurvesWidget::LinScaleHistogram);

    QPushButton *logHistoButton = new QPushButton( scaleBox );
    logHistoButton->setToolTip( i18n( "<p>Logarithmic" ) );
    logHistoButton->setIcon(KIcon("view-object-histogram-logarithmic"));
    logHistoButton->setCheckable(true);
    m_scaleBG->addButton(logHistoButton, Digikam::CurvesWidget::LogScaleHistogram);

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

    QWidget *curveBox = new QWidget(gboxSettings);
    QGridLayout* gl   = new QGridLayout(curveBox);

    m_histogramWidget = new Digikam::HistogramWidget(256, 140, curveBox, false, true, true);
    m_histogramWidget->setWhatsThis( i18n("<p>Here you can see the target preview image histogram drawing "
                                          "of the selected image channel. This one is re-computed at any "
                                          "curves settings changes."));

    m_vGradient = new Digikam::ColorGradientWidget( Digikam::ColorGradientWidget::Vertical, 10, curveBox );
    m_vGradient->setColors( QColor( "white" ), QColor( "black" ) );

    QLabel *spacev = new QLabel(curveBox);
    spacev->setFixedWidth(1);

    m_curvesWidget = new Digikam::CurvesWidget(256, 256, m_originalImage.bits(), m_originalImage.width(),
                                               m_originalImage.height(), m_originalImage.sixteenBit(),
                                               m_curves, curveBox);
    m_curvesWidget->setWhatsThis( i18n("<p>This is the curve drawing of the selected channel from "
                                       "original image"));

    QLabel *spaceh = new QLabel(curveBox);
    spaceh->setFixedHeight(1);

    m_hGradient = new Digikam::ColorGradientWidget( Digikam::ColorGradientWidget::Horizontal, 10, curveBox );
    m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );

    gl->addWidget(m_histogramWidget, 0, 2, 1, 1);
    gl->addWidget(m_vGradient, 2, 0, 1, 1);
    gl->addWidget(spacev, 2, 1, 1, 1);
    gl->addWidget(m_curvesWidget, 2, 2, 1, 1);
    gl->addWidget(spaceh, 3, 2, 1, 1);
    gl->addWidget(m_hGradient, 4, 2, 1, 1);
    gl->setRowMinimumHeight(1, spacingHint());
    gl->setMargin(0);
    gl->setSpacing(0);

    // -------------------------------------------------------------

    QWidget *typeBox   = new QWidget(gboxSettings);
    QHBoxLayout *hlay2 = new QHBoxLayout(typeBox);
    m_curveType        = new QButtonGroup(typeBox);

    m_curveFree = new QPushButton(typeBox);
    m_curveFree->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/curvefree.png")));
    m_curveFree->setCheckable(true);
    m_curveFree->setToolTip( i18n( "Curve free mode" ) );
    m_curveFree->setWhatsThis( i18n("<p>With this button, you can draw your curve free-hand "
                                    "with the mouse."));
    m_curveType->addButton(m_curveFree, FreeDrawing);

    m_curveSmooth = new QPushButton(typeBox);
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

    m_pickerBox              = new QWidget(gboxSettings);
    QHBoxLayout *hlay3       = new QHBoxLayout(m_pickerBox);
    m_pickerColorButtonGroup = new QButtonGroup(m_pickerBox);

    m_pickBlack = new QPushButton(m_pickerBox);
    m_pickerColorButtonGroup->addButton(m_pickBlack, BlackTonal);
    m_pickBlack->setIcon(KIcon("color-picker-black.png"));
    m_pickBlack->setCheckable(true);
    m_pickBlack->setToolTip( i18n( "All channels shadow tone color picker" ) );
    m_pickBlack->setWhatsThis( i18n("<p>With this button, you can pick the color from original "
                                    "image used to set <b>Shadow Tone</b> "
                                    "smooth curves point on Red, Green, Blue, and Luminosity channels."));

    m_pickGray  = new QPushButton(m_pickerBox);
    m_pickerColorButtonGroup->addButton(m_pickGray, GrayTonal);
    m_pickGray->setIcon(KIcon("color-picker-grey.png"));
    m_pickGray->setCheckable(true);
    m_pickGray->setToolTip( i18n( "All channels middle tone color picker" ) );
    m_pickGray->setWhatsThis( i18n("<p>With this button, you can pick the color from original "
                                   "image used to set <b>Middle Tone</b> "
                                   "smooth curves point on Red, Green, Blue, and Luminosity channels."));

    m_pickWhite = new QPushButton(m_pickerBox);
    m_pickerColorButtonGroup->addButton(m_pickWhite, WhiteTonal);
    m_pickWhite->setIcon(KIcon("color-picker-white.png"));
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

    m_resetButton = new QPushButton(i18n("&Reset"), gboxSettings);
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

    grid->setMargin(spacingHint());
    grid->setSpacing(spacingHint());
    grid->addWidget(label1, 0, 1, 1, 1);
    grid->addWidget(m_channelCB, 0, 2, 1, 1);
    grid->addLayout(l1, 0, 4, 1, 5- 4+1);
    grid->addWidget(curveBox, 1, 0, 3, 5+1);
    grid->addLayout(l3, 4, 1, 1, 5);
    grid->setRowStretch(5, 10);

    setUserAreaWidget(gboxSettings);

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

AdjustCurveDialog::~AdjustCurveDialog()
{
    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData)
       delete [] m_destinationPreviewData;

    delete m_histogramWidget;
    delete m_curvesWidget;
    delete m_previewWidget;
    delete m_curves;
}

void AdjustCurveDialog::slotPickerColorButtonActived()
{
    // Save previous rendering mode and toggle to original image.
    m_currentPreviewMode = m_previewWidget->getRenderingPreviewMode();
    m_previewWidget->setRenderingPreviewMode(Digikam::ImageGuideWidget::PreviewOriginalImage);
}

void AdjustCurveDialog::slotSpotColorChanged(const Digikam::DColor &color)
{
    Digikam::DColor sc = color;

    if ( m_pickBlack->isChecked() )
    {
       // Black tonal curves point.
       m_curves->setCurvePoint(Digikam::ImageHistogram::ValueChannel, 1,
                               QPoint(qMax(qMax(sc.red(), sc.green()), sc.blue()), 42*m_histoSegments/256));
       m_curves->setCurvePoint(Digikam::ImageHistogram::RedChannel, 1, QPoint(sc.red(), 42*m_histoSegments/256));
       m_curves->setCurvePoint(Digikam::ImageHistogram::GreenChannel, 1, QPoint(sc.green(), 42*m_histoSegments/256));
       m_curves->setCurvePoint(Digikam::ImageHistogram::BlueChannel, 1, QPoint(sc.blue(), 42*m_histoSegments/256));
       m_pickBlack->setChecked(false);
    }
    else if ( m_pickGray->isChecked() )
    {
       // Gray tonal curves point.
       m_curves->setCurvePoint(Digikam::ImageHistogram::ValueChannel, 8,
                               QPoint(qMax(qMax(sc.red(), sc.green()), sc.blue()), 128*m_histoSegments/256));
       m_curves->setCurvePoint(Digikam::ImageHistogram::RedChannel, 8, QPoint(sc.red(), 128*m_histoSegments/256));
       m_curves->setCurvePoint(Digikam::ImageHistogram::GreenChannel, 8, QPoint(sc.green(), 128*m_histoSegments/256));
       m_curves->setCurvePoint(Digikam::ImageHistogram::BlueChannel, 8, QPoint(sc.blue(), 128*m_histoSegments/256));
       m_pickGray->setChecked(false);
    }
    else if ( m_pickWhite->isChecked() )
    {
       // White tonal curves point.
       m_curves->setCurvePoint(Digikam::ImageHistogram::ValueChannel, 15,
                               QPoint(qMax(qMax(sc.red(), sc.green()), sc.blue()), 213*m_histoSegments/256));
       m_curves->setCurvePoint(Digikam::ImageHistogram::RedChannel, 15, QPoint(sc.red(), 213*m_histoSegments/256));
       m_curves->setCurvePoint(Digikam::ImageHistogram::GreenChannel, 15, QPoint(sc.green(), 213*m_histoSegments/256));
       m_curves->setCurvePoint(Digikam::ImageHistogram::BlueChannel, 15, QPoint(sc.blue(), 213*m_histoSegments/256));
       m_pickWhite->setChecked(false);
    }
    else
    {
       m_curvesWidget->setCurveGuide(color);
       return;
    }

    // Calculate Red, green, blue curves.

    for (int i = Digikam::ImageHistogram::ValueChannel ; i <= Digikam::ImageHistogram::BlueChannel ; i++)
       m_curves->curvesCalculateCurve(i);

    m_curvesWidget->repaint();

    // restore previous rendering mode.
    m_previewWidget->setRenderingPreviewMode(m_currentPreviewMode);

    slotEffect();
}

void AdjustCurveDialog::slotColorSelectedFromTarget( const Digikam::DColor &color )
{
    m_histogramWidget->setHistogramGuideByColor(color);
}

void AdjustCurveDialog::slotResetCurrentChannel()
{
    m_curves->curvesChannelReset(m_channelCB->currentIndex());

    m_curvesWidget->reset();
    slotEffect();
    m_histogramWidget->reset();
}

void AdjustCurveDialog::slotEffect()
{
    Digikam::ImageIface* iface = m_previewWidget->imageIface();
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
    m_curves->curvesLutSetup(Digikam::ImageHistogram::AlphaChannel);

    // Apply the lut to the image.
    m_curves->curvesLutProcess(orgData, m_destinationPreviewData, w, h);

    iface->putPreviewImage(m_destinationPreviewData);
    m_previewWidget->updatePreview();

    // Update histogram.
    m_histogramWidget->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);
    delete [] orgData;
}

void AdjustCurveDialog::finalRendering()
{
    kapp->setOverrideCursor( Qt::WaitCursor );
    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    uchar *orgData             = iface->getOriginalImage();
    int w                      = iface->originalWidth();
    int h                      = iface->originalHeight();
    bool sb                    = iface->originalSixteenBit();

    // Create the new empty destination image data space.
    uchar* desData = new uchar[w*h*(sb ? 8 : 4)];

    // Calculate the LUT to apply on the image.
    m_curves->curvesLutSetup(Digikam::ImageHistogram::AlphaChannel);

    // Apply the lut to the image.
    m_curves->curvesLutProcess(orgData, desData, w, h);

    iface->putOriginalImage(i18n("Adjust Curve"), desData);
    kapp->restoreOverrideCursor();

    delete [] orgData;
    delete [] desData;
    accept();
}

void AdjustCurveDialog::slotChannelChanged(int channel)
{
    switch(channel)
    {
       case LuminosityChannel:
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::ValueHistogram;
          m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
          m_curvesWidget->m_channelType = Digikam::CurvesWidget::ValueHistogram;
          m_vGradient->setColors( QColor( "white" ), QColor( "black" ) );
          break;

        case RedChannel:
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::RedChannelHistogram;
          m_hGradient->setColors( QColor( "black" ), QColor( "red" ) );
          m_curvesWidget->m_channelType = Digikam::CurvesWidget::RedChannelHistogram;
          m_vGradient->setColors( QColor( "red" ), QColor( "black" ) );
          break;

       case GreenChannel:
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::GreenChannelHistogram;
          m_hGradient->setColors( QColor( "black" ), QColor( "green" ) );
          m_curvesWidget->m_channelType = Digikam::CurvesWidget::GreenChannelHistogram;
          m_vGradient->setColors( QColor( "green" ), QColor( "black" ) );
          break;

       case BlueChannel:
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::BlueChannelHistogram;
          m_hGradient->setColors( QColor( "black" ), QColor( "blue" ) );
          m_curvesWidget->m_channelType = Digikam::CurvesWidget::BlueChannelHistogram;
          m_vGradient->setColors( QColor( "blue" ), QColor( "black" ) );
          break;

       case AlphaChannel:
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::AlphaChannelHistogram;
          m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
          m_curvesWidget->m_channelType = Digikam::CurvesWidget::AlphaChannelHistogram;
          m_vGradient->setColors( QColor( "white" ), QColor( "black" ) );
          break;
    }

    m_curveType->button(m_curves->getCurveType(channel))->setChecked(true);

    m_curvesWidget->repaint();
    m_histogramWidget->repaint();
}

void AdjustCurveDialog::slotScaleChanged(int scale)
{
    m_curvesWidget->m_scaleType = scale;
    m_histogramWidget->m_scaleType = scale;
    m_histogramWidget->repaint();
    m_curvesWidget->repaint();
}

void AdjustCurveDialog::slotCurveTypeChanged(int type)
{
    switch(type)
    {
       case SmoothDrawing:
       {
          m_curves->setCurveType(m_curvesWidget->m_channelType, Digikam::ImageCurves::CURVE_SMOOTH);
          m_pickerBox->setEnabled(true);
          break;
       }

       case FreeDrawing:
       {
          m_curves->setCurveType(m_curvesWidget->m_channelType, Digikam::ImageCurves::CURVE_FREE);
          m_pickerBox->setEnabled(false);
          break;
       }
    }

    m_curvesWidget->curveTypeChanged();
}

void AdjustCurveDialog::readUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("adjustcurves Tool Dialog");

    m_channelCB->setCurrentIndex(group.readEntry("Histogram Channel", 0));    // Luminosity.
    m_scaleBG->button(group.readEntry("Histogram Scale",
                      (int)Digikam::CurvesWidget::LogScaleHistogram))->setChecked(true);

    m_curvesWidget->reset();

    for (int i = 0 ; i < 5 ; i++)
    {
        m_curves->curvesChannelReset(i);
        m_curves->setCurveType(i,
                  (Digikam::ImageCurves::CurveType)group.readEntry(QString("CurveTypeChannel%1").arg(i),
                  (int)Digikam::ImageCurves::CURVE_SMOOTH));

        for (int j = 0 ; j < 17 ; j++)
        {
            QPoint disable(-1, -1);
            QPoint p = group.readEntry(QString("CurveAjustmentChannel%1Point%2").arg(i).arg(j), disable);

            if (m_originalImage.sixteenBit() && p.x() != -1)
            {
                p.setX(p.x()*255);
                p.setY(p.y()*255);
            }

            m_curves->setCurvePoint(i, j, p);
        }

        m_curves->curvesCalculateCurve(i);
    }

    slotChannelChanged(m_channelCB->currentIndex());
    slotScaleChanged(m_scaleBG->checkedId());
}

void AdjustCurveDialog::writeUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("adjustcurves Tool Dialog");
    group.writeEntry("Histogram Channel", m_channelCB->currentIndex());
    group.writeEntry("Histogram Scale", m_scaleBG->checkedId());

    for (int i = 0 ; i < 5 ; i++)
    {
        group.writeEntry(QString("CurveTypeChannel%1").arg(i), m_curves->getCurveType(i));

        for (int j = 0 ; j < 17 ; j++)
        {
            QPoint p = m_curves->getCurvePoint(i, j);

            if (m_originalImage.sixteenBit() && p.x() != -1)
            {
                p.setX(p.x()/255);
                p.setY(p.y()/255);
            }

            group.writeEntry(QString("CurveAjustmentChannel%1Point%2").arg(i).arg(j), p);
        }
    }

    config->sync();
}

void AdjustCurveDialog::resetValues()
{
    for (int channel = 0 ; channel < 5 ; channel++)
       m_curves->curvesChannelReset(channel);

    m_curvesWidget->reset();
    m_histogramWidget->reset();
}

// Load all settings.
void AdjustCurveDialog::slotUser3()
{
    KUrl loadCurvesFile;

    loadCurvesFile = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                                             QString( "*" ), this,
                                             QString( i18n("Select Gimp Curves File to Load")) );
    if( loadCurvesFile.isEmpty() )
       return;

    if ( m_curves->loadCurvesFromGimpCurvesFile( loadCurvesFile ) == false )
    {
       KMessageBox::error(this, i18n("Cannot load from the Gimp curves text file."));
       return;
    }

    // Refresh the current curves config.
    slotChannelChanged(m_channelCB->currentIndex());
    slotEffect();
}

// Save all settings.
void AdjustCurveDialog::slotUser2()
{
    KUrl saveCurvesFile;

    saveCurvesFile = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
                                             QString( "*" ), this,
                                             QString( i18n("Gimp Curves File to Save")) );
    if( saveCurvesFile.isEmpty() )
       return;

    if ( m_curves->saveCurvesToGimpCurvesFile( saveCurvesFile ) == false )
    {
       KMessageBox::error(this, i18n("Cannot save to the Gimp curves text file."));
       return;
    }

    // Refresh the current curves config.
    slotChannelChanged(m_channelCB->currentIndex());
}

}  // NameSpace DigikamAdjustCurvesImagesPlugin
