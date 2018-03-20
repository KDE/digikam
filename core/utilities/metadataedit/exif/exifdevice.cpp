/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-10-18
 * Description : EXIF device settings page.
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "exifdevice.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QLabel>
#include <QValidator>
#include <QGridLayout>
#include <QApplication>
#include <QStyle>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "metadatacheckbox.h"
#include "dmetadata.h"
#include "dexpanderbox.h"

namespace Digikam
{

class EXIFDevice::Private
{
public:

    Private()
    {
        makeCheck                = 0;
        modelCheck               = 0;
        deviceTypeCheck          = 0;
        exposureTimeCheck        = 0;
        exposureProgramCheck     = 0;
        exposureModeCheck        = 0;
        exposureBiasCheck        = 0;
        ISOSpeedCheck            = 0;
        meteringModeCheck        = 0;
        sensingMethodCheck       = 0;
        sceneTypeCheck           = 0;
        subjectDistanceTypeCheck = 0;
        exposureProgramCB        = 0;
        exposureModeCB           = 0;
        ISOSpeedCB               = 0;
        meteringModeCB           = 0;
        sensingMethodCB          = 0;
        sceneTypeCB              = 0;
        subjectDistanceTypeCB    = 0;
        exposureTimeNumEdit      = 0;
        exposureTimeDenEdit      = 0;
        exposureBiasEdit         = 0;
        makeEdit                 = 0;
        modelEdit                = 0;
        deviceTypeCB             = 0;
    }

    QCheckBox*        makeCheck;
    QCheckBox*        modelCheck;
    QCheckBox*        exposureTimeCheck;
    QCheckBox*        exposureBiasCheck;

    QComboBox*        deviceTypeCB;
    QComboBox*        exposureProgramCB;
    QComboBox*        exposureModeCB;
    QComboBox*        ISOSpeedCB;
    QComboBox*        meteringModeCB;
    QComboBox*        sensingMethodCB;
    QComboBox*        sceneTypeCB;
    QComboBox*        subjectDistanceTypeCB;

    QLineEdit*        makeEdit;
    QLineEdit*        modelEdit;

    QSpinBox*         exposureTimeNumEdit;
    QSpinBox*         exposureTimeDenEdit;

    QDoubleSpinBox*   exposureBiasEdit;

