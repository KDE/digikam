/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-18
 * Description : Channel mixer settings view.
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "mixersettings.h"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QCheckBox>
#include <QPushButton>
#include <QStandardPaths>
#include <QApplication>
#include <QStyle>
#include <QComboBox>
#include <QMessageBox>
#include <QUrl>
#include <QFontDatabase>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dnuminput.h"
#include "dfiledialog.h"
#include "dexpanderbox.h"
#include "digikam_debug.h"

namespace Digikam
{

class MixerSettings::Private
{
public:

    Private() :
        currentChannel(RedChannel),
        monochromeTips(0),
        totalPercents(0),
        outChannelLabel(0),
        resetButton(0),
        preserveLuminosity(0),
        monochrome(0),
        outChannelCB(0),
        redGain(0),
        greenGain(0),
        blueGain(0)
    {
    }

    static const QString  configMonochromeEntry;
    static const QString  configPreserveLuminosityEntry;
    static const QString  configRedRedGainEntry;
    static const QString  configRedGreenGainEntry;
    static const QString  configRedBlueGainEntry;
    static const QString  configGreenRedGainEntry;
    static const QString  configGreenGreenGainEntry;
    static const QString  configGreenBlueGainEntry;
    static const QString  configBlueRedGainEntry;
    static const QString  configBlueGreenGainEntry;
    static const QString  configBlueBlueGainEntry;
    static const QString  configBlackRedGainEntry;
    static const QString  configBlackGreenGainEntry;
    static const QString  configBlackBlueGainEntry;

    int                   currentChannel;

    QLabel*               monochromeTips;
    QLabel*               totalPercents;
    QLabel*               outChannelLabel;

    QPushButton*          resetButton;

    QCheckBox*            preserveLuminosity;
    QCheckBox*            monochrome;

    QComboBox*            outChannelCB;

    MixerContainer        mixerSettings;

