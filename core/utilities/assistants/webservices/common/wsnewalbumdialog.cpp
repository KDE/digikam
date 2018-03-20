/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-08-01
 * Description : new album creation dialog for remote web service.
 *
 * Copyright (C) 2010 by Jens Mueller <tschenser at gmx dot de>
 * Copyright (C) 2015 by Shourya Singh Gupta <shouryasgupta at gmail dot com>
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

#include "wsnewalbumdialog.h"

// Qt includes

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QDialogButtonBox>
#include <QIcon>
#include <QApplication>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

class WSNewAlbumDialog::Private
{
public:

    explicit Private(QWidget* const widget, const QString& name)
    {
        titleEdt       = new QLineEdit;
        descEdt        = new QTextEdit;
        locEdt         = new QLineEdit;
        dtEdt          = new QDateTimeEdit(QDateTime::currentDateTime());

        mainWidget     = widget;
        mainLayout     = new QVBoxLayout(mainWidget);

        albumBox       = new QGroupBox(i18n("Album"), mainWidget);
        albumBoxLayout = new QGridLayout(albumBox);

        titleLabel     = new QLabel(i18n("Title: "), albumBox);
        dateLabel      = new QLabel(i18n("Time Stamp: "), albumBox);
        descLabel      = new QLabel(i18n("Description: "), albumBox);
        locLabel       = new QLabel(i18n("Location: "), albumBox);

        buttonBox      = new QDialogButtonBox();

        toolName       = name;
    }

    QLineEdit*         titleEdt;
    QTextEdit*         descEdt;
    QLineEdit*         locEdt;
    QDateTimeEdit*     dtEdt;

    QLabel*            titleLabel;
    QLabel*            dateLabel;
    QLabel*            descLabel;
    QLabel*            locLabel;

    QString            toolName;
    QDialogButtonBox*  buttonBox;

    QGridLayout*       albumBoxLayout;
    QGroupBox*         albumBox;

    QVBoxLayout*       mainLayout;
    QWidget*           mainWidget;
};

WSNewAlbumDialog::WSNewAlbumDialog(QWidget* const parent, const QString& toolName)
    : QDialog(parent),
      d(new Private(this, toolName))
{
    d->mainWidget->setMinimumSize(500, 500);
    setWindowTitle(QString(d->toolName + QString::fromLatin1(" New Album")));
    setModal(false);

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    d->buttonBox->addButton(QDialogButtonBox::Ok);
    d->buttonBox->addButton(QDialogButtonBox::Cancel);
    d->buttonBox->button(QDialogButtonBox::Cancel)->setDefault(true);
    d->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    connect(d->titleEdt, SIGNAL(textChanged(QString)),
            this, SLOT(slotTextChanged(QString)));

    connect(d->buttonBox, SIGNAL(accepted()),
            this, SLOT(accept()));

    connect(d->buttonBox, SIGNAL(rejected()),
            this, SLOT(reject()));

    d->albumBox->setLayout(d->albumBoxLayout);
    d->albumBox->setWhatsThis(i18n("These are basic settings for the new %1 album.",d->toolName));

    d->titleEdt->setToolTip(i18n("Title of the album that will be created (required)."));

    d->dtEdt->setDisplayFormat(QString::fromLatin1("dd.MM.yyyy HH:mm"));
    d->dtEdt->setWhatsThis(i18n("Date and Time of the album that will be created (optional)."));

    d->descEdt->setToolTip(i18n("Description of the album that will be created (optional)."));

    d->locEdt->setToolTip(i18n("Location of the album that will be created (optional)."));

    d->albumBoxLayout->addWidget(d->titleLabel, 0, 0);
    d->albumBoxLayout->addWidget(d->titleEdt,   0, 1);
    d->albumBoxLayout->addWidget(d->dateLabel,  1, 0);
    d->albumBoxLayout->addWidget(d->dtEdt,      1, 1);
    d->albumBoxLayout->addWidget(d->descLabel,  2, 0);
    d->albumBoxLayout->addWidget(d->descEdt,    2, 1);
    d->albumBoxLayout->addWidget(d->locLabel,   3, 0);
    d->albumBoxLayout->addWidget(d->locEdt,     3, 1);
    d->albumBoxLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    d->albumBoxLayout->setSpacing(spacing);

    d->mainLayout->addWidget(d->albumBox);
    d->mainLayout->addWidget(d->buttonBox);
    d->mainLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    d->mainLayout->setSpacing(spacing);
    setLayout(d->mainLayout);
}

WSNewAlbumDialog::~WSNewAlbumDialog()
{
    delete d;
}

void WSNewAlbumDialog::slotTextChanged(const QString& /*text*/)
{
    if (QString::compare(getTitleEdit()->text(), QLatin1String(""), Qt::CaseInsensitive) == 0)
        d->buttonBox->button( QDialogButtonBox::Ok )->setEnabled(false);
    else
        d->buttonBox->button( QDialogButtonBox::Ok )->setEnabled(true);
}

void WSNewAlbumDialog::hideDateTime()
{
    d->dtEdt->hide();
    d->dateLabel->hide();
    d->albumBoxLayout->removeWidget(d->dtEdt);
    d->albumBoxLayout->removeWidget(d->dateLabel);
}

void WSNewAlbumDialog::hideDesc()
{
    d->descEdt->hide();
    d->descLabel->hide();
    d->albumBoxLayout->removeWidget(d->descEdt);
    d->albumBoxLayout->removeWidget(d->descLabel);
}

void WSNewAlbumDialog::hideLocation()
{
    d->locEdt->hide();
    d->locLabel->hide();
    d->albumBoxLayout->removeWidget(d->locEdt);
    d->albumBoxLayout->removeWidget(d->locLabel);
}

QWidget* WSNewAlbumDialog::getMainWidget() const
{
    return d->mainWidget;
}

QGroupBox* WSNewAlbumDialog::getAlbumBox() const
{
    return d->albumBox;
}

QLineEdit* WSNewAlbumDialog::getTitleEdit() const
{
    return d->titleEdt;
}

QTextEdit* WSNewAlbumDialog::getDescEdit() const
{
    return d->descEdt;
}

QLineEdit* WSNewAlbumDialog::getLocEdit() const
{
    return d->locEdt;
}

QDateTimeEdit* WSNewAlbumDialog::getDateTimeEdit() const
{
    return d->dtEdt;
}

void WSNewAlbumDialog::addToMainLayout(QWidget* const widget)
{
    d->mainLayout->addWidget(widget);
    d->mainLayout->removeWidget(d->buttonBox);
    d->mainLayout->addWidget(d->buttonBox);
}

QDialogButtonBox* WSNewAlbumDialog::getButtonBox() const
{
    return d->buttonBox;
}

} // namespace Digikam
