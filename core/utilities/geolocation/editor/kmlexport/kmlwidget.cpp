/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-05-16
 * Description : a tool to export GPS data to KML file.
 *
 * Copyright (C) 2006-2007 by Stephane Pontier <shadow dot walker at free dot fr>
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "kmlwidget.h"

// Qt includes

#include <QApplication>
#include <QWindow>
#include <QButtonGroup>
#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QComboBox>
#include <QLineEdit>
#include <QCloseEvent>
#include <QSpinBox>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

namespace Digikam
{

KmlWidget::KmlWidget(GeolocationEdit* const dlg,
                     GPSImageModel* const imageModel,
                     DInfoInterface* const iface)
    : QWidget(dlg),
      m_model(imageModel),
      m_dlg(dlg),
      m_kmlExport(iface)
{
    KMLExportConfigLayout = new QGridLayout(this);

    const int spacing     = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    // --------------------------------------------------------------
    // Target preferences

    TargetPreferenceGroupBox       = new QGroupBox(i18n("Target Preferences"), this);
    TargetPreferenceGroupBoxLayout = new QGridLayout(TargetPreferenceGroupBox);

    // target type
    TargetTypeGroupBox             = new QGroupBox(i18n("Target Type"), this);
    buttonGroupTargetTypeLayout    = new QGridLayout(TargetTypeGroupBox);
    buttonGroupTargetType          = new QButtonGroup(TargetTypeGroupBox);
    LocalTargetRadioButton_        = new QRadioButton(i18n("&Local or web target used by GoogleEarth"), TargetTypeGroupBox);
    LocalTargetRadioButton_->setChecked(true);

    GoogleMapTargetRadioButton_    = new QRadioButton(i18n("Web target used by GoogleMaps"), TargetTypeGroupBox);
    GoogleMapTargetRadioButton_->setToolTip(i18n("When using GoogleMaps, all images must have complete URLs, icons are "
                                                 "squared, and when drawing a track, only line track is exported."));

    buttonGroupTargetTypeLayout->addWidget(LocalTargetRadioButton_,     0, 0, 1, 1);
    buttonGroupTargetTypeLayout->addWidget(GoogleMapTargetRadioButton_, 1, 0, 1, 1);
    buttonGroupTargetTypeLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    buttonGroupTargetTypeLayout->setAlignment(Qt::AlignTop);

    // --------------------------------------------------------------
    // target preference, suite

    QLabel* const AltitudeLabel_ = new QLabel(i18n("Picture Altitude:"), TargetPreferenceGroupBox);
    AltitudeCB_                  = new QComboBox(TargetPreferenceGroupBox);
    AltitudeCB_->addItem(i18n("clamp to ground"));
    AltitudeCB_->addItem(i18n("relative to ground"));
    AltitudeCB_->addItem(i18n("absolute"));
    AltitudeCB_->setWhatsThis(i18n("<p>Specifies how pictures are displayed"
                                   "<dl><dt>clamp to ground (default)</dt>"
                                   "<dd>Indicates to ignore an altitude specification</dd>"
                                   "<dt>relative to ground</dt>"
                                   "<dd>Sets the altitude of the element relative to the actual ground "
                                   "elevation of a particular location.</dd>"
                                   "<dt>absolute</dt>"
                                   "<dd>Sets the altitude of the coordinate relative to sea level, regardless "
                                   "of the actual elevation of the terrain beneath the element.</dd></dl>"));

    destinationDirectoryLabel_ = new QLabel(i18n("Destination Directory:"), TargetPreferenceGroupBox);

    DestinationDirectory_= new DFileSelector(TargetPreferenceGroupBox);
    DestinationDirectory_->setFileDlgMode(QFileDialog::Directory);
    DestinationDirectory_->setFileDlgTitle(i18n("Select a directory in which to save the kml file and pictures"));

    DestinationUrlLabel_ = new QLabel(i18n("Destination Path:"), TargetPreferenceGroupBox);
    DestinationUrl_      = new QLineEdit(TargetPreferenceGroupBox);
    FileNameLabel_       = new QLabel(i18n("Filename:"), TargetPreferenceGroupBox);
    FileName_            = new QLineEdit(TargetPreferenceGroupBox);

    TargetPreferenceGroupBoxLayout->addWidget(TargetTypeGroupBox,         0, 0, 2, 5);
    TargetPreferenceGroupBoxLayout->addWidget(AltitudeLabel_,             2, 0, 1, 1);
    TargetPreferenceGroupBoxLayout->addWidget(AltitudeCB_,                2, 1, 1, 4);
    TargetPreferenceGroupBoxLayout->addWidget(destinationDirectoryLabel_, 3, 0, 1, 1);
    TargetPreferenceGroupBoxLayout->addWidget(DestinationDirectory_,      3, 1, 1, 4);
    TargetPreferenceGroupBoxLayout->addWidget(DestinationUrlLabel_,       4, 0, 1, 1);
    TargetPreferenceGroupBoxLayout->addWidget(DestinationUrl_,            4, 1, 1, 4);
    TargetPreferenceGroupBoxLayout->addWidget(FileNameLabel_,             5, 0, 1, 1);
    TargetPreferenceGroupBoxLayout->addWidget(FileName_,                  5, 1, 1, 4);
    TargetPreferenceGroupBoxLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    TargetPreferenceGroupBoxLayout->setAlignment(Qt::AlignTop);

    // --------------------------------------------------------------
    // Sizes

    QGroupBox* const SizeGroupBox = new QGroupBox(i18n("Sizes"), this);
    SizeGroupBoxLayout            = new QGridLayout(SizeGroupBox);
    IconSizeLabel                 = new QLabel(i18n("Icon Size:"), SizeGroupBox);
    IconSizeInput_                = new QSpinBox(SizeGroupBox);
    IconSizeInput_->setRange(1, 100);
    IconSizeInput_->setValue(33);

    ImageSizeLabel  = new QLabel(i18n("Image Size:"), SizeGroupBox);
    ImageSizeInput_ = new QSpinBox(SizeGroupBox);
    ImageSizeInput_->setRange(1, 10000);
    ImageSizeInput_->setValue(320);

    SizeGroupBoxLayout->addWidget(IconSizeLabel,   0, 0, 1, 1);
    SizeGroupBoxLayout->addWidget(IconSizeInput_,  0, 1, 1, 1);
    SizeGroupBoxLayout->addWidget(ImageSizeLabel,  0, 2, 1, 1);
    SizeGroupBoxLayout->addWidget(ImageSizeInput_, 0, 3, 1, 1);
    SizeGroupBoxLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    SizeGroupBoxLayout->setAlignment(Qt::AlignTop);

    // --------------------------------------------------------------
    // GPX Tracks

    QGroupBox* const GPXTracksGroupBox         = new QGroupBox(i18n("GPX Tracks"), this);
    QGridLayout* const GPXTracksGroupBoxLayout = new QGridLayout(GPXTracksGroupBox);

    // add a gpx track checkbox
    GPXTracksCheckBox_   = new QCheckBox(i18n("Draw GPX Track"), GPXTracksGroupBox);

    // file selector
    GPXFileLabel_ = new QLabel(i18n("GPX file:"), GPXTracksGroupBox);

    GPXFileUrlRequester_ = new DFileSelector(GPXTracksGroupBox);
    GPXFileUrlRequester_->setFileDlgFilter(i18n("%1|GPS Exchange Format", QLatin1String("*.gpx")));
    GPXFileUrlRequester_->setFileDlgTitle(i18n("Select GPX File to Load"));
    GPXFileUrlRequester_->setFileDlgMode(QFileDialog::ExistingFile);

    timeZoneLabel_ = new QLabel(i18n("Time Zone:"), GPXTracksGroupBox);
    timeZoneCB     = new QComboBox(GPXTracksGroupBox);
    timeZoneCB->addItem(i18n("GMT-12:00"), 0);
    timeZoneCB->addItem(i18n("GMT-11:00"), 1);
    timeZoneCB->addItem(i18n("GMT-10:00"), 2);
    timeZoneCB->addItem(i18n("GMT-09:00"), 3);
    timeZoneCB->addItem(i18n("GMT-08:00"), 4);
    timeZoneCB->addItem(i18n("GMT-07:00"), 5);
    timeZoneCB->addItem(i18n("GMT-06:00"), 6);
    timeZoneCB->addItem(i18n("GMT-05:00"), 7);
    timeZoneCB->addItem(i18n("GMT-04:00"), 8);
    timeZoneCB->addItem(i18n("GMT-03:00"), 9);
    timeZoneCB->addItem(i18n("GMT-02:00"), 10);
    timeZoneCB->addItem(i18n("GMT-01:00"), 11);
    timeZoneCB->addItem(i18n("GMT"),       12);
    timeZoneCB->addItem(i18n("GMT+01:00"), 13);
    timeZoneCB->addItem(i18n("GMT+02:00"), 14);
    timeZoneCB->addItem(i18n("GMT+03:00"), 15);
    timeZoneCB->addItem(i18n("GMT+04:00"), 16);
    timeZoneCB->addItem(i18n("GMT+05:00"), 17);
    timeZoneCB->addItem(i18n("GMT+06:00"), 18);
    timeZoneCB->addItem(i18n("GMT+07:00"), 19);
    timeZoneCB->addItem(i18n("GMT+08:00"), 20);
    timeZoneCB->addItem(i18n("GMT+09:00"), 21);
    timeZoneCB->addItem(i18n("GMT+10:00"), 22);
    timeZoneCB->addItem(i18n("GMT+11:00"), 23);
    timeZoneCB->addItem(i18n("GMT+12:00"), 24);
    timeZoneCB->addItem(i18n("GMT+13:00"), 25);
    timeZoneCB->addItem(i18n("GMT+14:00"), 26);
    timeZoneCB->setWhatsThis(i18n("Sets the time zone of the camera during "
                                  "picture shooting, so that the time stamps of the GPS "
                                  "can be converted to match the local time"));

    GPXLineWidthLabel_ = new QLabel(i18n("Track Width:"), GPXTracksGroupBox);
    GPXLineWidthInput_ = new QSpinBox(GPXTracksGroupBox);
    GPXLineWidthInput_->setValue(4);

    GPXColorLabel_ = new QLabel(i18n("Track Color:"), GPXTracksGroupBox);
    GPXTrackColor_ = new DColorSelector(GPXTracksGroupBox);
    GPXTrackColor_->setColor(QColor("#ffffff"));

    GPXTracksOpacityLabel_ = new QLabel(i18n("Opacity (%):"), GPXTracksGroupBox);
    GPXTracksOpacityInput_ = new QSpinBox(GPXTracksGroupBox);
    GPXTracksOpacityInput_->setRange(0, 100);
    GPXTracksOpacityInput_->setSingleStep(1);
    GPXTracksOpacityInput_->setValue(100);

    GPXAltitudeLabel_ = new QLabel(i18n("Track Altitude:"), GPXTracksGroupBox);
    GPXAltitudeCB_    = new QComboBox(GPXTracksGroupBox);
    GPXAltitudeCB_->addItem(i18n("clamp to ground"));
    GPXAltitudeCB_->addItem(i18n("relative to ground"));
    GPXAltitudeCB_->addItem(i18n("absolute"));
    GPXAltitudeCB_->setWhatsThis(i18n("<p>Specifies how the points are displayed"
                                      "<dl><dt>clamp to ground (default)</dt>"
                                      "<dd>Indicates to ignore an altitude specification</dd>"
                                      "<dt>relative to ground</dt>"
                                      "<dd>Sets the altitude of the element relative to the actual ground "
                                      "elevation of a particular location.</dd>"
                                      "<dt>absolute</dt>"
                                      "<dd>Sets the altitude of the coordinate relative to sea level, "
                                      "regardless of the actual elevation of the terrain beneath "
                                      "the element.</dd></dl>"));

    GPXTracksGroupBoxLayout->addWidget(GPXTracksCheckBox_,     0, 0, 1, 4);
    GPXTracksGroupBoxLayout->addWidget(GPXFileLabel_,          1, 0, 1, 1);
    GPXTracksGroupBoxLayout->addWidget(GPXFileUrlRequester_,   1, 1, 1, 3);
    GPXTracksGroupBoxLayout->addWidget(timeZoneLabel_,         2, 0, 1, 1);
    GPXTracksGroupBoxLayout->addWidget(timeZoneCB,             2, 1, 1, 3);
    GPXTracksGroupBoxLayout->addWidget(GPXLineWidthLabel_,     3, 0, 1, 1);
    GPXTracksGroupBoxLayout->addWidget(GPXLineWidthInput_,     3, 1, 1, 3);
    GPXTracksGroupBoxLayout->addWidget(GPXColorLabel_,         4, 0, 1, 1);
    GPXTracksGroupBoxLayout->addWidget(GPXTrackColor_,         4, 1, 1, 1);
    GPXTracksGroupBoxLayout->addWidget(GPXTracksOpacityLabel_, 4, 2, 1, 1);
    GPXTracksGroupBoxLayout->addWidget(GPXTracksOpacityInput_, 4, 3, 1, 1);
    GPXTracksGroupBoxLayout->addWidget(GPXAltitudeLabel_,      5, 0, 1, 1);
    GPXTracksGroupBoxLayout->addWidget(GPXAltitudeCB_,         5, 1, 1, 3);
    GPXTracksGroupBoxLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    GPXTracksGroupBoxLayout->setAlignment(Qt::AlignTop);

    // --------------------------------------------------------------

    m_geneBtn = new QPushButton(i18n("Generate KML file"), this);

    KMLExportConfigLayout->addWidget(TargetPreferenceGroupBox, 0, 0);
    KMLExportConfigLayout->addWidget(SizeGroupBox,             1, 0);
    KMLExportConfigLayout->addWidget(GPXTracksGroupBox,        2, 0);
    KMLExportConfigLayout->addWidget(m_geneBtn,                3, 0);
    KMLExportConfigLayout->setContentsMargins(spacing, spacing, spacing, spacing);

    // --------------------------------------------------------------

    connect(m_geneBtn, SIGNAL(clicked()),
            this, SLOT(slotKMLGenerate()));

    connect(GoogleMapTargetRadioButton_, SIGNAL(toggled(bool)),
            this, SLOT(slotGoogleMapTargetRadioButtonToggled(bool)));

    connect(GPXTracksCheckBox_, SIGNAL(toggled(bool)),
            this, SLOT(slotKMLTracksCheckButtonToggled(bool)));

    connect(this, SIGNAL(signalSetUIEnabled(bool)),
            m_dlg, SLOT(slotSetUIEnabled(bool)));

    connect(this, SIGNAL(signalProgressSetup(int,QString)),
            m_dlg, SLOT(slotProgressSetup(int,QString)));

    connect(&m_kmlExport, SIGNAL(signalProgressChanged(int)),
            m_dlg, SLOT(slotProgressChanged(int)));

    // --------------------------------------------------------------

    readSettings();
    slotGoogleMapTargetRadioButtonToggled(true);
    slotKMLTracksCheckButtonToggled(false);
}

KmlWidget::~KmlWidget()
{
    saveSettings();
}

void KmlWidget::slotKMLGenerate()
{
    emit(signalSetUIEnabled(false));
    m_geneBtn->setEnabled(false);
    emit(signalProgressSetup(m_model->rowCount(), i18n("Generate KML file")));

    saveSettings();

    QList<QUrl> urls;

    for (int i = 0; i < m_model->rowCount(); ++i)
    {
        GPSImageItem* const item = m_model->itemFromIndex(m_model->index(i, 0));

        if (item)
        {
            urls << item->url();
        }
    }

    m_kmlExport.setUrls(urls);
    m_kmlExport.generate();

    m_geneBtn->setEnabled(true);
    emit(signalSetUIEnabled(true));
}

void KmlWidget::slotGoogleMapTargetRadioButtonToggled(bool)
{
    if (GoogleMapTargetRadioButton_->isChecked())
    {
        DestinationUrlLabel_->setEnabled(true);
        DestinationUrl_->setEnabled(true);
        IconSizeLabel->setEnabled(false);
        IconSizeInput_->setEnabled(false);
    }
    else
    {
        DestinationUrlLabel_->setEnabled(false);
        DestinationUrl_->setEnabled(false);
        IconSizeLabel->setEnabled(true);
        IconSizeInput_->setEnabled(true);
    }
}

void KmlWidget::slotKMLTracksCheckButtonToggled(bool)
{
    if (GPXTracksCheckBox_->isChecked())
    {
        GPXFileUrlRequester_->setEnabled(true);
        GPXFileLabel_->setEnabled(true);
        timeZoneCB->setEnabled(true);
        GPXColorLabel_->setEnabled(true);
        GPXAltitudeLabel_->setEnabled(true);
        timeZoneLabel_->setEnabled(true);
        GPXAltitudeCB_->setEnabled(true);
        GPXTrackColor_->setEnabled(true);
        GPXLineWidthLabel_->setEnabled(true);
        GPXLineWidthInput_->setEnabled(true);
        GPXTracksOpacityInput_->setEnabled(true);
    }
    else
    {
        GPXFileUrlRequester_->setEnabled(false);
        GPXFileLabel_->setEnabled(false);
        timeZoneCB->setEnabled(false);
        GPXColorLabel_->setEnabled(false);
        GPXAltitudeLabel_->setEnabled(false);
        timeZoneLabel_->setEnabled(false);
        GPXAltitudeCB_->setEnabled(false);
        GPXTrackColor_->setEnabled(false);
        GPXLineWidthLabel_->setEnabled(false);
        GPXLineWidthInput_->setEnabled(false);
        GPXTracksOpacityInput_->setEnabled(false);
    }
}

void KmlWidget::readSettings()
{
    bool    localTarget;
    bool    optimize_googlemap;
    int     iconSize;

    //	int googlemapSize;
    int     size;
    QString UrlDestDir;
    QString baseDestDir;
    QString KMLFileName;
    int     AltitudeMode;

    bool    GPXtracks;
    QString GPXFile;
    int     TimeZone;
    int     LineWidth;
    QString GPXColor;
    int     GPXOpacity;
    int     GPXAltitudeMode;

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("KMLExport Settings"));

