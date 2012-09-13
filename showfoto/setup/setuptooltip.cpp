/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-07-09
 * Description : item tool tip configuration setup tab
 *
 * Copyright (C) 2006-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setuptooltip.moc"

// Qt includes

#include <QCheckBox>
#include <QGroupBox>
#include <QVBoxLayout>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <khbox.h>
#include <klocale.h>

// Local includes

#include "dfontselect.h"

using namespace Digikam;

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
        showPhotoModeBox(0),
        showPhotoWbBox(0),
        showToolTipsBox(0),
        fileSettingBox(0),
        photoSettingBox(0),
        fontSelect(0)
    {}

    static const QString configGroupName;
    static const QString configShowToolTipsEntry;
    static const QString configToolTipsFontEntry;
    static const QString configToolTipsShowFileNameEntry;
    static const QString configToolTipsShowFileDateEntry;
    static const QString configToolTipsShowFileSizeEntry;
    static const QString configToolTipsShowImageTypeEntry;
    static const QString configToolTipsShowImageDimEntry;
    static const QString configToolTipsShowPhotoMakeEntry;
    static const QString configToolTipsShowPhotoDateEntry;
    static const QString configToolTipsShowPhotoFocalEntry;
    static const QString configToolTipsShowPhotoExpoEntry;
    static const QString configToolTipsShowPhotoModeEntry;
    static const QString configToolTipsShowPhotoFlashEntry;
    static const QString configToolTipsShowPhotoWBEntry;

    QCheckBox*           showFileDateBox;
    QCheckBox*           showFileNameBox;
    QCheckBox*           showFileSizeBox;
    QCheckBox*           showImageDimBox;
    QCheckBox*           showImageTypeBox;
    QCheckBox*           showPhotoDateBox;
    QCheckBox*           showPhotoExpoBox;
    QCheckBox*           showPhotoFlashBox;
    QCheckBox*           showPhotoFocalBox;
    QCheckBox*           showPhotoMakeBox;
    QCheckBox*           showPhotoModeBox;
    QCheckBox*           showPhotoWbBox;
    QCheckBox*           showToolTipsBox;

    QGroupBox*           fileSettingBox;
    QGroupBox*           photoSettingBox;

    DFontSelect*         fontSelect;
};

const QString SetupToolTip::Private::configGroupName("ImageViewer Settings");
const QString SetupToolTip::Private::configShowToolTipsEntry("Show ToolTips");
const QString SetupToolTip::Private::configToolTipsFontEntry("ToolTips Font");
const QString SetupToolTip::Private::configToolTipsShowFileNameEntry("ToolTips Show File Name");
const QString SetupToolTip::Private::configToolTipsShowFileDateEntry("ToolTips Show File Date");
const QString SetupToolTip::Private::configToolTipsShowFileSizeEntry("ToolTips Show File Size");
const QString SetupToolTip::Private::configToolTipsShowImageTypeEntry("ToolTips Show Image Type");
const QString SetupToolTip::Private::configToolTipsShowImageDimEntry("ToolTips Show Image Dim");
const QString SetupToolTip::Private::configToolTipsShowPhotoMakeEntry("ToolTips Show Photo Make");
const QString SetupToolTip::Private::configToolTipsShowPhotoDateEntry("ToolTips Show Photo Date");
const QString SetupToolTip::Private::configToolTipsShowPhotoFocalEntry("ToolTips Show Photo Focal");
const QString SetupToolTip::Private::configToolTipsShowPhotoExpoEntry("ToolTips Show Photo Expo");
const QString SetupToolTip::Private::configToolTipsShowPhotoModeEntry("ToolTips Show Photo Mode");
const QString SetupToolTip::Private::configToolTipsShowPhotoFlashEntry("ToolTips Show Photo Flash");
const QString SetupToolTip::Private::configToolTipsShowPhotoWBEntry("ToolTips Show Photo WB");

// --------------------------------------------------------

