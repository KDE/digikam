/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-05-15
 * Description : a dialog to see preview ICC color correction
 *               before to apply color profile.
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "colorcorrectiondlg.h"

// Qt includes

#include <QCheckBox>
#include <QFrame>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QStandardPaths>
#include <QIcon>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "dimg.h"
#include "iccmanager.h"
#include "iccsettings.h"
#include "icctransform.h"
#include "iccprofileinfodlg.h"
#include "iccprofilescombobox.h"
#include "dxmlguiwindow.h"
#include "dexpanderbox.h"



namespace Digikam
{

class ColorCorrectionDlg::Private
{
public:

    Private() :
        imageProfileTitle(0),
        imageProfileDesc(0),
        previewTarget(0),
        keepProfile(0),
        convertToWorkingSpace(0),
        thirdOption(0),
        thirdCheckBox(0),
        imageSRGB(0),
        imageWorkingSpace(0),
        imageOtherSpace(0),
        buttons(0),
        otherProfileBox(0),
        imageProfileBox(0),
        mode(ColorCorrectionDlg::ProfileMismatch)
    {
    }

    DImg                     preview;
    QString                  filePath;

    QLabel*                  imageProfileTitle;
    QLabel*                  imageProfileDesc;
    QLabel*                  previewTarget;

    QRadioButton*            keepProfile;
    QRadioButton*            convertToWorkingSpace;
    QRadioButton*            thirdOption;
    QCheckBox*               thirdCheckBox;

    QRadioButton*            imageSRGB;
    QRadioButton*            imageWorkingSpace;
    QRadioButton*            imageOtherSpace;

    QDialogButtonBox*        buttons;

    IccProfilesComboBox*     otherProfileBox;
    IccProfilesComboBox*     imageProfileBox;

    ColorCorrectionDlg::Mode mode;

    IccProfile               workspaceProfile;
    IccProfile               imageProfile;
    IccProfile               outputProfile;
};

ColorCorrectionDlg::ColorCorrectionDlg(Mode mode, const DImg& preview,
                                       const QString& file, QWidget* const parent)
    : QDialog(parent),
      d(new Private)
{
    setModal(true);

    d->buttons = new QDialogButtonBox(QDialogButtonBox::Help | QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    d->buttons->button(QDialogButtonBox::Cancel)->setText(i18n("Don't know"));
    d->buttons->button(QDialogButtonBox::Cancel)->setToolTip(i18n("Take the safest and most appropriate action"));

    d->mode                          = mode;
    d->preview                       = preview;
    d->filePath                      = file;
    ICCSettingsContainer iccSettings = IccSettings::instance()->settings();
    d->workspaceProfile              = IccProfile(iccSettings.workspaceProfile);

    QString caption;

    if (d->mode == ProfileMismatch)
    {
        caption = i18n("Color Profile Mismatch");
    }
    else if (d->mode == MissingProfile)
    {
        caption = i18n("Missing Color Profile");
    }
    else if (d->mode == UncalibratedColor)
    {
        caption = i18n("Image with Uncalibrated Color");
    }

    if (!file.isNull())
    {
        QFileInfo fi(file);
        caption = i18nc("<Problem> - <b>", "%1 - %2", caption, fi.fileName());
    }

    setWindowTitle(caption);

    QWidget* const page     = new QWidget(this);
    QGridLayout* const grid = new QGridLayout(page);

    if (d->mode == ProfileMismatch)
    {
        grid->addLayout(createHeading(),       0, 0, 1, 2);
        grid->addLayout(createProfilesInfo(),  1, 0);
        grid->addLayout(createPreviews(),      1, 1, 2, 1);
        grid->addWidget(createOptions(),       3, 0, 1, 2);
    }
    else if (d->mode == MissingProfile)
    {
        QVBoxLayout* vbox = new QVBoxLayout;
        vbox->addWidget(createAssumeOptions());
        vbox->addLayout(createProfilesInfo());
        vbox->addStretch(1);

        grid->addLayout(createHeading(),       0, 0, 1, 2);
        grid->addLayout(vbox,                  1, 0);
        grid->addLayout(createPreviews(),      1, 1, 2, 1);
        grid->addWidget(createOptions(),       3, 0, 1, 2);
        grid->setRowStretch(1, 1);
        grid->setRowStretch(3, 1);
    }
    else if (d->mode == UncalibratedColor)
    {
        grid->addLayout(createHeading(),       0, 0, 1, 2);
        grid->addLayout(createProfilesInfo(),  1, 0,       Qt::AlignTop);
        grid->addLayout(createPreviews(),      1, 1, 2, 1);
        grid->addWidget(createAssumeOptions(), 3, 0, 1, 2);
    }

    page->setLayout(grid);

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(page);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(slotOk()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));

    connect(d->buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));

