/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date  : 2004-11-17
 * Description : A tab to display Colors image informations
 *
 * Copyright 2004-2006 by Gilles Caulier
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

// C++ include.

#include <cmath>

// Qt Includes.

#include <qlayout.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qgroupbox.h>
#include <qhbuttongroup.h> 
#include <qpushbutton.h>
#include <qtooltip.h>

// KDE includes.

#include <klocale.h>
#include <ksqueezedtextlabel.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdialogbase.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <ktabwidget.h>

// Local includes.

#include "dimg.h"
#include "imagehistogram.h"
#include "histogramwidget.h"
#include "colorgradientwidget.h"
#include "navigatebarwidget.h"
#include "sharedloadsavethread.h"
#include "iccprofilewidget.h"
#include "cietonguewidget.h"
#include "imagepropertiescolorstab.h"

namespace Digikam
{

class ImagePropertiesColorsTabPriv
{
public:

    enum MetadataTab
    {
        HISTOGRAM=0,
        ICCPROFILE
    };

    ImagePropertiesColorsTabPriv()
    {
        imageLoaderThread    = 0;
        tab                  = 0;
        channelCB            = 0;
        colorsCB             = 0;
        renderingCB          = 0;
        scaleBG              = 0;
        regionBG             = 0;
        minInterv            = 0;
        maxInterv            = 0;
        labelMeanValue       = 0;
        labelPixelsValue     = 0;
        labelStdDevValue     = 0;
        labelCountValue      = 0;
        labelMedianValue     = 0;
        labelPercentileValue = 0;
        labelColorDepth      = 0;
        labelAlphaChannel    = 0;
        selectionArea        = 0;

        iccProfileWidget     = 0;
        hGradient            = 0;
        histogramWidget      = 0;
        navigateBar          = 0;
        imageLoaderThread    = 0;
    }

    bool                   blinkFlag;

    QComboBox             *channelCB;
    QComboBox             *colorsCB;
    QComboBox             *renderingCB;

    QHButtonGroup         *scaleBG;
    QHButtonGroup         *regionBG;

    QSpinBox              *minInterv;
    QSpinBox              *maxInterv;

    QLabel                *labelMeanValue;
    QLabel                *labelPixelsValue;
    QLabel                *labelStdDevValue;
    QLabel                *labelCountValue;
    QLabel                *labelMedianValue;
    QLabel                *labelPercentileValue;
    QLabel                *labelColorDepth;
    QLabel                *labelAlphaChannel;

    QString                currentFilePath;
    LoadingDescription     currentLoadingDescription;

    QRect                 *selectionArea;

    QByteArray             embedded_profile;

    KTabWidget            *tab;

    DImg                   image;
    DImg                   imageSelection;

