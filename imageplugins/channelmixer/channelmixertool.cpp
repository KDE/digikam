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
 * Gimp 2.2.3 and copyrighted 2002 by Martin Guldahl <mguldahl at xmission dot com>
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


#include "channelmixertool.h"
#include "channelmixertool.moc"

// C++ includes

#include <cstdio>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <cerrno>

// Qt includes

#include <QCheckBox>
#include <QColor>
#include <QFile>
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
#include <kvbox.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "colorgradientwidget.h"
#include "daboutdata.h"
#include "dimg.h"
#include "dimgimagefilters.h"
#include "editortoolsettings.h"
#include "histogramwidget.h"
#include "histogrambox.h"
#include "imagehistogram.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "version.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamChannelMixerImagesPlugin
{

ChannelMixerTool::ChannelMixerTool(QObject* parent)
                : EditorTool(parent)
{
    setObjectName("channelmixer");
    setToolName(i18n("Channel Mixer"));
    setToolIcon(SmallIcon("channelmixer"));

    // -------------------------------------------------------------

    m_destinationPreviewData = 0;

    m_previewWidget = new ImageWidget("channelmixer Tool", 0,
                                      i18n("You can see here the image's color channels' "
                                           "gain adjustments preview. You can pick a color on the image "
                                           "to see the corresponding color level on the histogram."));
    setToolView(m_previewWidget);

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Load|
                                            EditorToolSettings::SaveAs|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel,
                                            EditorToolSettings::Histogram,
                                            HistogramBox::RGB);

    QGridLayout* grid     = new QGridLayout(m_gboxSettings->plainPage());

    // -------------------------------------------------------------

    QLabel *redLabel = new QLabel(i18n("Red:"), m_gboxSettings->plainPage());
    m_redGain = new RDoubleNumInput(m_gboxSettings->plainPage());
    m_redGain->setDecimals(0);
    m_redGain->setRange(-200.0, 200.0, 1);
    m_redGain->setDefaultValue(0);
    m_redGain->setWhatsThis(i18n("Select the red color gain, as a percentage, "
                                 "for the current channel."));

    QLabel *greenLabel = new QLabel(i18n("Green:"), m_gboxSettings->plainPage());
    m_greenGain = new RDoubleNumInput(m_gboxSettings->plainPage());
    m_greenGain->setDecimals(0);
    m_greenGain->setRange(-200.0, 200.0, 1);
    m_greenGain->setDefaultValue(0);
    m_greenGain->setWhatsThis(i18n("Select the green color gain, as a percentage, "
                                   "for the current channel."));

    QLabel *blueLabel = new QLabel(i18n("Blue:"), m_gboxSettings->plainPage());
    m_blueGain = new RDoubleNumInput(m_gboxSettings->plainPage());
    m_blueGain->setDecimals(0);
    m_blueGain->setRange(-200.0, 200.0, 1);
    m_blueGain->setDefaultValue(0);
    m_blueGain->setWhatsThis(i18n("Select the blue color gain, as a percentage, "
                                  "for the current channel."));

    m_resetButton = new QPushButton(i18n("&Reset"), m_gboxSettings->plainPage());
    m_resetButton->setIcon(KIconLoader::global()->loadIcon("document-revert", KIconLoader::Toolbar));
    m_resetButton->setWhatsThis(i18n("Reset color channels' gains settings from "
                                     "the currently selected channel."));

    // -------------------------------------------------------------

    m_monochrome = new QCheckBox(i18n("Monochrome"), m_gboxSettings->plainPage());
    m_monochrome->setWhatsThis(i18n("Enable this option if you want the image rendered "
                                    "in monochrome mode. "
                                    "In this mode, the histogram will display only luminosity values."));

    m_preserveLuminosity = new QCheckBox(i18n("Preserve luminosity"), m_gboxSettings->plainPage());
    m_preserveLuminosity->setWhatsThis(i18n("Enable this option is you want preserve "
                                            "the image luminosity."));

    // -------------------------------------------------------------

    grid->addWidget(redLabel,               0, 0, 1, 1);
    grid->addWidget(m_redGain,              0, 1, 1, 4);
    grid->addWidget(greenLabel,             1, 0, 1, 1);
    grid->addWidget(m_greenGain,            1, 1, 1, 4);
    grid->addWidget(blueLabel,              2, 0, 1, 1);
    grid->addWidget(m_blueGain,             2, 1, 1, 4);
    grid->addWidget(m_resetButton,          3, 0, 1, 2);
    grid->addWidget(m_monochrome,           4, 0, 1, 5);
    grid->addWidget(m_preserveLuminosity,   5, 0, 1, 5);
    grid->setRowStretch(6, 10);
    grid->setMargin(m_gboxSettings->spacingHint());
    grid->setSpacing(m_gboxSettings->spacingHint());

    setToolSettings(m_gboxSettings);
    init();

    // -------------------------------------------------------------
    // Channels and scale selection slots.

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
    // Buttons slots.

    connect(m_resetButton, SIGNAL(clicked()),
            this, SLOT(slotResetCurrentChannel()));
}