    readSettings();
    updateImageProfileUI();
    updateUsedProfileUI();
    updateInfo();
}

ColorCorrectionDlg::~ColorCorrectionDlg()
{
    delete d;
}

void ColorCorrectionDlg::slotOk()
{
    accept();
    setSettings();
}

QLayout* ColorCorrectionDlg::createHeading() const
{
    QLabel* const icon = new QLabel;
    icon->setPixmap(QIcon::fromTheme(QLatin1String("fill-color")).pixmap(32));

    QLabel* const message = new QLabel;

    if (d->mode == ProfileMismatch)
    {
        message->setText(i18n("<p>This image has an <b>embedded color profile</b><br/> "
                              "which does not match your working space color profile.<br/>"
                              "How do you want to proceed?</p>"));
    }
    else if (d->mode == MissingProfile)
    {
        message->setText(i18n("<p>This image has <b>no information about the color space</b><br/>"
                              "that was used when creating the image. <br/>"
                              "How do you want to proceed?</p>"));

    }
    else if (d->mode == UncalibratedColor)
    {
        message->setText(i18n("<p>The color information of this image is uncalibrated.<br/>"
                              "How do you want to proceed?</p>"));
    }

    message->setWordWrap(true);

    QLabel* const logo      = new QLabel;
    logo->setPixmap(QIcon::fromTheme(QLatin1String("digikam")).pixmap(QSize(48,48)));

    DLineWidget* const line  = new DLineWidget(Qt::Horizontal);

    QGridLayout* const grid = new QGridLayout;
    grid->addWidget(icon,    0, 0);
    grid->addWidget(message, 0, 1);
    grid->addWidget(logo,    0, 2);
    grid->addWidget(line,    1, 0, 1, 3);
    grid->setColumnStretch(1, 1);
    return grid;
}

QLayout* ColorCorrectionDlg::createProfilesInfo() const
{
    QVBoxLayout* const vbox = new QVBoxLayout;

    if (d->mode == ProfileMismatch || d->mode == UncalibratedColor)
    {
        d->imageProfileTitle = new QLabel;

        if (d->mode == ProfileMismatch)
        {
            d->imageProfileTitle->setText(i18n("Embedded Color Profile:"));
        }
        //else if (d->mode == MissingProfile)
        //  d->imageProfileTitle->setText(i18n("Image Color Profile:"));
        else if (d->mode == UncalibratedColor)
        {
            d->imageProfileTitle->setText(i18n("Input Color Profile:"));
        }

        d->imageProfileDesc              = new QLabel;
        QPushButton* const imageProfInfo = new QPushButton(i18n("Info..."));
        //d->imageProfileTitle->setWordWrap(true);
        d->imageProfileDesc->setWordWrap(true);

        vbox->addWidget(d->imageProfileTitle);
        vbox->addWidget(d->imageProfileDesc);
        vbox->addWidget(imageProfInfo, 0, Qt::AlignLeft);

        connect(imageProfInfo, SIGNAL(clicked()),
                this, SLOT(slotImageProfInfo()));
    }

    QLabel* const workspaceProfileTitle  = new QLabel(i18n("Working Color Space:"));
    QLabel* const workspaceProfileDesc   = new QLabel(QString::fromUtf8("<b>%1</b>").arg(d->workspaceProfile.description()));
    QPushButton* const workspaceProfInfo = new QPushButton(i18n("Info..."));
    //workspaceProfileTitle->setWordWrap(true);
    workspaceProfileDesc->setWordWrap(true);

    vbox->addWidget(workspaceProfileTitle);
    vbox->addWidget(workspaceProfileDesc);
    vbox->addWidget(workspaceProfInfo, 0, Qt::AlignLeft);

    connect(workspaceProfInfo, SIGNAL(clicked()),
            this, SLOT(slotWorkspaceProfInfo()));

    return vbox;
}

