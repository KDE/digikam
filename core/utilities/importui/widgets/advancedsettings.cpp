/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-08-12
 * Description : advanced settings for camera interface.
 *
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "advancedsettings.h"

// Qt includes

#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QToolButton>
#include <QApplication>
#include <QStyle>
#include <QComboBox>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "templateselector.h"
#include "ddatetimeedit.h"
#include "template.h"

namespace Digikam
{

class AdvancedSettings::Private
{
public:

    Private()
        : formatLabel(0),
          autoRotateCheck(0),
          convertJpegCheck(0),
          fixDateTimeCheck(0),
          documentNameCheck(0),
          losslessFormat(0),
          dateTimeEdit(0),
          templateSelector(0)
    {
    }

    QLabel*           formatLabel;

    QCheckBox*        autoRotateCheck;
    QCheckBox*        convertJpegCheck;
    QCheckBox*        fixDateTimeCheck;
    QCheckBox*        documentNameCheck;

    QComboBox*        losslessFormat;

    DDateTimeEdit*    dateTimeEdit;

    TemplateSelector* templateSelector;
};

AdvancedSettings::AdvancedSettings(QWidget* const parent)
    : QWidget(parent), d(new Private)
{
    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QVBoxLayout* const onFlyVlay = new QVBoxLayout(this);
    d->templateSelector          = new TemplateSelector(this);
    d->documentNameCheck         = new QCheckBox(i18nc("@option:check", "Write the document name to EXIF"), this);
    d->fixDateTimeCheck          = new QCheckBox(i18nc("@option:check", "Fix internal date && time"), this);
    d->dateTimeEdit              = new DDateTimeEdit(this, QLatin1String("datepicker"));
    d->autoRotateCheck           = new QCheckBox(i18nc("@option:check", "Auto-rotate/flip image"), this);
    d->convertJpegCheck          = new QCheckBox(i18nc("@option:check", "Convert to lossless file format"), this);
    DHBox* const hbox2           = new DHBox(this);
    d->formatLabel               = new QLabel(i18n("New image format:"), hbox2);
    d->losslessFormat            = new QComboBox(hbox2);
    d->losslessFormat->insertItem(0, QLatin1String("PNG"));
    d->losslessFormat->insertItem(1, QLatin1String("TIF"));
    d->losslessFormat->insertItem(2, QLatin1String("PGF"));
#ifdef HAVE_JASPER
    d->losslessFormat->insertItem(3, QLatin1String("JP2"));
#endif // HAVE_JASPER

    onFlyVlay->addWidget(d->templateSelector);
    onFlyVlay->addWidget(d->documentNameCheck);
    onFlyVlay->addWidget(d->fixDateTimeCheck);
    onFlyVlay->addWidget(d->dateTimeEdit);
    onFlyVlay->addWidget(d->autoRotateCheck);
    onFlyVlay->addWidget(d->convertJpegCheck);
    onFlyVlay->addWidget(hbox2);
    onFlyVlay->addStretch();
    onFlyVlay->setContentsMargins(spacing, spacing, spacing, spacing);
    onFlyVlay->setSpacing(spacing);

    setWhatsThis(i18n("Set here all options to fix/transform JPEG files automatically "
                      "as they are downloaded."));
    d->autoRotateCheck->setWhatsThis(i18n("Enable this option if you want images automatically "
                                          "rotated or flipped using EXIF information provided by the camera."));
    d->templateSelector->setWhatsThis(i18n("Select here which metadata template you want to apply "
                                           "to images."));
    d->documentNameCheck->setWhatsThis(i18n("Enable this option to write the document name to the EXIF metadata. "
                                            "The document name is the original file name of the imported file."));
    d->fixDateTimeCheck->setWhatsThis(i18n("Enable this option to set date and time metadata "
                                           "tags to the right values if your camera does not set "
                                           "these tags correctly when pictures are taken. The values will "
                                           "be saved in the DateTimeDigitized and DateTimeCreated EXIF, XMP, and IPTC tags."));
    d->convertJpegCheck->setWhatsThis(i18n("Enable this option to automatically convert "
                                           "all JPEG files to a lossless image format. <b>Note:</b> Image conversion can take a "
                                           "while on a slow computer."));
    d->losslessFormat->setWhatsThis(i18n("Select your preferred lossless image file format to "
                                         "convert to. <b>Note:</b> All metadata will be preserved during the conversion."));

    // ---------------------------------------------------------------------------------------

    connect(d->convertJpegCheck, SIGNAL(toggled(bool)),
            d->losslessFormat, SLOT(setEnabled(bool)));

    connect(d->convertJpegCheck, SIGNAL(toggled(bool)),
            d->formatLabel, SLOT(setEnabled(bool)));

    connect(d->convertJpegCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalDownloadNameChanged()));

    connect(d->losslessFormat, SIGNAL(activated(int)),
            this, SIGNAL(signalDownloadNameChanged()));

    connect(d->fixDateTimeCheck, SIGNAL(toggled(bool)),
            d->dateTimeEdit, SLOT(setEnabled(bool)));
}

AdvancedSettings::~AdvancedSettings()
{
    delete d;
}

void AdvancedSettings::readSettings(KConfigGroup& group)
{
    d->autoRotateCheck->setChecked(group.readEntry(QLatin1String("AutoRotate"),         true));
    d->fixDateTimeCheck->setChecked(group.readEntry(QLatin1String("FixDateTime"),       false));
    d->documentNameCheck->setChecked(group.readEntry(QLatin1String("DocumentName"),     false));
    d->templateSelector->setTemplateIndex(group.readEntry(QLatin1String("Template"),    0));
    d->convertJpegCheck->setChecked(group.readEntry(QLatin1String("ConvertJpeg"),       false));
    d->losslessFormat->setCurrentIndex(group.readEntry(QLatin1String("LossLessFormat"), 0));      // PNG by default

    d->dateTimeEdit->setEnabled(d->fixDateTimeCheck->isChecked());
    d->losslessFormat->setEnabled(d->convertJpegCheck->isChecked());
    d->formatLabel->setEnabled(d->convertJpegCheck->isChecked());
}

void AdvancedSettings::saveSettings(KConfigGroup& group)
{
    group.writeEntry(QLatin1String("AutoRotate"),     d->autoRotateCheck->isChecked());
    group.writeEntry(QLatin1String("FixDateTime"),    d->fixDateTimeCheck->isChecked());
    group.writeEntry(QLatin1String("DocumentName"),   d->documentNameCheck->isChecked());
    group.writeEntry(QLatin1String("Template"),       d->templateSelector->getTemplateIndex());
    group.writeEntry(QLatin1String("ConvertJpeg"),    d->convertJpegCheck->isChecked());
    group.writeEntry(QLatin1String("LossLessFormat"), d->losslessFormat->currentIndex());
}

DownloadSettings AdvancedSettings::settings() const
{
    DownloadSettings settings;

    settings.autoRotate     = d->autoRotateCheck->isChecked();
    settings.fixDateTime    = d->fixDateTimeCheck->isChecked();
    settings.convertJpeg    = d->convertJpegCheck->isChecked();
    settings.newDateTime    = d->dateTimeEdit->dateTime();
    settings.documentName   = d->documentNameCheck->isChecked();
    settings.losslessFormat = d->losslessFormat->currentText();
    settings.templateTitle  = d->templateSelector->getTemplate().templateTitle();

    return settings;
}

}  // namespace Digikam
