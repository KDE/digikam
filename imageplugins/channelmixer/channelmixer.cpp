/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-26
 * Description : image channels mixer.
 *
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * Load and save mixer gains methods inspired from
 * Gimp 2.2.3 and copyrighted 2002 by Martin Guldahl
 * <mguldahl at xmission dot com>.
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

#include <cstdio>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <cerrno>

// Qt includes.

#include <QColor>
#include <QGroupBox>
#include <QButtonGroup>
#include <QLabel>
#include <QPainter>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QFrame>
#include <QTimer>
#include <QCheckBox>
#include <QFile>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPixmap>

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
#include <kvbox.h>

// Local includes.

#include "version.h"
#include "daboutdata.h"
#include "dimg.h"
#include "ddebug.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "imagehistogram.h"
#include "histogramwidget.h"
#include "colorgradientwidget.h"
#include "dimgimagefilters.h"
#include "channelmixer.h"
#include "channelmixer.moc"

namespace DigikamChannelMixerImagesPlugin
{

ChannelMixerDialog::ChannelMixerDialog(QWidget* parent)
                  : Digikam::ImageDlgBase(parent, i18n("Color Channel Mixer"),
                    "channelmixer", true, false)
{
    m_destinationPreviewData = 0L;

    // About data and help button.

    KAboutData* about = new KAboutData("digikam", 0,
                                       ki18n("Color Channel Mixer"),
                                       digiKamVersion().toAscii(),
                                       ki18n("An image color channel mixer plugin for digiKam."),
                                       KAboutData::License_GPL,
                                       ki18n("(c) 2005-2007, Gilles Caulier"),
                                       KLocalizedString(),
                                       Digikam::webProjectUrl().url().toUtf8());

    about->addAuthor(ki18n("Gilles Caulier"), ki18n("Author and maintainer"),
                     "caulier dot gilles at gmail dot com");

    setAboutData(about);

    // -------------------------------------------------------------

    m_previewWidget = new Digikam::ImageWidget("channelmixer Tool Dialog", mainWidget(),
                          i18n("<p>You can see here the image's color channels' "
                               "gains adjustments preview. You can pick color on image "
                               "to see the color level corresponding on histogram."));
    setPreviewAreaWidget(m_previewWidget);

    // -------------------------------------------------------------

    QWidget *gboxSettings = new QWidget(mainWidget());
    QGridLayout* grid     = new QGridLayout(gboxSettings);

    QLabel *label1 = new QLabel(i18n("Channel:"), gboxSettings);
    label1->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    m_channelCB = new QComboBox( gboxSettings );
    m_channelCB->addItem( i18n("Red") );
    m_channelCB->addItem( i18n("Green") );
    m_channelCB->addItem( i18n("Blue") );
    m_channelCB->setCurrentIndex( 0 );
    m_channelCB->setWhatsThis( i18n("<p>Select the color channel to mix here:<p>"
                                    "<b>Red</b>: display the red image-channel values.<p>"
                                    "<b>Green</b>: display the green image-channel values.<p>"
                                    "<b>Blue</b>: display the blue image-channel values.<p>"));

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
    linHistoButton->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/histogram-lin.png")));
    linHistoButton->setCheckable(true);
    m_scaleBG->addButton(linHistoButton, Digikam::HistogramWidget::LinScaleHistogram);

    QPushButton *logHistoButton = new QPushButton( scaleBox );
    logHistoButton->setToolTip( i18n( "<p>Logarithmic" ) );
    logHistoButton->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/histogram-log.png")));
    logHistoButton->setCheckable(true);
    m_scaleBG->addButton(logHistoButton, Digikam::HistogramWidget::LogScaleHistogram);

    hlay->setMargin(0);
    hlay->setSpacing(0);
    hlay->addWidget(linHistoButton);
    hlay->addWidget(logHistoButton);

    m_scaleBG->setExclusive(true);
    logHistoButton->setChecked(true);

    QHBoxLayout* l1 = new QHBoxLayout();
    l1->addWidget(label1);
    l1->addWidget(m_channelCB);
    l1->addStretch(10);
    l1->addWidget(scaleBox);

    // -------------------------------------------------------------

    KVBox *histoBox   = new KVBox(gboxSettings);
    m_histogramWidget = new Digikam::HistogramWidget(256, 140, histoBox, false, true, true);
    m_histogramWidget->setWhatsThis( i18n("<p>Here you can see the target preview image histogram drawing "
                                          "of the selected image channel. This one is re-computed at any "
                                          "mixer settings changes."));
    QLabel *space = new QLabel(histoBox);
    space->setFixedHeight(1);
    m_hGradient = new Digikam::ColorGradientWidget( Digikam::ColorGradientWidget::Horizontal, 10, histoBox );
    m_hGradient->setColors( QColor( "black" ), QColor( "red" ) );

    // -------------------------------------------------------------

    QLabel *redLabel = new QLabel(i18n("Red:"), gboxSettings);
    m_redGain = new KDoubleNumInput(gboxSettings);
    m_redGain->setDecimals(0);
    m_redGain->setRange(-200.0, 200.0, 1, true);
    m_redGain->setWhatsThis( i18n("<p>Select the red color gain in percent for "
                                  "the current channel here."));

    QLabel *blueLabel = new QLabel(i18n("Blue:"), gboxSettings);
    m_greenGain = new KDoubleNumInput(gboxSettings);
    m_greenGain->setDecimals(0);
    m_greenGain->setRange(-200.0, 200.0, 1, true);
    m_greenGain->setWhatsThis( i18n("<p>Select the green color gain in percent "
                                    "for the current channel here."));

    QLabel *greenLabel = new QLabel(i18n("Green:"), gboxSettings);
    m_blueGain = new KDoubleNumInput(gboxSettings);
    m_blueGain->setDecimals(0);
    m_blueGain->setRange(-200.0, 200.0, 1, true);
    m_blueGain->setWhatsThis( i18n("<p>Select the blue color gain in percent for "
                                   "the current channel here."));

    m_resetButton = new QPushButton(i18n("&Reset"), gboxSettings);
    m_resetButton->setIcon(KIconLoader::global()->loadIcon("document-revert", KIconLoader::Toolbar));
    m_resetButton->setWhatsThis( i18n("Reset color channels' gains settings from "
                                      "the currently selected channel."));

    // -------------------------------------------------------------

    m_monochrome = new QCheckBox( i18n("Monochrome"), gboxSettings);
    m_monochrome->setWhatsThis( i18n("<p>Enable this option if you want the image rendered "
                                     "in monochrome mode. "
                                     "In this mode, the histogram will display only luminosity values."));

    m_preserveLuminosity = new QCheckBox( i18n("Preserve luminosity"), gboxSettings);
    m_preserveLuminosity->setWhatsThis( i18n("<p>Enable this option is you want preserve "
                                             "the image luminosity."));

    // -------------------------------------------------------------

    grid->addLayout(l1, 0, 0, 1, 5 );
    grid->addWidget(histoBox, 1, 0, 2, 5 );
    grid->addWidget(redLabel, 3, 0, 1, 1);
    grid->addWidget(greenLabel, 4, 0, 1, 1);
    grid->addWidget(blueLabel, 5, 0, 1, 1);
    grid->addWidget(m_redGain, 3, 1, 1, 4);
    grid->addWidget(m_greenGain, 4, 1, 1, 4);
    grid->addWidget(m_blueGain, 5, 1, 1, 4);
    grid->addWidget(m_resetButton, 6, 0, 1, 2 );
    grid->addWidget(m_monochrome, 7, 0, 1, 5 );
    grid->addWidget(m_preserveLuminosity, 8, 0, 1, 5 );
    grid->setRowStretch(9, 10);
    grid->setMargin(spacingHint());
    grid->setSpacing(spacingHint());

    setUserAreaWidget(gboxSettings);

    // -------------------------------------------------------------
    // Channels and scale selection slots.

    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(m_scaleBG, SIGNAL(buttonReleased(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(m_previewWidget, SIGNAL(spotPositionChangedFromTarget( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotColorSelectedFromTarget( const Digikam::DColor & )));

    connect(m_previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));

    // -------------------------------------------------------------
    // Gains settings slots.

    connect(m_redGain, SIGNAL(valueChanged(double)),
            this, SLOT(slotGainsChanged()));

    connect(m_greenGain, SIGNAL(valueChanged(double)),
            this, SLOT(slotGainsChanged()));

    connect(m_blueGain, SIGNAL(valueChanged(double)),
            this, SLOT(slotGainsChanged()));

    connect(m_preserveLuminosity, SIGNAL(toggled (bool)),
            this, SLOT(slotEffect()));

    connect(m_monochrome, SIGNAL(toggled (bool)),
            this, SLOT(slotMonochromeActived(bool)));

    // -------------------------------------------------------------
    // Bouttons slots.

    connect(m_resetButton, SIGNAL(clicked()),
            this, SLOT(slotResetCurrentChannel()));
}

ChannelMixerDialog::~ChannelMixerDialog()
{
    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData)
       delete [] m_destinationPreviewData;

    delete m_histogramWidget;
}

void ChannelMixerDialog::slotResetCurrentChannel()
{
    switch( m_channelCB->currentIndex() )
    {
       case GreenChannelGains:           // Green.
          m_greenRedGain   = 0.0;
          m_greenGreenGain = 1.0;
          m_greenBlueGain  = 0.0;
          break;

       case BlueChannelGains:            // Blue.
          m_blueRedGain   = 0.0;
          m_blueGreenGain = 0.0;
          m_blueBlueGain  = 1.0;
          break;

       default:          // Red or monochrome.
          if ( m_monochrome->isChecked() )
          {
             m_blackRedGain   = 1.0;
             m_blackGreenGain = 0.0;
             m_blackBlueGain  = 0.0;
          }
          else
          {
             m_redRedGain   = 1.0;
             m_redGreenGain = 0.0;
             m_redBlueGain  = 0.0;
          }
          break;
    }

    adjustSliders();
    slotEffect();
    m_histogramWidget->reset();
}

void ChannelMixerDialog::slotColorSelectedFromTarget( const Digikam::DColor &color )
{
    m_histogramWidget->setHistogramGuideByColor(color);
}

void ChannelMixerDialog::slotGainsChanged()
{
    switch( m_channelCB->currentIndex() )
    {
       case GreenChannelGains:           // Green.
          m_greenRedGain   = m_redGain->value()   / 100.0;
          m_greenGreenGain = m_greenGain->value() / 100.0;
          m_greenBlueGain  = m_blueGain->value()  / 100.0;
          break;

       case BlueChannelGains:            // Blue.
          m_blueRedGain   = m_redGain->value()   / 100.0;
          m_blueGreenGain = m_greenGain->value() / 100.0;
          m_blueBlueGain  = m_blueGain->value()  / 100.0;
          break;

       default:          // Red or monochrome.
          if ( m_monochrome->isChecked() )
          {
             m_blackRedGain   = m_redGain->value()   / 100.0;
             m_blackGreenGain = m_greenGain->value() / 100.0;
             m_blackBlueGain  = m_blueGain->value()  / 100.0;
          }
          else
          {
             m_redRedGain   = m_redGain->value()   / 100.0;
             m_redGreenGain = m_greenGain->value() / 100.0;
             m_redBlueGain  = m_blueGain->value()  / 100.0;
          }
          break;
    }

    slotTimer();
}

void ChannelMixerDialog::adjustSliders(void)
{
    m_redGain->blockSignals(true);
    m_greenGain->blockSignals(true);
    m_blueGain->blockSignals(true);

    switch( m_channelCB->currentIndex() )
    {
       case GreenChannelGains:           // Green.
          m_redGain->setValue(m_greenRedGain     * 100.0);
          m_greenGain->setValue(m_greenGreenGain * 100.0);
          m_blueGain->setValue(m_greenBlueGain   * 100.0);
          break;

       case BlueChannelGains:            // Blue.
          m_redGain->setValue(m_blueRedGain     * 100.0);
          m_greenGain->setValue(m_blueGreenGain * 100.0);
          m_blueGain->setValue(m_blueBlueGain   * 100.0);
          break;

       default:          // Red or monochrome.
          if ( m_monochrome->isChecked() )
          {
             m_redGain->setValue(m_blackRedGain     * 100.0);
             m_greenGain->setValue(m_blackGreenGain * 100.0);
             m_blueGain->setValue(m_blackBlueGain   * 100.0);
          }
          else
          {
             m_redGain->setValue(m_redRedGain     * 100.0);
             m_greenGain->setValue(m_redGreenGain * 100.0);
             m_blueGain->setValue(m_redBlueGain   * 100.0);
          }
          break;
    }

    m_redGain->blockSignals(false);
    m_greenGain->blockSignals(false);
    m_blueGain->blockSignals(false);
}

void ChannelMixerDialog::slotMonochromeActived(bool mono)
{
    m_channelCB->setEnabled(!mono);
    m_channelCB->setCurrentIndex(RedChannelGains); // Red for monochrome.
    slotChannelChanged(RedChannelGains);          // Monochrome => display luminosity histogram value.
}

void ChannelMixerDialog::slotEffect()
{
    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    uchar *data                = iface->getPreviewImage();
    int w                      = iface->previewWidth();
    int h                      = iface->previewHeight();
    bool sb                    = iface->previewSixteenBit();

    // Create the new empty destination image data space.
    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData)
       delete [] m_destinationPreviewData;

    m_destinationPreviewData = new uchar[w*h*(sb ? 8 : 4)];
    Digikam::DImgImageFilters filter;

    if (m_monochrome->isChecked())
    {
       filter.channelMixerImage(data, w, h, sb,                                 // Image data.
                m_preserveLuminosity->isChecked(),                              // Preserve luminosity.
                m_monochrome->isChecked(),                                      // Monochrome.
                m_blackRedGain, m_blackGreenGain, m_blackBlueGain,              // Red channel gains.
                0.0,            1.0,              0.0,                          // Green channel gains (not used).
                0.0,            0.0,              1.0);                         // Blue channel gains (not used).
    }
    else
    {
       filter.channelMixerImage(data, w, h, sb,                                 // Image data.
                m_preserveLuminosity->isChecked(),                              // Preserve luminosity.
                m_monochrome->isChecked(),                                      // Monochrome.
                m_redRedGain,   m_redGreenGain,   m_redBlueGain,                // Red channel gains.
                m_greenRedGain, m_greenGreenGain, m_greenBlueGain,              // Green channel gains.
                m_blueRedGain,  m_blueGreenGain,  m_blueBlueGain);              // Blue channel gains.
    }

    iface->putPreviewImage(data);
    m_previewWidget->updatePreview();

    // Update histogram.
    memcpy (m_destinationPreviewData, data, w*h*(sb ? 8 : 4));
    m_histogramWidget->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);
    delete [] data;
}

void ChannelMixerDialog::finalRendering()
{
    kapp->setOverrideCursor( Qt::WaitCursor );
    Digikam::ImageIface* iface = m_previewWidget->imageIface();
    uchar *data                = iface->getOriginalImage();
    int w                      = iface->originalWidth();
    int h                      = iface->originalHeight();
    bool sb                    = iface->originalSixteenBit();

    Digikam::DImgImageFilters filter;

    if (m_monochrome->isChecked())
    {
       filter.channelMixerImage(data, w, h, sb,                     // Image data.
                m_preserveLuminosity->isChecked(),                  // Preserve luminosity.
                m_monochrome->isChecked(),                          // Monochrome.
                m_blackRedGain, m_blackGreenGain, m_blackBlueGain,  // Red channel gains.
                0.0,            1.0,              0.0,              // Green channel gains (not used).
                0.0,            0.0,              1.0);             // Blue channel gains (not used).
    }
    else
    {
       filter.channelMixerImage(data, w, h, sb,                     // Image data.
                m_preserveLuminosity->isChecked(),                  // Preserve luminosity.
                m_monochrome->isChecked(),                          // Monochrome.
                m_redRedGain,   m_redGreenGain,   m_redBlueGain,    // Red channel gains.
                m_greenRedGain, m_greenGreenGain, m_greenBlueGain,  // Green channel gains.
                m_blueRedGain,  m_blueGreenGain,  m_blueBlueGain);  // Blue channel gains.
    }

    iface->putOriginalImage(i18n("Channel Mixer"), data);
    delete [] data;
    kapp->restoreOverrideCursor();
    accept();
}

void ChannelMixerDialog::slotChannelChanged(int channel)
{
    switch(channel)
    {
       case GreenChannelGains:           // Green.
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::GreenChannelHistogram;
          m_hGradient->setColors( QColor( "black" ), QColor( "green" ) );
          break;

       case BlueChannelGains:            // Blue.
          m_histogramWidget->m_channelType = Digikam::HistogramWidget::BlueChannelHistogram;
          m_hGradient->setColors( QColor( "black" ), QColor( "blue" ) );
          break;

       default:          // Red or monochrome.
          if ( m_monochrome->isChecked() )
          {
             m_histogramWidget->m_channelType = Digikam::HistogramWidget::ValueHistogram;
             m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
          }
          else
          {
             m_histogramWidget->m_channelType = Digikam::HistogramWidget::RedChannelHistogram;
             m_hGradient->setColors( QColor( "black" ), QColor( "red" ) );
          }
          break;
    }

    m_histogramWidget->repaint();
    adjustSliders();
    slotEffect();
}

void ChannelMixerDialog::slotScaleChanged(int scale)
{
    m_histogramWidget->m_scaleType = scale;
    m_histogramWidget->repaint();
}

void ChannelMixerDialog::readUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("channelmixer Tool Dialog");