QLayout* ColorCorrectionDlg::createPreviews() const
{
    QGridLayout* const grid     = new QGridLayout;
    QLabel* const originalTitle = new QLabel;

    if (d->mode == ProfileMismatch)
    {
        originalTitle->setText(i18n("Original Colors:"));
    }
    else if (d->mode == MissingProfile)
    {
        originalTitle->setText(i18n("Uncorrected Colors:"));
    }
    else if (d->mode == UncalibratedColor)
    {
        originalTitle->setText(i18n("Raw Colors:"));
    }

    originalTitle->setWordWrap(true);

    QLabel* const previewOriginal = new QLabel;
    DImg copyOriginal             = d->preview.copy();
    IccManager manager(copyOriginal);
    manager.transformForDisplay();
    previewOriginal->setPixmap(copyOriginal.convertToPixmap());

    QLabel* const targetTitle = new QLabel;

    if (d->mode == ProfileMismatch)
    {
        targetTitle->setText(i18n("Resulting Colors:"));
    }
    else if (d->mode == MissingProfile)
    {
        targetTitle->setText(i18n("Correction Applied:"));
    }
    else if (d->mode == UncalibratedColor)
    {
        targetTitle->setText(i18n("Corrected Colors:"));
    }

    targetTitle->setWordWrap(true);

    d->previewTarget = new QLabel;

    if (d->preview.width() > d->preview.height())
    {
        grid->addWidget(originalTitle,    0, 0, Qt::AlignTop);
        grid->addWidget(previewOriginal,  1, 0);
        grid->addWidget(targetTitle,      2, 0, Qt::AlignTop);
        grid->addWidget(d->previewTarget, 3, 0);
    }
    else
    {
        grid->addWidget(originalTitle,    0, 0, Qt::AlignTop);
        grid->addWidget(previewOriginal,  1, 0);
        grid->addWidget(targetTitle,      0, 1, Qt::AlignTop);
        grid->addWidget(d->previewTarget, 1, 1);
    }

    return grid;
}

