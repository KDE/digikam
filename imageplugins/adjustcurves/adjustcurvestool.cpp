/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-01
 * Description : image histogram adjust curves.
 *
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "dimg.h"
#include "dimgimagefilters.h"
#include "editortoolsettings.h"
#include "histogrambox.h"
#include "histogramwidget.h"
#include "imagecurves.h"
#include "imagehistogram.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "version.h"

using namespace Digikam;

namespace DigikamAdjustCurvesImagesPlugin
{

class AdjustCurvesToolPriv
{
public:

    AdjustCurvesToolPriv()
    {
        destinationPreviewData = 0;
        histoSegments          = 0;
        currentPreviewMode     = 0;
        pickerBox              = 0;
        resetButton            = 0;
        pickBlack              = 0;
        pickGray               = 0;
        pickWhite              = 0;
        curveFree              = 0;
        curveSmooth            = 0;
        pickerColorButtonGroup = 0;
        curveType              = 0;
        channelCB              = 0;
        curvesWidget           = 0;
        hGradient              = 0;
        vGradient              = 0;
        previewWidget          = 0;
        originalImage          = 0;
        gboxSettings           = 0;
    }

    uchar*               destinationPreviewData;

    int                  histoSegments;
    int                  currentPreviewMode;

    QWidget*             pickerBox;

    QPushButton*         resetButton;
    QToolButton*         pickBlack;
    QToolButton*         pickGray;
    QToolButton*         pickWhite;
    QToolButton*         curveFree;
    QToolButton*         curveSmooth;

    QButtonGroup*        pickerColorButtonGroup;
    QButtonGroup*        curveType;

    KComboBox*           channelCB;

    CurvesWidget*        curvesWidget;

    ColorGradientWidget* hGradient;
    ColorGradientWidget* vGradient;

    ImageWidget*         previewWidget;

    DImg*                originalImage;

    EditorToolSettings*  gboxSettings;
};

AdjustCurvesTool::AdjustCurvesTool(QObject* parent)
                : EditorTool(parent),
                  d(new AdjustCurvesToolPriv)
{
    setObjectName("adjustcurves");
    setToolName(i18n("Adjust Curves"));
    setToolIcon(SmallIcon("adjustcurves"));

    d->destinationPreviewData = 0;

    ImageIface iface(0, 0);
    d->originalImage = iface.getOriginalImg();

    d->histoSegments = d->originalImage->sixteenBit() ? 65535 : 255;

    // -------------------------------------------------------------

    d->previewWidget = new ImageWidget("adjustcurves Tool", 0,
                                      i18n("This is the image's curve-adjustments preview. "
                                           "You can pick a spot on the image "
                                           "to see the corresponding level in the histogram."));
    setToolView(d->previewWidget);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Load|
                                            EditorToolSettings::SaveAs|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel,
                                            EditorToolSettings::Histogram,
                                            HistogramBox::LRGBA);

    d->gboxSettings->histogramBox()->histogram()->setWhatsThis(i18n("Here you can see the target preview "
                                                  "image histogram drawing of the selected image "
                                                  "channel. This one is re-computed at any curves "
                                                  "settings changes."));

    // we don't need to use the Gradient widget in this tool
    d->gboxSettings->histogramBox()->setGradientVisible(false);

    QGridLayout* grid = new QGridLayout(d->gboxSettings->plainPage());

    // -------------------------------------------------------------

    QWidget *curveBox = new QWidget(d->gboxSettings->plainPage());
    QGridLayout* gl   = new QGridLayout(curveBox);

    d->vGradient = new ColorGradientWidget(Qt::Vertical, 10, curveBox);
    d->vGradient->setColors(QColor("white"), QColor("black"));

    d->curvesWidget = new CurvesWidget(256, 256, d->originalImage->bits(), d->originalImage->width(),
                                                 d->originalImage->height(), d->originalImage->sixteenBit(),
                                                 curveBox);
    d->curvesWidget->setWhatsThis(i18n("This is the curve drawing of the selected channel from "
                                       "original image"));

    QLabel *spaceh = new QLabel(curveBox);
    spaceh->setFixedHeight(1);

    d->hGradient = new ColorGradientWidget(Qt::Horizontal, 10, curveBox);
    d->hGradient->setColors(QColor("black"), QColor("white"));

