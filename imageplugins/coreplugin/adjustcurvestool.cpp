/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-01
 * Description : image histogram adjust curves.
 *
 * Copyright (C) 2004-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <kapplication.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kselector.h>
#include <kstandarddirs.h>

// Local includes

#include "colorgradientwidget.h"
#include "curveswidget.h"
#include "curvesbox.h"
#include "dimg.h"
#include "dimgimagefilters.h"
#include "editortoolsettings.h"
#include "histogrambox.h"
#include "histogramwidget.h"
#include "curvesfilter.h"
#include "imagehistogram.h"
#include "imagecurves.h"
#include "imageiface.h"
#include "imageregionwidget.h"

namespace DigikamImagesPluginCore
{

class AdjustCurvesToolPriv
{
public:

    AdjustCurvesToolPriv() :
        configGroupName("adjustcurves Tool"),
        configHistogramChannelEntry("Histogram Channel"),
        configHistogramScaleEntry("Histogram Scale"),
        configCurveEntry("AdjustCurves"),
        destinationPreviewData(0),
        histoSegments(0),
        channelCB(0),
        curvesBox(0),
        previewWidget(0),
        originalImage(0),
        gboxSettings(0)
        {}

    const QString       configGroupName;
    const QString       configHistogramChannelEntry;
    const QString       configHistogramScaleEntry;
    const QString       configCurveEntry;

    uchar*              destinationPreviewData;

    int                 histoSegments;

    KComboBox*          channelCB;

    CurvesBox*          curvesBox;
    ImageRegionWidget*  previewWidget;

    DImg*               originalImage;

    EditorToolSettings* gboxSettings;
};

AdjustCurvesTool::AdjustCurvesTool(QObject* parent)
                : EditorToolThreaded(parent),
                  d(new AdjustCurvesToolPriv)
{
    setObjectName("adjustcurves");
    setToolName(i18n("Adjust Curves"));
    setToolIcon(SmallIcon("adjustcurves"));

    ImageIface iface(0, 0);
    d->originalImage = iface.getOriginalImg();

    d->histoSegments = d->originalImage->sixteenBit() ? 65535 : 255;

    // -------------------------------------------------------------

    d->previewWidget = new ImageRegionWidget;
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setTools(EditorToolSettings::Histogram);
    d->gboxSettings->setHistogramType(Digikam::LRGBA);
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Load|
                                EditorToolSettings::SaveAs|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel|
                                EditorToolSettings::Try);

    // we don't need to use the Gradient widget in this tool
    d->gboxSettings->histogramBox()->setGradientVisible(false);

    // -------------------------------------------------------------

    d->curvesBox = new CurvesBox(256, 256, d->originalImage->bits(), d->originalImage->width(),
                                 d->originalImage->height(), d->originalImage->sixteenBit());

    d->curvesBox->enableGradients(true);
    d->curvesBox->enableControlWidgets(true);

    // -------------------------------------------------------------

    QGridLayout* grid = new QGridLayout();
    grid->addWidget(d->curvesBox, 0, 0, 1, 1);
    grid->setRowStretch(1, 10);
    grid->setMargin(0);
    grid->setSpacing(d->gboxSettings->spacingHint());
    d->gboxSettings->plainPage()->setLayout(grid);

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(d->curvesBox, SIGNAL(signalCurvesChanged()),
            this, SLOT(slotTimer()));
            
    connect(d->curvesBox, SIGNAL(signalChannelReset(int)),
            this, SLOT(slotResetCurrentChannel()));

    connect(d->curvesBox, SIGNAL(signalPickerChanged(int)),
            this, SLOT(slotPickerColorButtonActived(int)));

    connect(d->previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));

    connect(d->previewWidget, SIGNAL(signalCapturedPointFromOriginal(const Digikam::DColor&, const QPoint&)),
            this, SLOT(slotSpotColorChanged(const Digikam::DColor&)));

/*
    connect(d->previewWidget, SIGNAL(spotPositionChangedFromTarget(const Digikam::DColor&, const QPoint&)),
            this, SLOT(slotColorSelectedFromTarget(const Digikam::DColor&)));
*/
}

