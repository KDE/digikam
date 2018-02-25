/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "htmlimagesettingspage.h"

// Qt includes

#include <QIcon>
#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QGridLayout>
#include <QSpacerItem>
#include <QCheckBox>
#include <QSpinBox>
#include <QComboBox>
#include <QRadioButton>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "invisiblebuttongroup.h"
#include "galleryinfo.h"
#include "htmlwizard.h"

namespace Digikam
{

class HTMLImageSettingsPage::Private
{
public:

    explicit Private()
      : kcfg_thumbnailSquare(0)
    {
    }

    QCheckBox* kcfg_thumbnailSquare;
};

HTMLImageSettingsPage::HTMLImageSettingsPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private)
{
    setObjectName(QLatin1String("ImageSettingsPage"));

    QWidget* const box  = new QWidget(this);

    QLabel* const label = new QLabel(this);
    label->setObjectName(QLatin1String("label"));
    QFont font;
    font.setBold(true);
    font.setWeight(75);
    label->setFont(font);
    label->setText(i18n("Full Image Properties:"));

    QRadioButton* const mSaveImageButton = new QRadioButton(this);
    mSaveImageButton->setObjectName(QLatin1String("mSaveImageButton"));
    mSaveImageButton->setChecked(true);
    mSaveImageButton->setText(i18n("Save image"));

    QLabel* const textLabel2_2_2_2 = new QLabel(this);
    textLabel2_2_2_2->setObjectName(QLatin1String("textLabel2_2_2_2"));
    textLabel2_2_2_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    textLabel2_2_2_2->setWordWrap(false);
    textLabel2_2_2_2->setText(i18n("Format:"));

    QComboBox* const kcfg_fullFormat = new QComboBox(this);
    kcfg_fullFormat->setObjectName(QLatin1String("kcfg_fullFormat"));
    kcfg_fullFormat->clear();
    kcfg_fullFormat->insertItems(0, QStringList() << i18n("JPEG") << i18n("PNG"));
    textLabel2_2_2_2->setBuddy(kcfg_fullFormat);

    QSpacerItem* const spacer4 = new QSpacerItem(312, 17, QSizePolicy::Expanding,
                                                 QSizePolicy::Minimum);

    QLabel* const textLabel4 = new QLabel(this);
    textLabel4->setObjectName(QLatin1String("textLabel4"));
    textLabel4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    textLabel4->setWordWrap(false);
    textLabel4->setText(i18n("Quality:"));

    QSpinBox* const kcfg_fullQuality = new QSpinBox(this);
    kcfg_fullQuality->setObjectName(QLatin1String("kcfg_fullQuality"));
    kcfg_fullQuality->setMaximum(100);
    textLabel4->setBuddy(kcfg_fullQuality);

    QSpacerItem* const horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Fixed,
                                                            QSizePolicy::Minimum);

    QWidget* const widget = new QWidget(this);
    widget->setObjectName(QLatin1String("widget"));

    QCheckBox* const kcfg_fullResize = new QCheckBox(widget);
    kcfg_fullResize->setObjectName(QLatin1String("kcfg_fullResize"));
    kcfg_fullResize->setChecked(true);
    kcfg_fullResize->setText(i18n("Max size:"));

    QSpinBox* const kcfg_fullSize = new QSpinBox(widget);
    kcfg_fullSize->setObjectName(QLatin1String("kcfg_fullSize"));
    kcfg_fullSize->setMinimum(1);
    kcfg_fullSize->setMaximum(9999);
    kcfg_fullSize->setValue(800);

    QSpacerItem* const horizontalSpacer_2 = new QSpacerItem(188, 27, QSizePolicy::Expanding,
                                                            QSizePolicy::Minimum);

    QHBoxLayout* const horizontalLayout_2 = new QHBoxLayout(widget);
    horizontalLayout_2->setContentsMargins(QMargins());
    horizontalLayout_2->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    horizontalLayout_2->setObjectName(QLatin1String("horizontalLayout_2"));
    horizontalLayout_2->addWidget(kcfg_fullResize);
    horizontalLayout_2->addWidget(kcfg_fullSize);
    horizontalLayout_2->addItem(horizontalSpacer_2);

    QCheckBox* const kcfg_copyOriginalImage = new QCheckBox(this);
    kcfg_copyOriginalImage->setObjectName(QLatin1String("kcfg_copyOriginalImage"));
    kcfg_copyOriginalImage->setChecked(false);
    kcfg_copyOriginalImage->setText(i18n("Include full-size original images for download"));

    QSpacerItem* const horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding,
                                                            QSizePolicy::Minimum);

    QHBoxLayout* const horizontalLayout = new QHBoxLayout();
    horizontalLayout->setContentsMargins(QMargins());
    horizontalLayout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    horizontalLayout->setObjectName(QLatin1String("horizontalLayout"));
    horizontalLayout->addWidget(kcfg_copyOriginalImage);
    horizontalLayout->addItem(horizontalSpacer_3);

    QRadioButton* const mUseOriginalImageButton = new QRadioButton(this);
    mUseOriginalImageButton->setObjectName(QLatin1String("mUseOriginalImageButton"));
    mUseOriginalImageButton->setText(i18n("Use original image"));

    QSpacerItem* const verticalSpacer = new QSpacerItem(20, 20, QSizePolicy::Minimum,
                                                        QSizePolicy::Fixed);

    QLabel* const label_2 = new QLabel(this);
    label_2->setObjectName(QLatin1String("label_2"));
    label_2->setFont(font);
    label_2->setText(i18n("Thumbnail Properties:"));

    QLabel* const textLabel2_2_2 = new QLabel(this);
    textLabel2_2_2->setObjectName(QLatin1String("textLabel2_2_2"));
    textLabel2_2_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    textLabel2_2_2->setWordWrap(false);
    textLabel2_2_2->setText(i18n("Format:"));

    QComboBox* const kcfg_thumbnailFormat = new QComboBox(this);
    kcfg_thumbnailFormat->setObjectName(QLatin1String("kcfg_thumbnailFormat"));
    kcfg_thumbnailFormat->clear();
    kcfg_thumbnailFormat->insertItems(0, QStringList() << i18n("JPEG") << i18n("PNG"));
    textLabel2_2_2->setBuddy(kcfg_thumbnailFormat);

    QSpacerItem* const horizontalSpacer_7 = new QSpacerItem(40, 27, QSizePolicy::Fixed,
                                                            QSizePolicy::Minimum);

    QLabel* const textLabel4_2 = new QLabel(this);
    textLabel4_2->setObjectName(QLatin1String("textLabel4_2"));
    textLabel4_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    textLabel4_2->setWordWrap(false);
    textLabel4_2->setText(i18n("Quality:"));

    QSpinBox* const kcfg_thumbnailQuality = new QSpinBox(this);
    kcfg_thumbnailQuality->setObjectName(QLatin1String("kcfg_thumbnailQuality"));
    kcfg_thumbnailQuality->setMaximum(100);
    textLabel4_2->setBuddy(kcfg_thumbnailQuality);

    QSpacerItem* const horizontalSpacer_4 = new QSpacerItem(309, 27, QSizePolicy::Expanding,
                                                            QSizePolicy::Minimum);

    QLabel* const textLabel2_2 = new QLabel(this);
    textLabel2_2->setObjectName(QLatin1String("textLabel2_2"));
    textLabel2_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    textLabel2_2->setWordWrap(false);
    textLabel2_2->setText(i18n("Size:"));

    QSpinBox* const kcfg_thumbnailSize = new QSpinBox(this);
    kcfg_thumbnailSize->setObjectName(QLatin1String("kcfg_thumbnailSize"));
    kcfg_thumbnailSize->setMinimum(1);
    kcfg_thumbnailSize->setMaximum(9999);
    kcfg_thumbnailSize->setValue(160);
    textLabel2_2->setBuddy(kcfg_thumbnailSize);

    d->kcfg_thumbnailSquare = new QCheckBox(this);
    d->kcfg_thumbnailSquare->setObjectName(QLatin1String("kcfg_thumbnailSquare"));
    d->kcfg_thumbnailSquare->setEnabled(false);
    d->kcfg_thumbnailSquare->setCheckable(true);
    d->kcfg_thumbnailSquare->setChecked(true);
    d->kcfg_thumbnailSquare->setText(i18n("Square thumbnails"));

    QSpacerItem* const verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum,
                                                          QSizePolicy::Expanding);

    QGridLayout* const gridLayout       = new QGridLayout(box);
    gridLayout->setContentsMargins(QMargins());
    gridLayout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    gridLayout->setObjectName(QLatin1String("gridLayout"));
    gridLayout->addWidget(label,                    0, 0, 1, 2);
    gridLayout->addWidget(mSaveImageButton,         1, 0, 1, 2);
    gridLayout->addWidget(textLabel2_2_2_2,         2, 1, 1, 1);
    gridLayout->addWidget(kcfg_fullFormat,          2, 2, 1, 1);
    gridLayout->addItem(spacer4,                    2, 3, 1, 1);
    gridLayout->addWidget(textLabel4,               3, 1, 1, 1);
    gridLayout->addWidget(kcfg_fullQuality,         3, 2, 1, 1);
    gridLayout->addItem(horizontalSpacer_6,         4, 0, 1, 1);
    gridLayout->addWidget(widget,                   4, 2, 1, 2);
    gridLayout->addLayout(horizontalLayout,         5, 2, 1, 2);
    gridLayout->addWidget(mUseOriginalImageButton,  6, 0, 1, 3);
    gridLayout->addItem(verticalSpacer,             7, 0, 1, 1);
    gridLayout->addWidget(label_2,                  8, 0, 1, 2);
    gridLayout->addWidget(textLabel2_2_2,           9, 1, 1, 1);
    gridLayout->addWidget(kcfg_thumbnailFormat,     9, 2, 1, 1);
    gridLayout->addItem(horizontalSpacer_7,        10, 0, 1, 1);
    gridLayout->addWidget(textLabel4_2,            10, 1, 1, 1);
    gridLayout->addWidget(kcfg_thumbnailQuality,   10, 2, 1, 1);
    gridLayout->addItem(horizontalSpacer_4,        10, 3, 1, 1);
    gridLayout->addWidget(textLabel2_2,            11, 1, 1, 1);
    gridLayout->addWidget(kcfg_thumbnailSize,      11, 2, 1, 1);
    gridLayout->addWidget(d->kcfg_thumbnailSquare, 12, 2, 1, 2);
    gridLayout->addItem(verticalSpacer_2,          13, 3, 1, 1);

    setPageWidget(box);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("configure")));

    setTabOrder(mSaveImageButton,        kcfg_fullFormat);
    setTabOrder(kcfg_fullFormat,         kcfg_fullQuality);
    setTabOrder(kcfg_fullQuality,        kcfg_fullResize);
    setTabOrder(kcfg_fullResize,         kcfg_fullSize);
    setTabOrder(kcfg_fullSize,           kcfg_copyOriginalImage);
    setTabOrder(kcfg_copyOriginalImage,  mUseOriginalImageButton);
    setTabOrder(mUseOriginalImageButton, kcfg_thumbnailFormat);
    setTabOrder(kcfg_thumbnailFormat,    kcfg_thumbnailQuality);
    setTabOrder(kcfg_thumbnailQuality,   kcfg_thumbnailSize);
    setTabOrder(kcfg_thumbnailSize,      d->kcfg_thumbnailSquare);

    connect(kcfg_fullResize, SIGNAL(toggled(bool)),
            kcfg_fullSize, SLOT(setEnabled(bool)));

    connect(mSaveImageButton, SIGNAL(toggled(bool)),
            kcfg_fullFormat, SLOT(setEnabled(bool)));

    connect(mSaveImageButton, SIGNAL(toggled(bool)),
            kcfg_fullQuality, SLOT(setEnabled(bool)));

    connect(mSaveImageButton, SIGNAL(toggled(bool)),
            kcfg_copyOriginalImage, SLOT(setEnabled(bool)));

    connect(mSaveImageButton, SIGNAL(toggled(bool)),
            textLabel2_2_2_2, SLOT(setEnabled(bool)));

    connect(mSaveImageButton, SIGNAL(toggled(bool)),
            textLabel4, SLOT(setEnabled(bool)));

    connect(mSaveImageButton, SIGNAL(toggled(bool)),
            widget, SLOT(setEnabled(bool)));

    InvisibleButtonGroup* const group = new InvisibleButtonGroup(this);
    group->setObjectName(QLatin1String("kcfg_useOriginalImageAsFullImage"));
    group->addButton(mSaveImageButton,        false);
    group->addButton(mUseOriginalImageButton, true);
}

HTMLImageSettingsPage::~HTMLImageSettingsPage()
{
    delete d;
}

void HTMLImageSettingsPage::initializePage()
{
    HTMLWizard* const wizard      = dynamic_cast<HTMLWizard*>(assistant());

    if (!wizard)
        return;

    GalleryTheme::Ptr theme       = wizard->galleryTheme();
    bool allowNonsquareThumbnails = theme->allowNonsquareThumbnails();

    d->kcfg_thumbnailSquare->setEnabled(allowNonsquareThumbnails);

    if (!allowNonsquareThumbnails)
    {
        d->kcfg_thumbnailSquare->setChecked(true);
    }
}

} // namespace Digikam