    DDoubleNumInput*      redGain;
    DDoubleNumInput*      greenGain;
    DDoubleNumInput*      blueGain;
};

const QString MixerSettings::Private::configMonochromeEntry(QLatin1String("Monochrome"));
const QString MixerSettings::Private::configPreserveLuminosityEntry(QLatin1String("PreserveLuminosity"));
const QString MixerSettings::Private::configRedRedGainEntry(QLatin1String("RedRedGain"));
const QString MixerSettings::Private::configRedGreenGainEntry(QLatin1String("RedGreenGain"));
const QString MixerSettings::Private::configRedBlueGainEntry(QLatin1String("RedBlueGain"));
const QString MixerSettings::Private::configGreenRedGainEntry(QLatin1String("GreenRedGain"));
const QString MixerSettings::Private::configGreenGreenGainEntry(QLatin1String("GreenGreenGain"));
const QString MixerSettings::Private::configGreenBlueGainEntry(QLatin1String("GreenBlueGain"));
const QString MixerSettings::Private::configBlueRedGainEntry(QLatin1String("BlueRedGain"));
const QString MixerSettings::Private::configBlueGreenGainEntry(QLatin1String("BlueGreenGain"));
const QString MixerSettings::Private::configBlueBlueGainEntry(QLatin1String("BlueBlueGain"));
const QString MixerSettings::Private::configBlackRedGainEntry(QLatin1String("BlackRedGain"));
const QString MixerSettings::Private::configBlackGreenGainEntry(QLatin1String("BlackGreenGain"));
const QString MixerSettings::Private::configBlackBlueGainEntry(QLatin1String("BlackBlueGain"));

// --------------------------------------------------------

MixerSettings::MixerSettings(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QGridLayout* const grid = new QGridLayout(this);

    d->outChannelLabel = new QLabel(i18n("Output Channel:"));
    d->outChannelCB    = new QComboBox;
    d->outChannelCB->addItem(i18n("Red"),   QVariant(RedChannel));
    d->outChannelCB->addItem(i18n("Green"), QVariant(GreenChannel));
    d->outChannelCB->addItem(i18n("Blue"),  QVariant(BlueChannel));

    QLabel* const redLabel = new QLabel(i18n("Red (%):"));
    d->redGain             = new DDoubleNumInput;
    d->redGain->setDecimals(1);
    d->redGain->setRange(-200.0, 200.0, 1);
    d->redGain->setDefaultValue(0);
    d->redGain->setWhatsThis(i18n("Select the red color gain, as a percentage, "
                                  "for the current channel."));

    QLabel* const greenLabel = new QLabel(i18n("Green (%):"));
    d->greenGain             = new DDoubleNumInput;
    d->greenGain->setDecimals(1);
    d->greenGain->setRange(-200.0, 200.0, 1);
    d->greenGain->setDefaultValue(0);
    d->greenGain->setWhatsThis(i18n("Select the green color gain, as a percentage, "
                                    "for the current channel."));

    QLabel* const blueLabel = new QLabel(i18n("Blue (%):"));
    d->blueGain             = new DDoubleNumInput;
    d->blueGain->setDecimals(1);
    d->blueGain->setRange(-200.0, 200.0, 1);
    d->blueGain->setDefaultValue(0);
    d->blueGain->setWhatsThis(i18n("Select the blue color gain, as a percentage, "
                                   "for the current channel."));

    // -------------------------------------------------------------

    d->resetButton = new QPushButton(i18n("&Reset"));
    d->resetButton->setIcon(QIcon::fromTheme(QLatin1String("document-revert")));
    d->resetButton->setWhatsThis(i18n("Reset color channels' gains settings from "
                                      "the currently selected channel."));

    d->totalPercents = new QLabel();
    d->totalPercents->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // -------------------------------------------------------------

    d->preserveLuminosity = new QCheckBox(i18n("Preserve luminosity"));
    d->preserveLuminosity->setWhatsThis(i18n("Enable this option is you want preserve "
                                             "the image luminosity."));

    // -------------------------------------------------------------

    d->monochrome     = new QCheckBox(i18n("Monochrome"));
    d->monochromeTips = new QLabel(i18n("<p>Use <b>Monochrome</b> mode to convert color picture to Black and White:</p>"
                                        "<p>The <qt><font color=\"red\">red channel</font></qt> modifies <a href='http://en.wikipedia.org/wiki/Contrast_(vision)'>the contrast</a> of photograph.</p>"
                                        "<p>The <qt><font color=\"green\">green channel</font></qt> enhances or reduces the details level of photograph.</p>"
                                        "<p>The <qt><font color=\"blue\">blue channel</font></qt> affects <a href='http://en.wikipedia.org/wiki/Image_noise'>the noise</a> of photograph.</p>"
                                        "<p><u>Note:</u> in this mode, the histogram will display only luminosity values.</p>"));

    d->monochromeTips->setEnabled(false);
    d->monochromeTips->setFont(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont));
    d->monochromeTips->setWordWrap(true);
    d->monochromeTips->setOpenExternalLinks(true);
    d->monochromeTips->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    d->monochromeTips->setLineWidth(1);

    // -------------------------------------------------------------

