/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-17
 * Description : a tab to display colors information of images
 *
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagepropertiescolorstab.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QButtonGroup>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QSpinBox>
#include <QScrollArea>
#include <QToolButton>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "dimg.h"
#include "imagehistogram.h"
#include "histogramwidget.h"
#include "histogrambox.h"
#include "colorgradientwidget.h"
#include "sharedloadsavethread.h"
#include "iccprofilewidget.h"
#include "cietonguewidget.h"
#include "imagepropertiestxtlabel.h"
#include "digikam_globals.h"



namespace Digikam
{

class ImagePropertiesColorsTab::Private
{
public:

    enum MetadataTab
    {
        HISTOGRAM=0,
        ICCPROFILE
    };

public:

    Private() :
        minInterv(0),
        maxInterv(0),
        labelMeanValue(0),
        labelPixelsValue(0),
        labelStdDevValue(0),
        labelCountValue(0),
        labelMedianValue(0),
        labelPercentileValue(0),
        labelColorDepth(0),
        labelAlphaChannel(0),
        labelImageRegion(0),
        iccProfileWidget(0),
        imageLoaderThread(0),
        histogramBox(0),
        redHistogram(0),
        greenHistogram(0),
        blueHistogram(0)
    {
    }

public:

    QSpinBox*             minInterv;
    QSpinBox*             maxInterv;

    DTextLabelValue*      labelMeanValue;
    DTextLabelValue*      labelPixelsValue;
    DTextLabelValue*      labelStdDevValue;
    DTextLabelValue*      labelCountValue;
    DTextLabelValue*      labelMedianValue;
    DTextLabelValue*      labelPercentileValue;
    DTextLabelValue*      labelColorDepth;
    DTextLabelValue*      labelAlphaChannel;
    DTextLabelValue*      labelImageRegion;

    QString               currentFilePath;
    LoadingDescription    currentLoadingDescription;

    QRect                 selectionArea;

    IccProfile            embeddedProfile;

    DImg                  image;
    DImg                  imageSelection;

    ICCProfileWidget*     iccProfileWidget;
    SharedLoadSaveThread* imageLoaderThread;

