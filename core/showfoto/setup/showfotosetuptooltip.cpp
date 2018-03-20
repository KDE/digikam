/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-07-09
 * Description : item tool tip configuration setup tab
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2013-2014 by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "showfotosetuptooltip.h"

// Qt includes

#include <QCheckBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dfontselect.h"
#include "showfotosettings.h"

namespace ShowFoto
{

class SetupToolTip::Private
{
public:

    Private() :
        showFileDateBox(0),
        showFileNameBox(0),
        showFileSizeBox(0),
        showImageDimBox(0),
        showImageTypeBox(0),
        showPhotoDateBox(0),
        showPhotoExpoBox(0),
        showPhotoFlashBox(0),
        showPhotoFocalBox(0),
        showPhotoMakeBox(0),
        showPhotoLensBox(0),
        showPhotoModeBox(0),
        showPhotoWbBox(0),
        showToolTipsBox(0),
        fileSettingBox(0),
        photoSettingBox(0),
        fontSelect(0),
        settings(0)
    {}

    QCheckBox*            showFileDateBox;
    QCheckBox*            showFileNameBox;
    QCheckBox*            showFileSizeBox;
    QCheckBox*            showImageDimBox;
    QCheckBox*            showImageTypeBox;
    QCheckBox*            showPhotoDateBox;
    QCheckBox*            showPhotoExpoBox;
    QCheckBox*            showPhotoFlashBox;
    QCheckBox*            showPhotoFocalBox;
    QCheckBox*            showPhotoMakeBox;
    QCheckBox*            showPhotoLensBox;
    QCheckBox*            showPhotoModeBox;
    QCheckBox*            showPhotoWbBox;
    QCheckBox*            showToolTipsBox;

    QGroupBox*            fileSettingBox;
    QGroupBox*            photoSettingBox;

