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

#ifndef HTML_IMAGE_SETTINGS_PAGE_H
#define HTML_IMAGE_SETTINGS_PAGE_H

// Qt includes

#include <QGridLayout>
#include <QSpinBox>
#include <QComboBox>
#include <QRadioButton>
#include <QWidget>
#include <QLabel>
#include <QSpacerItem>
#include <QHBoxLayout>
#include <QCheckBox>

// Local includes

#include "dwizardpage.h"

namespace Digikam
{

class HTMLImageSettingsPage : public DWizardPage
{
public:

    explicit HTMLImageSettingsPage(QWizard* const dialog, const QString& title);
    ~HTMLImageSettingsPage();

public:

    QGridLayout*    gridLayout;
    QLabel*         label;
    QRadioButton*   mSaveImageButton;
    QLabel*         textLabel2_2_2_2;
    QComboBox*      kcfg_fullFormat;
    QSpacerItem*    spacer4;
    QLabel*         textLabel4;
    QSpinBox*       kcfg_fullQuality;
    QSpacerItem*    horizontalSpacer_6;
    QWidget*        widget;
    QHBoxLayout*    horizontalLayout_2;
    QCheckBox*      kcfg_fullResize;
    QSpinBox*       kcfg_fullSize;
    QSpacerItem*    horizontalSpacer_2;
    QHBoxLayout*    horizontalLayout;
    QCheckBox*      kcfg_copyOriginalImage;
    QSpacerItem*    horizontalSpacer_3;
    QRadioButton*   mUseOriginalImageButton;
    QSpacerItem*    verticalSpacer;
    QLabel*         label_2;
    QLabel*         textLabel2_2_2;
    QComboBox*      kcfg_thumbnailFormat;
    QSpacerItem*    horizontalSpacer_7;
    QLabel*         textLabel4_2;
    QSpinBox*       kcfg_thumbnailQuality;
    QSpacerItem*    horizontalSpacer_4;
    QLabel*         textLabel2_2;
    QSpinBox*       kcfg_thumbnailSize;
    QCheckBox*      kcfg_thumbnailSquare;
    QSpacerItem*    verticalSpacer_2;
};

} // namespace Digikam

#endif // HTML_IMAGE_SETTINGS_PAGE_H