    m_channelCB->setCurrentIndex(group.readEntry("Histogram Channel", 0));    // Luminosity.
    m_scaleBG->button(group.readEntry("Histogram Scale",
                      (int)Digikam::HistogramWidget::LogScaleHistogram))->setChecked(true);

    m_monochrome->setChecked(group.readEntry("Monochrome", false));
    m_preserveLuminosity->setChecked(group.readEntry("PreserveLuminosity", false));

    m_redRedGain     = group.readEntry("RedRedGain", 1.0);
    m_redGreenGain   = group.readEntry("RedGreenGain", 0.0);
    m_redBlueGain    = group.readEntry("RedBlueGain", 0.0);

    m_greenRedGain   = group.readEntry("GreenRedGain", 0.0);
    m_greenGreenGain = group.readEntry("GreenGreenGain", 1.0);
    m_greenBlueGain  = group.readEntry("GreenBlueGain", 0.0);

    m_blueRedGain    = group.readEntry("BlueRedGain", 0.0);
    m_blueGreenGain  = group.readEntry("BlueGreenGain", 0.0);
    m_blueBlueGain   = group.readEntry("BlueBlueGain", 1.0);

    m_blackRedGain   = group.readEntry("BlackRedGain", 1.0);
    m_blackGreenGain = group.readEntry("BlackGreenGain", 0.0);
    m_blackBlueGain  = group.readEntry("BlackBlueGain", 0.0);