AdjustCurvesTool::~AdjustCurvesTool()
{
    if (d->destinationPreviewData)
       delete [] d->destinationPreviewData;

    delete d;
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

void AdjustCurvesTool::slotPickerColorButtonActived(int type)
{
    if (type == CurvesBox::NoPicker) return;

    d->previewWidget->setCapturePointMode(true);
}

void AdjustCurvesTool::slotSpotColorChanged(const DColor& color)
{
    DColor sc = color;

    switch (d->curvesBox->picker())
    {
        case CurvesBox::BlackTonal:
        {
            // Black tonal curves point.
            d->curvesBox->curves()->setCurvePoint(LuminosityChannel, 1,
                    QPoint(qMax(qMax(sc.red(), sc.green()), sc.blue()), 42*d->histoSegments/256));
            d->curvesBox->curves()->setCurvePoint(RedChannel, 1, QPoint(sc.red(), 42*d->histoSegments/256));
            d->curvesBox->curves()->setCurvePoint(GreenChannel, 1, QPoint(sc.green(), 42*d->histoSegments/256));
            d->curvesBox->curves()->setCurvePoint(BlueChannel, 1, QPoint(sc.blue(), 42*d->histoSegments/256));
            d->curvesBox->resetPickers();
            break;
        }
        case CurvesBox::GrayTonal:
        {
            // Gray tonal curves point.
            d->curvesBox->curves()->setCurvePoint(LuminosityChannel, 8,
                    QPoint(qMax(qMax(sc.red(), sc.green()), sc.blue()), 128*d->histoSegments/256));
            d->curvesBox->curves()->setCurvePoint(RedChannel, 8, QPoint(sc.red(), 128*d->histoSegments/256));
            d->curvesBox->curves()->setCurvePoint(GreenChannel, 8, QPoint(sc.green(), 128*d->histoSegments/256));
            d->curvesBox->curves()->setCurvePoint(BlueChannel, 8, QPoint(sc.blue(), 128*d->histoSegments/256));
            d->curvesBox->resetPickers();
            break;
        }
        case CurvesBox::WhiteTonal:
        {
            // White tonal curves point.
            d->curvesBox->curves()->setCurvePoint(LuminosityChannel, 15,
                    QPoint(qMax(qMax(sc.red(), sc.green()), sc.blue()), 213*d->histoSegments/256));
            d->curvesBox->curves()->setCurvePoint(RedChannel, 15, QPoint(sc.red(), 213*d->histoSegments/256));
            d->curvesBox->curves()->setCurvePoint(GreenChannel, 15, QPoint(sc.green(), 213*d->histoSegments/256));
            d->curvesBox->curves()->setCurvePoint(BlueChannel, 15, QPoint(sc.blue(), 213*d->histoSegments/256));
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

    for (int i = LuminosityChannel ; i <= BlueChannel ; ++i)
       d->curvesBox->curves()->curvesCalculateCurve(i);

    d->curvesBox->repaint();
    d->previewWidget->setCapturePointMode(false);
    
    slotEffect();
}

void AdjustCurvesTool::slotColorSelectedFromTarget(const DColor& color)
{
    d->gboxSettings->histogramBox()->histogram()->setHistogramGuideByColor(color);
}

void AdjustCurvesTool::slotResetCurrentChannel()
{
    slotEffect();
    d->gboxSettings->histogramBox()->histogram()->reset();
}

void AdjustCurvesTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->curvesBox->reset();
    d->curvesBox->readCurveSettings(group, d->configCurveEntry);

    // we need to call the set methods here, otherwise the curve will not be updated correctly
    d->gboxSettings->histogramBox()->setChannel((ChannelType)group.readEntry(d->configHistogramChannelEntry,
                    (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,
                    (int)LogScaleHistogram));

    d->curvesBox->setScale(d->gboxSettings->histogramBox()->scale());
    d->curvesBox->setChannel(d->gboxSettings->histogramBox()->channel());
    d->curvesBox->update();

    slotEffect();
}

void AdjustCurvesTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    group.writeEntry(d->configHistogramChannelEntry, (int)d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,   (int)d->gboxSettings->histogramBox()->scale());

    d->curvesBox->writeCurveSettings(group, d->configCurveEntry);

    config->sync();
}

void AdjustCurvesTool::slotResetSettings()
{
    d->curvesBox->resetChannels();
    d->curvesBox->resetPickers();
    d->gboxSettings->histogramBox()->histogram()->reset();

    slotEffect();
}

void AdjustCurvesTool::prepareEffect()
{
    kapp->setOverrideCursor(Qt::WaitCursor);
    toolSettings()->setEnabled(false);
    toolView()->setEnabled(false);

    CurvesContainer settings;
    settings.lumCurvePts   = d->curvesBox->curves()->getCurvePoints(LuminosityChannel);
    settings.redCurvePts   = d->curvesBox->curves()->getCurvePoints(RedChannel);
    settings.greenCurvePts = d->curvesBox->curves()->getCurvePoints(GreenChannel);
    settings.blueCurvePts  = d->curvesBox->curves()->getCurvePoints(BlueChannel);
    settings.alphaCurvePts = d->curvesBox->curves()->getCurvePoints(AlphaChannel);

    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    DImg preview = d->previewWidget->getOriginalRegionImage(true);
    setFilter(new CurvesFilter(&preview, this, settings));
}

void AdjustCurvesTool::putPreviewData()
{
    DImg preview = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(preview);

    // Update histogram.

    if (d->destinationPreviewData)
       delete [] d->destinationPreviewData;

    d->destinationPreviewData = preview.copyBits();
    d->gboxSettings->histogramBox()->histogram()->updateData(d->destinationPreviewData,
                                                             preview.width(), preview.height(), preview.sixteenBit(),
                                                             0, 0, 0, false);
}

void AdjustCurvesTool::prepareFinal()
{
    toolSettings()->setEnabled(false);
    toolView()->setEnabled(false);

    CurvesContainer settings;
    settings.lumCurvePts   = d->curvesBox->curves()->getCurvePoints(LuminosityChannel);
    settings.redCurvePts   = d->curvesBox->curves()->getCurvePoints(RedChannel);
    settings.greenCurvePts = d->curvesBox->curves()->getCurvePoints(GreenChannel);
    settings.blueCurvePts  = d->curvesBox->curves()->getCurvePoints(BlueChannel);
    settings.alphaCurvePts = d->curvesBox->curves()->getCurvePoints(AlphaChannel);

    ImageIface iface(0, 0);
    setFilter(new CurvesFilter(iface.getOriginalImg(), this, settings));
}

void AdjustCurvesTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Adjust Curve"), filter()->getTargetImage().bits());
}

void AdjustCurvesTool::renderingFinished()
{
    kapp->restoreOverrideCursor();
    toolSettings()->setEnabled(true);
    toolView()->setEnabled(true);
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

}  // namespace DigikamImagesPluginCore