    localTarget	        = group.readEntry(QLatin1String("localTarget"), true);
    optimize_googlemap  = group.readEntry(QLatin1String("optimize_googlemap"), false);
    iconSize            = group.readEntry(QLatin1String("iconSize"), 33);
    // not saving this size as it should not change
    //	googlemapSize = group.readNumEntry("googlemapSize", 32);
    size                = group.readEntry(QLatin1String("size"), 320);
    // UrlDestDir have to have the trailing /
    baseDestDir	        = group.readEntry(QLatin1String("baseDestDir"), QString::fromUtf8("/tmp/"));
    UrlDestDir	        = group.readEntry(QLatin1String("UrlDestDir"),  QString::fromUtf8("http://www.example.com/"));
    KMLFileName         = group.readEntry(QLatin1String("KMLFileName"), QString::fromUtf8("kmldocument"));
    AltitudeMode        = group.readEntry(QLatin1String("Altitude Mode"), 0);

    GPXtracks           = group.readEntry(QLatin1String("UseGPXTracks"), false);
    GPXFile             = group.readEntry(QLatin1String("GPXFile"), QString());
    TimeZone            = group.readEntry(QLatin1String("Time Zone"), 12);
    LineWidth           = group.readEntry(QLatin1String("Line Width"), 4);
    GPXColor            = group.readEntry(QLatin1String("Track Color"), QString::fromUtf8("#17eeee"));
    GPXOpacity          = group.readEntry(QLatin1String("Track Opacity"), 64);
    GPXAltitudeMode     = group.readEntry(QLatin1String("GPX Altitude Mode"), 0);