    grid->addWidget(d->outChannelLabel,    0, 0, 1, 1);
    grid->addWidget(d->outChannelCB,       0, 3, 1, 2);
    grid->addWidget(redLabel,              1, 0, 1, 1);
    grid->addWidget(d->redGain,            1, 1, 1, 4);
    grid->addWidget(greenLabel,            2, 0, 1, 1);
    grid->addWidget(d->greenGain,          2, 1, 1, 4);
    grid->addWidget(blueLabel,             3, 0, 1, 1);
    grid->addWidget(d->blueGain,           3, 1, 1, 4);
    grid->addWidget(d->resetButton,        4, 0, 1, 2);
    grid->addWidget(d->totalPercents,      4, 3, 1, 1);
    grid->addWidget(d->preserveLuminosity, 5, 0, 1, 5);
    grid->addWidget(d->monochrome,         6, 0, 1, 5);
    grid->addWidget(d->monochromeTips,     7, 0, 1, 5);
    grid->setRowStretch(8, 10);
    grid->setColumnStretch(2, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // -------------------------------------------------------------

    connect(d->redGain, SIGNAL(valueChanged(double)),
            this, SLOT(slotGainsChanged()));

    connect(d->greenGain, SIGNAL(valueChanged(double)),
            this, SLOT(slotGainsChanged()));

    connect(d->blueGain, SIGNAL(valueChanged(double)),
            this, SLOT(slotGainsChanged()));

    connect(d->resetButton, SIGNAL(clicked()),
            this, SLOT(slotResetCurrentChannel()));

    connect(d->monochrome, SIGNAL(toggled(bool)),
            this, SLOT(slotMonochromeActived(bool)));

    connect(d->preserveLuminosity, SIGNAL(toggled(bool)),
            this, SLOT(slotLuminosityChanged(bool)));

    connect(d->outChannelCB, SIGNAL(activated(int)),
            this, SLOT(slotOutChannelChanged()));
}

MixerSettings::~MixerSettings()
{
    delete d;
}

void MixerSettings::setMonochromeTipsVisible(bool b)
{
    b ? d->monochromeTips->show() : d->monochromeTips->hide();
}

void MixerSettings::slotOutChannelChanged()
{
    int index         = d->outChannelCB->currentIndex();
    d->currentChannel = (ChannelType)(d->outChannelCB->itemData(index).toInt());
    updateSettingsWidgets();
    emit signalOutChannelChanged();
}

int MixerSettings::currentChannel() const
{
    return d->currentChannel;
}

void MixerSettings::slotResetCurrentChannel()
{
    switch (d->currentChannel)
    {
        case GreenChannel:
        {
            d->mixerSettings.greenRedGain   = 0.0;
            d->mixerSettings.greenGreenGain = 1.0;
            d->mixerSettings.greenBlueGain  = 0.0;
            break;
        }

        case BlueChannel:
        {
            d->mixerSettings.blueRedGain   = 0.0;
            d->mixerSettings.blueGreenGain = 0.0;
            d->mixerSettings.blueBlueGain  = 1.0;
            break;
        }

        default:                        // Red or monochrome.
        {
            if (d->monochrome->isChecked())
            {
                d->mixerSettings.blackRedGain   = 1.0;
                d->mixerSettings.blackGreenGain = 0.0;
                d->mixerSettings.blackBlueGain  = 0.0;
            }
            else
            {
                d->mixerSettings.redRedGain   = 1.0;
                d->mixerSettings.redGreenGain = 0.0;
                d->mixerSettings.redBlueGain  = 0.0;
            }

            break;
        }
    }

    updateSettingsWidgets();
    emit signalSettingsChanged();
}

void MixerSettings::slotGainsChanged()
{
    switch (d->currentChannel)
    {
        case GreenChannel:
        {
            d->mixerSettings.greenRedGain   = d->redGain->value()   / 100.0;
            d->mixerSettings.greenGreenGain = d->greenGain->value() / 100.0;
            d->mixerSettings.greenBlueGain  = d->blueGain->value()  / 100.0;
            break;
        }

        case BlueChannel:
        {
            d->mixerSettings.blueRedGain   = d->redGain->value()   / 100.0;
            d->mixerSettings.blueGreenGain = d->greenGain->value() / 100.0;
            d->mixerSettings.blueBlueGain  = d->blueGain->value()  / 100.0;
            break;
        }

        default:                         // Red or monochrome.
        {
            if (d->monochrome->isChecked())
            {
                d->mixerSettings.blackRedGain   = d->redGain->value()   / 100.0;
                d->mixerSettings.blackGreenGain = d->greenGain->value() / 100.0;
                d->mixerSettings.blackBlueGain  = d->blueGain->value()  / 100.0;
            }
            else
            {
                d->mixerSettings.redRedGain   = d->redGain->value()   / 100.0;
                d->mixerSettings.redGreenGain = d->greenGain->value() / 100.0;
                d->mixerSettings.redBlueGain  = d->blueGain->value()  / 100.0;
            }

            break;
        }
    }

    updateTotalPercents();
    emit signalSettingsChanged();
}

void MixerSettings::updateTotalPercents()
{
    double total = d->redGain->value() + d->greenGain->value() + d->blueGain->value();
    QString str;
    d->totalPercents->setText(i18n("Total: %1 (%)", str.sprintf("%3.1f", total)));
}

void MixerSettings::updateSettingsWidgets()
{
    d->monochrome->blockSignals(true);
    d->preserveLuminosity->blockSignals(true);
    d->redGain->blockSignals(true);
    d->greenGain->blockSignals(true);
    d->blueGain->blockSignals(true);

    switch (d->currentChannel)
    {
        case GreenChannel:
        {
            d->redGain->setDefaultValue(0);
            d->greenGain->setDefaultValue(100);
            d->blueGain->setDefaultValue(0);
            d->redGain->setValue(d->mixerSettings.greenRedGain     * 100.0);
            d->greenGain->setValue(d->mixerSettings.greenGreenGain * 100.0);
            d->blueGain->setValue(d->mixerSettings.greenBlueGain   * 100.0);
            break;
        }

        case BlueChannel:
        {
            d->redGain->setDefaultValue(0);
            d->greenGain->setDefaultValue(0);
            d->blueGain->setDefaultValue(100);
            d->redGain->setValue(d->mixerSettings.blueRedGain     * 100.0);
            d->greenGain->setValue(d->mixerSettings.blueGreenGain * 100.0);
            d->blueGain->setValue(d->mixerSettings.blueBlueGain   * 100.0);
            break;
        }

        default:          // Red or monochrome.
        {
            if (d->monochrome->isChecked())
            {
                d->redGain->setDefaultValue(100);
                d->greenGain->setDefaultValue(0);
                d->blueGain->setDefaultValue(0);
                d->redGain->setValue(d->mixerSettings.blackRedGain     * 100.0);
                d->greenGain->setValue(d->mixerSettings.blackGreenGain * 100.0);
                d->blueGain->setValue(d->mixerSettings.blackBlueGain   * 100.0);
            }
            else
            {
                d->redGain->setDefaultValue(100);
                d->greenGain->setDefaultValue(0);
                d->blueGain->setDefaultValue(0);
                d->redGain->setValue(d->mixerSettings.redRedGain     * 100.0);
                d->greenGain->setValue(d->mixerSettings.redGreenGain * 100.0);
                d->blueGain->setValue(d->mixerSettings.redBlueGain   * 100.0);
            }

            break;
        }
    }

    d->monochrome->setChecked(d->mixerSettings.bMonochrome);
    d->preserveLuminosity->setChecked(d->mixerSettings.bPreserveLum);
    updateTotalPercents();

    d->monochrome->blockSignals(false);
    d->preserveLuminosity->blockSignals(false);
    d->redGain->blockSignals(false);
    d->greenGain->blockSignals(false);
    d->blueGain->blockSignals(false);
}

void MixerSettings::slotMonochromeActived(bool mono)
{
    d->mixerSettings.bMonochrome = mono;
    d->monochromeTips->setEnabled(mono);

    d->outChannelLabel->setEnabled(!mono);
    d->outChannelCB->setEnabled(!mono);
    int id = d->outChannelCB->findData(QVariant(RedChannel));
    d->outChannelCB->setCurrentIndex(id);
    slotOutChannelChanged();

    emit signalMonochromeActived(mono);
    emit signalSettingsChanged();
}

void MixerSettings::slotLuminosityChanged(bool lum)
{
    d->mixerSettings.bPreserveLum = lum;
    emit signalSettingsChanged();
}

MixerContainer MixerSettings::settings() const
{
    return d->mixerSettings;
}

void MixerSettings::setSettings(const MixerContainer& settings)
{
    blockSignals(true);
    d->mixerSettings = settings;
    updateSettingsWidgets();
    slotMonochromeActived(d->mixerSettings.bMonochrome);
    blockSignals(false);
}

void MixerSettings::resetToDefault()
{
    setSettings(defaultSettings());
}

MixerContainer MixerSettings::defaultSettings() const
{
    MixerContainer prm;

    prm.bMonochrome    = false;
    prm.bPreserveLum   = true;

    prm.redRedGain     = 1.0;
    prm.redGreenGain   = 0.0;
    prm.redBlueGain    = 0.0;

    prm.greenRedGain   = 0.0;
    prm.greenGreenGain = 1.0;
    prm.greenBlueGain  = 0.0;

    prm.blueRedGain    = 0.0;
    prm.blueGreenGain  = 0.0;
    prm.blueBlueGain   = 1.0;

    prm.blackRedGain   = 1.0;
    prm.blackGreenGain = 0.0;
    prm.blackBlueGain  = 0.0;

    return prm;
}

void MixerSettings::readSettings(KConfigGroup& group)
{
    MixerContainer prm;
    MixerContainer defaultPrm = defaultSettings();

    prm.bMonochrome    = group.readEntry(d->configMonochromeEntry,         defaultPrm.bMonochrome);
    prm.bPreserveLum   = group.readEntry(d->configPreserveLuminosityEntry, defaultPrm.bPreserveLum);

    prm.redRedGain     = group.readEntry(d->configRedRedGainEntry,         defaultPrm.redRedGain);
    prm.redGreenGain   = group.readEntry(d->configRedGreenGainEntry,       defaultPrm.redGreenGain);
    prm.redBlueGain    = group.readEntry(d->configRedBlueGainEntry,        defaultPrm.redBlueGain);

    prm.greenRedGain   = group.readEntry(d->configGreenRedGainEntry,       defaultPrm.greenRedGain);
    prm.greenGreenGain = group.readEntry(d->configGreenGreenGainEntry,     defaultPrm.greenGreenGain);
    prm.greenBlueGain  = group.readEntry(d->configGreenBlueGainEntry,      defaultPrm.greenBlueGain);

    prm.blueRedGain    = group.readEntry(d->configBlueRedGainEntry,        defaultPrm.blueRedGain);
    prm.blueGreenGain  = group.readEntry(d->configBlueGreenGainEntry,      defaultPrm.blueGreenGain);
    prm.blueBlueGain   = group.readEntry(d->configBlueBlueGainEntry,       defaultPrm.blueBlueGain);

    prm.blackRedGain   = group.readEntry(d->configBlackRedGainEntry,       defaultPrm.blackRedGain);
    prm.blackGreenGain = group.readEntry(d->configBlackGreenGainEntry,     defaultPrm.blackGreenGain);
    prm.blackBlueGain  = group.readEntry(d->configBlackBlueGainEntry,      defaultPrm.blackBlueGain);

    setSettings(prm);
}

void MixerSettings::writeSettings(KConfigGroup& group)
{
    MixerContainer prm = settings();

    group.writeEntry(d->configMonochromeEntry,         prm.bMonochrome);
    group.writeEntry(d->configPreserveLuminosityEntry, prm.bPreserveLum);

    group.writeEntry(d->configRedRedGainEntry,         prm.redRedGain);
    group.writeEntry(d->configRedGreenGainEntry,       prm.redGreenGain);
    group.writeEntry(d->configRedBlueGainEntry,        prm.redBlueGain);

    group.writeEntry(d->configGreenRedGainEntry,       prm.greenRedGain);
    group.writeEntry(d->configGreenGreenGainEntry,     prm.greenGreenGain);
    group.writeEntry(d->configGreenBlueGainEntry,      prm.greenBlueGain);

    group.writeEntry(d->configBlueRedGainEntry,        prm.blueRedGain);
    group.writeEntry(d->configBlueGreenGainEntry,      prm.blueGreenGain);
    group.writeEntry(d->configBlueBlueGainEntry,       prm.blueBlueGain);

    group.writeEntry(d->configBlackRedGainEntry,       prm.blackRedGain);
    group.writeEntry(d->configBlackGreenGainEntry,     prm.blackGreenGain);
    group.writeEntry(d->configBlackBlueGainEntry,      prm.blackBlueGain);
}

void MixerSettings::loadSettings()
{
    QUrl           loadGainsFileUrl;
    FILE*          fp = 0L;
    MixerContainer settings;

    loadGainsFileUrl = DFileDialog::getOpenFileUrl(qApp->activeWindow(), i18n("Select Gimp Gains Mixer File to Load"),
                                                   QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)),
                                                   QLatin1String("*"));

    if (loadGainsFileUrl.isEmpty())
    {
        return;
    }

    fp = fopen(QFile::encodeName(loadGainsFileUrl.toLocalFile()).constData(), "r");

    if (fp)
    {
        char buf1[1024];
        char buf2[1024];
        char buf3[1024];

        buf1[0] = '\0';

        fgets(buf1, 1023, fp);

        fscanf(fp, "%*s %256s", buf1);
        fscanf(fp, "%*s %256s", buf1);  // preview flag, preserved for compatibility
        fscanf(fp, "%*s %256s", buf1);

        if (strcmp(buf1, "true") == 0)
        {
            settings.bMonochrome = true;
        }
        else
        {
            settings.bMonochrome = false;
        }

        fscanf(fp, "%*s %256s", buf1);

        if (strcmp(buf1, "true") == 0)
        {
            settings.bPreserveLum = true;
        }
        else
        {
            settings.bPreserveLum = false;
        }

        fscanf(fp, "%*s %256s %256s %256s", buf1, buf2, buf3);
        settings.redRedGain   = atof(buf1);
        settings.redGreenGain = atof(buf2);
        settings.redBlueGain  = atof(buf3);

        fscanf(fp, "%*s %256s %256s %256s", buf1, buf2, buf3);
        settings.greenRedGain   = atof(buf1);
        settings.greenGreenGain = atof(buf2);
        settings.greenBlueGain  = atof(buf3);

        fscanf(fp, "%*s %256s %256s %256s", buf1, buf2, buf3);
        settings.blueRedGain   = atof(buf1);
        settings.blueGreenGain = atof(buf2);
        settings.blueBlueGain  = atof(buf3);

        fscanf(fp, "%*s %256s %256s %256s", buf1, buf2, buf3);
        settings.blackRedGain   = atof(buf1);
        settings.blackGreenGain = atof(buf2);
        settings.blackBlueGain  = atof(buf3);

        fclose(fp);

        setSettings(settings);
    }
    else
    {
        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                              i18n("Cannot load settings from the Gains Mixer text file."));
        return;
    }
}

