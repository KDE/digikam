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

#include "htmlparameterspage.h"

// Qt includes

#include <QIcon>
#include <QApplication>
#include <QStyle>
#include <QSpacerItem>
#include <QGridLayout>
#include <QScrollArea>
#include <QByteArray>
#include <QMap>
#include <QLabel>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "htmlwizard.h"
#include "galleryinfo.h"
#include "abstractthemeparameter.h"
#include "dlayoutbox.h"
#include "gallerytheme.h"

namespace Digikam
{

class HTMLParametersPage::Private
{
public:

    explicit Private()
      : content(0)
    {
    }

    QMap<QByteArray, QWidget*> themePrmWdgtList;
    QWidget*                   content;
};

HTMLParametersPage::HTMLParametersPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private)
{
    setObjectName(QLatin1String("ThemeParametersPage"));

    DVBox* const vbox        = new DVBox(this);

    QLabel* const textLabel1 = new QLabel(vbox);
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

    QScrollArea* const mScrollArea = new QScrollArea(vbox);
    mScrollArea->setObjectName(QLatin1String("mScrollArea"));
    mScrollArea->setFrameShape(QFrame::NoFrame);
    mScrollArea->setWidgetResizable(true);

    d->content = new QWidget();
    d->content->setObjectName(QLatin1String("d->content"));
    d->content->setGeometry(QRect(0, 0, 600, 430));
    mScrollArea->setWidget(d->content);

    vbox->setContentsMargins(QMargins());
    vbox->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("text-css")));
}

HTMLParametersPage::~HTMLParametersPage()
{
    delete d;
}

QWidget* HTMLParametersPage::themeParameterWidgetFromName(const QByteArray& name) const
{
    return d->themePrmWdgtList[name];
}

void HTMLParametersPage::initializePage()
{
    HTMLWizard* const wizard = dynamic_cast<HTMLWizard*>(assistant());

    if (!wizard)
        return;

    GalleryInfo* const info  = wizard->galleryInfo();
    GalleryTheme::Ptr theme  = wizard->galleryTheme();

    qDeleteAll(d->content->children());
    d->themePrmWdgtList.clear();

    // Create layout. We need to recreate it every time, to get rid of
    // spacers
    QGridLayout* const layout = new QGridLayout(d->content);
    layout->setContentsMargins(QMargins());
    layout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // Create widgets
    GalleryTheme::ParameterList parameterList      = theme->parameterList();
    QString themeInternalName                      = theme->internalName();
    GalleryTheme::ParameterList::ConstIterator it  = parameterList.constBegin();
    GalleryTheme::ParameterList::ConstIterator end = parameterList.constEnd();

    for (; it != end ; ++it)
    {
        AbstractThemeParameter* const themeParameter = *it;
        QByteArray internalName                      = themeParameter->internalName();
        QString value                                = info->getThemeParameterValue(themeInternalName,
                                                            QString::fromLatin1(internalName),
                                                            themeParameter->defaultValue());

        QString name          = themeParameter->name();
        name                  = i18nc("'%1' is a label for a theme parameter", "%1:", name);

        QLabel* const label   = new QLabel(name, d->content);
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        QWidget* const widget = themeParameter->createWidget(d->content, value);
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

        d->themePrmWdgtList[internalName] = widget;
    }

    // Add spacer at the end, so that widgets aren't spread on the whole parent height

    QSpacerItem* const spacer = new QSpacerItem(1, 1, QSizePolicy::Minimum,
                                                QSizePolicy::Expanding);

    layout->addItem(spacer, layout->rowCount(), 0);
}

} // namespace Digikam