    gl->addWidget(d->vGradient,     0, 0, 1, 1);
    gl->addWidget(d->curvesWidget,  0, 2, 1, 1);
    gl->addWidget(spaceh,           1, 2, 1, 1);
    gl->addWidget(d->hGradient,     2, 2, 1, 1);
    gl->setRowMinimumHeight(1, d->gboxSettings->spacingHint());
    gl->setMargin(0);
    gl->setSpacing(0);

    // -------------------------------------------------------------

    QWidget *typeBox   = new QWidget(d->gboxSettings->plainPage());
    QHBoxLayout *hlay2 = new QHBoxLayout(typeBox);
    d->curveType       = new QButtonGroup(typeBox);

    d->curveFree = new QToolButton(typeBox);
    d->curveFree->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/curvefree.png")));
    d->curveFree->setCheckable(true);
    d->curveFree->setToolTip(i18n("Curve free mode"));
    d->curveFree->setWhatsThis( i18n("With this button, you can draw your curve free-hand "
                                     "with the mouse."));
    d->curveType->addButton(d->curveFree, FreeDrawing);

    d->curveSmooth = new QToolButton(typeBox);
    d->curveSmooth->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/curvemooth.png")));
    d->curveSmooth->setCheckable(true);
    d->curveSmooth->setToolTip(i18n("Curve smooth mode"));
    d->curveSmooth->setWhatsThis( i18n("With this button, the curve type is constrained to "
                                       "be a smooth line with tension."));
    d->curveType->addButton(d->curveSmooth, SmoothDrawing);

    hlay2->setMargin(0);
    hlay2->setSpacing(0);
    hlay2->addWidget(d->curveFree);
    hlay2->addWidget(d->curveSmooth);

    d->curveType->setExclusive(true);
    d->curveSmooth->setChecked(true);

    // -------------------------------------------------------------

    d->pickerBox              = new QWidget(d->gboxSettings->plainPage());
    QHBoxLayout *hlay3        = new QHBoxLayout(d->pickerBox);
    d->pickerColorButtonGroup = new QButtonGroup(d->pickerBox);

    d->pickBlack = new QToolButton(d->pickerBox);
    d->pickerColorButtonGroup->addButton(d->pickBlack, BlackTonal);
    d->pickBlack->setIcon(KIcon("color-picker-black"));
    d->pickBlack->setCheckable(true);
    d->pickBlack->setToolTip(i18n("All channels shadow tone color picker"));
    d->pickBlack->setWhatsThis( i18n("With this button, you can pick the color from original "
                                     "image used to set <b>Shadow Tone</b> "
                                     "smooth curves point on Red, Green, Blue, and Luminosity channels."));

    d->pickGray = new QToolButton(d->pickerBox);
    d->pickerColorButtonGroup->addButton(d->pickGray, GrayTonal);
    d->pickGray->setIcon(KIcon("color-picker-grey"));
    d->pickGray->setCheckable(true);
    d->pickGray->setToolTip(i18n("All channels middle tone color picker"));
    d->pickGray->setWhatsThis( i18n("With this button, you can pick the color from original "
                                    "image used to set <b>Middle Tone</b> "
                                    "smooth curves point on Red, Green, Blue, and Luminosity channels."));

    d->pickWhite = new QToolButton(d->pickerBox);
    d->pickerColorButtonGroup->addButton(d->pickWhite, WhiteTonal);
    d->pickWhite->setIcon(KIcon("color-picker-white"));
    d->pickWhite->setCheckable(true);
    d->pickWhite->setToolTip( i18n( "All channels highlight tone color picker" ) );
    d->pickWhite->setWhatsThis( i18n("With this button, you can pick the color from original "
                                     "image used to set <b>Highlight Tone</b> "
                                     "smooth curves point on Red, Green, Blue, and Luminosity channels."));

    hlay3->setMargin(0);
    hlay3->setSpacing(0);
    hlay3->addWidget(d->pickBlack);
    hlay3->addWidget(d->pickGray);
    hlay3->addWidget(d->pickWhite);

    d->pickerColorButtonGroup->setExclusive(true);

    // -------------------------------------------------------------

    d->resetButton = new QPushButton(i18n("&Reset"), d->gboxSettings->plainPage());
    d->resetButton->setIcon(KIconLoader::global()->loadIcon("document-revert", KIconLoader::Toolbar));
    d->resetButton->setToolTip(i18n("Reset current channel curves' values."));
    d->resetButton->setWhatsThis(i18n("If you press this button, all curves' values "
                                      "from the currently selected channel "
                                      "will be reset to the default values."));