QWidget* ColorCorrectionDlg::createOptions() const
{
    QGroupBox* const box    = new QGroupBox;
    QVBoxLayout* const vbox = new QVBoxLayout(box);

    if (d->mode == ProfileMismatch)
    {
        d->keepProfile           = new QRadioButton(i18n("Keep the embedded profile, do not convert"));
        d->convertToWorkingSpace = new QRadioButton(i18n("Convert to working color space"));
        d->thirdOption           = new QRadioButton(i18n("Ignore embedded profile, assign this profile:"));
        d->otherProfileBox       = new IccProfilesComboBox;
        d->otherProfileBox->addProfilesSqueezed(IccSettings::instance()->workspaceProfiles());
        d->thirdCheckBox         = new QCheckBox(i18n("and then convert to working space"));

        d->keepProfile->setChecked(true);
        d->otherProfileBox->setCurrentProfile(IccProfile::adobeRGB());
        d->otherProfileBox->setNoProfileIfEmpty(i18n("No Profile Available"));

        if (d->otherProfileBox->count() == 0) // disable if empty
        {
            d->thirdOption->setEnabled(false);
            d->otherProfileBox->setEnabled(false);
        }

        QHBoxLayout* const hboxAssign = new QHBoxLayout;
        hboxAssign->addSpacing(10);
        hboxAssign->addWidget(d->otherProfileBox);
        hboxAssign->addWidget(d->thirdCheckBox);
        hboxAssign->setSpacing(0);

        vbox->addWidget(d->keepProfile);
        vbox->addWidget(d->convertToWorkingSpace);
        vbox->addWidget(d->thirdOption);
        vbox->addLayout(hboxAssign);

        connect(d->keepProfile, SIGNAL(toggled(bool)),
                this, SLOT(imageProfileToggled(bool)));

        connect(d->convertToWorkingSpace, SIGNAL(toggled(bool)),
                this, SLOT(imageProfileToggled(bool)));

        connect(d->thirdOption, SIGNAL(toggled(bool)),
                this, SLOT(imageProfileToggled(bool)));

        connect(d->thirdCheckBox, SIGNAL(toggled(bool)),
                this, SLOT(imageProfileToggled(bool)));

        connect(d->otherProfileBox, SIGNAL(currentIndexChanged(int)),
                this, SLOT(imageProfileChanged()));
    }
    else if (d->mode == MissingProfile)
    {
        d->convertToWorkingSpace = new QRadioButton(i18n("Assign profile and convert to working color space"));
        d->keepProfile           = new QRadioButton(i18n("Assign and keep color profile"));
        d->thirdOption           = new QRadioButton(i18n("Leave the file untagged, do not color manage"));

        d->convertToWorkingSpace->setChecked(true);

        vbox->addWidget(d->convertToWorkingSpace);
        vbox->addWidget(d->keepProfile);
        vbox->addWidget(d->thirdOption);

        connect(d->keepProfile, SIGNAL(toggled(bool)),
                this, SLOT(missingProfileToggled(bool)));

        connect(d->convertToWorkingSpace, SIGNAL(toggled(bool)),
                this, SLOT(missingProfileToggled(bool)));

        connect(d->thirdOption, SIGNAL(toggled(bool)),
                this, SLOT(missingProfileToggled(bool)));
    }
    else if (d->mode == UncalibratedColor)
    {
        // empty
/*
        d->convertToWorkingSpace = new QRadioButton(i18n("Convert to working color space"));
        d->thirdOption         = new QRadioButton(i18n("Convert to this profile:"));
        d->otherProfileBox       = new IccProfilesComboBox;
        d->otherProfileBox->addProfilesSqueezed(IccSettings::instance()->workspaceProfiles());

        vbox->addWidget(d->convertToWorkingSpace);
        vbox->addWidget(d->thirdOption);
        vbox->addWidget(d->otherProfileBox);
*/
    }

    return box;
}

