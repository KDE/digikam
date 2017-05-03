/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QHBoxLayout>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "invisiblebuttongroup.h"

namespace Digikam
{

HTMLImageSettingsPage::HTMLImageSettingsPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title)
{
    setObjectName(QStringLiteral("ImageSettingsPage"));

    gridLayout = new QGridLayout(this);
    gridLayout->setSpacing(6);
    gridLayout->setContentsMargins(11, 11, 11, 11);
    gridLayout->setObjectName(QStringLiteral("gridLayout"));
    label = new QLabel(this);
    label->setObjectName(QStringLiteral("label"));
    QFont font;
    font.setBold(true);
    font.setWeight(75);
    label->setFont(font);

    gridLayout->addWidget(label, 0, 0, 1, 2);

    mSaveImageButton = new QRadioButton(this);
    mSaveImageButton->setObjectName(QStringLiteral("mSaveImageButton"));
    mSaveImageButton->setChecked(true);

    gridLayout->addWidget(mSaveImageButton, 1, 0, 1, 2);

    textLabel2_2_2_2 = new QLabel(this);
    textLabel2_2_2_2->setObjectName(QStringLiteral("textLabel2_2_2_2"));
    textLabel2_2_2_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    textLabel2_2_2_2->setWordWrap(false);

    gridLayout->addWidget(textLabel2_2_2_2, 2, 1, 1, 1);

    kcfg_fullFormat = new QComboBox(this);
    kcfg_fullFormat->setObjectName(QStringLiteral("kcfg_fullFormat"));

    gridLayout->addWidget(kcfg_fullFormat, 2, 2, 1, 1);

    spacer4 = new QSpacerItem(312, 17, QSizePolicy::Expanding, QSizePolicy::Minimum);

    gridLayout->addItem(spacer4, 2, 3, 1, 1);

    textLabel4 = new QLabel(this);
    textLabel4->setObjectName(QStringLiteral("textLabel4"));
    textLabel4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    textLabel4->setWordWrap(false);

    gridLayout->addWidget(textLabel4, 3, 1, 1, 1);

    kcfg_fullQuality = new QSpinBox(this);
    kcfg_fullQuality->setObjectName(QStringLiteral("kcfg_fullQuality"));
    kcfg_fullQuality->setMaximum(100);

    gridLayout->addWidget(kcfg_fullQuality, 3, 2, 1, 1);

    horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

    gridLayout->addItem(horizontalSpacer_6, 4, 0, 1, 1);

    widget = new QWidget(this);
    widget->setObjectName(QStringLiteral("widget"));
    horizontalLayout_2 = new QHBoxLayout(widget);
    horizontalLayout_2->setSpacing(6);
    horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
    horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
    kcfg_fullResize = new QCheckBox(widget);
    kcfg_fullResize->setObjectName(QStringLiteral("kcfg_fullResize"));
    kcfg_fullResize->setChecked(true);

    horizontalLayout_2->addWidget(kcfg_fullResize);

    kcfg_fullSize = new QSpinBox(widget);
    kcfg_fullSize->setObjectName(QStringLiteral("kcfg_fullSize"));
    kcfg_fullSize->setMinimum(1);
    kcfg_fullSize->setMaximum(9999);
    kcfg_fullSize->setValue(800);

    horizontalLayout_2->addWidget(kcfg_fullSize);

    horizontalSpacer_2 = new QSpacerItem(188, 27, QSizePolicy::Expanding, QSizePolicy::Minimum);

    horizontalLayout_2->addItem(horizontalSpacer_2);

    gridLayout->addWidget(widget, 4, 2, 1, 2);

    horizontalLayout = new QHBoxLayout();
    horizontalLayout->setSpacing(6);
    horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
    kcfg_copyOriginalImage = new QCheckBox(this);
    kcfg_copyOriginalImage->setObjectName(QStringLiteral("kcfg_copyOriginalImage"));
    kcfg_copyOriginalImage->setChecked(false);

    horizontalLayout->addWidget(kcfg_copyOriginalImage);

    horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    horizontalLayout->addItem(horizontalSpacer_3);

    gridLayout->addLayout(horizontalLayout, 5, 2, 1, 2);

    mUseOriginalImageButton = new QRadioButton(this);
    mUseOriginalImageButton->setObjectName(QStringLiteral("mUseOriginalImageButton"));

    gridLayout->addWidget(mUseOriginalImageButton, 6, 0, 1, 3);

    verticalSpacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed);

    gridLayout->addItem(verticalSpacer, 7, 0, 1, 1);

    label_2 = new QLabel(this);
    label_2->setObjectName(QStringLiteral("label_2"));
    label_2->setFont(font);

    gridLayout->addWidget(label_2, 8, 0, 1, 2);

    textLabel2_2_2 = new QLabel(this);
    textLabel2_2_2->setObjectName(QStringLiteral("textLabel2_2_2"));
    textLabel2_2_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    textLabel2_2_2->setWordWrap(false);

    gridLayout->addWidget(textLabel2_2_2, 9, 1, 1, 1);

    kcfg_thumbnailFormat = new QComboBox(this);
    kcfg_thumbnailFormat->setObjectName(QStringLiteral("kcfg_thumbnailFormat"));

    gridLayout->addWidget(kcfg_thumbnailFormat, 9, 2, 1, 1);

    horizontalSpacer_7 = new QSpacerItem(40, 27, QSizePolicy::Fixed, QSizePolicy::Minimum);

    gridLayout->addItem(horizontalSpacer_7, 10, 0, 1, 1);

    textLabel4_2 = new QLabel(this);
    textLabel4_2->setObjectName(QStringLiteral("textLabel4_2"));
    textLabel4_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    textLabel4_2->setWordWrap(false);

    gridLayout->addWidget(textLabel4_2, 10, 1, 1, 1);

    kcfg_thumbnailQuality = new QSpinBox(this);
    kcfg_thumbnailQuality->setObjectName(QStringLiteral("kcfg_thumbnailQuality"));
    kcfg_thumbnailQuality->setMaximum(100);

    gridLayout->addWidget(kcfg_thumbnailQuality, 10, 2, 1, 1);

    horizontalSpacer_4 = new QSpacerItem(309, 27, QSizePolicy::Expanding, QSizePolicy::Minimum);

    gridLayout->addItem(horizontalSpacer_4, 10, 3, 1, 1);

    textLabel2_2 = new QLabel(this);
    textLabel2_2->setObjectName(QStringLiteral("textLabel2_2"));
    textLabel2_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    textLabel2_2->setWordWrap(false);

    gridLayout->addWidget(textLabel2_2, 11, 1, 1, 1);

    kcfg_thumbnailSize = new QSpinBox(this);
    kcfg_thumbnailSize->setObjectName(QStringLiteral("kcfg_thumbnailSize"));
    kcfg_thumbnailSize->setMinimum(1);
    kcfg_thumbnailSize->setMaximum(9999);
    kcfg_thumbnailSize->setValue(160);

    gridLayout->addWidget(kcfg_thumbnailSize, 11, 2, 1, 1);

    kcfg_thumbnailSquare = new QCheckBox(this);
    kcfg_thumbnailSquare->setObjectName(QStringLiteral("kcfg_thumbnailSquare"));
    kcfg_thumbnailSquare->setEnabled(false);
    kcfg_thumbnailSquare->setCheckable(true);
    kcfg_thumbnailSquare->setChecked(true);

    gridLayout->addWidget(kcfg_thumbnailSquare, 12, 2, 1, 2);

    verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    gridLayout->addItem(verticalSpacer_2, 13, 3, 1, 1);

    textLabel2_2_2_2->setBuddy(kcfg_fullFormat);
    textLabel4->setBuddy(kcfg_fullQuality);
    textLabel2_2_2->setBuddy(kcfg_thumbnailFormat);
    textLabel4_2->setBuddy(kcfg_thumbnailQuality);
    textLabel2_2->setBuddy(kcfg_thumbnailSize);

    label->setText(i18n("Full Image"));
    mSaveImageButton->setText(i18n("Save image"));
    textLabel2_2_2_2->setText(i18n("Format:"));
    kcfg_fullFormat->clear();
    kcfg_fullFormat->insertItems(0, QStringList() << i18n("JPEG") << i18n("PNG"));
    textLabel4->setText(i18n("Quality:"));
    kcfg_fullResize->setText(i18n("Max size:"));
    kcfg_copyOriginalImage->setText(i18n("Include full-size original images for download"));
    mUseOriginalImageButton->setText(i18n("Use original image"));
    label_2->setText(i18n("Thumbnail"));
    textLabel2_2_2->setText(i18n("Format:"));
    kcfg_thumbnailFormat->clear();
    kcfg_thumbnailFormat->insertItems(0, QStringList() << i18n("JPEG") << i18n("PNG"));
    textLabel4_2->setText(i18n("Quality:"));
    textLabel2_2->setText(i18n("Size:"));
    kcfg_thumbnailSquare->setText(i18n("Square thumbnails"));

    QWidget::setTabOrder(mSaveImageButton, kcfg_fullFormat);
    QWidget::setTabOrder(kcfg_fullFormat, kcfg_fullQuality);
    QWidget::setTabOrder(kcfg_fullQuality, kcfg_fullResize);
    QWidget::setTabOrder(kcfg_fullResize, kcfg_fullSize);
    QWidget::setTabOrder(kcfg_fullSize, kcfg_copyOriginalImage);
    QWidget::setTabOrder(kcfg_copyOriginalImage, mUseOriginalImageButton);
    QWidget::setTabOrder(mUseOriginalImageButton, kcfg_thumbnailFormat);
    QWidget::setTabOrder(kcfg_thumbnailFormat, kcfg_thumbnailQuality);
    QWidget::setTabOrder(kcfg_thumbnailQuality, kcfg_thumbnailSize);
    QWidget::setTabOrder(kcfg_thumbnailSize, kcfg_thumbnailSquare);

    QObject::connect(kcfg_fullResize,  SIGNAL(toggled(bool)), kcfg_fullSize, SLOT(setEnabled(bool)));
    QObject::connect(mSaveImageButton, SIGNAL(toggled(bool)), kcfg_fullFormat, SLOT(setEnabled(bool)));
    QObject::connect(mSaveImageButton, SIGNAL(toggled(bool)), kcfg_fullQuality, SLOT(setEnabled(bool)));
    QObject::connect(mSaveImageButton, SIGNAL(toggled(bool)), kcfg_copyOriginalImage, SLOT(setEnabled(bool)));
    QObject::connect(mSaveImageButton, SIGNAL(toggled(bool)), textLabel2_2_2_2, SLOT(setEnabled(bool)));
    QObject::connect(mSaveImageButton, SIGNAL(toggled(bool)), textLabel4, SLOT(setEnabled(bool)));
    QObject::connect(mSaveImageButton, SIGNAL(toggled(bool)), widget, SLOT(setEnabled(bool)));

    InvisibleButtonGroup* const group = new InvisibleButtonGroup(this);
    group->setObjectName(QLatin1String("kcfg_useOriginalImageAsFullImage"));
    group->addButton(mSaveImageButton,        false);
    group->addButton(mUseOriginalImageButton, true);
}

HTMLImageSettingsPage::~HTMLImageSettingsPage()
{
}

} // namespace Digikam