    HistogramBox*         histogramBox;
    HistogramWidget*      redHistogram;
    HistogramWidget*      greenHistogram;
    HistogramWidget*      blueHistogram;
};

ImagePropertiesColorsTab::ImagePropertiesColorsTab(QWidget* const parent)
    : QTabWidget(parent), d(new Private)
{
    // Histogram tab area -----------------------------------------------------

    QScrollArea* sv = new QScrollArea(this);
    sv->setFrameStyle(QFrame::NoFrame);
    sv->setWidgetResizable(true);

    QWidget* histogramPage = new QWidget(sv->viewport());
    QGridLayout* topLayout = new QGridLayout(histogramPage);
    sv->setWidget(histogramPage);

    // -------------------------------------------------------------

    DVBox* histoBox    = new DVBox(histogramPage);
    d->histogramBox    = new HistogramBox(histoBox, LRGBAC, true);
    d->histogramBox->setStatisticsVisible(false);

    QLabel* space = new QLabel(histoBox);
    space->setFixedHeight(1);

    // -------------------------------------------------------------

    QHBoxLayout* hlay3 = new QHBoxLayout();
    QLabel* label3     = new QLabel(i18n("Range:"), histogramPage);
    label3->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    d->minInterv = new QSpinBox(histogramPage);
    d->minInterv->setRange(0, 255);
    d->minInterv->setSingleStep(1);
    d->minInterv->setValue(0);
    d->minInterv->setWhatsThis( i18n("Select the minimal intensity "
                                     "value of the histogram selection here."));
    d->maxInterv = new QSpinBox(histogramPage);
    d->minInterv->setRange(0, 255);
    d->minInterv->setSingleStep(1);
    d->maxInterv->setValue(255);
    d->minInterv->setWhatsThis( i18n("Select the maximal intensity value "
                                     "of the histogram selection here."));
    hlay3->addWidget(label3);
    hlay3->addWidget(d->minInterv);
    hlay3->addWidget(d->maxInterv);

    // -------------------------------------------------------------

    QGroupBox* gbox         = new QGroupBox(i18n("Statistics"), histogramPage);
    gbox->setWhatsThis(i18n("Here you can see the statistical results calculated from the "
                            "selected histogram part. These values are available for all "
                            "channels."));
    QGridLayout* grid       = new QGridLayout(gbox);

    DTextLabelName* label5  = new DTextLabelName(i18n("Pixels: "), gbox);
    d->labelPixelsValue     = new DTextLabelValue(QString(), gbox);

    DTextLabelName* label7  = new DTextLabelName(i18n("Count: "), gbox);
    d->labelCountValue      = new DTextLabelValue(QString(), gbox);

    DTextLabelName* label4  = new DTextLabelName(i18n("Mean: "), gbox);
    d->labelMeanValue       = new DTextLabelValue(QString(), gbox);

    DTextLabelName* label6  = new DTextLabelName(i18n("Std. deviation: "), gbox);
    d->labelStdDevValue     = new DTextLabelValue(QString(), gbox);

    DTextLabelName* label8  = new DTextLabelName(i18n("Median: "), gbox);
    d->labelMedianValue     = new DTextLabelValue(QString(), gbox);

    DTextLabelName* label9  = new DTextLabelName(i18n("Percentile: "), gbox);
    d->labelPercentileValue = new DTextLabelValue(QString(), gbox);

    DTextLabelName* label10 = new DTextLabelName(i18n("Color depth: "), gbox);
    d->labelColorDepth      = new DTextLabelValue(QString(), gbox);

    DTextLabelName* label11 = new DTextLabelName(i18n("Alpha Channel: "), gbox);
    d->labelAlphaChannel    = new DTextLabelValue(QString(), gbox);

    DTextLabelName* label12 = new DTextLabelName(i18n("Source: "), gbox);
    d->labelImageRegion     = new DTextLabelValue(QString(), gbox);

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    grid->addWidget(label5,                  0, 0, 1, 1);
    grid->addWidget(d->labelPixelsValue,     0, 1, 1, 1);
    grid->addWidget(label7,                  1, 0, 1, 1);
    grid->addWidget(d->labelCountValue,      1, 1, 1, 1);
    grid->addWidget(label4,                  2, 0, 1, 1);
    grid->addWidget(d->labelMeanValue,       2, 1, 1, 1);
    grid->addWidget(label6,                  3, 0, 1, 1);
    grid->addWidget(d->labelStdDevValue,     3, 1, 1, 1);
    grid->addWidget(label8,                  4, 0, 1, 1);
    grid->addWidget(d->labelMedianValue,     4, 1, 1, 1);
    grid->addWidget(label9,                  5, 0, 1, 1);
    grid->addWidget(d->labelPercentileValue, 5, 1, 1, 1);
    grid->addWidget(label10,                 6, 0, 1, 1);
    grid->addWidget(d->labelColorDepth,      6, 1, 1, 1);
    grid->addWidget(label11,                 7, 0, 1, 1);
    grid->addWidget(d->labelAlphaChannel,    7, 1, 1, 1);
    grid->addWidget(label12,                 8, 0, 1, 1);
    grid->addWidget(d->labelImageRegion,     8, 1, 1, 1);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(0);

    // -------------------------------------------------------------

    d->redHistogram   = new HistogramWidget(256, 100, histogramPage);
    d->greenHistogram = new HistogramWidget(256, 100, histogramPage);
    d->blueHistogram  = new HistogramWidget(256, 100, histogramPage);

    d->redHistogram->setChannelType(RedChannel);
    d->redHistogram->setStatisticsVisible(true);
    d->greenHistogram->setChannelType(GreenChannel);
    d->greenHistogram->setStatisticsVisible(true);
    d->blueHistogram->setChannelType(BlueChannel);
    d->blueHistogram->setStatisticsVisible(true);

    // -------------------------------------------------------------

    topLayout->addWidget(histoBox,          0, 0, 2, 4);
    topLayout->addLayout(hlay3,             2, 0, 1, 4);
    topLayout->addWidget(gbox,              3, 0, 1, 4);
    topLayout->addWidget(d->redHistogram,   4, 0, 1, 4);
    topLayout->addWidget(d->greenHistogram, 5, 0, 1, 4);
    topLayout->addWidget(d->blueHistogram,  6, 0, 1, 4);
    topLayout->setRowStretch(7, 10);
    topLayout->setColumnStretch(2, 10);
    topLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    topLayout->setSpacing(spacing);

    insertTab(Private::HISTOGRAM, sv, i18n("Histogram"));

    // ICC Profiles tab area ---------------------------------------

    QScrollArea* sv2 = new QScrollArea(this);
    sv2->setFrameStyle(QFrame::NoFrame);
    sv2->setWidgetResizable(true);

    d->iccProfileWidget = new ICCProfileWidget(sv2->viewport());
    sv2->setWidget(d->iccProfileWidget);
    insertTab(Private::ICCPROFILE, sv2, i18n("ICC profile"));

    // -------------------------------------------------------------
    // histogramBox connections

    connect(d->histogramBox->histogram(), SIGNAL(signalIntervalChanged(int,int)),
            this, SLOT(slotUpdateInterval(int,int)));

    connect(d->redHistogram, SIGNAL(signalIntervalChanged(int,int)),
            this, SLOT(slotUpdateIntervalFromRGB(int,int)));

    connect(d->greenHistogram, SIGNAL(signalIntervalChanged(int,int)),
            this, SLOT(slotUpdateIntervalFromRGB(int,int)));

    connect(d->blueHistogram, SIGNAL(signalIntervalChanged(int,int)),
            this, SLOT(slotUpdateIntervalFromRGB(int,int)));

    connect(d->histogramBox->histogram(), SIGNAL(signalMaximumValueChanged(int)),
            this, SLOT(slotUpdateIntervRange(int)));

    connect(d->histogramBox->histogram(), SIGNAL(signalHistogramComputationDone(bool)),
            this, SLOT(slotRefreshOptions()));

    connect(d->histogramBox->histogram(), SIGNAL(signalHistogramComputationFailed()),
            this, SLOT(slotHistogramComputationFailed()));

    connect(d->histogramBox, SIGNAL(signalChannelChanged(ChannelType)),
            this, SLOT(slotChannelChanged()));

    connect(d->histogramBox, SIGNAL(signalScaleChanged(HistogramScale)),
            this, SLOT(slotScaleChanged()));

    // -------------------------------------------------------------

    connect(d->minInterv, SIGNAL(valueChanged(int)),
            this, SLOT(slotMinValueChanged(int)));

    connect(d->maxInterv, SIGNAL(valueChanged(int)),
            this, SLOT(slotMaxValueChanged(int)));
}

ImagePropertiesColorsTab::~ImagePropertiesColorsTab()
{
    // If there is a currently histogram computation when dialog is closed,
    // stop it before the d->image data are deleted automatically!
    d->histogramBox->histogram()->stopHistogramComputation();
    d->redHistogram->stopHistogramComputation();
    d->greenHistogram->stopHistogramComputation();
    d->blueHistogram->stopHistogramComputation();

    if (d->imageLoaderThread)
    {
        delete d->imageLoaderThread;
    }

    delete d;
}

void ImagePropertiesColorsTab::readSettings(const KConfigGroup& group)
{
    setCurrentIndex(group.readEntry("ImagePropertiesColors Tab",                  (int)Private::HISTOGRAM));
    d->iccProfileWidget->setMode(group.readEntry("ICC Level",                     (int)ICCProfileWidget::CUSTOM));
    d->iccProfileWidget->setCurrentItemByKey(group.readEntry("Current ICC Item",  QString()));
    d->histogramBox->setChannel((ChannelType)group.readEntry("Histogram Channel", (int)LuminosityChannel));
    d->histogramBox->setScale((HistogramScale)group.readEntry("Histogram Scale",  (int)LogScaleHistogram));
}

void ImagePropertiesColorsTab::writeSettings(KConfigGroup& group)
{
    group.writeEntry("ImagePropertiesColors Tab", currentIndex());
    group.writeEntry("Histogram Channel",         (int)d->histogramBox->channel());
    group.writeEntry("Histogram Scale",           (int)d->histogramBox->scale());
    group.writeEntry("ICC Level",                 d->iccProfileWidget->getMode());
    group.writeEntry("Current ICC Item",          d->iccProfileWidget->getCurrentItemKey());
}

void ImagePropertiesColorsTab::setData(const QUrl& url, const QRect& selectionArea, DImg* const img)
{
    // We might be getting duplicate events from AlbumIconView,
    // which will cause all sorts of duplicate work.
    // More importantly, while the loading thread can handle this pretty well,
    // this will completely mess up the timing of progress info in the histogram widget.
    // So filter here, before the stopHistogramComputation!
    // But do not filter if current path is null, as it would not disable the widget on first run.
    if (!img && !d->currentFilePath.isNull() && url.toLocalFile() == d->currentFilePath)
    {
        return;
    }

    // This is necessary to stop computation because d->image.bits() is currently used by
    // threaded histogram algorithm.
    d->histogramBox->histogram()->stopHistogramComputation();
    d->redHistogram->stopHistogramComputation();
    d->greenHistogram->stopHistogramComputation();
    d->blueHistogram->stopHistogramComputation();

    d->currentFilePath.clear();
    d->currentLoadingDescription = LoadingDescription();
    d->iccProfileWidget->loadFromURL(QUrl());

    // Clear information.
    d->labelMeanValue->setAdjustedText();
    d->labelPixelsValue->setAdjustedText();
    d->labelStdDevValue->setAdjustedText();
    d->labelCountValue->setAdjustedText();
    d->labelMedianValue->setAdjustedText();
    d->labelPercentileValue->setAdjustedText();
    d->labelColorDepth->setAdjustedText();
    d->labelAlphaChannel->setAdjustedText();

    if (url.isEmpty())
    {
        setEnabled(false);
        d->image.reset();
        return;
    }

    d->selectionArea = selectionArea;
    d->image.reset();
    setEnabled(true);

    if (!img)
    {
        loadImageFromUrl(url);
    }
    else
    {
        d->image = img->copy();

        if ( !d->image.isNull() )
        {
            getICCData();

            // If a selection area is done in Image Editor and if the current image is the same
            // in Image Editor, then compute too the histogram for this selection.
            if (d->selectionArea.isValid())
            {
                d->imageSelection = d->image.copy(d->selectionArea);
                d->histogramBox->histogram()->updateData(d->image, d->imageSelection);
                d->redHistogram->updateData(d->image, d->imageSelection);
                d->greenHistogram->updateData(d->image, d->imageSelection);
                d->blueHistogram->updateData(d->image, d->imageSelection);

                slotRenderingChanged(ImageSelectionHistogram);
                updateInformation();
            }
            else
            {
                d->histogramBox->histogram()->updateData(d->image);
                d->redHistogram->updateData(d->image);
                d->greenHistogram->updateData(d->image);
                d->blueHistogram->updateData(d->image);
                slotRenderingChanged(FullImageHistogram);
                updateInformation();
            }
        }
        else
        {
            d->histogramBox->histogram()->setLoadingFailed();
            d->redHistogram->setLoadingFailed();
            d->greenHistogram->setLoadingFailed();
            d->blueHistogram->setLoadingFailed();
            d->iccProfileWidget->setLoadingFailed();
            slotHistogramComputationFailed();
        }
    }
}

void ImagePropertiesColorsTab::loadImageFromUrl(const QUrl& url)
{
    // create thread on demand
    if (!d->imageLoaderThread)
    {
        d->imageLoaderThread = new SharedLoadSaveThread();

        connect(d->imageLoaderThread, SIGNAL(signalImageLoaded(LoadingDescription,DImg)),
                this, SLOT(slotLoadImageFromUrlComplete(LoadingDescription,DImg)));

        connect(d->imageLoaderThread, SIGNAL(signalMoreCompleteLoadingAvailable(LoadingDescription,LoadingDescription)),
                this, SLOT(slotMoreCompleteLoadingAvailable(LoadingDescription,LoadingDescription)));
    }

    LoadingDescription desc = LoadingDescription(url.toLocalFile());

    if (DImg::fileFormat(desc.filePath) == DImg::RAW)
    {
        // use raw settings optimized for speed

        DRawDecoding rawDecodingSettings = DRawDecoding();
        rawDecodingSettings.optimizeTimeLoading();
        desc = LoadingDescription(desc.filePath, rawDecodingSettings, LoadingDescription::RawDecodingTimeOptimized);
    }

    if (d->currentLoadingDescription.equalsOrBetterThan(desc))
    {
        return;
    }

    d->currentFilePath           = desc.filePath;
    d->currentLoadingDescription = desc;

    d->imageLoaderThread->load(d->currentLoadingDescription,
                               SharedLoadSaveThread::AccessModeRead,
                               SharedLoadSaveThread::LoadingPolicyFirstRemovePrevious);

    d->histogramBox->histogram()->setDataLoading();
    d->redHistogram->setDataLoading();
    d->greenHistogram->setDataLoading();
    d->blueHistogram->setDataLoading();
    d->iccProfileWidget->setDataLoading();
}

void ImagePropertiesColorsTab::slotLoadImageFromUrlComplete(const LoadingDescription& loadingDescription, const DImg& img)
{
    // Discard any leftover messages from previous, possibly aborted loads
    if ( !loadingDescription.equalsOrBetterThan(d->currentLoadingDescription) )
    {
        return;
    }

    if ( !img.isNull() )
    {
        d->histogramBox->histogram()->updateData(img);
        d->redHistogram->updateData(img);
        d->greenHistogram->updateData(img);
        d->blueHistogram->updateData(img);

        // As a safety precaution, this must be changed only after updateData is called,
        // which stops computation because d->image.bits() is currently used by threaded histogram algorithm.
        d->image = img;
        updateInformation();
        getICCData();
    }
    else
    {
        d->histogramBox->histogram()->setLoadingFailed();
        d->redHistogram->setLoadingFailed();
        d->greenHistogram->setLoadingFailed();
        d->blueHistogram->setLoadingFailed();
        d->iccProfileWidget->setLoadingFailed();
        slotHistogramComputationFailed();
    }
}

void ImagePropertiesColorsTab::slotMoreCompleteLoadingAvailable(const LoadingDescription& oldLoadingDescription,
                                                                const LoadingDescription& newLoadingDescription)
{
    if (oldLoadingDescription == d->currentLoadingDescription &&
        newLoadingDescription.equalsOrBetterThan(d->currentLoadingDescription))
    {
        // Yes, we do want to stop our old time-optimized loading and chain to the current, more complete loading.
        // Even the time-optimized raw loading takes significant time, and we must avoid two Raw engine instances running
        // at a time.
        d->currentLoadingDescription = newLoadingDescription;
        d->imageLoaderThread->load(newLoadingDescription,
                                   SharedLoadSaveThread::AccessModeRead,
                                   SharedLoadSaveThread::LoadingPolicyFirstRemovePrevious);
    }
}

void ImagePropertiesColorsTab::setSelection(const QRect& selectionArea)
{
    if (selectionArea == d->selectionArea)
    {
        return;
    }

    // This is necessary to stop computation because d->image.bits() is currently used by
    // threaded histogram algorithm.
    d->histogramBox->histogram()->stopHistogramComputation();
    d->redHistogram->stopHistogramComputation();
    d->greenHistogram->stopHistogramComputation();
    d->blueHistogram->stopHistogramComputation();
    d->selectionArea = selectionArea;

    if (d->selectionArea.isValid())
    {
        d->imageSelection = d->image.copy(d->selectionArea);
        d->histogramBox->histogram()->updateSelectionData(d->imageSelection);
        d->redHistogram->updateSelectionData(d->imageSelection);
        d->greenHistogram->updateSelectionData(d->imageSelection);
        d->blueHistogram->updateSelectionData(d->imageSelection);
        slotRenderingChanged(ImageSelectionHistogram);
    }
    else
    {
        d->imageSelection.reset();
        slotRenderingChanged(FullImageHistogram);
    }
}

void ImagePropertiesColorsTab::slotRefreshOptions()
{
    slotChannelChanged();
    slotScaleChanged();

    if (d->selectionArea.isValid())
    {
        slotRenderingChanged(ImageSelectionHistogram);
    }
}

void ImagePropertiesColorsTab::slotHistogramComputationFailed()
{
    d->imageSelection.reset();
    d->image.reset();
}

void ImagePropertiesColorsTab::slotChannelChanged()
{
    updateStatistics();
}

void ImagePropertiesColorsTab::slotScaleChanged()
{
    HistogramScale scale = d->histogramBox->histogram()->scaleType();
    d->redHistogram->setScaleType(scale);
    d->greenHistogram->setScaleType(scale);
    d->blueHistogram->setScaleType(scale);
    updateStatistics();
}

void ImagePropertiesColorsTab::slotRenderingChanged(int rendering)
{
    d->histogramBox->histogram()->setRenderingType((HistogramRenderingType)rendering);
    d->redHistogram->setRenderingType((HistogramRenderingType)rendering);
    d->greenHistogram->setRenderingType((HistogramRenderingType)rendering);
    d->blueHistogram->setRenderingType((HistogramRenderingType)rendering);
    updateStatistics();
}

void ImagePropertiesColorsTab::slotMinValueChanged(int min)
{
    // Called when user changes values of spin box.
    // Communicate the change to histogram widget.

    // make the one control "push" the other
    if (min == d->maxInterv->value()+1)
    {
        d->maxInterv->setValue(min);
    }

    d->maxInterv->setMinimum(min-1);
    d->histogramBox->histogram()->slotMinValueChanged(min);
    d->redHistogram->slotMinValueChanged(min);
    d->greenHistogram->slotMinValueChanged(min);
    d->blueHistogram->slotMinValueChanged(min);
    updateStatistics();
}

void ImagePropertiesColorsTab::slotMaxValueChanged(int max)
{
    if (max == d->minInterv->value()-1)
    {
        d->minInterv->setValue(max);
    }

    d->minInterv->setMaximum(max+1);
    d->histogramBox->histogram()->slotMaxValueChanged(max);
    d->redHistogram->slotMaxValueChanged(max);
    d->greenHistogram->slotMaxValueChanged(max);
    d->blueHistogram->slotMaxValueChanged(max);
    updateStatistics();
}

void ImagePropertiesColorsTab::slotUpdateIntervalFromRGB(int min, int max)
{
    d->histogramBox->histogram()->slotMinValueChanged(min);
    d->histogramBox->histogram()->slotMaxValueChanged(max);
    slotUpdateInterval(min, max);
}

void ImagePropertiesColorsTab::slotUpdateInterval(int min, int max)
{
    // Called when value is set from within histogram widget.
    // Block signals to prevent slotMinValueChanged and
    // slotMaxValueChanged being called.
    d->minInterv->blockSignals(true);
    d->minInterv->setMaximum(max+1);
    d->minInterv->setValue(min);
    d->minInterv->blockSignals(false);

    d->maxInterv->blockSignals(true);
    d->maxInterv->setMinimum(min-1);
    d->maxInterv->setValue(max);
    d->maxInterv->blockSignals(false);

    d->redHistogram->slotMinValueChanged(min);
    d->redHistogram->slotMaxValueChanged(max);
    d->greenHistogram->slotMinValueChanged(min);
    d->greenHistogram->slotMaxValueChanged(max);
    d->blueHistogram->slotMinValueChanged(min);
    d->blueHistogram->slotMaxValueChanged(max);

    updateStatistics();
}

void ImagePropertiesColorsTab::slotUpdateIntervRange(int range)
{
    d->maxInterv->setMaximum( range );
}

void ImagePropertiesColorsTab::updateInformation()
{
    d->labelColorDepth->setAdjustedText(d->image.sixteenBit() ? i18n("16 bits") : i18n("8 bits"));
    d->labelAlphaChannel->setAdjustedText(d->image.hasAlpha() ? i18n("Yes")     : i18n("No"));
}

void ImagePropertiesColorsTab::updateStatistics()
{
    ImageHistogram* const renderedHistogram = d->histogramBox->histogram()->currentHistogram();

    if (!renderedHistogram)
    {
        return;
    }

    QString value;
    int min                     = d->minInterv->value();
    int max                     = d->maxInterv->value();
    int channel                 = d->histogramBox->channel();
    HistogramRenderingType type = d->histogramBox->histogram()->renderingType();

    if ( channel == ColorChannels )
    {
        channel = LuminosityChannel;
    }

    double mean = renderedHistogram->getMean(channel, min, max);
    d->labelMeanValue->setAdjustedText(value.setNum(mean, 'f', 1));

    double pixels = renderedHistogram->getPixels();
    d->labelPixelsValue->setAdjustedText(value.setNum((float)pixels, 'f', 0));

    double stddev = renderedHistogram->getStdDev(channel, min, max);
    d->labelStdDevValue->setAdjustedText(value.setNum(stddev, 'f', 1));

    double counts = renderedHistogram->getCount(channel, min, max);
    d->labelCountValue->setAdjustedText(value.setNum((float)counts, 'f', 0));

    double median = renderedHistogram->getMedian(channel, min, max);
    d->labelMedianValue->setAdjustedText(value.setNum(median, 'f', 1));

    double percentile = (pixels > 0 ? (100.0 * counts / pixels) : 0.0);
    d->labelPercentileValue->setAdjustedText(value.setNum(percentile, 'f', 1));

    d->labelImageRegion->setAdjustedText( (type == FullImageHistogram) ? i18n("Full Image") : i18n("Image Region") );
}

void ImagePropertiesColorsTab::getICCData()
{
    if (DImg::fileFormat(d->currentFilePath) == DImg::RAW)
    {
        d->iccProfileWidget->setUncalibratedColor();
    }
    else if (!d->image.getIccProfile().isNull())
    {
        d->embeddedProfile = d->image.getIccProfile();
        d->iccProfileWidget->loadProfile(d->currentFilePath, d->embeddedProfile);
    }
    else
    {
        d->iccProfileWidget->setLoadingFailed();
    }
}

}  // namespace Digikam
