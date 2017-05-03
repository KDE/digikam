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

#include "htmlparameterspage.h"

// Qt includes

#include <QApplication>
#include <QStyle>
#include <QSpacerItem>
#include <QGridLayout>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "galleryinfo.h"
#include "abstractthemeparameter.h"
#include "dwidgetutils.h"

namespace Digikam
{

HTMLParametersPage::HTMLParametersPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title)
{
    setObjectName(QLatin1String("ThemeParametersPage"));

    DVBox* const vb          = new DVBox(this);

    QLabel* const textLabel1 = new QLabel(vb);
    textLabel1->setObjectName(QLatin1String("textLabel1"));
    textLabel1->setText(i18n("In this page, you can change some theme parameters. "
                             "Depending on the theme, different parameters are available."));

    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(textLabel1->sizePolicy().hasHeightForWidth());

    textLabel1->setSizePolicy(sizePolicy);
    textLabel1->setAlignment(Qt::AlignVCenter);
    textLabel1->setWordWrap(true);

    mScrollArea = new QScrollArea(vb);
    mScrollArea->setObjectName(QLatin1String("mScrollArea"));
    mScrollArea->setFrameShape(QFrame::NoFrame);
    mScrollArea->setWidgetResizable(true);

    mContent = new QWidget();
    mContent->setObjectName(QLatin1String("mContent"));
    mContent->setGeometry(QRect(0, 0, 592, 429));
    mScrollArea->setWidget(mContent);

    setPageWidget(vb);
}

HTMLParametersPage::~HTMLParametersPage()
{
}

void HTMLParametersPage::fillThemeParametersPage(Theme::Ptr theme, GalleryInfo* const info)
{
    mThemeParameterWidgetFromName.clear();

    // Create layout. We need to recreate it every time, to get rid of
    // spacers
    QGridLayout* const layout = new QGridLayout(mContent);
    layout->setContentsMargins(QMargins());
    layout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // Create widgets
    Theme::ParameterList parameterList      = theme->parameterList();
    QString themeInternalName               = theme->internalName();
    Theme::ParameterList::ConstIterator it  = parameterList.constBegin();
    Theme::ParameterList::ConstIterator end = parameterList.constEnd();

    for (; it != end ; ++it)
    {
        AbstractThemeParameter* const themeParameter = *it;
        QByteArray internalName                      = themeParameter->internalName();
        QString value                                = info->getThemeParameterValue(themeInternalName,
                                                            QString::fromLatin1(internalName),
                                                            themeParameter->defaultValue());

        QString name          = themeParameter->name();
        name                  = i18nc("'%1' is a label for a theme parameter", "%1:", name);

        QLabel* const label   = new QLabel(name, mContent);
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        QWidget* const widget = themeParameter->createWidget(mContent, value);
        label->setBuddy(widget);

        int row               = layout->rowCount();
        layout->addWidget(label, row, 0);

        if (widget->sizePolicy().expandingDirections() & Qt::Horizontal)
        {
            // Widget wants full width
            layout->addWidget(widget, row, 1, 1, 2);
        }
        else
        {
            // Widget doesn't like to be stretched, add a spacer next to it
            layout->addWidget(widget, row, 1);
            QSpacerItem* const spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding,
                                                        QSizePolicy::Minimum);
            layout->addItem(spacer, row, 2);
        }

        mThemeParameterWidgetFromName[internalName] = widget;
    }

    // Add spacer at the end, so that widgets aren't spread on the whole parent height

    QSpacerItem* const spacer = new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);
    layout->addItem(spacer, layout->rowCount(), 0);
}

} // namespace Digikam
