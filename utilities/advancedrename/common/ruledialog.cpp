/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-01
 * Description : a dialog that can be used to display a configuration
 *               dialog for a rule
 *
 * Copyright (C) 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
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

#include "ruledialog.moc"

// Qt includes

#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QRegExp>
#include <QWidget>

// KDE includes

#include <kiconloader.h>

// Local includes

#include "rule.h"

namespace Digikam
{

class RuleDialog::Private
{
public:

    Private() :
        dialogTitle(0),
        dialogDescription(0),
        dialogIcon(0),
        settingsWidget(0)
    {
    }

    QLabel*  dialogTitle;
    QLabel*  dialogDescription;
    QLabel*  dialogIcon;
    QWidget* settingsWidget;
};

RuleDialog::RuleDialog(Rule* parent)
    : KDialog(0), d(new Private)
{
    d->dialogTitle       = new QLabel(this);
    d->dialogDescription = new QLabel(this);
    d->dialogIcon        = new QLabel(this);

    setDialogTitle(parent->objectName());
    setDialogDescription(parent->description());
    setDialogIcon(parent->icon(Rule::Dialog));

    d->dialogTitle->setAlignment(Qt::AlignHCenter);
    d->dialogDescription->setAlignment(Qt::AlignHCenter);
    d->dialogDescription->setWordWrap(true);

    QFrame* line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    QWidget* header           = new QWidget(this);
    QGridLayout* headerLayout = new QGridLayout(this);
    headerLayout->addWidget(d->dialogIcon,        0, 0, 4, 1);
    headerLayout->addWidget(d->dialogTitle,       1, 1, 1, 1);
    headerLayout->addWidget(d->dialogDescription, 2, 1, 1, 1);
    headerLayout->addWidget(line,                 4, 0, 1, -1);
    headerLayout->setColumnStretch(1, 10);
    header->setLayout(headerLayout);

    QWidget* container           = new QWidget(this);
    QGridLayout* containerLayout = new QGridLayout(this);
    containerLayout->addWidget(header,            0, 0, 1, 1);
    containerLayout->addWidget(d->settingsWidget, 1, 0, 1, 1);
    containerLayout->setRowStretch(1, 10);
    container->setLayout(containerLayout);
    setMainWidget(container);
    setMinimumWidth(300);
}

RuleDialog::~RuleDialog()
{
    delete d;
}

void RuleDialog::setDialogTitle(const QString& title)
{
    // remove ellipsis and "&&" from the string
    QString _title = title;
    _title.remove(QRegExp("\\.{3,}")).replace("&&", "&");

    d->dialogTitle->setText(QString("<b>%1</b>").arg(_title));
    setCaption(_title);
}

void RuleDialog::setDialogDescription(const QString& description)
{
    d->dialogDescription->setText(description);
}

void RuleDialog::setDialogIcon(const QPixmap& icon)
{
    d->dialogIcon->setPixmap(icon);
}

void RuleDialog::setSettingsWidget(QWidget* settingsWidget)
{
    delete d->settingsWidget;

    d->settingsWidget = new QWidget(this);
    QGridLayout* l    = new QGridLayout(this);
    l->addWidget(settingsWidget);
    l->setSpacing(0);
    l->setMargin(0);
    d->settingsWidget->setLayout(l);
    mainWidget()->layout()->addWidget(d->settingsWidget);
}

} // namespace Digikam