    adjustSliders();
    m_histogramWidget->reset();

    slotChannelChanged(m_channelCB->currentIndex());
    slotScaleChanged(m_scaleBG->checkedId());
}

void ChannelMixerDialog::writeUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("channelmixer Tool Dialog");
    group.writeEntry("Histogram Channel", m_channelCB->currentIndex());
    group.writeEntry("Histogram Scale", m_scaleBG->checkedId());

    group.writeEntry("Monochrome", m_monochrome->isChecked());
    group.writeEntry("PreserveLuminosity", m_preserveLuminosity->isChecked());

    group.writeEntry("RedRedGain", m_redRedGain);
    group.writeEntry("RedGreenGain", m_redGreenGain);
    group.writeEntry("RedBlueGain", m_redBlueGain);

    group.writeEntry("GreenRedGain", m_greenRedGain);
    group.writeEntry("GreenGreenGain", m_greenGreenGain);
    group.writeEntry("GreenBlueGain", m_greenBlueGain);

    group.writeEntry("BlueRedGain", m_blueRedGain);
    group.writeEntry("BlueGreenGain", m_blueGreenGain);
    group.writeEntry("BlueBlueGain", m_blueBlueGain);

    group.writeEntry("BlackRedGain", m_blackRedGain);
    group.writeEntry("BlackGreenGain", m_blackGreenGain);
    group.writeEntry("BlackBlueGain", m_blackBlueGain);

    config->sync();
}

