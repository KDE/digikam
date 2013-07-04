/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 20013-07-03
 * Description : Tag Properties widget to display tag properties
 *               when a tag or multiple tags are selected
 *
 * Copyright (C) 2013 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

#include "tagpropwidget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <kkeysequencewidget.h>
#include <kseparator.h>
#include <kicon.h>
#include <kstandarddirs.h>
#include "searchtextbar.h"

namespace Digikam
{

class TagPropWidget::PrivateTagProp
{
public:

    PrivateTagProp()
    {
        titleEdit       = 0;
        iconButton      = 0;
        resetIconButton = 0;
        topLabel        = 0;
        keySeqWidget    = 0;
        create          = false;
    }

    bool                create;

    QLabel*             topLabel;

    QString             icon;

    QPushButton*        iconButton;
    QPushButton*        resetIconButton;

    KKeySequenceWidget* keySeqWidget;
    SearchTextBar*      titleEdit;
};

TagPropWidget::TagPropWidget(QWidget* const parent)
    : QWidget(parent), d(new PrivateTagProp())
{

    QGridLayout* const grid = new QGridLayout(this);
    QLabel* const logo      = new QLabel(this);
    logo->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                    .scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    d->topLabel = new QLabel(this);
    d->topLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->topLabel->setWordWrap(false);

    KSeparator* const line = new KSeparator(Qt::Horizontal, this);

    // --------------------------------------------------------

    QLabel* const titleLabel = new QLabel(this);
    titleLabel->setText(i18n("&Title:"));

    d->titleEdit = new SearchTextBar(this, "TagEditDlgTitleEdit", i18n("Enter tag name here..."));
    d->titleEdit->setCaseSensitive(false);
    titleLabel->setBuddy(d->titleEdit);

    QLabel* const tipLabel = new QLabel(this);
    tipLabel->setTextFormat(Qt::RichText);
    tipLabel->setWordWrap(true);

    QLabel* const iconTextLabel = new QLabel(this);
    iconTextLabel->setText(i18n("&Icon:"));

    d->iconButton         = new QPushButton(this);
    d->iconButton->setFixedSize(40, 40);
    iconTextLabel->setBuddy(d->iconButton);


    d->resetIconButton = new QPushButton(KIcon("view-refresh"), i18n("Reset"), this);


    QLabel* const kscTextLabel = new QLabel(this);
    kscTextLabel->setText(i18n("&Shortcut:"));

    d->keySeqWidget      = new KKeySequenceWidget(this);
    kscTextLabel->setBuddy(d->keySeqWidget);

    QLabel* const tipLabel2 = new QLabel(this);
    tipLabel2->setTextFormat(Qt::RichText);
    tipLabel2->setWordWrap(true);
    tipLabel2->setText(i18n("<p><b>Note</b>: this shortcut can be used to assign or unassign tag to items.</p>"));

    // --------------------------------------------------------

    grid->addWidget(logo,               0, 0, 1, 1);
    grid->addWidget(d->topLabel,        0, 1, 1, 4);
    grid->addWidget(line,               1, 0, 1, 4);
    grid->addWidget(tipLabel,           2, 0, 1, 4);
    grid->addWidget(titleLabel,         3, 0, 1, 1);
    grid->addWidget(d->titleEdit,       3, 1, 1, 3);
    grid->addWidget(iconTextLabel,      4, 0, 1, 1);
    grid->addWidget(d->iconButton,      4, 1, 1, 1);
    grid->addWidget(d->resetIconButton, 4, 2, 1, 1);
    grid->addWidget(kscTextLabel,       5, 0, 1, 1);
    grid->addWidget(d->keySeqWidget,    5, 1, 1, 3);
    grid->addWidget(tipLabel2,          6, 0, 1, 4);
    grid->setRowStretch(7, 10);
    grid->setColumnStretch(3, 10);
    grid->setMargin(0);

    adjustSize();
}

}