ChannelMixerTool::~ChannelMixerTool()
{
    if (m_destinationPreviewData)
       delete [] m_destinationPreviewData;
}

void ChannelMixerTool::slotResetCurrentChannel()
{
    switch (m_gboxSettings->histogramBox()->channel())
    {
        case EditorToolSettings::GreenChannel:         // Green.
            m_greenRedGain = 0.0;
            m_greenGreenGain = 1.0;
            m_greenBlueGain = 0.0;
            break;

        case EditorToolSettings::BlueChannel:          // Blue.
            m_blueRedGain = 0.0;
            m_blueGreenGain = 0.0;
            m_blueBlueGain = 1.0;
            break;

        default:                        // Red or monochrome.
            if (m_monochrome->isChecked())
            {
                m_blackRedGain = 1.0;
                m_blackGreenGain = 0.0;
                m_blackBlueGain = 0.0;
            }
            else
            {
                m_redRedGain = 1.0;
                m_redGreenGain = 0.0;
                m_redBlueGain = 0.0;
            }
            break;
    }

    adjustSliders();
    slotEffect();
    m_gboxSettings->histogramBox()->histogram()->reset();
}

void ChannelMixerTool::slotColorSelectedFromTarget( const DColor &color )
{
    m_gboxSettings->histogramBox()->histogram()->setHistogramGuideByColor(color);
}

void ChannelMixerTool::slotGainsChanged()
{
    switch(m_gboxSettings->histogramBox()->channel())
    {
       case EditorToolSettings::GreenChannel:           // Green.
          m_greenRedGain   = m_redGain->value()   / 100.0;
          m_greenGreenGain = m_greenGain->value() / 100.0;
          m_greenBlueGain  = m_blueGain->value()  / 100.0;
          break;

       case EditorToolSettings::BlueChannel:            // Blue.
          m_blueRedGain   = m_redGain->value()   / 100.0;
          m_blueGreenGain = m_greenGain->value() / 100.0;
          m_blueBlueGain  = m_blueGain->value()  / 100.0;
          break;

       default:                         // Red or monochrome.
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

void ChannelMixerTool::adjustSliders()
{
    m_redGain->blockSignals(true);
    m_greenGain->blockSignals(true);
    m_blueGain->blockSignals(true);

    switch(m_gboxSettings->histogramBox()->channel())
    {
       case EditorToolSettings::GreenChannel:           // Green.
          m_redGain->setValue(m_greenRedGain     * 100.0);
          m_greenGain->setValue(m_greenGreenGain * 100.0);
          m_blueGain->setValue(m_greenBlueGain   * 100.0);
          break;

       case EditorToolSettings::BlueChannel:            // Blue.
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

void ChannelMixerTool::slotMonochromeActived(bool mono)
{
    m_gboxSettings->histogramBox()->setChannelEnabled(!mono);
    m_gboxSettings->histogramBox()->setChannel(EditorToolSettings::RedChannel);
}

void ChannelMixerTool::slotEffect()
{
    ImageIface* iface = m_previewWidget->imageIface();
    uchar *data                = iface->getPreviewImage();
    int w                      = iface->previewWidth();
    int h                      = iface->previewHeight();
    bool sb                    = iface->previewSixteenBit();

    // Create the new empty destination image data space.
    m_gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    if (m_destinationPreviewData)
       delete [] m_destinationPreviewData;

    m_destinationPreviewData = new uchar[w*h*(sb ? 8 : 4)];
    DImgImageFilters filter;

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
    m_gboxSettings->histogramBox()->histogram()->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);
    delete [] data;
}

void ChannelMixerTool::finalRendering()
{
    kapp->setOverrideCursor( Qt::WaitCursor );
    ImageIface* iface = m_previewWidget->imageIface();
    uchar *data                = iface->getOriginalImage();
    int w                      = iface->originalWidth();
    int h                      = iface->originalHeight();
    bool sb                    = iface->originalSixteenBit();

    DImgImageFilters filter;

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
}

void ChannelMixerTool::slotChannelChanged()
{
    if (m_monochrome->isChecked())
        m_gboxSettings->histogramBox()->setGradientColors(QColor("black"), QColor("white"));

    adjustSliders();
    slotEffect();
}

void ChannelMixerTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("channelmixer Tool");

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

    // we need to call the set methods here, otherwise the histogram will not be updated correctly
    m_gboxSettings->histogramBox()->setChannel(group.readEntry("Histogram Channel",
                        (int)EditorToolSettings::LuminosityChannel));
    m_gboxSettings->histogramBox()->setScale(group.readEntry("Histogram Scale",
                        (int)HistogramWidget::LogScaleHistogram));

    m_gboxSettings->histogramBox()->histogram()->reset();
}

void ChannelMixerTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("channelmixer Tool");

    group.writeEntry("Histogram Channel", m_gboxSettings->histogramBox()->channel());
    group.writeEntry("Histogram Scale", m_gboxSettings->histogramBox()->scale());

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

    m_previewWidget->writeSettings();

    config->sync();
}

void ChannelMixerTool::slotResetSettings()
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
    m_gboxSettings->histogramBox()->histogram()->reset();
    m_gboxSettings->histogramBox()->setChannel(EditorToolSettings::RedChannel);
}

