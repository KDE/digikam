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

#include "newalbumdialog.h"

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

class NewAlbumDialog::Private
{
public:

    Private(QWidget* const widget, const QString& pluginName)
    {
        m_titleEdt     = new QLineEdit;
        m_descEdt      = new QTextEdit;
        m_locEdt       = new QLineEdit;
        m_dtEdt        = new QDateTimeEdit(QDateTime::currentDateTime());

        mainWidget     = widget;
        mainLayout     = new QVBoxLayout(mainWidget);

        albumBox       = new QGroupBox(i18n("Album"), mainWidget);
        albumBoxLayout = new QGridLayout(albumBox);

        titleLabel     = new QLabel(i18n("Title: "), albumBox);
        dateLabel      = new QLabel(i18n("Time Stamp: "), albumBox);
        descLabel      = new QLabel(i18n("Description: "), albumBox);
        locLabel       = new QLabel(i18n("Location: "), albumBox);

        buttonBox      = new QDialogButtonBox();

        m_pluginName   = pluginName;
    }

    QLineEdit*         m_titleEdt;
    QTextEdit*         m_descEdt;
    QLineEdit*         m_locEdt;
    QDateTimeEdit*     m_dtEdt;

    QLabel*            titleLabel;
    QLabel*            dateLabel;
    QLabel*            descLabel;
    QLabel*            locLabel;

    QString            m_pluginName;
    QDialogButtonBox*  buttonBox;

    QGridLayout*       albumBoxLayout;
    QGroupBox*         albumBox;

    QVBoxLayout*       mainLayout;
    QWidget*           mainWidget;
};

NewAlbumDialog::NewAlbumDialog(QWidget* const parent, const QString& pluginName)
    : QDialog(parent),
      d(new Private(this, pluginName))
{
    d->m_pluginName = pluginName;
    d->mainWidget->setMinimumSize(500, 500);
    setWindowTitle(QString(d->m_pluginName + QString::fromLatin1(" New Album")));
    setModal(false);

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    d->buttonBox->addButton(QDialogButtonBox::Ok);
    d->buttonBox->addButton(QDialogButtonBox::Cancel);
    d->buttonBox->button(QDialogButtonBox::Cancel)->setDefault(true);
    d->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    connect(d->m_titleEdt, SIGNAL(textChanged(QString)),
            this, SLOT(slotTextChanged(QString)));

    connect(d->buttonBox, SIGNAL(accepted()),
            this, SLOT(accept()));

    connect(d->buttonBox, SIGNAL(rejected()),
            this, SLOT(reject()));

    d->albumBox->setLayout(d->albumBoxLayout);
    d->albumBox->setWhatsThis(i18n("These are basic settings for the new %1 album.",d->m_pluginName));

    d->m_titleEdt->setToolTip(i18n("Title of the album that will be created (required)."));

    d->m_dtEdt->setDisplayFormat(QString::fromLatin1("dd.MM.yyyy HH:mm"));
    d->m_dtEdt->setWhatsThis(i18n("Date and Time of the album that will be created (optional)."));

    d->m_descEdt->setToolTip(i18n("Description of the album that will be created (optional)."));

    d->m_locEdt->setToolTip(i18n("Location of the album that will be created (optional)."));

    d->albumBoxLayout->addWidget(d->titleLabel, 0, 0);
    d->albumBoxLayout->addWidget(d->m_titleEdt, 0, 1);
    d->albumBoxLayout->addWidget(d->dateLabel,  1, 0);
    d->albumBoxLayout->addWidget(d->m_dtEdt,    1, 1);
    d->albumBoxLayout->addWidget(d->descLabel,  2, 0);
    d->albumBoxLayout->addWidget(d->m_descEdt,  2, 1);
    d->albumBoxLayout->addWidget(d->locLabel,   3, 0);
    d->albumBoxLayout->addWidget(d->m_locEdt,   3, 1);
    d->albumBoxLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    d->albumBoxLayout->setSpacing(spacing);

    d->mainLayout->addWidget(d->albumBox);
    d->mainLayout->addWidget(d->buttonBox);
    d->mainLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    d->mainLayout->setSpacing(spacing);
    setLayout(d->mainLayout);
}

NewAlbumDialog::~NewAlbumDialog()
{
    delete d;
}

void NewAlbumDialog::slotTextChanged(const QString& /*text*/)
{
    if (QString::compare(getTitleEdit()->text(), QLatin1String(""), Qt::CaseInsensitive) == 0)
        d->buttonBox->button( QDialogButtonBox::Ok )->setEnabled(false);
    else
        d->buttonBox->button( QDialogButtonBox::Ok )->setEnabled(true);
}

void NewAlbumDialog::hideDateTime()
{
    d->m_dtEdt->hide();
    d->dateLabel->hide();
    d->albumBoxLayout->removeWidget(d->m_dtEdt);
    d->albumBoxLayout->removeWidget(d->dateLabel);
}

void NewAlbumDialog::hideDesc()
{
    d->m_descEdt->hide();
    d->descLabel->hide();
    d->albumBoxLayout->removeWidget(d->m_descEdt);
    d->albumBoxLayout->removeWidget(d->descLabel);
}

void NewAlbumDialog::hideLocation()
{
    d->m_locEdt->hide();
    d->locLabel->hide();
    d->albumBoxLayout->removeWidget(d->m_locEdt);
    d->albumBoxLayout->removeWidget(d->locLabel);
}

QWidget* NewAlbumDialog::getMainWidget() const
{
    return d->mainWidget;
}

QGroupBox* NewAlbumDialog::getAlbumBox() const
{
    return d->albumBox;
}

QLineEdit* NewAlbumDialog::getTitleEdit() const
{
    return d->m_titleEdt;
}

QTextEdit* NewAlbumDialog::getDescEdit() const
{
    return d->m_descEdt;
}

QLineEdit* NewAlbumDialog::getLocEdit() const
{
    return d->m_locEdt;
}

QDateTimeEdit* NewAlbumDialog::getDateTimeEdit() const
{
    return d->m_dtEdt;
}

void NewAlbumDialog::addToMainLayout(QWidget* const widget)
{
    d->mainLayout->addWidget(widget);
    d->mainLayout->removeWidget(d->buttonBox);
    d->mainLayout->addWidget(d->buttonBox);
}

QDialogButtonBox* NewAlbumDialog::getButtonBox() const
{
    return d->buttonBox;
}

} // namespace Digikam