    Digikam::DFontSelect* fontSelect;
    ShowfotoSettings*     settings;
};

// --------------------------------------------------------

SetupToolTip::SetupToolTip(QWidget* const parent)
    : QScrollArea(parent), d(new Private)
{
    QWidget* const panel      = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QVBoxLayout* const layout = new QVBoxLayout(panel);
    d->showToolTipsBox        = new QCheckBox(i18n("Show Thumbbar items' toolti&ps"), panel);
    d->showToolTipsBox->setWhatsThis(i18n("Set this option to display the image information when "
                                          "the mouse hovers over a thumbbar item."));

    d->fontSelect             = new Digikam::DFontSelect(i18n("Font:"), panel);
    d->fontSelect->setToolTip(i18n("Select here the font used to display text in tooltips."));

    // --------------------------------------------------------

    d->fileSettingBox           = new QGroupBox(i18n("File/Image Information"), panel);
    QVBoxLayout* const gLayout1 = new QVBoxLayout(d->fileSettingBox);

    d->showFileNameBox = new QCheckBox(i18n("Show file name"), d->fileSettingBox);
    d->showFileNameBox->setWhatsThis( i18n("Set this option to display the image file name."));

    d->showFileDateBox = new QCheckBox(i18n("Show file date"), d->fileSettingBox);
    d->showFileDateBox->setWhatsThis( i18n("Set this option to display the image file date."));

    d->showFileSizeBox = new QCheckBox(i18n("Show file size"), d->fileSettingBox);
    d->showFileSizeBox->setWhatsThis( i18n("Set this option to display the image file size."));

    d->showImageTypeBox = new QCheckBox(i18n("Show image type"), d->fileSettingBox);
    d->showImageTypeBox->setWhatsThis( i18n("Set this option to display the image type."));

    d->showImageDimBox = new QCheckBox(i18n("Show image dimensions"), d->fileSettingBox);
    d->showImageDimBox->setWhatsThis( i18n("Set this option to display the image dimensions in pixels."));

    gLayout1->addWidget(d->showFileNameBox);
    gLayout1->addWidget(d->showFileDateBox);
    gLayout1->addWidget(d->showFileSizeBox);
    gLayout1->addWidget(d->showImageTypeBox);
    gLayout1->addWidget(d->showImageDimBox);
    gLayout1->setContentsMargins(spacing, spacing, spacing, spacing);
    gLayout1->setSpacing(0);

    // --------------------------------------------------------

    d->photoSettingBox    = new QGroupBox(i18n("Photograph Information"), panel);
    QVBoxLayout* gLayout2 = new QVBoxLayout(d->photoSettingBox);

    d->showPhotoMakeBox = new QCheckBox(i18n("Show camera make and model"), d->photoSettingBox);
    d->showPhotoMakeBox->setWhatsThis( i18n("Set this option to display the make and model of the "
                                            "camera with which the image has been taken."));

    d->showPhotoLensBox = new QCheckBox(i18n("Camera lens model"), d->photoSettingBox);
    d->showPhotoLensBox->setWhatsThis(i18n("Set this option to display the lens model with "
                                           "which the image was taken."));

    d->showPhotoDateBox = new QCheckBox(i18n("Show camera date"), d->photoSettingBox);
    d->showPhotoDateBox->setWhatsThis( i18n("Set this option to display the date when the image was taken."));

    d->showPhotoFocalBox = new QCheckBox(i18n("Show camera aperture and focal length"), d->photoSettingBox);
    d->showPhotoFocalBox->setWhatsThis( i18n("Set this option to display the camera aperture and focal settings "
                                        "used to take the image."));

    d->showPhotoExpoBox = new QCheckBox(i18n("Show camera exposure and sensitivity"), d->photoSettingBox);
    d->showPhotoExpoBox->setWhatsThis( i18n("Set this option to display the camera exposure and sensitivity "
                                            "used to take the image."));

    d->showPhotoModeBox = new QCheckBox(i18n("Show camera mode and program"), d->photoSettingBox);
    d->showPhotoModeBox->setWhatsThis( i18n("Set this option to display the camera mode and program "
                                            "used to take the image."));

    d->showPhotoFlashBox = new QCheckBox(i18n("Show camera flash settings"), d->photoSettingBox);
    d->showPhotoFlashBox->setWhatsThis( i18n("Set this option to display the camera flash settings "
                                        "used to take the image."));

    d->showPhotoWbBox = new QCheckBox(i18n("Show camera white balance settings"), d->photoSettingBox);
    d->showPhotoWbBox->setWhatsThis( i18n("Set this option to display the camera white balance settings "
                                          "used to take the image."));

    gLayout2->addWidget(d->showPhotoMakeBox);
    gLayout2->addWidget(d->showPhotoLensBox);
    gLayout2->addWidget(d->showPhotoDateBox);
    gLayout2->addWidget(d->showPhotoFocalBox);
    gLayout2->addWidget(d->showPhotoExpoBox);
    gLayout2->addWidget(d->showPhotoModeBox);
    gLayout2->addWidget(d->showPhotoFlashBox);
    gLayout2->addWidget(d->showPhotoWbBox);
    gLayout2->setContentsMargins(spacing, spacing, spacing, spacing);
    gLayout2->setSpacing(0);

    layout->addWidget(d->showToolTipsBox);
    layout->addWidget(d->fontSelect);
    layout->addWidget(d->fileSettingBox);
    layout->addWidget(d->photoSettingBox);
    layout->addStretch();
    layout->setContentsMargins(spacing, spacing, spacing, spacing);
    layout->setSpacing(spacing);

    // --------------------------------------------------------

    connect(d->showToolTipsBox, SIGNAL(toggled(bool)),
            d->fileSettingBox, SLOT(setEnabled(bool)));

    connect(d->showToolTipsBox, SIGNAL(toggled(bool)),
            d->photoSettingBox, SLOT(setEnabled(bool)));

    connect(d->showToolTipsBox, SIGNAL(toggled(bool)),
            d->fontSelect, SLOT(setEnabled(bool)));

    // --------------------------------------------------------

    readSettings();
    adjustSize();

    // --------------------------------------------------------
}

SetupToolTip::~SetupToolTip()
{
    delete d;
}

void SetupToolTip::readSettings()
{
    d->settings = ShowfotoSettings::instance();

    d->showToolTipsBox->setChecked(d->settings->getShowToolTip());
    d->fontSelect->setFont(d->settings->getToolTipFont());

    d->showFileNameBox->setChecked(d->settings->getShowFileName());
    d->showFileDateBox->setChecked(d->settings->getShowFileDate());
    d->showFileSizeBox->setChecked(d->settings->getShowFileSize());
    d->showImageTypeBox->setChecked(d->settings->getShowFileType());
    d->showImageDimBox->setChecked(d->settings->getShowFileDim());

    d->showPhotoMakeBox->setChecked(d->settings->getShowPhotoMake());
    d->showPhotoLensBox->setChecked(d->settings->getShowPhotoLens());
    d->showPhotoDateBox->setChecked(d->settings->getShowPhotoDate());
    d->showPhotoFocalBox->setChecked(d->settings->getShowPhotoFocal());
    d->showPhotoExpoBox->setChecked(d->settings->getShowPhotoExpo());
    d->showPhotoModeBox->setChecked(d->settings->getShowPhotoMode());
    d->showPhotoFlashBox->setChecked(d->settings->getShowPhotoFlash());
    d->showPhotoWbBox->setChecked(d->settings->getShowPhotoWB());

    d->fileSettingBox->setEnabled(d->settings->getShowToolTip());
    d->photoSettingBox->setEnabled(d->settings->getShowToolTip());
}

void SetupToolTip::applySettings()
{
    d->settings->setShowToolTip(d->showToolTipsBox->isChecked());
    d->settings->setToolTipFont(d->fontSelect->font());

    d->settings->setShowFileName(d->showFileNameBox->isChecked());
    d->settings->setShowFileDate(d->showFileDateBox->isChecked());
    d->settings->setShowFileSize(d->showFileSizeBox->isChecked());
    d->settings->setShowFileType(d->showImageTypeBox->isChecked());
    d->settings->setShowFileDim(d->showImageDimBox->isChecked());

    d->settings->setShowPhotoMake(d->showPhotoMakeBox->isChecked());
    d->settings->setShowPhotoLens(d->showPhotoLensBox->isChecked());
    d->settings->setShowPhotoDate(d->showPhotoDateBox->isChecked());
    d->settings->setShowPhotoFocal(d->showPhotoFocalBox->isChecked());
    d->settings->setShowPhotoExpo(d->showPhotoExpoBox->isChecked());
    d->settings->setShowPhotoMode(d->showPhotoModeBox->isChecked());
    d->settings->setShowPhotoFlash(d->showPhotoFlashBox->isChecked());
    d->settings->setShowPhotoWB(d->showPhotoWbBox->isChecked());

    d->settings->syncConfig();
}

}  // namespace ShowFoto
