/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-17
 * Description : a tab to display colors information of images
 *
 * Copyright (C) 2004-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagepropertiescolorstab.moc"

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

// KDE includes


#include <kapplication.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <ksqueezedtextlabel.h>
#include <kstandarddirs.h>
#include <kvbox.h>

// Local includes

#include "dimg.h"
#include "imagehistogram.h"
#include "histogramwidget.h"
#include "histogrambox.h"
#include "colorgradientwidget.h"
#include "sharedloadsavethread.h"
#include "iccprofilewidget.h"
#include "cietonguewidget.h"
#include "imagepropertiestxtlabel.h"
#include "globals.h"

namespace Digikam
{

class ImagePropertiesColorsTab::ImagePropertiesColorsTabPriv
{
public:

    enum MetadataTab
    {
        HISTOGRAM=0,
        ICCPROFILE
    };

    ImagePropertiesColorsTabPriv() :
        blinkFlag(false),
        regionBox(0),
        regionBG(0),
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
        iccProfileWidget(0),
        imageLoaderThread(0),
        histogramBox(0)
    {
    }

    bool                   blinkFlag;

    QWidget*               regionBox;

    QButtonGroup*          regionBG;

    QSpinBox*              minInterv;
    QSpinBox*              maxInterv;

    DTextLabelValue*       labelMeanValue;
    DTextLabelValue*       labelPixelsValue;
    DTextLabelValue*       labelStdDevValue;
    DTextLabelValue*       labelCountValue;
    DTextLabelValue*       labelMedianValue;
    DTextLabelValue*       labelPercentileValue;
    DTextLabelValue*       labelColorDepth;
    DTextLabelValue*       labelAlphaChannel;

    QString                currentFilePath;
    LoadingDescription     currentLoadingDescription;

    QRect                  selectionArea;

    IccProfile             embedded_profile;

    DImg                   image;
    DImg                   imageSelection;

    ICCProfileWidget*      iccProfileWidget;
    SharedLoadSaveThread*  imageLoaderThread;

