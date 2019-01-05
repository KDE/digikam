/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-09-13
 * Description : a widget to provide options to save image.
 *
 * Copyright (C) 2006-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dsavesettingswidget.h"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QApplication>
#include <QStyle>
#include <QComboBox>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

namespace Digikam
{

class Q_DECL_HIDDEN DSaveSettingsWidget::Private
{
public:

    explicit Private()
    {
        formatLabel    = 0;
        formatComboBox = 0;
        conflictBox    = 0;
        grid           = 0;
    }

    QLabel*              formatLabel;

    QGridLayout*         grid;

    QComboBox*           formatComboBox;

    FileSaveConflictBox* conflictBox;
};

DSaveSettingsWidget::DSaveSettingsWidget(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    d->grid           = new QGridLayout(this);
    d->formatLabel    = new QLabel(i18n("Output file format:"), this);
    d->formatComboBox = new QComboBox(this);
    d->formatComboBox->setWhatsThis(i18n("<p>Set the output file format to use here:</p>"
                                         "<p><b>JPEG</b>: output the processed image in JPEG format. "
                                         "This format will give smaller-sized files.</p>"
                                         "<p><b>Warning: Due to the destructive compression algorithm, "
                                         "JPEG is a lossy quality format.</b></p>"
                                         "<p><b>TIFF</b>: output the processed image in TIFF format. "
                                         "This generates large files, without "
                                         "losing quality. Image is compressed.</p>"
                                         "<p><b>PNG</b>: output the processed image in PNG format. "
                                         "This generates large files, without "
                                         "losing quality. Image is compressed.</p>"
                                         "<p><b>PPM</b>: output the processed image in PPM format. "
                                         "This generates the largest files, without "
                                         "losing quality. Image is not compressed.</p>"));
    slotPopulateImageFormat(false);

    d->conflictBox    = new FileSaveConflictBox(this);

    d->grid->addWidget(d->formatLabel,    0, 0, 1, 1);
    d->grid->addWidget(d->formatComboBox, 0, 1, 1, 1);
    d->grid->addWidget(d->conflictBox,    1, 0, 1, 2);
    d->grid->setRowStretch(3, 10);
    d->grid->setContentsMargins(spacing, spacing, spacing, spacing);
    d->grid->setSpacing(spacing);

    connect(d->formatComboBox, SIGNAL(activated(int)),
            this, SIGNAL(signalSaveFormatChanged()));

    connect(d->conflictBox, SIGNAL(signalConflictButtonChanged(int)),
            this, SIGNAL(signalConflictButtonChanged(int)));
}

DSaveSettingsWidget::~DSaveSettingsWidget()
{
    delete d;
}

void DSaveSettingsWidget::setCustomSettingsWidget(QWidget* const custom)
{
    d->grid->addWidget(custom, 2, 0, 1, 2);
}

void DSaveSettingsWidget::resetToDefault()
{
    setFileFormat(OUTPUT_PNG);
    d->conflictBox->resetToDefault();
}

DSaveSettingsWidget::OutputFormat DSaveSettingsWidget::fileFormat() const
{
    return (OutputFormat)(d->formatComboBox->currentIndex());
}

void DSaveSettingsWidget::setFileFormat(OutputFormat f)
{
    d->formatComboBox->setCurrentIndex((int)f);
}

FileSaveConflictBox::ConflictRule DSaveSettingsWidget::conflictRule() const
{
    return d->conflictBox->conflictRule();
}

void DSaveSettingsWidget::setConflictRule(FileSaveConflictBox::ConflictRule r)
{
    d->conflictBox->setConflictRule(r);
}

void DSaveSettingsWidget::readSettings(KConfigGroup& group)
{
    setFileFormat((DSaveSettingsWidget::OutputFormat)group.readEntry("Output Format", (int)(DSaveSettingsWidget::OUTPUT_PNG)));
    d->conflictBox->readSettings(group);
}

void DSaveSettingsWidget::writeSettings(KConfigGroup& group)
{
    group.writeEntry("Output Format", (int)fileFormat());
    d->conflictBox->writeSettings(group);
}

void DSaveSettingsWidget::slotPopulateImageFormat(bool sixteenBits)
{
    d->formatComboBox->clear();
    d->formatComboBox->insertItem( OUTPUT_PNG,  QLatin1String("PNG") );
    d->formatComboBox->insertItem( OUTPUT_TIFF, QLatin1String("TIFF") );

    if (!sixteenBits)
    {
        d->formatComboBox->insertItem( OUTPUT_JPEG, QLatin1String("JPEG") );
        d->formatComboBox->insertItem( OUTPUT_PPM,  QLatin1String("PPM") );
    }

    emit signalSaveFormatChanged();
}

QString DSaveSettingsWidget::extension() const
{
    return extensionForFormat(fileFormat());
}

QString DSaveSettingsWidget::typeMime() const
{
    QString mime;

    switch(fileFormat())
    {
        case OUTPUT_JPEG:
            mime = QLatin1String("image/jpeg");
            break;
        case OUTPUT_TIFF:
            mime = QLatin1String("image/tiff");
            break;
        case OUTPUT_PPM:
            mime = QLatin1String("image/ppm");
            break;
        case OUTPUT_PNG:
            mime = QLatin1String("image/png");
            break;
    }

    return mime;
}

QString DSaveSettingsWidget::extensionForFormat(DSaveSettingsWidget::OutputFormat format)
{
    QString ext;

    switch(format)
    {
        case OUTPUT_JPEG:
            ext = QLatin1String(".jpg");
            break;
        case OUTPUT_TIFF:
            ext = QLatin1String(".tif");
            break;
        case OUTPUT_PPM:
            ext = QLatin1String(".ppm");
            break;
        case OUTPUT_PNG:
            ext = QLatin1String(".png");
            break;
    }

    return ext;
}

} // namespace Digikam