QWidget* ColorCorrectionDlg::createAssumeOptions() const
{
    QGroupBox* const box    = new QGroupBox;
    QGridLayout* const grid = new QGridLayout(box);

    if (d->mode == ProfileMismatch)
    {
        // unused
    }
    else if (d->mode == MissingProfile)
    {
        QLabel* const label = new QLabel(i18n("Which color space shall be used to interpret the colors of this image?"));
        label->setWordWrap(true);

        d->imageSRGB         = new QRadioButton(i18n("sRGB (Internet standard)"));
        d->imageWorkingSpace = new QRadioButton(i18n("Current working color space"));
        d->imageOtherSpace   = new QRadioButton(i18n("This profile:"));
        d->imageProfileBox   = new IccProfilesComboBox;
        d->imageProfileBox->addProfilesSqueezed(IccSettings::instance()->workspaceProfiles()
                                                << IccSettings::instance()->inputProfiles());
        QPushButton* const usedProfInfo = new QPushButton(i18n("Info..."));

        d->imageSRGB->setChecked(true);
        d->imageProfileBox->setCurrentProfile(IccProfile::adobeRGB());
        d->imageProfileBox->setNoProfileIfEmpty(i18n("No Profile Available"));

        if (d->imageProfileBox->count() == 0) // disable if empty
        {
            d->imageOtherSpace->setEnabled(false);
            d->imageProfileBox->setEnabled(false);
        }

        grid->addWidget(label,                0, 0, 1, 2);
        grid->addWidget(d->imageSRGB,         1, 0, 1, 2);
        grid->addWidget(d->imageWorkingSpace, 2, 0, 1, 2);
        grid->addWidget(d->imageOtherSpace,   3, 0, 1, 2);
        grid->addWidget(d->imageProfileBox,   4, 1);
        grid->addWidget(usedProfInfo,         5, 0, 1, 2, Qt::AlignLeft);
        grid->setColumnMinimumWidth(0, 10);
        grid->setColumnStretch(1, 1);

        connect(d->imageProfileBox, SIGNAL(currentIndexChanged(int)),
                this, SLOT(usedProfileChanged()));

        connect(d->imageSRGB, SIGNAL(toggled(bool)),
                this, SLOT(usedProfileToggled(bool)));

        connect(d->imageWorkingSpace, SIGNAL(toggled(bool)),
                this, SLOT(usedProfileToggled(bool)));

        connect(d->imageOtherSpace, SIGNAL(toggled(bool)),
                this, SLOT(usedProfileToggled(bool)));

        connect(usedProfInfo, SIGNAL(clicked()),
                this, SLOT(slotImageProfInfo()));
    }
    else if (d->mode == UncalibratedColor)
    {
        QLabel* const label = new QLabel(i18n("Please select the input color profile of the device (camera) used to create this image:"));
        label->setWordWrap(true);

        d->imageProfileBox = new IccProfilesComboBox;
        d->imageProfileBox->addProfilesSqueezed(IccSettings::instance()->inputProfiles());
        d->imageProfileBox->setCurrentProfile(IccSettings::instance()->settings().defaultInputProfile);
        d->imageProfileBox->setNoProfileIfEmpty(i18n("No Input Profile Available"));

        grid->addWidget(label);
        grid->addWidget(d->imageProfileBox);

        connect(d->imageProfileBox, SIGNAL(currentIndexChanged(int)),
                this, SLOT(inputProfileChanged()));
    }

    return box;
}

void ColorCorrectionDlg::imageProfileToggled(bool on)
{
    if (!on)
    {
        return;
    }

    imageProfileChanged();
}

void ColorCorrectionDlg::imageProfileChanged()
{
    updateImageProfileUI();
    updateInfo();
}

void ColorCorrectionDlg::updateImageProfileUI()
{
    if (d->otherProfileBox)
    {
        d->otherProfileBox->setEnabled(d->thirdOption->isChecked());
    }

    if (d->thirdCheckBox)
    {
        d->thirdCheckBox->setEnabled(d->thirdOption->isChecked());
    }
}

void ColorCorrectionDlg::missingProfileToggled(bool on)
{
    if (!on)
    {
        return;
    }

    missingProfileChanged();
}

void ColorCorrectionDlg::missingProfileChanged()
{
    updateInfo();
}

void ColorCorrectionDlg::usedProfileToggled(bool on)
{
    if (!on)
    {
        return;
    }

    usedProfileChanged();
}

void ColorCorrectionDlg::usedProfileChanged()
{
    updateUsedProfileUI();
    updateInfo();
}

void ColorCorrectionDlg::updateUsedProfileUI()
{
    if (d->imageProfileBox && d->imageOtherSpace)
    {
        d->imageProfileBox->setEnabled(d->imageOtherSpace->isChecked());
    }
}

void ColorCorrectionDlg::inputProfileChanged()
{
    updateInfo();
}