    QHBoxLayout* l3 = new QHBoxLayout();
    l3->addWidget(typeBox);
    l3->addWidget(d->pickerBox);
    l3->addWidget(d->resetButton);
    l3->addStretch(10);

    // -------------------------------------------------------------

    grid->addWidget(curveBox,   0, 0, 3, 6);
    grid->addLayout(l3,         3, 1, 1, 4);
    grid->setRowStretch(4, 10);
    grid->setMargin(d->gboxSettings->spacingHint());
    grid->setSpacing(d->gboxSettings->spacingHint());

    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(d->curvesWidget, SIGNAL(signalCurvesChanged()),
            this, SLOT(slotTimer()));

    connect(d->previewWidget, SIGNAL(spotPositionChangedFromOriginal( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotSpotColorChanged( const Digikam::DColor & )));

    connect(d->previewWidget, SIGNAL(spotPositionChangedFromTarget( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotColorSelectedFromTarget( const Digikam::DColor & )));

    connect(d->previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));

    // -------------------------------------------------------------
    // ComboBox slots.

    connect(d->curveType, SIGNAL(buttonClicked(int)),
            this, SLOT(slotCurveTypeChanged(int)));

    // -------------------------------------------------------------
    // Buttons slots.

    connect(d->resetButton, SIGNAL(clicked()),
            this, SLOT(slotResetCurrentChannel()));

    connect(d->pickerColorButtonGroup, SIGNAL(buttonReleased(int)),
            this, SLOT(slotPickerColorButtonActived()));
}

AdjustCurvesTool::~AdjustCurvesTool()
{
    delete [] d->destinationPreviewData;
    delete d;
}

void AdjustCurvesTool::slotPickerColorButtonActived()
{
    // Save previous rendering mode and toggle to original image.
    d->currentPreviewMode = d->previewWidget->getRenderingPreviewMode();
    d->previewWidget->setRenderingPreviewMode(ImageGuideWidget::PreviewOriginalImage);
}

void AdjustCurvesTool::slotSpotColorChanged(const DColor& color)
{
    DColor sc = color;

    if ( d->pickBlack->isChecked() )
    {
       // Black tonal curves point.
       d->curvesWidget->curves()->setCurvePoint(ImageHistogram::ValueChannel, 1,
                                  QPoint(qMax(qMax(sc.red(), sc.green()), sc.blue()), 42*d->histoSegments/256));
       d->curvesWidget->curves()->setCurvePoint(ImageHistogram::RedChannel, 1, QPoint(sc.red(), 42*d->histoSegments/256));
       d->curvesWidget->curves()->setCurvePoint(ImageHistogram::GreenChannel, 1, QPoint(sc.green(), 42*d->histoSegments/256));
       d->curvesWidget->curves()->setCurvePoint(ImageHistogram::BlueChannel, 1, QPoint(sc.blue(), 42*d->histoSegments/256));
       d->pickBlack->setChecked(false);
    }
    else if ( d->pickGray->isChecked() )
    {
       // Gray tonal curves point.
       d->curvesWidget->curves()->setCurvePoint(ImageHistogram::ValueChannel, 8,
                                  QPoint(qMax(qMax(sc.red(), sc.green()), sc.blue()), 128*d->histoSegments/256));
       d->curvesWidget->curves()->setCurvePoint(ImageHistogram::RedChannel, 8, QPoint(sc.red(), 128*d->histoSegments/256));
       d->curvesWidget->curves()->setCurvePoint(ImageHistogram::GreenChannel, 8, QPoint(sc.green(), 128*d->histoSegments/256));
       d->curvesWidget->curves()->setCurvePoint(ImageHistogram::BlueChannel, 8, QPoint(sc.blue(), 128*d->histoSegments/256));
       d->pickGray->setChecked(false);
    }
    else if ( d->pickWhite->isChecked() )
    {
       // White tonal curves point.
       d->curvesWidget->curves()->setCurvePoint(ImageHistogram::ValueChannel, 15,
                                  QPoint(qMax(qMax(sc.red(), sc.green()), sc.blue()), 213*d->histoSegments/256));
       d->curvesWidget->curves()->setCurvePoint(ImageHistogram::RedChannel, 15, QPoint(sc.red(), 213*d->histoSegments/256));
       d->curvesWidget->curves()->setCurvePoint(ImageHistogram::GreenChannel, 15, QPoint(sc.green(), 213*d->histoSegments/256));
       d->curvesWidget->curves()->setCurvePoint(ImageHistogram::BlueChannel, 15, QPoint(sc.blue(), 213*d->histoSegments/256));
       d->pickWhite->setChecked(false);
    }
    else
    {
       d->curvesWidget->setCurveGuide(color);
       return;
    }

    // Calculate Red, green, blue curves.

    for (int i = ImageHistogram::ValueChannel ; i <= ImageHistogram::BlueChannel ; ++i)
       d->curvesWidget->curves()->curvesCalculateCurve(i);

    d->curvesWidget->repaint();

    // restore previous rendering mode.
    d->previewWidget->setRenderingPreviewMode(d->currentPreviewMode);

    slotEffect();
}

void AdjustCurvesTool::slotColorSelectedFromTarget( const DColor& color )
{
    d->gboxSettings->histogramBox()->histogram()->setHistogramGuideByColor(color);
}

void AdjustCurvesTool::slotResetCurrentChannel()
{
    d->curvesWidget->curves()->curvesChannelReset(d->gboxSettings->histogramBox()->channel());

    d->curvesWidget->repaint();
    slotEffect();
    d->gboxSettings->histogramBox()->histogram()->reset();
}

void AdjustCurvesTool::slotEffect()
{
    ImageIface* iface = d->previewWidget->imageIface();
    uchar *orgData    = iface->getPreviewImage();
    int w             = iface->previewWidth();
    int h             = iface->previewHeight();
    bool sb           = iface->previewSixteenBit();

    // Create the new empty destination image data space.
    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    if (d->destinationPreviewData)
       delete [] d->destinationPreviewData;

    d->destinationPreviewData = new uchar[w*h*(sb ? 8 : 4)];

    // Calculate the LUT to apply on the image.
    d->curvesWidget->curves()->curvesLutSetup(ImageHistogram::AlphaChannel);

    // Apply the LUT to the image.
    d->curvesWidget->curves()->curvesLutProcess(orgData, d->destinationPreviewData, w, h);

    iface->putPreviewImage(d->destinationPreviewData);
    d->previewWidget->updatePreview();

    // Update histogram.
    d->gboxSettings->histogramBox()->histogram()->updateData(d->destinationPreviewData, w, h, sb, 0, 0, 0, false);
    delete [] orgData;
}

void AdjustCurvesTool::finalRendering()
{
    kapp->setOverrideCursor( Qt::WaitCursor );

    ImageIface* iface = d->previewWidget->imageIface();
    uchar *orgData    = iface->getOriginalImage();
    int w             = iface->originalWidth();
    int h             = iface->originalHeight();
    bool sb           = iface->originalSixteenBit();

    // Create the new empty destination image data space.
    uchar* desData = new uchar[w*h*(sb ? 8 : 4)];

    // Calculate the LUT to apply on the image.
    d->curvesWidget->curves()->curvesLutSetup(ImageHistogram::AlphaChannel);

    // Apply the LUT to the image.
    d->curvesWidget->curves()->curvesLutProcess(orgData, desData, w, h);

    iface->putOriginalImage(i18n("Adjust Curve"), desData);
    kapp->restoreOverrideCursor();

    delete [] orgData;
    delete [] desData;
}

void AdjustCurvesTool::slotChannelChanged()
{
    int channel = d->gboxSettings->histogramBox()->channel();
    switch (channel)
    {
        case EditorToolSettings::LuminosityChannel:
            d->curvesWidget->m_channelType = CurvesWidget::ValueHistogram;
            d->hGradient->setColors(QColor("white"), QColor("black"));
            d->vGradient->setColors(QColor("white"), QColor("black"));
            break;

        case EditorToolSettings::RedChannel:
            d->curvesWidget->m_channelType = CurvesWidget::RedChannelHistogram;
            d->hGradient->setColors(QColor("red"), QColor("black"));
            d->vGradient->setColors(QColor("red"), QColor("black"));
            break;

        case EditorToolSettings::GreenChannel:
            d->curvesWidget->m_channelType = CurvesWidget::GreenChannelHistogram;
            d->hGradient->setColors(QColor("green"), QColor("black"));
            d->vGradient->setColors(QColor("green"), QColor("black"));
            break;

        case EditorToolSettings::BlueChannel:
            d->curvesWidget->m_channelType = CurvesWidget::BlueChannelHistogram;
            d->hGradient->setColors(QColor("blue"), QColor("black"));
            d->vGradient->setColors(QColor("blue"), QColor("black"));
            break;

        case EditorToolSettings::AlphaChannel:
            d->curvesWidget->m_channelType = CurvesWidget::AlphaChannelHistogram;
            d->hGradient->setColors(QColor("white"), QColor("black"));
            d->vGradient->setColors(QColor("white"), QColor("black"));
            break;
    }

    d->curveType->button(d->curvesWidget->curves()->getCurveType(channel))->setChecked(true);
    d->curvesWidget->repaint();
    d->gboxSettings->histogramBox()->slotChannelChanged();
}

void AdjustCurvesTool::slotScaleChanged()
{
    d->curvesWidget->m_scaleType = d->gboxSettings->histogramBox()->scale();
    d->curvesWidget->repaint();
}

void AdjustCurvesTool::slotCurveTypeChanged(int type)
{
    switch(type)
    {
       case SmoothDrawing:
       {
          d->curvesWidget->curves()->setCurveType(d->curvesWidget->m_channelType, ImageCurves::CURVE_SMOOTH);
          d->pickerBox->setEnabled(true);
          break;
       }

       case FreeDrawing:
       {
          d->curvesWidget->curves()->setCurveType(d->curvesWidget->m_channelType, ImageCurves::CURVE_FREE);
          d->pickerBox->setEnabled(false);
          break;
       }
    }

    d->curvesWidget->curveTypeChanged();
}

void AdjustCurvesTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("adjustcurves Tool");