    ICCProfileWidget      *iccProfileWidget;
    ColorGradientWidget   *hGradient;
    HistogramWidget       *histogramWidget;
    NavigateBarWidget     *navigateBar;
    SharedLoadSaveThread  *imageLoaderThread;
};

ImagePropertiesColorsTab::ImagePropertiesColorsTab(QWidget* parent, QRect* selectionArea, bool navBar)
                        : QWidget(parent)
{
    d = new ImagePropertiesColorsTabPriv;

    d->selectionArea = selectionArea;

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    d->navigateBar       = new NavigateBarWidget(this, navBar);
    d->tab               = new KTabWidget(this);
    vLayout->addWidget(d->navigateBar);
    vLayout->addSpacing(KDialog::spacingHint());
    vLayout->addWidget(d->tab);

    // Histogram tab area -----------------------------------------------------

    QWidget* histogramPage = new QWidget( d->tab );
    QGridLayout *topLayout = new QGridLayout(histogramPage, 8, 3, KDialog::marginHint(), KDialog::spacingHint());

    QLabel *label1 = new QLabel(i18n("Channel:"), histogramPage);
    label1->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    d->channelCB = new QComboBox( false, histogramPage );
    d->channelCB->insertItem( i18n("Luminosity") );
    d->channelCB->insertItem( i18n("Red") );
    d->channelCB->insertItem( i18n("Green") );
    d->channelCB->insertItem( i18n("Blue") );
    d->channelCB->insertItem( i18n("Alpha") );
    d->channelCB->insertItem( i18n("Colors") );
    QWhatsThis::add( d->channelCB, i18n("<p>Select here the histogram channel to display:<p>"
                                        "<b>Luminosity</b>: Display luminosity (perceived brightness) values.<p>"
                                        "<b>Red</b>: Display the red image channel.<p>"
                                        "<b>Green</b>: Display the green image channel.<p>"
                                        "<b>Blue</b>: Display the blue image channel.<p>"
                                        "<b>Alpha</b>: Display the alpha image channel. "
                                        "This channel corresponds to the transparency value and "
                                        "is supported by some image formats such as PNG or TIFF.<p>"
                                        "<b>Colors</b>: Display all color channel values at the same time."));

    d->scaleBG = new QHButtonGroup(histogramPage);
    d->scaleBG->setExclusive(true);
    d->scaleBG->setFrameShape(QFrame::NoFrame);
    d->scaleBG->setInsideMargin( 0 );
    QWhatsThis::add( d->scaleBG, i18n("<p>Select here the histogram scale.<p>"
                                      "If the image's maximal values are small, you can use the linear scale.<p>"
                                      "Logarithmic scale can be used when the maximal values are big; "
                                      "if it is used, all values (small and large) will be visible on the "
                                      "graph."));

    QPushButton *linHistoButton = new QPushButton( d->scaleBG );
    QToolTip::add( linHistoButton, i18n( "<p>Linear" ) );
    d->scaleBG->insert(linHistoButton, HistogramWidget::LinScaleHistogram);
    KGlobal::dirs()->addResourceType("histogram-lin", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("histogram-lin", "histogram-lin.png");
    linHistoButton->setPixmap( QPixmap( directory + "histogram-lin.png" ) );
    linHistoButton->setToggleButton(true);

    QPushButton *logHistoButton = new QPushButton( d->scaleBG );
    QToolTip::add( logHistoButton, i18n( "<p>Logarithmic" ) );
    d->scaleBG->insert(logHistoButton, HistogramWidget::LogScaleHistogram);
    KGlobal::dirs()->addResourceType("histogram-log", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("histogram-log", "histogram-log.png");
    logHistoButton->setPixmap( QPixmap( directory + "histogram-log.png" ) );
    logHistoButton->setToggleButton(true);

    QLabel *label10 = new QLabel(i18n("Colors:"), histogramPage);
    label10->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    d->colorsCB = new QComboBox( false, histogramPage );
    d->colorsCB->insertItem( i18n("Red") );
    d->colorsCB->insertItem( i18n("Green") );
    d->colorsCB->insertItem( i18n("Blue") );
    d->colorsCB->setEnabled( false );
    QWhatsThis::add( d->colorsCB, i18n("<p>Select here the main color displayed with Colors Channel mode:<p>"
                                       "<b>Red</b>: Draw the red image channel in the foreground.<p>"
                                       "<b>Green</b>: Draw the green image channel in the foreground.<p>"
                                       "<b>Blue</b>: Draw the blue image channel in the foreground.<p>"));

    d->regionBG = new QHButtonGroup(histogramPage);
    d->regionBG->setExclusive(true);
    d->regionBG->setFrameShape(QFrame::NoFrame);
    d->regionBG->setInsideMargin( 0 );
    d->regionBG->hide();
    QWhatsThis::add( d->regionBG, i18n("<p>Select here from which region the histogram will be computed:<p>"
                                       "<b>Full Image</b>: Compute histogram using the full image.<p>"
                                       "<b>Selection</b>: Compute histogram using the current image "
                                       "selection."));

    QPushButton *fullImageButton = new QPushButton( d->regionBG );
    QToolTip::add( fullImageButton, i18n( "<p>Full Image" ) );
    d->regionBG->insert(fullImageButton, HistogramWidget::FullImageHistogram);
    KGlobal::dirs()->addResourceType("image-full", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("image-full", "image-full.png");
    fullImageButton->setPixmap( QPixmap( directory + "image-full.png" ) );
    fullImageButton->setToggleButton(true);

    QPushButton *SelectionImageButton = new QPushButton( d->regionBG );
    QToolTip::add( SelectionImageButton, i18n( "<p>Selection" ) );
    d->regionBG->insert(SelectionImageButton, HistogramWidget::ImageSelectionHistogram);
    KGlobal::dirs()->addResourceType("image-selection", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("image-selection", "image-selection.png");
    SelectionImageButton->setPixmap( QPixmap( directory + "image-selection.png" ) );
    SelectionImageButton->setToggleButton(true);

    topLayout->addMultiCellWidget(label1, 1, 1, 0, 0);
    topLayout->addMultiCellWidget(d->channelCB, 1, 1, 1, 1);
    topLayout->addMultiCellWidget(d->scaleBG, 1, 1, 3, 3);
    topLayout->addMultiCellWidget(label10, 2, 2, 0, 0);
    topLayout->addMultiCellWidget(d->colorsCB, 2, 2, 1, 1);
    topLayout->addMultiCellWidget(d->regionBG, 2, 2, 3, 3);
    topLayout->setColStretch(2, 10);

    // -------------------------------------------------------------

    d->histogramWidget = new HistogramWidget(256, 140, histogramPage);
    QWhatsThis::add( d->histogramWidget, i18n("<p>This is the histogram drawing of the "
                                              "selected image channel"));

    d->hGradient = new ColorGradientWidget( ColorGradientWidget::Horizontal, 10, histogramPage );
    d->hGradient->setColors( QColor( "black" ), QColor( "white" ) );
    topLayout->addMultiCellWidget(d->histogramWidget, 3, 3, 0, 3);
    topLayout->addMultiCellWidget(d->hGradient, 4, 4, 0, 3);

    // -------------------------------------------------------------

    QHBoxLayout *hlay2 = new QHBoxLayout(KDialog::spacingHint());
    QLabel *label3     = new QLabel(i18n("Range:"), histogramPage);
    label3->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->minInterv = new QSpinBox(0, 255, 1, histogramPage);
    d->minInterv->setValue(0);
    QWhatsThis::add(d->minInterv, i18n("<p>Select here the minimal intensity "
                                       "value of the histogram selection."));
    d->maxInterv = new QSpinBox(0, 255, 1, histogramPage);
    d->maxInterv->setValue(255);
    QWhatsThis::add(d->minInterv, i18n("<p>Select here the maximal intensity value "
                                       "of the histogram selection."));
    hlay2->addWidget(label3);
    hlay2->addWidget(d->minInterv);
    hlay2->addWidget(d->maxInterv);
    topLayout->addMultiCellLayout(hlay2, 5, 5, 0, 3);

    // -------------------------------------------------------------

    QGroupBox *gbox = new QGroupBox(2, Qt::Horizontal, i18n("Statistics"), histogramPage);
    QWhatsThis::add( gbox, i18n("<p>Here you can see the statistical results calculated from the "
                                "selected histogram part. These values are available for all "
                                "channels."));

    QLabel *label4 = new QLabel(i18n("Mean:"), gbox);
    label4->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    d->labelMeanValue = new QLabel(gbox);
    d->labelMeanValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);

    QLabel *label5 = new QLabel(i18n("Pixels:"), gbox);
    label5->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    d->labelPixelsValue = new QLabel(gbox);
    d->labelPixelsValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);

    QLabel *label6 = new QLabel(i18n("Standard deviation:"), gbox);
    label6->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    d->labelStdDevValue = new QLabel(gbox);
    d->labelStdDevValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);

    QLabel *label7 = new QLabel(i18n("Count:"), gbox);
    label7->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    d->labelCountValue = new QLabel(gbox);
    d->labelCountValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);

    QLabel *label8 = new QLabel(i18n("Median:"), gbox);
    label8->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    d->labelMedianValue = new QLabel(gbox);
    d->labelMedianValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);