    HistogramBox*          histogramBox;
};

ImagePropertiesColorsTab::ImagePropertiesColorsTab(QWidget* parent)
    : KTabWidget(parent), d(new ImagePropertiesColorsTabPriv)
{
    // Histogram tab area -----------------------------------------------------

    QScrollArea* sv = new QScrollArea(this);
    sv->setFrameStyle(QFrame::NoFrame);
    sv->setWidgetResizable(true);

    QWidget* histogramPage = new QWidget(sv->viewport());
    QGridLayout* topLayout = new QGridLayout(histogramPage);
    sv->setWidget(histogramPage);

    // -------------------------------------------------------------

    d->regionBox       = new QWidget(histogramPage);
    QHBoxLayout* hlay2 = new QHBoxLayout(d->regionBox);
    d->regionBG        = new QButtonGroup(d->regionBox);
    d->regionBG->setExclusive(true);
    d->regionBox->hide();
    d->regionBox->setWhatsThis( i18n("<p>Select from which region the histogram will be computed here:</p>"
                                     "<p><b>Full Image</b>: Compute histogram using the full image.<br/>"
                                     "<b>Selection</b>: Compute histogram using the current image "
                                     "selection.</p>"));

    QPushButton* fullImageButton = new QPushButton(d->regionBox);
    fullImageButton->setToolTip( i18n( "Full Image" ) );
    fullImageButton->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/image-full.png")));
    fullImageButton->setCheckable(true);
    d->regionBG->addButton(fullImageButton, FullImageHistogram);

    QPushButton* SelectionImageButton = new QPushButton(d->regionBox);
    SelectionImageButton->setToolTip( i18n( "Selection" ) );
    SelectionImageButton->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/image-selection.png")));
    SelectionImageButton->setCheckable(true);
    d->regionBG->addButton(SelectionImageButton, ImageSelectionHistogram);

    hlay2->setMargin(0);
    hlay2->setSpacing(0);
    hlay2->addWidget(fullImageButton);
    hlay2->addWidget(SelectionImageButton);

    // -------------------------------------------------------------

    KVBox* histoBox    = new KVBox(histogramPage);
    d->histogramBox    = new HistogramBox(histoBox, Digikam::LRGBAC, true);

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
    hlay2->setSpacing(KDialog::spacingHint());
    hlay2->setMargin(0);

    // -------------------------------------------------------------

    QGroupBox* gbox = new QGroupBox(i18n("Statistics"), histogramPage);
    gbox->setWhatsThis(i18n("Here you can see the statistical results calculated from the "
                            "selected histogram part. These values are available for all "
                            "channels."));
    QGridLayout* grid       = new QGridLayout(gbox);

    DTextLabelName* label5  = new DTextLabelName(i18n("Pixels: "), gbox);
    d->labelPixelsValue     = new DTextLabelValue(0, gbox);

    DTextLabelName* label7  = new DTextLabelName(i18n("Count: "), gbox);
    d->labelCountValue      = new DTextLabelValue(0, gbox);

    DTextLabelName* label4  = new DTextLabelName(i18n("Mean: "), gbox);
    d->labelMeanValue       = new DTextLabelValue(0, gbox);

    DTextLabelName* label6  = new DTextLabelName(i18n("Std. deviation: "), gbox);
    d->labelStdDevValue     = new DTextLabelValue(0, gbox);

    DTextLabelName* label8  = new DTextLabelName(i18n("Median: "), gbox);
    d->labelMedianValue     = new DTextLabelValue(0, gbox);

    DTextLabelName* label9  = new DTextLabelName(i18n("Percentile: "), gbox);
    d->labelPercentileValue = new DTextLabelValue(0, gbox);

    DTextLabelName* label10 = new DTextLabelName(i18n("Color depth: "), gbox);
    d->labelColorDepth      = new DTextLabelValue(0, gbox);

    DTextLabelName* label11 = new DTextLabelName(i18n("Alpha Channel: "), gbox);
    d->labelAlphaChannel    = new DTextLabelValue(0, gbox);

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
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(0);

    // -------------------------------------------------------------

    topLayout->addWidget(d->regionBox, 0, 3, 1, 1);
    topLayout->addWidget(histoBox,     1, 0, 2, 4);
    topLayout->addLayout(hlay3,        3, 0, 1, 4);
    topLayout->addWidget(gbox,         4, 0, 1, 4);
    topLayout->setRowStretch(5, 10);
    topLayout->setColumnStretch(2, 10);
    topLayout->setMargin(KDialog::spacingHint());
    topLayout->setSpacing(KDialog::spacingHint());

    insertTab(ImagePropertiesColorsTabPriv::HISTOGRAM, sv, i18n("Histogram"));

    // ICC Profiles tab area ---------------------------------------

    QScrollArea* sv2 = new QScrollArea(this);
    sv2->setFrameStyle(QFrame::NoFrame);
    sv2->setWidgetResizable(true);

    d->iccProfileWidget = new ICCProfileWidget(sv2->viewport());
    sv2->setWidget(d->iccProfileWidget);
    insertTab(ImagePropertiesColorsTabPriv::ICCPROFILE, sv2, i18n("ICC profile"));

    // -------------------------------------------------------------

    connect(d->regionBG, SIGNAL(buttonReleased(int)),
            this, SLOT(slotRenderingChanged(int)));

    // -------------------------------------------------------------
    // histogramBox connections

    connect(d->histogramBox->histogram(), SIGNAL(signalIntervalChanged( int, int )),
            this, SLOT(slotUpdateInterval(int, int)));

    connect(d->histogramBox->histogram(), SIGNAL(signalMaximumValueChanged( int )),
            this, SLOT(slotUpdateIntervRange(int)));

    connect(d->histogramBox->histogram(), SIGNAL(signalHistogramComputationDone(bool)),
            this, SLOT(slotRefreshOptions(bool)));

    connect(d->histogramBox->histogram(), SIGNAL(signalHistogramComputationFailed(void)),
            this, SLOT(slotHistogramComputationFailed(void)));

    connect(d->histogramBox, SIGNAL(signalChannelChanged(ChannelType)),
            this, SLOT(slotChannelChanged()));

    connect(d->histogramBox, SIGNAL(signalScaleChanged(HistogramScale)),
            this, SLOT(slotScaleChanged()));

    // -------------------------------------------------------------

    connect(d->minInterv, SIGNAL(valueChanged (int)),
            this, SLOT(slotMinValueChanged(int)));

    connect(d->maxInterv, SIGNAL(valueChanged (int)),
            this, SLOT(slotMaxValueChanged(int)));
}

ImagePropertiesColorsTab::~ImagePropertiesColorsTab()
{
    // If there is a currently histogram computation when dialog is closed,
    // stop it before the d->image data are deleted automatically!
    d->histogramBox->histogram()->stopHistogramComputation();

    if (d->imageLoaderThread)
    {
        delete d->imageLoaderThread;
    }

    delete d;
}

void ImagePropertiesColorsTab::readSettings(const KConfigGroup& group)
{
    setCurrentIndex(group.readEntry("ImagePropertiesColors Tab",
                                    (int)ImagePropertiesColorsTabPriv::HISTOGRAM));
    d->iccProfileWidget->setMode(group.readEntry("ICC Level", (int)ICCProfileWidget::CUSTOM));
    d->iccProfileWidget->setCurrentItemByKey(group.readEntry("Current ICC Item", QString()));

    d->histogramBox->setChannel((ChannelType)group.readEntry("Histogram Channel",
                                (int)Digikam::LuminosityChannel));
    d->histogramBox->setScale((HistogramScale)group.readEntry("Histogram Scale",
                              (int)LogScaleHistogram));
    d->regionBG->button(group.readEntry("Histogram Rendering",
                                        (int)FullImageHistogram))->setChecked(true);

}

void ImagePropertiesColorsTab::writeSettings(KConfigGroup& group)
{
    group.writeEntry("ImagePropertiesColors Tab", currentIndex());
    group.writeEntry("Histogram Channel", (int)d->histogramBox->channel());
    group.writeEntry("Histogram Scale", (int)d->histogramBox->scale());
    group.writeEntry("Histogram Rendering", d->regionBG->checkedId());
    group.writeEntry("ICC Level", d->iccProfileWidget->getMode());
    group.writeEntry("Current ICC Item", d->iccProfileWidget->getCurrentItemKey());
}

void ImagePropertiesColorsTab::setData(const KUrl& url, const QRect& selectionArea,
                                       DImg* img)
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