    MetadataCheckBox* deviceTypeCheck;
    MetadataCheckBox* exposureProgramCheck;
    MetadataCheckBox* exposureModeCheck;
    MetadataCheckBox* meteringModeCheck;
    MetadataCheckBox* ISOSpeedCheck;
    MetadataCheckBox* sensingMethodCheck;
    MetadataCheckBox* sceneTypeCheck;
    MetadataCheckBox* subjectDistanceTypeCheck;
};

EXIFDevice::EXIFDevice(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    QGridLayout* const grid = new QGridLayout(this);

    // EXIF only accept printable Ascii char.
    QRegExp asciiRx(QLatin1String("[\x20-\x7F]+$"));
    QValidator* const asciiValidator = new QRegExpValidator(asciiRx, this);

    // --------------------------------------------------------

    d->makeCheck = new QCheckBox(i18n("Device manufacturer (*):"), this);
    d->makeEdit  = new QLineEdit(this);
    d->makeEdit->setClearButtonEnabled(true);
    d->makeEdit->setValidator(asciiValidator);
    d->makeEdit->setWhatsThis(i18n("Set here the manufacturer of image input equipment used to "
                                   "take the picture. This field is limited to ASCII characters."));

    // --------------------------------------------------------

    d->modelCheck = new QCheckBox(i18n("Device model (*):"), this);
    d->modelEdit  = new QLineEdit(this);
    d->modelEdit->setClearButtonEnabled(true);
    d->modelEdit->setValidator(asciiValidator);
    d->modelEdit->setWhatsThis(i18n("Set here the model of image input equipment used to "
                                    "take the picture. This field is limited to ASCII characters."));

    // --------------------------------------------------------

    d->deviceTypeCheck = new MetadataCheckBox(i18n("Device type:"), this);
    d->deviceTypeCB    = new QComboBox(this);
    d->deviceTypeCB->insertItem(0, i18n("Film scanner"));
    d->deviceTypeCB->insertItem(1, i18n("Reflection print scanner"));
    d->deviceTypeCB->insertItem(2, i18n("Digital still camera"));
    d->deviceTypeCB->setWhatsThis(i18n("Select here the image input equipment type used to "
                                       "take the picture."));

    QLabel* const warning = new QLabel(i18n("<b>Warning: EXIF <b><a href="
                 "'http://en.wikipedia.org/wiki/Exchangeable_image_file_format#MakerNote_data'>"
                 "Makernotes</a></b> can be unreadable if you set "
                 "wrong device manufacturer/model description.</b>"), this);
    warning->setOpenExternalLinks(true);
    warning->setWordWrap(true);
    warning->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    DLineWidget* const line = new DLineWidget(Qt::Horizontal, this);

    // --------------------------------------------------------

    d->exposureTimeCheck        = new QCheckBox(i18n("Exposure time (seconds):"), this);
    d->exposureTimeNumEdit      = new QSpinBox(this);
    d->exposureTimeNumEdit->setMinimum(1);
    d->exposureTimeNumEdit->setMaximum(100000);
    d->exposureTimeNumEdit->setSingleStep(1);
    d->exposureTimeNumEdit->setValue(1);
    d->exposureTimeDenEdit      = new QSpinBox(this);
    d->exposureTimeDenEdit->setMinimum(1);
    d->exposureTimeDenEdit->setMaximum(100000);
    d->exposureTimeDenEdit->setSingleStep(1);
    d->exposureTimeDenEdit->setValue(1);
    QLabel* const exposureLabel = new QLabel(QLatin1String("/"), this);
    exposureLabel->setAlignment (Qt::AlignRight|Qt::AlignVCenter);
    d->exposureTimeCheck->setWhatsThis(i18n("Set on this option to set the exposure time "
                                            "of picture, given in seconds."));

    // --------------------------------------------------------

    d->exposureProgramCheck = new MetadataCheckBox(i18n("Exposure program:"), this);
    d->exposureProgramCB    = new QComboBox(this);
    d->exposureProgramCB->insertItem(0, i18nc("exposure program", "Not defined"));
    d->exposureProgramCB->insertItem(1, i18nc("exposure program", "Manual"));
    d->exposureProgramCB->insertItem(2, i18nc("exposure program", "Auto"));
    d->exposureProgramCB->insertItem(3, i18nc("exposure program", "Aperture priority"));
    d->exposureProgramCB->insertItem(4, i18nc("exposure program", "Shutter priority"));
    d->exposureProgramCB->insertItem(5, i18nc("exposure program", "Creative program"));
    d->exposureProgramCB->insertItem(6, i18nc("exposure program", "Action program"));
    d->exposureProgramCB->insertItem(7, i18nc("exposure program", "Portrait mode"));
    d->exposureProgramCB->insertItem(8, i18nc("exposure program", "Landscape mode"));
    d->exposureProgramCB->setWhatsThis(i18n("Select here the program used by the camera "
                                            "to set exposure when the picture was taken."));

    // --------------------------------------------------------

    d->exposureModeCheck = new MetadataCheckBox(i18n("Exposure mode:"), this);
    d->exposureModeCB    = new QComboBox(this);
    d->exposureModeCB->insertItem(0, i18nc("exposure mode", "Auto"));
    d->exposureModeCB->insertItem(1, i18nc("exposure mode", "Manual"));
    d->exposureModeCB->insertItem(2, i18nc("exposure mode", "Auto bracket"));
    d->exposureModeCB->setWhatsThis(i18n("Select here the mode used by the camera "
                                         "to set exposure when the picture was taken. "
                                         "In auto-bracketing mode, the camera shoots a "
                                         "series of frames of the same scene at different "
                                         "exposure settings."));

    // --------------------------------------------------------

    d->exposureBiasCheck = new QCheckBox(i18n("Exposure bias (APEX):"), this);
    d->exposureBiasEdit  = new QDoubleSpinBox(this);
    d->exposureBiasEdit->setRange(-99.99, 99.99);
    d->exposureBiasEdit->setSingleStep(0.1);
    d->exposureBiasEdit->setValue(0.0);
    d->exposureBiasEdit->setWhatsThis(i18n("Set here the exposure bias value (in APEX units) "
                                           "used by camera to take the picture."));

    DLineWidget* const line2 = new DLineWidget(Qt::Horizontal, this);

    // --------------------------------------------------------

    d->meteringModeCheck = new MetadataCheckBox(i18n("Metering mode:"), this);
    d->meteringModeCB    = new QComboBox(this);
    d->meteringModeCB->insertItem(0, i18nc("metering mode", "Unknown"));
    d->meteringModeCB->insertItem(1, i18nc("metering mode", "Average"));
    d->meteringModeCB->insertItem(2, i18nc("metering mode", "Center weighted average"));
    d->meteringModeCB->insertItem(3, i18nc("metering mode", "Spot"));
    d->meteringModeCB->insertItem(4, i18nc("metering mode", "Multi-spot"));
    d->meteringModeCB->insertItem(5, i18nc("metering mode", "Multi-segment"));
    d->meteringModeCB->insertItem(6, i18nc("metering mode", "Partial"));
    d->meteringModeCB->insertItem(7, i18nc("metering mode", "Other"));
    d->meteringModeCB->setWhatsThis(i18n("Select here the metering mode used by the camera "
                                         "to set exposure when the picture was taken."));

    // --------------------------------------------------------

    d->ISOSpeedCheck = new MetadataCheckBox(i18n("Sensitivity (ISO):"), this);
    d->ISOSpeedCB    = new QComboBox(this);
    d->ISOSpeedCB->insertItem(1, QLatin1String("10"));
    d->ISOSpeedCB->insertItem(2, QLatin1String("12"));
    d->ISOSpeedCB->insertItem(3, QLatin1String("16"));
    d->ISOSpeedCB->insertItem(4, QLatin1String("20"));
    d->ISOSpeedCB->insertItem(5, QLatin1String("25"));
    d->ISOSpeedCB->insertItem(6, QLatin1String("32"));
    d->ISOSpeedCB->insertItem(7, QLatin1String("40"));
    d->ISOSpeedCB->insertItem(8, QLatin1String("50"));
    d->ISOSpeedCB->insertItem(9, QLatin1String("64"));
    d->ISOSpeedCB->insertItem(10, QLatin1String("80"));
    d->ISOSpeedCB->insertItem(11, QLatin1String("100"));
    d->ISOSpeedCB->insertItem(12, QLatin1String("125"));
    d->ISOSpeedCB->insertItem(13, QLatin1String("160"));
    d->ISOSpeedCB->insertItem(14, QLatin1String("200"));
    d->ISOSpeedCB->insertItem(15, QLatin1String("250"));
    d->ISOSpeedCB->insertItem(16, QLatin1String("320"));
    d->ISOSpeedCB->insertItem(17, QLatin1String("400"));
    d->ISOSpeedCB->insertItem(18, QLatin1String("500"));
    d->ISOSpeedCB->insertItem(19, QLatin1String("640"));
    d->ISOSpeedCB->insertItem(20, QLatin1String("800"));
    d->ISOSpeedCB->insertItem(21, QLatin1String("1000"));
    d->ISOSpeedCB->insertItem(22, QLatin1String("1250"));
    d->ISOSpeedCB->insertItem(23, QLatin1String("1600"));
    d->ISOSpeedCB->insertItem(24, QLatin1String("2000"));
    d->ISOSpeedCB->insertItem(25, QLatin1String("2500"));
    d->ISOSpeedCB->insertItem(26, QLatin1String("3200"));
    d->ISOSpeedCB->insertItem(27, QLatin1String("4000"));
    d->ISOSpeedCB->insertItem(28, QLatin1String("5000"));
    d->ISOSpeedCB->insertItem(29, QLatin1String("6400"));
    d->ISOSpeedCB->insertItem(30, QLatin1String("8000"));
    d->ISOSpeedCB->insertItem(31, QLatin1String("10000"));
    d->ISOSpeedCB->insertItem(32, QLatin1String("12500"));
    d->ISOSpeedCB->insertItem(33, QLatin1String("16000"));
    d->ISOSpeedCB->insertItem(34, QLatin1String("20000"));
    d->ISOSpeedCB->insertItem(35, QLatin1String("25000"));
    d->ISOSpeedCB->insertItem(36, QLatin1String("32000"));
    d->ISOSpeedCB->setWhatsThis(i18n("Select here the ISO Speed of the camera "
                                     "which took the picture."));

    // --------------------------------------------------------

    d->sensingMethodCheck = new MetadataCheckBox(i18n("Sensing method:"), this);
    d->sensingMethodCB    = new QComboBox(this);
    d->sensingMethodCB->insertItem(0, i18n("Not defined"));
    d->sensingMethodCB->insertItem(1, i18n("One-chip color area"));
    d->sensingMethodCB->insertItem(2, i18n("Two-chip color area"));
    d->sensingMethodCB->insertItem(3, i18n("Three-chip color area"));
    d->sensingMethodCB->insertItem(4, i18n("Color sequential area"));
    d->sensingMethodCB->insertItem(5, i18n("Trilinear sensor"));
    d->sensingMethodCB->insertItem(6, i18n("Color sequential linear"));
    d->sensingMethodCB->setWhatsThis(i18n("Select here the image sensor type used by the camera "
                                          "to take the picture."));

    // --------------------------------------------------------

    d->sceneTypeCheck = new MetadataCheckBox(i18n("Scene capture type:"), this);
    d->sceneTypeCB    = new QComboBox(this);
    d->sceneTypeCB->insertItem(0, i18n("Standard"));
    d->sceneTypeCB->insertItem(1, i18n("Landscape"));
    d->sceneTypeCB->insertItem(2, i18n("Portrait"));
    d->sceneTypeCB->insertItem(3, i18n("Night scene"));
    d->sceneTypeCB->setWhatsThis(i18n("Select here the type of scene used by the camera "
                                      "to take the picture."));

    // --------------------------------------------------------

    d->subjectDistanceTypeCheck = new MetadataCheckBox(i18n("Subject distance type:"), this);
    d->subjectDistanceTypeCB    = new QComboBox(this);
    d->subjectDistanceTypeCB->insertItem(1, i18n("Unknown"));
    d->subjectDistanceTypeCB->insertItem(2, i18n("Macro"));
    d->subjectDistanceTypeCB->insertItem(3, i18n("Close view"));
    d->subjectDistanceTypeCB->insertItem(4, i18n("Distant view"));
    d->subjectDistanceTypeCB->setWhatsThis(i18n("Select here the type of distance between "
                                                "the subject and the image input equipment."));

    // --------------------------------------------------------

    QLabel* const note = new QLabel(i18n("<b>Note: "
                 "<b><a href='http://en.wikipedia.org/wiki/EXIF'>EXIF</a></b> "
                 "text tags marked by (*) only support printable "
                 "<b><a href='http://en.wikipedia.org/wiki/Ascii'>ASCII</a></b> "
                 "characters.</b>"), this);
    note->setOpenExternalLinks(true);
    note->setWordWrap(true);
    note->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    // --------------------------------------------------------

    grid->addWidget(d->makeCheck,                   0, 0, 1, 1);
    grid->addWidget(d->makeEdit,                    0, 2, 1, 4);
    grid->addWidget(d->modelCheck,                  1, 0, 1, 1);
    grid->addWidget(d->modelEdit,                   1, 2, 1, 4);
    grid->addWidget(d->deviceTypeCheck,             2, 0, 1, 1);
    grid->addWidget(d->deviceTypeCB,                2, 2, 1, 4);
    grid->addWidget(warning,                        3, 0, 1, 6);
    grid->addWidget(line,                           4, 0, 1, 6);
    grid->addWidget(d->exposureTimeCheck,           5, 0, 1, 1);
    grid->addWidget(d->exposureTimeNumEdit,         5, 2, 1, 1);
    grid->addWidget(exposureLabel,                  5, 3, 1, 1);
    grid->addWidget(d->exposureTimeDenEdit,         5, 4, 1, 1);
    grid->addWidget(d->exposureProgramCheck,        6, 0, 1, 1);
    grid->addWidget(d->exposureProgramCB,           6, 2, 1, 4);
    grid->addWidget(d->exposureModeCheck,           7, 0, 1, 1);
    grid->addWidget(d->exposureModeCB,              7, 2, 1, 4);
    grid->addWidget(d->exposureBiasCheck,           8, 0, 1, 1);
    grid->addWidget(d->exposureBiasEdit,            8, 2, 1, 1);
    grid->addWidget(line2,                          9, 0, 1, 6);
    grid->addWidget(d->meteringModeCheck,          10, 0, 1, 1);
    grid->addWidget(d->meteringModeCB,             10, 2, 1, 4);
    grid->addWidget(d->ISOSpeedCheck,              11, 0, 1, 1);
    grid->addWidget(d->ISOSpeedCB,                 11, 2, 1, 4);
    grid->addWidget(d->sensingMethodCheck,         12, 0, 1, 1);
    grid->addWidget(d->sensingMethodCB,            12, 2, 1, 4);
    grid->addWidget(d->sceneTypeCheck,             13, 0, 1, 1);
    grid->addWidget(d->sceneTypeCB,                13, 2, 1, 4);
    grid->addWidget(d->subjectDistanceTypeCheck,   14, 0, 1, 1);
    grid->addWidget(d->subjectDistanceTypeCB,      14, 2, 1, 4);
    grid->addWidget(note, 15, 0, 1, 5+1);
    grid->setColumnStretch(1, 10);
    grid->setColumnStretch(5, 10);
    grid->setRowStretch(16, 10);
    grid->setContentsMargins(QMargins());
    grid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // --------------------------------------------------------

    connect(d->makeCheck, SIGNAL(toggled(bool)),
            d->makeEdit, SLOT(setEnabled(bool)));

    connect(d->modelCheck, SIGNAL(toggled(bool)),
            d->modelEdit, SLOT(setEnabled(bool)));

    connect(d->deviceTypeCheck, SIGNAL(toggled(bool)),
            d->deviceTypeCB, SLOT(setEnabled(bool)));

    connect(d->exposureTimeCheck, SIGNAL(toggled(bool)),
            d->exposureTimeNumEdit, SLOT(setEnabled(bool)));

    connect(d->exposureTimeCheck, SIGNAL(toggled(bool)),
            d->exposureTimeDenEdit, SLOT(setEnabled(bool)));

    connect(d->exposureProgramCheck, SIGNAL(toggled(bool)),
            d->exposureProgramCB, SLOT(setEnabled(bool)));

    connect(d->exposureModeCheck, SIGNAL(toggled(bool)),
            d->exposureModeCB, SLOT(setEnabled(bool)));

    connect(d->exposureBiasCheck, SIGNAL(toggled(bool)),
            d->exposureBiasEdit, SLOT(setEnabled(bool)));

    connect(d->meteringModeCheck, SIGNAL(toggled(bool)),
            d->meteringModeCB, SLOT(setEnabled(bool)));

    connect(d->ISOSpeedCheck, SIGNAL(toggled(bool)),
            d->ISOSpeedCB, SLOT(setEnabled(bool)));

    connect(d->sensingMethodCheck, SIGNAL(toggled(bool)),
            d->sensingMethodCB, SLOT(setEnabled(bool)));

    connect(d->sceneTypeCheck, SIGNAL(toggled(bool)),
            d->sceneTypeCB, SLOT(setEnabled(bool)));

    connect(d->subjectDistanceTypeCheck, SIGNAL(toggled(bool)),
            d->subjectDistanceTypeCB, SLOT(setEnabled(bool)));

    // --------------------------------------------------------

    connect(d->makeCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->modelCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->exposureTimeCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->exposureBiasCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->deviceTypeCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->exposureProgramCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->exposureModeCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->meteringModeCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->ISOSpeedCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->sensingMethodCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->sceneTypeCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->subjectDistanceTypeCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    // --------------------------------------------------------

    connect(d->deviceTypeCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->exposureProgramCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->exposureModeCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->ISOSpeedCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->meteringModeCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->sensingMethodCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->sceneTypeCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->subjectDistanceTypeCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->makeEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(signalModified()));

    connect(d->modelEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(signalModified()));

    connect(d->exposureTimeNumEdit, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalModified()));

    connect(d->exposureTimeDenEdit, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalModified()));