    QLabel *label9 = new QLabel(i18n("Percentile:"), gbox);
    label9->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    d->labelPercentileValue = new QLabel(gbox);
    d->labelPercentileValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);

    topLayout->addMultiCellWidget(gbox, 6, 6, 0, 3);

    // -------------------------------------------------------------

    QGroupBox *gbox2 = new QGroupBox(2, Qt::Horizontal, histogramPage);
    gbox2->setFrameStyle( QFrame::NoFrame );

    QLabel *label11     = new QLabel(i18n("Color depth:"), gbox2);
    label11->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    d->labelColorDepth   = new QLabel(gbox2);
    d->labelColorDepth->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    QLabel *label12     = new QLabel(i18n("Alpha Channel:"), gbox2);
    label12->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    d->labelAlphaChannel = new QLabel(gbox2);
    d->labelAlphaChannel->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);

    topLayout->addMultiCellWidget(gbox2, 7, 7, 0, 3);

    topLayout->setRowStretch(8, 10);
    d->tab->insertTab(histogramPage, i18n("Histogram"), ImagePropertiesColorsTabPriv::HISTOGRAM );

    // ICC Profiles tab area ---------------------------------------

    d->iccProfileWidget = new ICCProfileWidget(d->tab);
    d->tab->insertTab(d->iccProfileWidget, i18n("ICC profile"), ImagePropertiesColorsTabPriv::ICCPROFILE);

    // -------------------------------------------------------------

    connect(d->navigateBar, SIGNAL(signalFirstItem()),
            this, SIGNAL(signalFirstItem()));

    connect(d->navigateBar, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));

    connect(d->navigateBar, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(d->navigateBar, SIGNAL(signalLastItem()),
            this, SIGNAL(signalLastItem()));

    // -------------------------------------------------------------

    connect(d->channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(d->scaleBG, SIGNAL(released(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(d->colorsCB, SIGNAL(activated(int)),
            this, SLOT(slotColorsChanged(int)));

    connect(d->regionBG, SIGNAL(released(int)),
            this, SLOT(slotRenderingChanged(int)));

    connect(d->histogramWidget, SIGNAL(signalIntervalChanged( int, int )),
            this, SLOT(slotUpdateInterval(int, int)));

    connect(d->histogramWidget, SIGNAL(signalMaximumValueChanged( int )),
            this, SLOT(slotUpdateIntervRange(int)));

    connect(d->histogramWidget, SIGNAL(signalHistogramComputationDone(bool)),
            this, SLOT(slotRefreshOptions(bool)));

    connect(d->histogramWidget, SIGNAL(signalHistogramComputationFailed(void)),
            this, SLOT(slotHistogramComputationFailed(void)));

    connect(d->minInterv, SIGNAL(valueChanged (int)),
            this, SLOT(slotMinValueChanged(int)));

    connect(d->maxInterv, SIGNAL(valueChanged (int)),
            this, SLOT(slotMaxValueChanged(int)));

    // -- read config ---------------------------------------------------------

    KConfig* config = kapp->config();
    config->setGroup("Image Properties SideBar");
    d->tab->setCurrentPage(config->readNumEntry("ImagePropertiesColors Tab",
                           ImagePropertiesColorsTabPriv::HISTOGRAM));
    d->iccProfileWidget->setMode(config->readNumEntry("ICC Level", ICCProfileWidget::SIMPLE));
    d->iccProfileWidget->setCurrentItemByKey(config->readEntry("Current ICC Item", QString()));

    d->channelCB->setCurrentItem(config->readNumEntry("Histogram Channel", 0));    // Luminosity.
    d->scaleBG->setButton(config->readNumEntry("Histogram Scale", HistogramWidget::LogScaleHistogram));
    d->colorsCB->setCurrentItem(config->readNumEntry("Histogram Color", 0));       // Red.
    d->regionBG->setButton(config->readNumEntry("Histogram Rendering", HistogramWidget::FullImageHistogram));
}

ImagePropertiesColorsTab::~ImagePropertiesColorsTab()
{
    // If there is a currently histogram computation when dialog is closed,
    // stop it before the d->image data are deleted automaticly!
    d->histogramWidget->stopHistogramComputation();

    KConfig* config = kapp->config();
    config->setGroup("Image Properties SideBar");
    config->writeEntry("ImagePropertiesColors Tab", d->tab->currentPageIndex());
    config->writeEntry("Histogram Channel", d->channelCB->currentItem());
    config->writeEntry("Histogram Scale", d->scaleBG->selectedId());
    config->writeEntry("Histogram Color", d->colorsCB->currentItem());
    config->writeEntry("Histogram Rendering", d->regionBG->selectedId());
    config->writeEntry("ICC Level", d->iccProfileWidget->getMode());
    config->writeEntry("Current ICC Item", d->iccProfileWidget->getCurrentItemKey());
    config->sync();

    if (d->imageLoaderThread)
       delete d->imageLoaderThread;

    if (d->histogramWidget)
       delete d->histogramWidget;

    if (d->hGradient)
       delete d->hGradient;

    delete d;
}

void ImagePropertiesColorsTab::setData(const KURL& url, QRect *selectionArea,
                                       DImg *img, int itemType)
{
    // We might be getting duplicate events from AlbumIconView,
    // which will cause all sorts of duplicate work.
    // More importantly, while the loading thread can handle this pretty well,
    // this will completely mess up the timing of progress info in the histogram widget.
    // So filter here, before the stopHistogramComputation!
    if (!img && url.path() == d->currentFilePath)
        return;

    // This is necessary to stop computation because d->image.bits() is currently used by
    // threaded histogram algorithm.
    d->histogramWidget->stopHistogramComputation();

    d->navigateBar->setFileName();
    d->currentFilePath = QString();
    d->currentLoadingDescription = LoadingDescription();
    d->iccProfileWidget->loadFromURL(KURL());

    // Clear informations.
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
       return;
    }

    d->navigateBar->setFileName(url.filename());
    d->navigateBar->setButtonsState(itemType);
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
            if (d->selectionArea)
            {
                d->imageSelection = d->image.copy(*d->selectionArea);
                d->histogramWidget->updateData(d->image.bits(), d->image.width(), d->image.height(),
                                               d->image.sixteenBit(), d->imageSelection.bits(),
                                               d->imageSelection.width(), d->imageSelection.height());
                d->regionBG->show();
                updateInformations();
            }
            else 
            {
                d->histogramWidget->updateData(d->image.bits(), d->image.width(), 
                                               d->image.height(), d->image.sixteenBit());
                d->regionBG->hide();
                updateInformations();
            }
        }
        else 
        {
            d->histogramWidget->setLoadingFailed();
            d->iccProfileWidget->setLoadingFailed();
            slotHistogramComputationFailed();
        }
    }
}

void ImagePropertiesColorsTab::loadImageFromUrl(const KURL& url)
{
    // create thread on demand
    if (!d->imageLoaderThread)
    {
        d->imageLoaderThread = new SharedLoadSaveThread();

        connect(d->imageLoaderThread, SIGNAL(signalImageLoaded(const LoadingDescription &, const DImg&)),
                this, SLOT(slotLoadImageFromUrlComplete(const LoadingDescription &, const DImg&)));

        connect(d->imageLoaderThread, SIGNAL(signalMoreCompleteLoadingAvailable(const LoadingDescription &, const LoadingDescription &)),
                this, SLOT(slotMoreCompleteLoadingAvailable(const LoadingDescription &, const LoadingDescription &)));
    }

    LoadingDescription desc = LoadingDescription(url.path());

    if (DImg::fileFormat(desc.filePath) == DImg::RAW)
    {
        // use raw settings optimized for speed

        RawDecodingSettings rawDecodingSettings = RawDecodingSettings();
        rawDecodingSettings.optimizeTimeLoading();
        desc = LoadingDescription(desc.filePath, rawDecodingSettings);
    }

    if (d->currentLoadingDescription.equalsOrBetterThan(desc))
        return;

    d->currentFilePath = desc.filePath;
    d->currentLoadingDescription = desc;

    d->imageLoaderThread->load(d->currentLoadingDescription,
                               SharedLoadSaveThread::AccessModeRead,
                               SharedLoadSaveThread::LoadingPolicyFirstRemovePrevious);

    d->histogramWidget->setDataLoading();
    d->iccProfileWidget->setDataLoading();
}

void ImagePropertiesColorsTab::slotLoadImageFromUrlComplete(const LoadingDescription &loadingDescription, const DImg& img)
{
    // Discard any leftover messages from previous, possibly aborted loads
    if ( !loadingDescription.equalsOrBetterThan(d->currentLoadingDescription) )
        return;

    if ( !img.isNull() )
    {
        d->histogramWidget->updateData(img.bits(), img.width(), img.height(), img.sixteenBit());

        // As a safety precaution, this must be changed only after updateData is called,
        // which stops computation because d->image.bits() is currently used by threaded histogram algorithm.
        d->image = img;
        d->regionBG->hide();
        updateInformations();
        getICCData();
    }
    else
    {
        d->histogramWidget->setLoadingFailed();
        d->iccProfileWidget->setLoadingFailed();
        slotHistogramComputationFailed();
    }
}

void ImagePropertiesColorsTab::slotMoreCompleteLoadingAvailable(const LoadingDescription &oldLoadingDescription,
                                                                const LoadingDescription &newLoadingDescription)
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

void ImagePropertiesColorsTab::setSelection(QRect *selectionArea)
{
    // This is necessary to stop computation because d->image.bits() is currently used by
    // threaded histogram algorithm.

    d->histogramWidget->stopHistogramComputation();
    d->selectionArea = selectionArea;

    if (d->selectionArea)
    {
        d->imageSelection = d->image.copy(*d->selectionArea);
        d->histogramWidget->updateSelectionData(d->imageSelection.bits(), d->imageSelection.width(),
                                                d->imageSelection.height(), d->imageSelection.sixteenBit());
        d->regionBG->show();
    }
    else 
    {
        d->regionBG->hide();
        slotRenderingChanged(HistogramWidget::FullImageHistogram);
    }
}

void ImagePropertiesColorsTab::slotRefreshOptions(bool /*sixteenBit*/)
{
    slotChannelChanged(d->channelCB->currentItem());
    slotScaleChanged(d->scaleBG->selectedId());
    slotColorsChanged(d->colorsCB->currentItem());

    if (d->selectionArea)
       slotRenderingChanged(d->regionBG->selectedId());
}

void ImagePropertiesColorsTab::slotHistogramComputationFailed()
{
    d->imageSelection.reset();
    d->image.reset();
}

void ImagePropertiesColorsTab::slotChannelChanged(int channel)
{
    switch(channel)
    {
    case RedChannel: 
        d->histogramWidget->m_channelType = HistogramWidget::RedChannelHistogram;
        d->hGradient->setColors( QColor( "black" ), QColor( "red" ) );
        d->colorsCB->setEnabled(false);
        break;

    case GreenChannel:
        d->histogramWidget->m_channelType = HistogramWidget::GreenChannelHistogram;
        d->hGradient->setColors( QColor( "black" ), QColor( "green" ) );
        d->colorsCB->setEnabled(false);
        break;

    case BlueChannel:
        d->histogramWidget->m_channelType = HistogramWidget::BlueChannelHistogram;
        d->hGradient->setColors( QColor( "black" ), QColor( "blue" ) );
        d->colorsCB->setEnabled(false);
        break;

    case AlphaChannel:
        d->histogramWidget->m_channelType = HistogramWidget::AlphaChannelHistogram;
        d->hGradient->setColors( QColor( "black" ), QColor( "white" ) );
        d->colorsCB->setEnabled(false);
        break;

    case ColorChannels:
        d->histogramWidget->m_channelType = HistogramWidget::ColorChannelsHistogram;
        d->hGradient->setColors( QColor( "black" ), QColor( "white" ) );
        d->colorsCB->setEnabled(true);
        break;

    default:          // Luminosity.
        d->histogramWidget->m_channelType = HistogramWidget::ValueHistogram;
        d->hGradient->setColors( QColor( "black" ), QColor( "white" ) );
        d->colorsCB->setEnabled(false);
        break;
    }

    d->histogramWidget->repaint(false);
    updateStatistiques();
}

void ImagePropertiesColorsTab::slotScaleChanged(int scale)
{
    d->histogramWidget->m_scaleType = scale;
    d->histogramWidget->repaint(false);
}

void ImagePropertiesColorsTab::slotColorsChanged(int color)
{
    switch(color)
    {
    case AllColorsGreen:
        d->histogramWidget->m_colorType = HistogramWidget::GreenColor;
        break;

    case AllColorsBlue:
        d->histogramWidget->m_colorType = HistogramWidget::BlueColor;
        break;

    default:          // Red.
        d->histogramWidget->m_colorType = HistogramWidget::RedColor;
        break;
    }

    d->histogramWidget->repaint(false);
    updateStatistiques();
}

void ImagePropertiesColorsTab::slotRenderingChanged(int rendering)
{
    d->histogramWidget->m_renderingType = rendering;
    d->histogramWidget->repaint(false);
    updateStatistiques();
}

void ImagePropertiesColorsTab::slotMinValueChanged(int min)
{
    // Called when user changes values of spin box.
    // Communicate the change to histogram widget.

    // make the one control "push" the other
    if (min == d->maxInterv->value()+1)
        d->maxInterv->setValue(min);
    d->maxInterv->setMinValue(min-1);
    d->histogramWidget->slotMinValueChanged(min);
    updateStatistiques();
}

void ImagePropertiesColorsTab::slotMaxValueChanged(int max)
{
    if (max == d->minInterv->value()-1)
        d->minInterv->setValue(max);
    d->minInterv->setMaxValue(max+1);
    d->histogramWidget->slotMaxValueChanged(max);
    updateStatistiques();
}

void ImagePropertiesColorsTab::slotUpdateInterval(int min, int max)
{
    // Called when value is set from within histogram widget.
    // Block signals to prevent slotMinValueChanged and
    // slotMaxValueChanged being called. 
    d->minInterv->blockSignals(true);
    d->minInterv->setMaxValue(max+1);
    d->minInterv->setValue(min);
    d->minInterv->blockSignals(false);

    d->maxInterv->blockSignals(true);
    d->maxInterv->setMinValue(min-1);
    d->maxInterv->setValue(max);
    d->maxInterv->blockSignals(false);

    updateStatistiques();
}

void ImagePropertiesColorsTab::slotUpdateIntervRange(int range)
{
    d->maxInterv->setMaxValue( range );
}

void ImagePropertiesColorsTab::updateInformations()
{
    d->labelColorDepth->setText(d->image.sixteenBit() ? i18n("16 bits") : i18n("8 bits"));
    d->labelAlphaChannel->setText(d->image.hasAlpha() ? i18n("Yes") : i18n("No"));
}

void ImagePropertiesColorsTab::updateStatistiques()
{
    QString value;
    int min = d->minInterv->value();
    int max = d->maxInterv->value();
    int channel = d->channelCB->currentItem();

    if ( channel == HistogramWidget::ColorChannelsHistogram )
        channel = d->colorsCB->currentItem()+1;

    double mean = d->histogramWidget->m_imageHistogram->getMean(channel, min, max);
    d->labelMeanValue->setText(value.setNum(mean, 'f', 1));

    double pixels = d->histogramWidget->m_imageHistogram->getPixels();
    d->labelPixelsValue->setText(value.setNum((float)pixels, 'f', 0));

    double stddev = d->histogramWidget->m_imageHistogram->getStdDev(channel, min, max);
    d->labelStdDevValue->setText(value.setNum(stddev, 'f', 1));

    double counts = d->histogramWidget->m_imageHistogram->getCount(channel, min, max);
    d->labelCountValue->setText(value.setNum((float)counts, 'f', 0));

    double median = d->histogramWidget->m_imageHistogram->getMedian(channel, min, max);
    d->labelMedianValue->setText(value.setNum(median, 'f', 1));

    double percentile = (pixels > 0 ? (100.0 * counts / pixels) : 0.0);
    d->labelPercentileValue->setText(value.setNum(percentile, 'f', 1));
}

void ImagePropertiesColorsTab::getICCData()
{
    if (d->image.getICCProfil().isNull())
    {
        d->iccProfileWidget->setLoadingFailed();
    }
    else
    {
        d->embedded_profile = d->image.getICCProfil();
        d->iccProfileWidget->loadFromData(d->currentFilePath, d->embedded_profile);
    }
}

}  // NameSpace Digikam

#include "imagepropertiescolorstab.moc"
