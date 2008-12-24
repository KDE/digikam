/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-07-09
 * Description : item tool tip configuration setup tab
 *
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setuptooltip.h"
#include "setuptooltip.moc"

// Qt includes.

#include <QGroupBox>
#include <QCheckBox>
#include <QVBoxLayout>

// KDE includes.

#include <khbox.h>
#include <klocale.h>
#include <kdialog.h>

// Local includes.

#include "dfontselect.h"
#include "albumsettings.h"

namespace Digikam
{

class SetupToolTipPriv
{
public:

    SetupToolTipPriv()
    {
        showToolTipsBox   = 0;
        fontSelect        = 0;

        showFileNameBox   = 0;
        showFileDateBox   = 0;
        showFileSizeBox   = 0;
        showImageTypeBox  = 0;
        showImageDimBox   = 0;

        showPhotoMakeBox  = 0;
        showPhotoDateBox  = 0;
        showPhotoFocalBox = 0;
        showPhotoExpoBox  = 0;
        showPhotoModeBox  = 0;
        showPhotoFlashBox = 0;
        showPhotoWbBox    = 0;

        showAlbumNameBox  = 0;
        showCommentsBox   = 0;
        showTagsBox       = 0;
        showRatingBox     = 0;

        fileSettingBox    = 0;
        photoSettingBox   = 0;
        digikamSettingBox = 0;
    }

    QCheckBox   *showToolTipsBox;

    QCheckBox   *showFileNameBox;
    QCheckBox   *showFileDateBox;
    QCheckBox   *showFileSizeBox;
    QCheckBox   *showImageTypeBox;
    QCheckBox   *showImageDimBox;

    QCheckBox   *showPhotoMakeBox;
    QCheckBox   *showPhotoDateBox;
    QCheckBox   *showPhotoFocalBox;
    QCheckBox   *showPhotoExpoBox;
    QCheckBox   *showPhotoModeBox;
    QCheckBox   *showPhotoFlashBox;
    QCheckBox   *showPhotoWbBox;

    QCheckBox   *showAlbumNameBox;
    QCheckBox   *showCommentsBox;
    QCheckBox   *showTagsBox;
    QCheckBox   *showRatingBox;

    QGroupBox   *fileSettingBox;
    QGroupBox   *photoSettingBox;
    QGroupBox   *digikamSettingBox;

    DFontSelect *fontSelect;
};

SetupToolTip::SetupToolTip(QWidget* parent)
            : QWidget(parent), d(new SetupToolTipPriv)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    d->showToolTipsBox  = new QCheckBox(i18n("Show icon-view/thumbbar items toolti&ps"), this);
    d->showToolTipsBox->setWhatsThis(i18n("Set this option to display image information when "
                                          "the mouse hovers over an icon-view or thumbbar item."));

    d->fontSelect       = new DFontSelect(i18n("Font:"), this);
    d->fontSelect->setToolTip(i18n("Select here the font used to display text in tooltips."));

    // --------------------------------------------------------

    d->fileSettingBox     = new QGroupBox(i18n("File/Image Information"), this);
    QVBoxLayout *gLayout1 = new QVBoxLayout(d->fileSettingBox);

    d->showFileNameBox    = new QCheckBox(i18n("Show file name"), d->fileSettingBox);
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

    d->photoSettingBox    = new QGroupBox(i18n("Photograph Information"), this);
    QVBoxLayout *gLayout2 = new QVBoxLayout(d->photoSettingBox);

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

    // --------------------------------------------------------

    d->digikamSettingBox  = new QGroupBox(i18n("digiKam Information"), this);
    QVBoxLayout *gLayout3 = new QVBoxLayout(d->digikamSettingBox);

    d->showAlbumNameBox = new QCheckBox(i18n("Show album name"), d->digikamSettingBox);
    d->showAlbumNameBox->setWhatsThis( i18n("Set this option to display the album name."));

    d->showCommentsBox = new QCheckBox(i18n("Show image caption"), d->digikamSettingBox);
    d->showCommentsBox->setWhatsThis( i18n("Set this option to display the image captions."));

    d->showTagsBox = new QCheckBox(i18n("Show image tags"), d->digikamSettingBox);
    d->showTagsBox->setWhatsThis( i18n("Set this option to display the image tags."));

    d->showRatingBox = new QCheckBox(i18n("Show image rating"), d->digikamSettingBox);
    d->showRatingBox->setWhatsThis( i18n("Set this option to display the image rating."));

    gLayout3->addWidget(d->showAlbumNameBox);
    gLayout3->addWidget(d->showCommentsBox);
    gLayout3->addWidget(d->showTagsBox);
    gLayout3->addWidget(d->showRatingBox);
    gLayout3->setMargin(KDialog::spacingHint());
    gLayout3->setSpacing(0);