void ColorCorrectionDlg::updateInfo()
{
    setCursor(Qt::WaitCursor);

    DImg colorPreview                = d->preview.copy();
    IccManager manager(colorPreview);
    ICCSettingsContainer::Behavior b = currentBehavior();
    d->imageProfile                  = manager.imageProfile(b, specifiedProfile());

    if (d->mode == ProfileMismatch)
    {
        if (b & ICCSettingsContainer::UseSpecifiedProfile)
        {
            d->imageProfileTitle->setText(i18n("Assigned Color Profile:"));
        }
        else
        {
            d->imageProfileTitle->setText(i18n("Embedded Color Profile:"));
        }
    }

    if (d->mode == ProfileMismatch || d->mode == UncalibratedColor)
    {
        QString description = d->imageProfile.description();

        if (description.isEmpty())
        {
            d->imageProfileDesc->setText(i18n("<b>No Profile</b>"));
        }
        else
        {
            d->imageProfileDesc->setText(QString::fromUtf8("<b>%1</b>").arg(description));
        }
    }

    manager.transform(currentBehavior(), specifiedProfile());
    manager.transformForDisplay(this);
    d->previewTarget->setPixmap(colorPreview.convertToPixmap());

    unsetCursor();
}

void ColorCorrectionDlg::slotWorkspaceProfInfo()
{
    ICCProfileInfoDlg infoDlg(parentWidget(), QString(), d->workspaceProfile);
    infoDlg.exec();
}

void ColorCorrectionDlg::slotImageProfInfo()
{
    if (d->imageProfile.isNull())
    {
        return;
    }

    ICCProfileInfoDlg infoDlg(parentWidget(), QString(), d->imageProfile);
    infoDlg.exec();
}

ICCSettingsContainer::Behavior ColorCorrectionDlg::behavior() const
{
    if (result() == QDialog::Rejected)
    {
        return ICCSettingsContainer::SafestBestAction;
    }

    return currentBehavior();
}

ICCSettingsContainer::Behavior ColorCorrectionDlg::currentBehavior() const
{
    if (d->mode == ProfileMismatch)
    {
        if (d->keepProfile->isChecked())
        {
            return ICCSettingsContainer::PreserveEmbeddedProfile;
        }
        else if (d->convertToWorkingSpace->isChecked())
        {
            return ICCSettingsContainer::EmbeddedToWorkspace;
        }
        else if (d->thirdOption->isChecked())
        {
            if (d->thirdCheckBox->isChecked())
            {
                return ICCSettingsContainer::UseSpecifiedProfile | ICCSettingsContainer::ConvertToWorkspace;
            }
            else
            {
                return ICCSettingsContainer::UseSpecifiedProfile | ICCSettingsContainer::KeepProfile;
            }
        }
    }
    else if (d->mode == MissingProfile)
    {
        if (d->thirdOption->isChecked())
        {
            return ICCSettingsContainer::NoColorManagement;
        }

        ICCSettingsContainer::Behavior behavior;

        if (d->keepProfile->isChecked())
        {
            behavior |= ICCSettingsContainer::KeepProfile;
        }
        else if (d->convertToWorkingSpace->isChecked())
        {
            behavior |= ICCSettingsContainer::ConvertToWorkspace;
        }

        if (d->imageSRGB->isChecked())
        {
            behavior |= ICCSettingsContainer::UseSRGB;
        }
        else if (d->imageWorkingSpace->isChecked())
        {
            behavior |= ICCSettingsContainer::UseWorkspace;
        }
        else if (d->imageOtherSpace->isChecked())
        {
            behavior |= ICCSettingsContainer::UseSpecifiedProfile;
        }

        return behavior;
    }
    else if (d->mode == UncalibratedColor)
    {
        return ICCSettingsContainer::ConvertToWorkspace | ICCSettingsContainer::UseSpecifiedProfile;
    }

    return ICCSettingsContainer::SafestBestAction;
}

IccProfile ColorCorrectionDlg::specifiedProfile() const
{
    if (d->mode == ProfileMismatch)
    {
        return d->otherProfileBox->currentProfile();
    }
    else if (d->mode == MissingProfile || d->mode == UncalibratedColor)
    {
        return d->imageProfileBox->currentProfile();
    }

    return IccProfile();
}