void ChannelMixerDialog::resetValues()
{
    m_monochrome->blockSignals(true);
    m_preserveLuminosity->blockSignals(true);

    m_redRedGain     = 1.0;
    m_redGreenGain   = 0.0;
    m_redBlueGain    = 0.0;

    m_greenRedGain   = 0.0;
    m_greenGreenGain = 1.0;
    m_greenBlueGain  = 0.0;

    m_blueRedGain    = 0.0;
    m_blueGreenGain  = 0.0;
    m_blueBlueGain   = 1.0;

    m_blackRedGain   = 1.0;
    m_blackGreenGain = 0.0;
    m_blackBlueGain  = 0.0;

    adjustSliders();

    m_monochrome->blockSignals(false);
    m_preserveLuminosity->blockSignals(false);
    m_channelCB->setEnabled(true);

    m_histogramWidget->reset();

    slotChannelChanged(RedChannelGains);
}

// Load all gains.
void ChannelMixerDialog::slotUser3()
{
    KUrl loadGainsFileUrl;
    FILE *fp = 0L;
    int currentOutputChannel;
    bool monochrome;

    loadGainsFileUrl = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                                            QString( "*" ), this,
                                            QString( i18n("Select Gimp Gains Mixer File to Load")) );
    if( loadGainsFileUrl.isEmpty() )
       return;

    fp = fopen(QFile::encodeName(loadGainsFileUrl.path()), "r");

    if ( fp )
    {
        char buf1[1024];
        char buf2[1024];
        char buf3[1024];

        buf1[0] = '\0';

        fgets(buf1, 1023, fp);

        fscanf (fp, "%*s %s", buf1);

        // Get the current output channel in dialog.

        if (strcmp (buf1, "RED") == 0)
            currentOutputChannel = RedChannelGains;
        else if (strcmp (buf1, "GREEN") == 0)
            currentOutputChannel = GreenChannelGains;
        else if (strcmp (buf1, "BLUE") == 0)
            currentOutputChannel = BlueChannelGains;

        fscanf (fp, "%*s %s", buf1); // preview flag, preserved for compatibility

        fscanf (fp, "%*s %s", buf1);

        if (strcmp (buf1, "true") == 0)
            monochrome = true;
        else
            monochrome = false;

        fscanf (fp, "%*s %s", buf1);

        if (strcmp (buf1, "true") == 0)
            m_preserveLuminosity->setChecked(true);
        else
            m_preserveLuminosity->setChecked(false);

        fscanf (fp, "%*s %s %s %s", buf1, buf2, buf3);
        m_redRedGain = atof(buf1);
        m_redGreenGain = atof(buf2);
        m_redBlueGain = atof(buf3);

        fscanf (fp, "%*s %s %s %s", buf1, buf2, buf3);
        m_greenRedGain = atof(buf1);
        m_greenGreenGain = atof(buf2);
        m_greenBlueGain = atof(buf3);

        fscanf (fp, "%*s %s %s %s", buf1, buf2, buf3);
        m_blueRedGain = atof(buf1);
        m_blueGreenGain = atof(buf2);
        m_blueBlueGain = atof(buf3);

        fscanf (fp, "%*s %s %s %s", buf1, buf2, buf3);
        m_blackRedGain = atof(buf1);
        m_blackGreenGain = atof(buf2);
        m_blackBlueGain = atof(buf3);

        fclose(fp);

        // Refresh settings.
        m_monochrome->setChecked(monochrome);
        m_channelCB->setCurrentIndex(currentOutputChannel);
        slotChannelChanged(currentOutputChannel);
    }
    else
    {
        KMessageBox::error(this, i18n("Cannot load settings from the Gains Mixer text file."));
        return;
    }
}

