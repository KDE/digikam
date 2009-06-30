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
#include "curvesbox.h"
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
        channelCB              = 0;
        curvesBox              = 0;
        previewWidget          = 0;
        originalImage          = 0;
        gboxSettings           = 0;
    }

    uchar*               destinationPreviewData;

    int                  histoSegments;
    int                  currentPreviewMode;

    KComboBox*           channelCB;

    CurvesBox*           curvesBox;
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

    // -------------------------------------------------------------

    d->curvesBox = new CurvesBox(256, 256, d->originalImage->bits(), d->originalImage->width(),
                                 d->originalImage->height(), d->originalImage->sixteenBit());

    d->curvesBox->enableGradients(true);
    d->curvesBox->enableControlWidgets(true);

    // -------------------------------------------------------------

    QGridLayout* mainLayout = new QGridLayout();
    mainLayout->addWidget(d->curvesBox, 0, 0, 1, 1);
    mainLayout->setRowStretch(1, 10);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(d->gboxSettings->spacingHint());
    d->gboxSettings->plainPage()->setLayout(mainLayout);

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(d->curvesBox, SIGNAL(signalCurvesChanged()),
            this, SLOT(slotTimer()));

    connect(d->previewWidget, SIGNAL(spotPositionChangedFromOriginal( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotSpotColorChanged( const Digikam::DColor & )));

    connect(d->previewWidget, SIGNAL(spotPositionChangedFromTarget( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotColorSelectedFromTarget( const Digikam::DColor & )));

    connect(d->previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));

    // -------------------------------------------------------------
    // Buttons slots.

    connect(d->curvesBox, SIGNAL(signalChannelReset(int)),
            this, SLOT(slotResetCurrentChannel()));
//
    connect(d->curvesBox, SIGNAL(signalPickerChanged(int)),
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

    switch (d->curvesBox->picker())
    {
        case CurvesBox::BlackTonal:
        {
            // Black tonal curves point.
            d->curvesBox->curves()->setCurvePoint(ImageHistogram::ValueChannel, 1,
                    QPoint(qMax(qMax(sc.red(), sc.green()), sc.blue()), 42*d->histoSegments/256));
            d->curvesBox->curves()->setCurvePoint(ImageHistogram::RedChannel, 1, QPoint(sc.red(), 42*d->histoSegments/256));
            d->curvesBox->curves()->setCurvePoint(ImageHistogram::GreenChannel, 1, QPoint(sc.green(), 42*d->histoSegments/256));
            d->curvesBox->curves()->setCurvePoint(ImageHistogram::BlueChannel, 1, QPoint(sc.blue(), 42*d->histoSegments/256));
            d->curvesBox->resetPickers();
            break;
        }
        case CurvesBox::GrayTonal:
        {
            // Gray tonal curves point.
            d->curvesBox->curves()->setCurvePoint(ImageHistogram::ValueChannel, 8,
                    QPoint(qMax(qMax(sc.red(), sc.green()), sc.blue()), 128*d->histoSegments/256));
            d->curvesBox->curves()->setCurvePoint(ImageHistogram::RedChannel, 8, QPoint(sc.red(), 128*d->histoSegments/256));
            d->curvesBox->curves()->setCurvePoint(ImageHistogram::GreenChannel, 8, QPoint(sc.green(), 128*d->histoSegments/256));
            d->curvesBox->curves()->setCurvePoint(ImageHistogram::BlueChannel, 8, QPoint(sc.blue(), 128*d->histoSegments/256));
            d->curvesBox->resetPickers();
            break;
        }
        case CurvesBox::WhiteTonal:
        {
            // White tonal curves point.
            d->curvesBox->curves()->setCurvePoint(ImageHistogram::ValueChannel, 15,
                    QPoint(qMax(qMax(sc.red(), sc.green()), sc.blue()), 213*d->histoSegments/256));
            d->curvesBox->curves()->setCurvePoint(ImageHistogram::RedChannel, 15, QPoint(sc.red(), 213*d->histoSegments/256));
            d->curvesBox->curves()->setCurvePoint(ImageHistogram::GreenChannel, 15, QPoint(sc.green(), 213*d->histoSegments/256));
            d->curvesBox->curves()->setCurvePoint(ImageHistogram::BlueChannel, 15, QPoint(sc.blue(), 213*d->histoSegments/256));
            d->curvesBox->resetPickers();
            break;
        }
        default:
        {
            d->curvesBox->setCurveGuide(color);
            return;
        }
    }

    // Calculate Red, green, blue curves.

    for (int i = ImageHistogram::ValueChannel ; i <= ImageHistogram::BlueChannel ; ++i)
       d->curvesBox->curves()->curvesCalculateCurve(i);

    d->curvesBox->repaint();

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
    d->curvesBox->curves()->curvesLutSetup(ImageHistogram::AlphaChannel);

    // Apply the LUT to the image.
    d->curvesBox->curves()->curvesLutProcess(orgData, d->destinationPreviewData, w, h);

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
    d->curvesBox->curves()->curvesLutSetup(ImageHistogram::AlphaChannel);

    // Apply the LUT to the image.
    d->curvesBox->curves()->curvesLutProcess(orgData, desData, w, h);

    iface->putOriginalImage(i18n("Adjust Curve"), desData);
    kapp->restoreOverrideCursor();

    delete [] orgData;
    delete [] desData;
}

void AdjustCurvesTool::slotChannelChanged()
{
    d->curvesBox->setChannel(d->gboxSettings->histogramBox()->channel());
    d->gboxSettings->histogramBox()->slotChannelChanged();
}

void AdjustCurvesTool::slotScaleChanged()
{
    d->curvesBox->setScale(d->gboxSettings->histogramBox()->scale());
}

void AdjustCurvesTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("adjustcurves Tool");

    d->curvesBox->reset();
    d->curvesBox->readCurveSettings(group);

    // we need to call the set methods here, otherwise the curve will not be updated correctly
    d->gboxSettings->histogramBox()->setChannel(group.readEntry("Histogram Channel",
                    (int)EditorToolSettings::LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale(group.readEntry("Histogram Scale",
                    (int)CurvesWidget::LogScaleHistogram));
    d->curvesBox->setScale(d->gboxSettings->histogramBox()->scale());
    d->curvesBox->setChannel(d->gboxSettings->histogramBox()->channel());
    d->curvesBox->update();

    slotEffect();
}

void AdjustCurvesTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("adjustcurves Tool");
    group.writeEntry("Histogram Channel", d->gboxSettings->histogramBox()->channel());
    group.writeEntry("Histogram Scale", d->gboxSettings->histogramBox()->scale());

    d->curvesBox->writeCurveSettings(group);
    d->previewWidget->writeSettings();

    config->sync();
}

void AdjustCurvesTool::slotResetSettings()
{
    d->curvesBox->resetChannels();
    d->curvesBox->resetPickers();
    d->gboxSettings->histogramBox()->histogram()->reset();

    slotEffect();
}

void AdjustCurvesTool::slotLoadSettings()
{
    KUrl loadCurvesFile;

    loadCurvesFile = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                                             QString( "*" ), kapp->activeWindow(),
                                             QString( i18n("Select Gimp Curves File to Load")) );
    if ( loadCurvesFile.isEmpty() )
       return;

    if ( d->curvesBox->curves()->loadCurvesFromGimpCurvesFile( loadCurvesFile ) == false )
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
    if ( saveCurvesFile.isEmpty() )
       return;

    if ( d->curvesBox->curves()->saveCurvesToGimpCurvesFile( saveCurvesFile ) == false )
    {
        KMessageBox::error(kapp->activeWindow(),
                           i18n("Cannot save to the Gimp curves text file."));
        return;
    }

    // Refresh the current curves config.
    slotChannelChanged();
}

}  // namespace DigikamAdjustCurvesImagesPlugin