    connect(d->exposureBiasEdit, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalModified()));
}

EXIFDevice::~EXIFDevice()
{
    delete d;
}

void EXIFDevice::readMetadata(QByteArray& exifData)
{
    blockSignals(true);
    DMetadata meta;
    meta.setExif(exifData);
    long int num=1, den=1;
    long     val=0;
    QString  data;

    d->makeEdit->clear();
    d->makeCheck->setChecked(false);
    data = meta.getExifTagString("Exif.Image.Make", false);

    if (!data.isNull())
    {
        d->makeEdit->setText(data);
        d->makeCheck->setChecked(true);
    }

    d->makeEdit->setEnabled(d->makeCheck->isChecked());

    d->modelEdit->clear();
    d->modelCheck->setChecked(false);
    data = meta.getExifTagString("Exif.Image.Model", false);

    if (!data.isNull())
    {
        d->modelEdit->setText(data);
        d->modelCheck->setChecked(true);
    }

    d->modelEdit->setEnabled(d->modelCheck->isChecked());

    d->deviceTypeCB->setCurrentIndex(2);     // DSC
    d->deviceTypeCheck->setChecked(false);

    if (meta.getExifTagLong("Exif.Photo.FileSource", val))
    {
        if (val>0 && val<4)
        {
            d->deviceTypeCB->setCurrentIndex(val-1);
            d->deviceTypeCheck->setChecked(true);
        }
        else
        {
            d->deviceTypeCheck->setValid(false);
        }
    }

    d->deviceTypeCB->setEnabled(d->deviceTypeCheck->isChecked());

    d->exposureTimeNumEdit->setValue(1);
    d->exposureTimeDenEdit->setValue(1);
    d->exposureTimeCheck->setChecked(false);

    if (meta.getExifTagRational("Exif.Photo.ExposureTime", num, den))
    {
        d->exposureTimeNumEdit->setValue(num);
        d->exposureTimeDenEdit->setValue(den);
        d->exposureTimeCheck->setChecked(true);
    }
    else if (meta.getExifTagRational("Exif.Photo.ShutterSpeedValue", num, den))
    {
        double tmp = std::exp(std::log(2.0) * (double)(num)/(double)(den));

        if (tmp > 1.0)
            num = (long int)(tmp + 0.5);
        else
            den = (long int)(1.0/tmp + 0.5);

        d->exposureTimeNumEdit->setValue(num);
        d->exposureTimeDenEdit->setValue(den);
        d->exposureTimeCheck->setChecked(true);
    }

    d->exposureTimeNumEdit->setEnabled(d->exposureTimeCheck->isChecked());
    d->exposureTimeDenEdit->setEnabled(d->exposureTimeCheck->isChecked());

    d->exposureProgramCB->setCurrentIndex(0);
    d->exposureProgramCheck->setChecked(false);

    if (meta.getExifTagLong("Exif.Photo.ExposureProgram", val))
    {
        if (val>=0 && val <=8)
        {
            d->exposureProgramCB->setCurrentIndex(val);
            d->exposureProgramCheck->setChecked(true);
        }
        else
        {
            d->exposureProgramCheck->setValid(false);
        }
    }

    d->exposureProgramCB->setEnabled(d->exposureProgramCheck->isChecked());

    d->exposureModeCB->setCurrentIndex(0);
    d->exposureModeCheck->setChecked(false);

    if (meta.getExifTagLong("Exif.Photo.ExposureMode", val))
    {
        if (val >= 0 && val <= 2)
        {
            d->exposureModeCB->setCurrentIndex(val);
            d->exposureModeCheck->setChecked(true);
        }
        else
        {
            d->exposureModeCheck->setValid(false);
        }
    }

    d->exposureModeCB->setEnabled(d->exposureModeCheck->isChecked());

    d->exposureBiasEdit->setValue(0.0);
    d->exposureBiasCheck->setChecked(false);

    if (meta.getExifTagRational("Exif.Photo.ExposureBiasValue", num, den))
    {
        d->exposureBiasEdit->setValue((double)(num) / (double)(den));
        d->exposureBiasCheck->setChecked(true);
    }

    d->exposureBiasEdit->setEnabled(d->exposureBiasCheck->isChecked());

    d->meteringModeCB->setCurrentIndex(0);
    d->meteringModeCheck->setChecked(false);

    if (meta.getExifTagLong("Exif.Photo.MeteringMode", val))
    {
        if ((val >= 0 && val <= 6) || val == 255)
        {
            d->meteringModeCB->setCurrentIndex(val == 255 ? 7 : val);
            d->meteringModeCheck->setChecked(true);
        }
        else
        {
            d->meteringModeCheck->setValid(false);
        }
    }

    d->meteringModeCB->setEnabled(d->meteringModeCheck->isChecked());

    d->ISOSpeedCB->setCurrentIndex(10);       // 100 ISO
    d->ISOSpeedCheck->setChecked(false);

    if (meta.getExifTagLong("Exif.Photo.ISOSpeedRatings", val))
    {
        int item = -1;

        for (int i = 0 ; i < d->ISOSpeedCB->count() ; ++i)
        {
            if (d->ISOSpeedCB->itemText(i) == QString::number(val))
                item = i;
        }

        if (item != -1)
        {
            d->ISOSpeedCB->setCurrentIndex(item);
            d->ISOSpeedCheck->setChecked(true);
        }
        else
        {
            d->ISOSpeedCheck->setValid(false);
        }
    }
    else if (meta.getExifTagRational("Exif.Photo.ExposureIndex", num, den))
    {
        val = num / den;
        int item = -1;

        for (int i = 0 ; i < d->ISOSpeedCB->count() ; ++i)
        {
            if (d->ISOSpeedCB->itemText(i) == QString::number(val))
                item = i;
        }

        if (item != -1)
        {
            d->ISOSpeedCB->setCurrentIndex(item);
            d->ISOSpeedCheck->setChecked(true);
        }
        else
        {
            d->ISOSpeedCheck->setValid(false);
        }
    }

    d->ISOSpeedCB->setEnabled(d->ISOSpeedCheck->isChecked());

    d->sensingMethodCB->setCurrentIndex(0);
    d->sensingMethodCheck->setChecked(false);

    if (meta.getExifTagLong("Exif.Photo.SensingMethod", val))
    {
        if (val >= 1 && val <= 8 && val != 6)
        {
            d->sensingMethodCB->setCurrentIndex(val > 6 ? val-2 : val-1);
            d->sensingMethodCheck->setChecked(true);
        }
        else
        {
            d->sensingMethodCheck->setValid(false);
        }
    }

    d->sensingMethodCB->setEnabled(d->sensingMethodCheck->isChecked());

    d->sceneTypeCB->setCurrentIndex(0);
    d->sceneTypeCheck->setChecked(false);

    if (meta.getExifTagLong("Exif.Photo.SceneCaptureType", val))
    {
        if (val >= 0 && val <= 3)
        {
            d->sceneTypeCB->setCurrentIndex(val);
            d->sceneTypeCheck->setChecked(true);
        }
        else
        {
            d->sceneTypeCheck->setValid(false);
        }
    }

    d->sceneTypeCB->setEnabled(d->sceneTypeCheck->isChecked());

    d->subjectDistanceTypeCB->setCurrentIndex(0);
    d->subjectDistanceTypeCheck->setChecked(false);

    if (meta.getExifTagLong("Exif.Photo.SubjectDistanceRange", val))
    {
        if (val >= 0 && val <= 3)
        {
            d->subjectDistanceTypeCB->setCurrentIndex(val);
            d->subjectDistanceTypeCheck->setChecked(true);
        }
        else
        {
            d->subjectDistanceTypeCheck->setValid(false);
        }
    }

    d->subjectDistanceTypeCB->setEnabled(d->subjectDistanceTypeCheck->isChecked());

    blockSignals(false);
}