    // -- Apply Settings to widgets ------------------------------

    LocalTargetRadioButton_->setChecked(localTarget);
    GoogleMapTargetRadioButton_->setChecked(optimize_googlemap);

    IconSizeInput_->setValue(iconSize);
    ImageSizeInput_->setValue(size);

    AltitudeCB_->setCurrentIndex(AltitudeMode);
    DestinationDirectory_->lineEdit()->setText(baseDestDir);
    DestinationUrl_->setText(UrlDestDir);
    FileName_->setText(KMLFileName);

    GPXTracksCheckBox_->setChecked(GPXtracks);
    timeZoneCB->setCurrentIndex(TimeZone);
    GPXLineWidthInput_->setValue(LineWidth);
    GPXTrackColor_->setColor(GPXColor);
    GPXTracksOpacityInput_->setValue(GPXOpacity);
    GPXAltitudeCB_->setCurrentIndex(GPXAltitudeMode);
}

void KmlWidget::saveSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("KMLExport Settings"));

    group.writeEntry(QLatin1String("localTarget"),        LocalTargetRadioButton_->isChecked());
    group.writeEntry(QLatin1String("optimize_googlemap"), GoogleMapTargetRadioButton_->isChecked());
    group.writeEntry(QLatin1String("iconSize"),           IconSizeInput_->value());
    group.writeEntry(QLatin1String("size"),               ImageSizeInput_->value());

    QString destination = DestinationDirectory_->lineEdit()->text();

    if (!destination.endsWith(QLatin1Char('/')))
    {
        destination.append(QLatin1Char('/'));
    }

    group.writeEntry(QLatin1String("baseDestDir"), destination);
    QString url = DestinationUrl_->text();

    if (!url.endsWith(QLatin1Char('/')))
    {
        url.append(QLatin1Char('/'));
    }

    group.writeEntry(QLatin1String("UrlDestDir"),        url);
    group.writeEntry(QLatin1String("KMLFileName"),       FileName_->text());
    group.writeEntry(QLatin1String("Altitude Mode"),     AltitudeCB_->currentIndex());
    group.writeEntry(QLatin1String("UseGPXTracks"),      GPXTracksCheckBox_->isChecked());
    group.writeEntry(QLatin1String("GPXFile"),           GPXFileUrlRequester_->lineEdit()->text());
    group.writeEntry(QLatin1String("Time Zone"),         timeZoneCB->currentIndex());
    group.writeEntry(QLatin1String("Line Width"),        GPXLineWidthInput_->value());
    group.writeEntry(QLatin1String("Track Color"),       GPXTrackColor_->color().name());
    group.writeEntry(QLatin1String("Track Opacity"),     GPXTracksOpacityInput_->value());
    group.writeEntry(QLatin1String("GPX Altitude Mode"), GPXAltitudeCB_->currentIndex());

    config->sync();
}

} // namespace Digikam