SetupToolTip::SetupToolTip(QWidget* const parent)
    : QScrollArea(parent), d(new Private)
{
    QWidget* panel      = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    QVBoxLayout* layout = new QVBoxLayout(panel);
    d->showToolTipsBox  = new QCheckBox(i18n("Show Thumbbar items' toolti&ps"), panel);
    d->showToolTipsBox->setWhatsThis(i18n("Set this option to display the image information when "
                                          "the mouse hovers over a thumbbar item."));

    d->fontSelect       = new DFontSelect(i18n("Font:"), panel);
    d->fontSelect->setToolTip(i18n("Select here the font used to display text in tooltips."));

    // --------------------------------------------------------

    d->fileSettingBox     = new QGroupBox(i18n("File/Image Information"), panel);
    QVBoxLayout* gLayout1 = new QVBoxLayout(d->fileSettingBox);

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
    gLayout1->setMargin(KDialog::spacingHint());
    gLayout1->setSpacing(0);

    // --------------------------------------------------------

    d->photoSettingBox    = new QGroupBox(i18n("Photograph Information"), panel);
    QVBoxLayout* gLayout2 = new QVBoxLayout(d->photoSettingBox);

    d->showPhotoMakeBox = new QCheckBox(i18n("Show camera make and model"), d->photoSettingBox);
    d->showPhotoMakeBox->setWhatsThis( i18n("Set this option to display the make and model of the "
                                            "camera with which the image has been taken."));

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
    gLayout2->addWidget(d->showPhotoDateBox);
    gLayout2->addWidget(d->showPhotoFocalBox);
    gLayout2->addWidget(d->showPhotoExpoBox);
    gLayout2->addWidget(d->showPhotoModeBox);
    gLayout2->addWidget(d->showPhotoFlashBox);
    gLayout2->addWidget(d->showPhotoWbBox);
    gLayout2->setMargin(KDialog::spacingHint());
    gLayout2->setSpacing(0);

    layout->addWidget(d->showToolTipsBox);
    layout->addWidget(d->fontSelect);
    layout->addWidget(d->fileSettingBox);
    layout->addWidget(d->photoSettingBox);
    layout->addStretch();
    layout->setMargin(KDialog::spacingHint());
    layout->setSpacing(KDialog::spacingHint());

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

    setAutoFillBackground(false);
    viewport()->setAutoFillBackground(false);
    panel->setAutoFillBackground(false);
}

SetupToolTip::~SetupToolTip()
{
    delete d;
}

void SetupToolTip::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->showToolTipsBox->setChecked(group.readEntry(d->configShowToolTipsEntry, true));
    d->fontSelect->setFont(group.readEntry(d->configToolTipsFontEntry,         KGlobalSettings::generalFont()));

    d->showFileNameBox->setChecked(group.readEntry(d->configToolTipsShowFileNameEntry,   true));
    d->showFileDateBox->setChecked(group.readEntry(d->configToolTipsShowFileDateEntry,   false));
    d->showFileSizeBox->setChecked(group.readEntry(d->configToolTipsShowFileSizeEntry,   false));
    d->showImageTypeBox->setChecked(group.readEntry(d->configToolTipsShowImageTypeEntry, false));
    d->showImageDimBox->setChecked(group.readEntry(d->configToolTipsShowImageDimEntry,   true));

    d->showPhotoMakeBox->setChecked(group.readEntry(d->configToolTipsShowPhotoMakeEntry,   true));
    d->showPhotoDateBox->setChecked(group.readEntry(d->configToolTipsShowPhotoDateEntry,   true));
    d->showPhotoFocalBox->setChecked(group.readEntry(d->configToolTipsShowPhotoFocalEntry, true));
    d->showPhotoExpoBox->setChecked(group.readEntry(d->configToolTipsShowPhotoExpoEntry,   true));
    d->showPhotoModeBox->setChecked(group.readEntry(d->configToolTipsShowPhotoModeEntry,   true));
    d->showPhotoFlashBox->setChecked(group.readEntry(d->configToolTipsShowPhotoFlashEntry, false));
    d->showPhotoWbBox->setChecked(group.readEntry(d->configToolTipsShowPhotoWBEntry,       false));

    d->fileSettingBox->setEnabled(d->showToolTipsBox->isChecked());
    d->photoSettingBox->setEnabled(d->showToolTipsBox->isChecked());
}

void SetupToolTip::applySettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configShowToolTipsEntry,           d->showToolTipsBox->isChecked());
    group.writeEntry(d->configToolTipsFontEntry,           d->fontSelect->font());

    group.writeEntry(d->configToolTipsShowFileNameEntry,   d->showFileNameBox->isChecked());
    group.writeEntry(d->configToolTipsShowFileDateEntry,   d->showFileDateBox->isChecked());
    group.writeEntry(d->configToolTipsShowFileSizeEntry,   d->showFileSizeBox->isChecked());
    group.writeEntry(d->configToolTipsShowImageTypeEntry,  d->showImageTypeBox->isChecked());
    group.writeEntry(d->configToolTipsShowImageDimEntry,   d->showImageDimBox->isChecked());

    group.writeEntry(d->configToolTipsShowPhotoMakeEntry,  d->showPhotoMakeBox->isChecked());
    group.writeEntry(d->configToolTipsShowPhotoDateEntry,  d->showPhotoDateBox->isChecked());
    group.writeEntry(d->configToolTipsShowPhotoFocalEntry, d->showPhotoFocalBox->isChecked());
    group.writeEntry(d->configToolTipsShowPhotoExpoEntry,  d->showPhotoExpoBox->isChecked());
    group.writeEntry(d->configToolTipsShowPhotoModeEntry,  d->showPhotoModeBox->isChecked());
    group.writeEntry(d->configToolTipsShowPhotoFlashEntry, d->showPhotoFlashBox->isChecked());
    group.writeEntry(d->configToolTipsShowPhotoWBEntry,    d->showPhotoWbBox->isChecked());

    config->sync();
}

}  // namespace ShowFoto