void EXIFDevice::applyMetadata(QByteArray& exifData)
{
    DMetadata meta;
    meta.setExif(exifData);
    long int num=1, den=1;

    if (d->makeCheck->isChecked())
        meta.setExifTagString("Exif.Image.Make", d->makeEdit->text());
    else
        meta.removeExifTag("Exif.Image.Make");

    if (d->modelCheck->isChecked())
        meta.setExifTagString("Exif.Image.Model", d->modelEdit->text());
    else
        meta.removeExifTag("Exif.Image.Model");

    if (d->deviceTypeCheck->isChecked())
        meta.setExifTagLong("Exif.Photo.FileSource", d->deviceTypeCB->currentIndex()+1);
    else if (d->deviceTypeCheck->isValid())
        meta.removeExifTag("Exif.Photo.FileSource");

    if (d->exposureTimeCheck->isChecked())
    {
        meta.setExifTagRational("Exif.Photo.ExposureTime", d->exposureTimeNumEdit->value(),
                                      d->exposureTimeDenEdit->value());

        double exposureTime = (double)(d->exposureTimeNumEdit->value())/
                              (double)(d->exposureTimeDenEdit->value());
        double shutterSpeed = (-1.0)*(std::log(exposureTime)/std::log(2.0));
        meta.convertToRational(shutterSpeed, &num, &den, 8);
        meta.setExifTagRational("Exif.Photo.ShutterSpeedValue", num, den);
    }
    else
    {
        meta.removeExifTag("Exif.Photo.ExposureTime");
        meta.removeExifTag("Exif.Photo.ShutterSpeedValue");
    }

    if (d->exposureProgramCheck->isChecked())
        meta.setExifTagLong("Exif.Photo.ExposureProgram", d->exposureProgramCB->currentIndex());
    else if (d->exposureProgramCheck->isValid())
        meta.removeExifTag("Exif.Photo.ExposureProgram");

    if (d->exposureModeCheck->isChecked())
        meta.setExifTagLong("Exif.Photo.ExposureMode", d->exposureModeCB->currentIndex());
    else if (d->exposureModeCheck->isValid())
        meta.removeExifTag("Exif.Photo.ExposureMode");

    if (d->exposureBiasCheck->isChecked())
    {
        meta.convertToRational(d->exposureBiasEdit->value(), &num, &den, 1);
        meta.setExifTagRational("Exif.Photo.ExposureBiasValue", num, den);
    }
    else
    {
        meta.removeExifTag("Exif.Photo.ExposureBiasValue");
    }

    if (d->meteringModeCheck->isChecked())
    {
        long met = d->meteringModeCB->currentIndex();
        meta.setExifTagLong("Exif.Photo.MeteringMode", met > 6 ? 255 : met);
    }
    else if (d->meteringModeCheck->isValid())
        meta.removeExifTag("Exif.Photo.MeteringMode");

    if (d->ISOSpeedCheck->isChecked())
    {
        meta.setExifTagLong("Exif.Photo.ISOSpeedRatings", d->ISOSpeedCB->currentText().toLong());

        meta.convertToRational(d->ISOSpeedCB->currentText().toDouble(), &num, &den, 1);
        meta.setExifTagRational("Exif.Photo.ExposureIndex", num, den);
    }
    else if (d->ISOSpeedCheck->isValid())
    {
        meta.removeExifTag("Exif.Photo.ISOSpeedRatings");
        meta.removeExifTag("Exif.Photo.ExposureIndex");
    }

    if (d->sensingMethodCheck->isChecked())
    {
        long sem = d->sensingMethodCB->currentIndex();
        meta.setExifTagLong("Exif.Photo.SensingMethod", sem > 4 ? sem+2 : sem+1);
    }
    else if (d->sensingMethodCheck->isValid())
    {
        meta.removeExifTag("Exif.Photo.SensingMethod");
    }

    if (d->sceneTypeCheck->isChecked())
        meta.setExifTagLong("Exif.Photo.SceneCaptureType", d->sceneTypeCB->currentIndex());
    else if (d->sceneTypeCheck->isValid())
        meta.removeExifTag("Exif.Photo.SceneCaptureType");

    if (d->subjectDistanceTypeCheck->isChecked())
        meta.setExifTagLong("Exif.Photo.SubjectDistanceRange", d->subjectDistanceTypeCB->currentIndex());
    else if (d->subjectDistanceTypeCheck->isValid())
        meta.removeExifTag("Exif.Photo.SubjectDistanceRange");

    exifData = meta.getExifEncoded();
}

}  // namespace Digikam