void ColorCorrectionDlg::readSettings()
{
    ICCSettingsContainer settings = IccSettings::instance()->settings();

    if (d->mode == ProfileMismatch)
    {
        if ((settings.lastMismatchBehavior & ICCSettingsContainer::UseSpecifiedProfile) &&
            d->otherProfileBox->count() > 0)
        {
            d->thirdOption->setChecked(true);
            d->thirdCheckBox->setChecked(settings.lastMismatchBehavior & ICCSettingsContainer::ConvertToWorkspace);
            d->thirdOption->setFocus();
        }
        else
        {
            if (settings.lastMismatchBehavior & ICCSettingsContainer::KeepProfile)
            {
                d->keepProfile->setChecked(true);
                d->keepProfile->setFocus();
            }
            else if (settings.lastMismatchBehavior & ICCSettingsContainer::ConvertToWorkspace)
            {
                d->convertToWorkingSpace->setChecked(true);
                d->convertToWorkingSpace->setFocus();
            }
        }

        if (!settings.lastSpecifiedAssignProfile.isEmpty())
        {
            d->otherProfileBox->setCurrentProfile(settings.lastSpecifiedAssignProfile);
        }
    }
    else if (d->mode == MissingProfile)
    {
        if (settings.lastMissingProfileBehavior == ICCSettingsContainer::NoColorManagement)
        {
            d->imageSRGB->setChecked(true); // ?
            d->thirdOption->setChecked(true);
            d->thirdOption->setFocus();
        }
        else
        {
            if (settings.lastMissingProfileBehavior & ICCSettingsContainer::KeepProfile)
            {
                d->keepProfile->setChecked(true);
                d->keepProfile->setFocus();
            }
            else if (settings.lastMissingProfileBehavior & ICCSettingsContainer::ConvertToWorkspace)
            {
                d->convertToWorkingSpace->setChecked(true);
                d->convertToWorkingSpace->setFocus();
            }

            if (settings.lastMissingProfileBehavior & ICCSettingsContainer::UseSRGB)
            {
                d->imageSRGB->setChecked(true);
            }

            if (settings.lastMissingProfileBehavior & ICCSettingsContainer::UseWorkspace)
            {
                d->imageWorkingSpace->setChecked(true);
            }

            if ((settings.lastMissingProfileBehavior & ICCSettingsContainer::UseSpecifiedProfile)
                && d->imageProfileBox->count() > 0)
            {
                d->imageOtherSpace->setChecked(true);
            }
        }

        if (!settings.lastSpecifiedInputProfile.isEmpty())
        {
            d->imageProfileBox->setCurrentProfile(settings.lastSpecifiedInputProfile);
        }
    }
    else if (d->mode == UncalibratedColor)
    {
        d->imageProfileBox->setCurrentProfile(settings.lastSpecifiedInputProfile);
        d->imageProfileBox->setFocus();
    }
}

void ColorCorrectionDlg::setSettings()
{
    ICCSettingsContainer settings = IccSettings::instance()->settings();

    if (d->mode == ProfileMismatch)
    {
        settings.lastMismatchBehavior = currentBehavior();
        settings.lastSpecifiedAssignProfile = specifiedProfile().filePath();
    }
    else if (d->mode == MissingProfile)
    {
        settings.lastMissingProfileBehavior = currentBehavior();
        settings.lastSpecifiedInputProfile = specifiedProfile().filePath();
    }
    else if (d->mode == UncalibratedColor)
    {
        settings.lastUncalibratedBehavior = currentBehavior();
        settings.lastSpecifiedInputProfile = specifiedProfile().filePath();
    }

    IccSettings::instance()->setSettings(settings);
}

void ColorCorrectionDlg::slotHelp()
{
    DXmlGuiWindow::openHandbook();
}

}  // namespace Digikam
