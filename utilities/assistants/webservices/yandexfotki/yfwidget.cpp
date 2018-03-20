/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-09-12
 * Description : a tool to export items to YandexFotki web service
 *
 * Copyright (C) 2015      by Shourya Singh Gupta <shouryasgupta at gmail dot com>
 * Copyright (C) 2015-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "yfwidget.h"

// Qt includes

#include <QLabel>
#include <QSpinBox>
#include <QRadioButton>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QApplication>
#include <QPushButton>
#include <QLineEdit>
#include <QButtonGroup>

// KDE includes

#include <klocalizedstring.h>

//local includes

#include "yfphoto.h"
#include "yfalbum.h"

namespace Digikam
{

class YFWidget::Private
{
public:

    explicit Private()
    {
        accessCombo          = 0;
        hideOriginalCheck    = 0;
        disableCommentsCheck = 0;
        adultCheck           = 0;
        policyGroup          = 0;
    }

    // upload settings
    QComboBox*    accessCombo;
    QCheckBox*    hideOriginalCheck;
    QCheckBox*    disableCommentsCheck;
    QCheckBox*    adultCheck;
    QButtonGroup* policyGroup;
};

YFWidget::YFWidget(QWidget* const parent, DInfoInterface* const iface, const QString& toolName)
    : WSSettingsWidget(parent, iface, toolName),
      d(new Private)
{
    QGroupBox* const optionsBox         = getOptionsBox();
    QGridLayout* const optionsBoxLayout = getOptionsBoxLayout();

    QSpacerItem* const spacer1 = new QSpacerItem(1, 10, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QSpacerItem* const spacer2 = new QSpacerItem(1, 10, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QLabel* const policyLabel  = new QLabel(i18n("Update policy:"), optionsBox);

    QRadioButton* const policyRadio1  = new QRadioButton(i18n("Update metadata"), optionsBox);
    policyRadio1->setWhatsThis(i18n("Update metadata of remote file and merge remote tags with local"));

    QRadioButton* const policyRadio3  = new QRadioButton(i18n("Skip photo"), optionsBox);
    policyRadio3->setWhatsThis(i18n("Simple skip photo"));
    QRadioButton* const policyRadio4  = new QRadioButton(i18n("Upload as new"), optionsBox);
    policyRadio4->setWhatsThis(i18n("Add photo as new"));

    QLabel* const accessLabel = new QLabel(i18n("Privacy settings:"), optionsBox);
    d->accessCombo             = new QComboBox(optionsBox);
    d->accessCombo->addItem(QIcon::fromTheme(QString::fromLatin1("folder")),
                           i18n("Public access"), YFPhoto::ACCESS_PUBLIC);
    d->accessCombo->addItem(QIcon::fromTheme(QString::fromLatin1("folder-red")),
                           i18n("Friends access"), YFPhoto::ACCESS_FRIENDS);
    d->accessCombo->addItem(QIcon::fromTheme(QString::fromLatin1("folder-locked")),
                           i18n("Private access"), YFPhoto::ACCESS_PRIVATE);

    d->hideOriginalCheck    = new QCheckBox(i18n("Hide original photo"), optionsBox);
    d->disableCommentsCheck = new QCheckBox(i18n("Disable comments"), optionsBox);
    d->adultCheck           = new QCheckBox(i18n("Adult content"), optionsBox);

    d->policyGroup          = new QButtonGroup(optionsBox);
    d->policyGroup->addButton(policyRadio1, POLICY_UPDATE_MERGE);
    d->policyGroup->addButton(policyRadio3, POLICY_SKIP);
    d->policyGroup->addButton(policyRadio4, POLICY_ADDNEW);

    optionsBoxLayout->addItem(spacer1,                  3, 0, 1, 5);
    optionsBoxLayout->addWidget(accessLabel,            4, 0, 1, 5);
    optionsBoxLayout->addWidget(d->accessCombo,          5, 1, 1, 4);
    optionsBoxLayout->addWidget(d->adultCheck,           6, 1, 1, 4);
    optionsBoxLayout->addWidget(d->hideOriginalCheck,    7, 1, 1, 4);
    optionsBoxLayout->addWidget(d->disableCommentsCheck, 8, 1, 1, 4);
    optionsBoxLayout->addItem(spacer2,                  9, 0, 1, 5);
    optionsBoxLayout->addWidget(policyLabel,            10, 0, 1, 5);
    optionsBoxLayout->addWidget(policyRadio1,           11, 1, 1, 4);
    optionsBoxLayout->addWidget(policyRadio3,           13, 1, 1, 4);
    optionsBoxLayout->addWidget(policyRadio4,           14, 1, 1, 4);

    getUploadBox()->hide();
    getSizeBox()->hide();
}

YFWidget::~YFWidget()
{
    delete d;
}

void YFWidget::updateLabels(const QString& /*name*/, const QString& /*url*/)
{
}

QComboBox* YFWidget::accessCB() const
{
    return d->accessCombo;
}

QCheckBox* YFWidget::hideOriginalCB() const
{
    return d->hideOriginalCheck;
}
    
QCheckBox* YFWidget::disableCommentsCB() const
{
    return d->disableCommentsCheck;
}
    
QCheckBox* YFWidget::adultCB() const
{
    return d->adultCheck;
}
    
QButtonGroup* YFWidget::policyGB() const
{
    return d->policyGroup;
}
    
} // namespace Digikam