    d->currentFilePath.clear();
    d->currentLoadingDescription = LoadingDescription();
    d->iccProfileWidget->loadFromURL(KUrl());

    // Clear information.
    d->labelMeanValue->clear();
    d->labelPixelsValue->clear();
    d->labelStdDevValue->clear();
    d->labelCountValue->clear();
    d->labelMedianValue->clear();
    d->labelPercentileValue->clear();
    d->labelColorDepth->clear();
    d->labelAlphaChannel->clear();

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
                d->histogramBox->histogram()->updateData(d->image.bits(), d->image.width(), d->image.height(),
                        d->image.sixteenBit(), d->imageSelection.bits(),
                        d->imageSelection.width(), d->imageSelection.height());
                d->regionBox->show();
                updateInformation();
            }
            else
            {
                d->histogramBox->histogram()->updateData(d->image.bits(), d->image.width(),
                        d->image.height(), d->image.sixteenBit());
                d->regionBox->hide();
                updateInformation();
            }
        }
        else
        {
            d->histogramBox->histogram()->setLoadingFailed();
            d->iccProfileWidget->setLoadingFailed();
            slotHistogramComputationFailed();
        }
    }
}

void ImagePropertiesColorsTab::loadImageFromUrl(const KUrl& url)
{
    // create thread on demand
    if (!d->imageLoaderThread)
    {
        d->imageLoaderThread = new SharedLoadSaveThread();

        connect(d->imageLoaderThread, SIGNAL(signalImageLoaded(const LoadingDescription&, const DImg&)),
                this, SLOT(slotLoadImageFromUrlComplete(const LoadingDescription&, const DImg&)));

        connect(d->imageLoaderThread, SIGNAL(signalMoreCompleteLoadingAvailable(const LoadingDescription&, const LoadingDescription&)),
                this, SLOT(slotMoreCompleteLoadingAvailable(const LoadingDescription&, const LoadingDescription&)));
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

    d->currentFilePath = desc.filePath;
    d->currentLoadingDescription = desc;

    d->imageLoaderThread->load(d->currentLoadingDescription,
                               SharedLoadSaveThread::AccessModeRead,
                               SharedLoadSaveThread::LoadingPolicyFirstRemovePrevious);

    d->histogramBox->histogram()->setDataLoading();
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
        d->histogramBox->histogram()->updateData(img.bits(), img.width(), img.height(), img.sixteenBit());

        // As a safety precaution, this must be changed only after updateData is called,
        // which stops computation because d->image.bits() is currently used by threaded histogram algorithm.
        d->image = img;
        d->regionBox->hide();
        updateInformation();
        getICCData();
    }
    else
    {
        d->histogramBox->histogram()->setLoadingFailed();
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
        // Even the time-optimized raw loading takes significant time, and we must avoid two dcraw instances running
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
    d->selectionArea = selectionArea;

    if (d->selectionArea.isValid())
    {
        d->imageSelection = d->image.copy(d->selectionArea);
        d->histogramBox->histogram()->updateSelectionData(d->imageSelection.bits(), d->imageSelection.width(),
                d->imageSelection.height(), d->imageSelection.sixteenBit());
        d->regionBox->show();
        slotRenderingChanged(d->regionBG->checkedId());
    }
    else
    {
        d->imageSelection.reset();
        d->regionBox->hide();
        slotRenderingChanged(FullImageHistogram);
    }
}

void ImagePropertiesColorsTab::slotRefreshOptions(bool /*sixteenBit*/)
{
    slotChannelChanged();
    slotScaleChanged();

    if (d->selectionArea.isValid())
    {
        slotRenderingChanged(d->regionBG->checkedId());
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
    updateStatistics();
}

void ImagePropertiesColorsTab::slotRenderingChanged(int rendering)
{
    d->histogramBox->histogram()->setRenderingType((HistogramRenderingType)rendering);
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
    updateStatistics();
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

    updateStatistics();
}

void ImagePropertiesColorsTab::slotUpdateIntervRange(int range)
{
    d->maxInterv->setMaximum( range );
}

void ImagePropertiesColorsTab::updateInformation()
{
    d->labelColorDepth->setText(d->image.sixteenBit() ? i18n("16 bits") : i18n("8 bits"));
    d->labelAlphaChannel->setText(d->image.hasAlpha() ? i18n("Yes") : i18n("No"));
}

void ImagePropertiesColorsTab::updateStatistics()
{
    ImageHistogram* renderedHistogram = d->histogramBox->histogram()->currentHistogram();

    if (!renderedHistogram)
    {
        return;
    }

    QString value;
    int min = d->minInterv->value();
    int max = d->maxInterv->value();
    int channel = d->histogramBox->channel();

    if ( channel == ColorChannels )
    {
        channel = LuminosityChannel;
    }

    double mean = renderedHistogram->getMean(channel, min, max);
    d->labelMeanValue->setText(value.setNum(mean, 'f', 1));

    double pixels = renderedHistogram->getPixels();
    d->labelPixelsValue->setText(value.setNum((float)pixels, 'f', 0));

    double stddev = renderedHistogram->getStdDev(channel, min, max);
    d->labelStdDevValue->setText(value.setNum(stddev, 'f', 1));

    double counts = renderedHistogram->getCount(channel, min, max);
    d->labelCountValue->setText(value.setNum((float)counts, 'f', 0));

    double median = renderedHistogram->getMedian(channel, min, max);
    d->labelMedianValue->setText(value.setNum(median, 'f', 1));

    double percentile = (pixels > 0 ? (100.0 * counts / pixels) : 0.0);
    d->labelPercentileValue->setText(value.setNum(percentile, 'f', 1));
}

void ImagePropertiesColorsTab::getICCData()
{
    if (DImg::fileFormat(d->currentFilePath) == DImg::RAW)
    {
        d->iccProfileWidget->setUncalibratedColor();
    }
    else if (!d->image.getIccProfile().isNull())
    {
        d->embedded_profile = d->image.getIccProfile();
        d->iccProfileWidget->loadProfile(d->currentFilePath, d->embedded_profile);
    }
    else
    {
        d->iccProfileWidget->setLoadingFailed();
    }
}

}  // namespace Digikam