void MixerSettings::saveAsSettings()
{
    QUrl  saveGainsFileUrl;
    FILE* fp = 0L;

    saveGainsFileUrl = DFileDialog::getSaveFileUrl(qApp->activeWindow(), i18n("Gimp Gains Mixer File to Save"),
                                                   QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)),
                                                   QLatin1String("*"));

    if (saveGainsFileUrl.isEmpty())
    {
        return;
    }

    fp = fopen(QFile::encodeName(saveGainsFileUrl.toLocalFile()).constData(), "w");

    if (fp)
    {
        const char* str = 0L;
        char        buf1[256];
        char        buf2[256];
        char        buf3[256];

        switch (d->currentChannel)
        {
            case RedChannel:
                str = "RED";
                break;

            case GreenChannel:
                str = "GREEN";
                break;

            case BlueChannel:
                str = "BLUE";
                break;

            default:
                qCWarning(DIGIKAM_DIMG_LOG) <<  "Unknown Color channel gains";
                break;
        }

        fprintf(fp, "# Channel Mixer Configuration File\n");

        fprintf(fp, "CHANNEL: %s\n", str);
        fprintf(fp, "PREVIEW: %s\n", "true");  // preserved for compatibility
        fprintf(fp, "MONOCHROME: %s\n",
                d->mixerSettings.bMonochrome ? "true" : "false");
        fprintf(fp, "PRESERVE_LUMINOSITY: %s\n",
                d->mixerSettings.bPreserveLum ? "true" : "false");

        sprintf(buf1, "%5.3f", d->mixerSettings.redRedGain);
        sprintf(buf2, "%5.3f", d->mixerSettings.redGreenGain);
        sprintf(buf3, "%5.3f", d->mixerSettings.redBlueGain);
        fprintf(fp, "RED: %s %s %s\n", buf1, buf2, buf3);

        sprintf(buf1, "%5.3f", d->mixerSettings.greenRedGain);
        sprintf(buf2, "%5.3f", d->mixerSettings.greenGreenGain);
        sprintf(buf3, "%5.3f", d->mixerSettings.greenBlueGain);
        fprintf(fp, "GREEN: %s %s %s\n", buf1, buf2, buf3);

        sprintf(buf1, "%5.3f", d->mixerSettings.blueRedGain);
        sprintf(buf2, "%5.3f", d->mixerSettings.blueGreenGain);
        sprintf(buf3, "%5.3f", d->mixerSettings.blueBlueGain);
        fprintf(fp, "BLUE: %s %s %s\n", buf1, buf2, buf3);

        sprintf(buf1, "%5.3f", d->mixerSettings.blackRedGain);
        sprintf(buf2, "%5.3f", d->mixerSettings.blackGreenGain);
        sprintf(buf3, "%5.3f", d->mixerSettings.blackBlueGain);
        fprintf(fp, "BLACK: %s %s %s\n", buf1, buf2, buf3);

        fclose(fp);
    }
    else
    {
        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                              i18n("Cannot save settings to the Gains Mixer text file."));
        return;
    }
}

}  // namespace Digikam