// Load all gains.
void ChannelMixerTool::slotLoadSettings()
{
    KUrl loadGainsFileUrl;
    FILE *fp = 0L;

    loadGainsFileUrl = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                                            QString( "*" ), kapp->activeWindow(),
                                            QString( i18n("Select Gimp Gains Mixer File to Load")) );
    if( loadGainsFileUrl.isEmpty() )
       return;

    fp = fopen(QFile::encodeName(loadGainsFileUrl.path()), "r");

    if ( fp )
    {
        bool monochrome;
        int  currentOutputChannel = EditorToolSettings::RedChannel;
        char buf1[1024];
        char buf2[1024];
        char buf3[1024];

        buf1[0] = '\0';

        fgets(buf1, 1023, fp);

        fscanf (fp, "%*s %s", buf1);

        // Get the current output channel in dialog.

        if (strcmp (buf1, "RED") == 0)
            currentOutputChannel = EditorToolSettings::RedChannel;
        else if (strcmp (buf1, "GREEN") == 0)
            currentOutputChannel = EditorToolSettings::GreenChannel;
        else if (strcmp (buf1, "BLUE") == 0)
            currentOutputChannel = EditorToolSettings::BlueChannel;

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
        m_gboxSettings->histogramBox()->setChannel(currentOutputChannel);
    }
    else
    {
        KMessageBox::error(kapp->activeWindow(),
                           i18n("Cannot load settings from the Gains Mixer text file."));
        return;
    }
}

// Save all gains.
void ChannelMixerTool::slotSaveAsSettings()
{
    KUrl saveGainsFileUrl;
    FILE *fp = 0L;

    saveGainsFileUrl = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
                                               QString( "*" ), kapp->activeWindow(),
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

        switch (m_gboxSettings->histogramBox()->channel())
        {
           case EditorToolSettings::RedChannel:
              str = "RED";
              break;
           case EditorToolSettings::GreenChannel:
              str = "GREEN";
              break;
           case EditorToolSettings::BlueChannel:
              str = "BLUE";
              break;
           default:
              kWarning(50006) <<  "Unknown Color channel gains" << endl;
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
        KMessageBox::error(kapp->activeWindow(),
                           i18n("Cannot save settings to the Gains Mixer text file."));
        return;
    }
}

}  // namespace DigikamChannelMixerImagesPlugin