    // --------------------------------------------------------

    layout->addWidget(d->showToolTipsBox);
    layout->addWidget(d->fontSelect);
    layout->addWidget(d->fileSettingBox);
    layout->addWidget(d->photoSettingBox);
    layout->addWidget(d->digikamSettingBox);
    layout->addStretch();
    layout->setMargin(0);
    layout->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------

    connect(d->showToolTipsBox, SIGNAL(toggled(bool)),
            d->fileSettingBox, SLOT(setEnabled(bool)));

    connect(d->showToolTipsBox, SIGNAL(toggled(bool)),
            d->photoSettingBox, SLOT(setEnabled(bool)));

    connect(d->showToolTipsBox, SIGNAL(toggled(bool)),
            d->digikamSettingBox, SLOT(setEnabled(bool)));

    connect(d->showToolTipsBox, SIGNAL(toggled(bool)),
            d->fontSelect, SLOT(setEnabled(bool)));

    // --------------------------------------------------------

    readSettings();
    adjustSize();
}

SetupToolTip::~SetupToolTip()
{
    delete d;
}

void SetupToolTip::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;

    settings->setShowToolTips(d->showToolTipsBox->isChecked());
    settings->setToolTipsFont(d->fontSelect->font());

    settings->setToolTipsShowFileName(d->showFileNameBox->isChecked());
    settings->setToolTipsShowFileDate(d->showFileDateBox->isChecked());
    settings->setToolTipsShowFileSize(d->showFileSizeBox->isChecked());
    settings->setToolTipsShowImageType(d->showImageTypeBox->isChecked());
    settings->setToolTipsShowImageDim(d->showImageDimBox->isChecked());

    settings->setToolTipsShowPhotoMake(d->showPhotoMakeBox->isChecked());
    settings->setToolTipsShowPhotoDate(d->showPhotoDateBox->isChecked());
    settings->setToolTipsShowPhotoFocal(d->showPhotoFocalBox->isChecked());
    settings->setToolTipsShowPhotoExpo(d->showPhotoExpoBox->isChecked());
    settings->setToolTipsShowPhotoMode(d->showPhotoModeBox->isChecked());
    settings->setToolTipsShowPhotoFlash(d->showPhotoFlashBox->isChecked());
    settings->setToolTipsShowPhotoWB(d->showPhotoWbBox->isChecked());

    settings->setToolTipsShowAlbumName(d->showAlbumNameBox->isChecked());
    settings->setToolTipsShowComments(d->showCommentsBox->isChecked());
    settings->setToolTipsShowTags(d->showTagsBox->isChecked());
    settings->setToolTipsShowRating(d->showRatingBox->isChecked());

    settings->saveSettings();
}

void SetupToolTip::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings) return;

    d->showToolTipsBox->setChecked(settings->getShowToolTips());
    d->fontSelect->setFont(settings->getToolTipsFont());

    d->showFileNameBox->setChecked(settings->getToolTipsShowFileName());
    d->showFileDateBox->setChecked(settings->getToolTipsShowFileDate());
    d->showFileSizeBox->setChecked(settings->getToolTipsShowFileSize());
    d->showImageTypeBox->setChecked(settings->getToolTipsShowImageType());
    d->showImageDimBox->setChecked(settings->getToolTipsShowImageDim());

    d->showPhotoMakeBox->setChecked(settings->getToolTipsShowPhotoMake());
    d->showPhotoDateBox->setChecked(settings->getToolTipsShowPhotoDate());
    d->showPhotoFocalBox->setChecked(settings->getToolTipsShowPhotoFocal());
    d->showPhotoExpoBox->setChecked(settings->getToolTipsShowPhotoExpo());
    d->showPhotoModeBox->setChecked(settings->getToolTipsShowPhotoMode());
    d->showPhotoFlashBox->setChecked(settings->getToolTipsShowPhotoFlash());
    d->showPhotoWbBox->setChecked(settings->getToolTipsShowPhotoWB());

    d->showAlbumNameBox->setChecked(settings->getToolTipsShowAlbumName());
    d->showCommentsBox->setChecked(settings->getToolTipsShowComments());
    d->showTagsBox->setChecked(settings->getToolTipsShowTags());
    d->showRatingBox->setChecked(settings->getToolTipsShowRating());

    d->fileSettingBox->setEnabled(d->showToolTipsBox->isChecked());
    d->photoSettingBox->setEnabled(d->showToolTipsBox->isChecked());
    d->digikamSettingBox->setEnabled(d->showToolTipsBox->isChecked());
}

}  // namespace Digikam