    d->curvesWidget->reset();

    QPoint disable(-1, -1);

    for (int i = 0 ; i < 5 ; ++i)
    {
        d->curvesWidget->curves()->curvesChannelReset(i);
        d->curvesWidget->curves()->setCurveType(i,
                  (ImageCurves::CurveType)group.readEntry(QString("CurveTypeChannel%1").arg(i),
                  (int)ImageCurves::CURVE_SMOOTH));

        for (int j = 0 ; j < 17 ; ++j)
        {
            QPoint p = group.readEntry(QString("CurveAdjustmentChannel%1Point%2").arg(i).arg(j), disable);

            if (d->originalImage->sixteenBit() && p.x() != -1)
            {
                p.setX(p.x()*255);
                p.setY(p.y()*255);
            }

            d->curvesWidget->curves()->setCurvePoint(i, j, p);
        }

        d->curvesWidget->curves()->curvesCalculateCurve(i);
    }

    // we need to call the set methods here, otherwise the curve will not be updated correctly
    d->gboxSettings->histogramBox()->setChannel(group.readEntry("Histogram Channel",
                    (int)EditorToolSettings::LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale(group.readEntry("Histogram Scale",
                    (int)CurvesWidget::LogScaleHistogram));

    slotEffect();
}

void AdjustCurvesTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("adjustcurves Tool");
    group.writeEntry("Histogram Channel", d->gboxSettings->histogramBox()->channel());
    group.writeEntry("Histogram Scale", d->gboxSettings->histogramBox()->scale());

    for (int i = 0 ; i < 5 ; ++i)
    {
        group.writeEntry(QString("CurveTypeChannel%1").arg(i), d->curvesWidget->curves()->getCurveType(i));

        for (int j = 0 ; j < 17 ; ++j)
        {
            QPoint p = d->curvesWidget->curves()->getCurvePoint(i, j);

            if (d->originalImage->sixteenBit() && p.x() != -1)
            {
                p.setX(p.x()/255);
                p.setY(p.y()/255);
            }

            group.writeEntry(QString("CurveAdjustmentChannel%1Point%2").arg(i).arg(j), p);
        }
    }

    d->previewWidget->writeSettings();

    config->sync();
}

void AdjustCurvesTool::slotResetSettings()
{
    for (int channel = 0 ; channel < 5 ; ++channel)
       d->curvesWidget->curves()->curvesChannelReset(channel);

    d->curvesWidget->reset();
    d->gboxSettings->histogramBox()->histogram()->reset();

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

    if ( d->curvesWidget->curves()->loadCurvesFromGimpCurvesFile( loadCurvesFile ) == false )
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

    if ( d->curvesWidget->curves()->saveCurvesToGimpCurvesFile( saveCurvesFile ) == false )
    {
       KMessageBox::error(kapp->activeWindow(),
                          i18n("Cannot save to the Gimp curves text file."));
       return;
    }

    // Refresh the current curves config.
    slotChannelChanged();
}

}  // namespace DigikamAdjustCurvesImagesPlugin