// Save all gains.
void ChannelMixerDialog::slotUser2()
{
    KUrl saveGainsFileUrl;
    FILE *fp = 0L;

    saveGainsFileUrl = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
                                               QString( "*" ), this,
                                               QString( i18n("Gimp Gains Mixer File to Save")) );
    if( saveGainsFileUrl.isEmpty() )
       return;

    fp = fopen(QFile::encodeName(saveGainsFileUrl.path()), "w");

    if ( fp )
    {
        const char *str = 0L;
        char        buf1[256];
        char        buf2[256];
        char        buf3[256];

        switch ( m_channelCB->currentIndex() )
        {
           case RedChannelGains:
              str = "RED";
              break;
           case GreenChannelGains:
              str = "GREEN";
              break;
           case BlueChannelGains:
              str = "BLUE";
              break;
           default:
              DWarning() <<  "Unknown Color channel gains" << endl;
              break;
        }

        fprintf (fp, "# Channel Mixer Configuration File\n");

        fprintf (fp, "CHANNEL: %s\n", str);
        fprintf (fp, "PREVIEW: %s\n", "true"); // preserved for compatibility
        fprintf (fp, "MONOCHROME: %s\n",
                 m_monochrome->isChecked() ? "true" : "false");
        fprintf (fp, "PRESERVE_LUMINOSITY: %s\n",
                 m_preserveLuminosity->isChecked() ? "true" : "false");

        sprintf (buf1, "%5.3f", m_redRedGain);
        sprintf (buf2, "%5.3f", m_redGreenGain);
        sprintf (buf3, "%5.3f", m_redBlueGain);
        fprintf (fp, "RED: %s %s %s\n", buf1, buf2,buf3);

        sprintf (buf1, "%5.3f", m_greenRedGain);
        sprintf (buf2, "%5.3f", m_greenGreenGain);
        sprintf (buf3, "%5.3f", m_greenBlueGain);
        fprintf (fp, "GREEN: %s %s %s\n", buf1, buf2,buf3);

        sprintf (buf1, "%5.3f", m_blueRedGain);
        sprintf (buf2, "%5.3f", m_blueGreenGain);
        sprintf (buf3, "%5.3f", m_blueBlueGain);
        fprintf (fp, "BLUE: %s %s %s\n", buf1, buf2,buf3);

        sprintf (buf1, "%5.3f", m_blackRedGain);
        sprintf (buf2, "%5.3f", m_blackGreenGain);
        sprintf (buf3, "%5.3f", m_blackBlueGain);
        fprintf (fp, "BLACK: %s %s %s\n", buf1, buf2,buf3);

        fclose (fp);
    }
    else
    {
        KMessageBox::error(this, i18n("Cannot save settings to the Gains Mixer text file."));
        return;
    }
}

}  // NameSpace DigikamChannelMixerImagesPlugin
