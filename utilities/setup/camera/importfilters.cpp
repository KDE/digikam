/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-12-16
 * Description : Import filters configuration dialog
 *
 * Copyright (C) 2010-2011 by Petri Damstén <petri dot damsten at iki dot fi>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "importfilters.h"

// Qt includes

#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QToolButton>
#include <QPainter>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>
#include <kmimetypechooser.h>

// Local includes

#include "digikam_debug.h"
#include "filtercombo.h"
#include "dexpanderbox.h"

namespace Digikam
{

class ImportFilters::Private
{
public:

    Private()
    {
        buttons          = 0;
        filterName       = 0;
        mimeCheckBox     = 0;
        mimeLabel        = 0;
        mimeButton       = 0;
        fileNameCheckBox = 0;
        fileNameEdit     = 0;
        pathCheckBox     = 0;
        pathEdit         = 0;
        newFilesCheckBox = 0;
    }

    QDialogButtonBox*   buttons;

    QLineEdit*          filterName;
    QCheckBox*          mimeCheckBox;
    DAdjustableLabel*   mimeLabel;
    QToolButton*        mimeButton;
    QCheckBox*          fileNameCheckBox;
    QLineEdit*          fileNameEdit;
    QCheckBox*          pathCheckBox;
    QLineEdit*          pathEdit;
    QCheckBox*          newFilesCheckBox;
};

// ----------------------------------------------------------------------------------------

ImportFilters::ImportFilters(QWidget* const parent)
    : QDialog(parent),
      d(new Private)
{
    setWindowTitle(i18n("Edit Import Filters"));

    d->buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);

    QWidget* const page = new QWidget(this);
    QVBoxLayout* const verticalLayout   = new QVBoxLayout(page);
    QLabel* label                       = 0;
    QHBoxLayout* horizontalLayout       = 0;
    QSpacerItem* spacer                 = 0;

    label            = new QLabel(page);
    label->setText(i18n("Name:"));
    verticalLayout->addWidget(label);
    d->filterName    = new QLineEdit(page);
    verticalLayout->addWidget(d->filterName);

    d->mimeCheckBox  = new QCheckBox(page);
    d->mimeCheckBox->setText(i18n("Mime filter:"));
    verticalLayout->addWidget(d->mimeCheckBox);
    horizontalLayout = new QHBoxLayout();
    spacer           = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
    horizontalLayout->addItem(spacer);
    d->mimeLabel     = new DAdjustableLabel(page);
    horizontalLayout->addWidget(d->mimeLabel);
    d->mimeButton    = new QToolButton(page);
    d->mimeButton->setText(i18n("Select Type Mime..."));
    horizontalLayout->addWidget(d->mimeButton);
    verticalLayout->addLayout(horizontalLayout);

    d->fileNameCheckBox = new QCheckBox(page);
    d->fileNameCheckBox->setText(i18n("File name filter:"));
    verticalLayout->addWidget(d->fileNameCheckBox);
    horizontalLayout    = new QHBoxLayout();
    spacer              = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
    horizontalLayout->addItem(spacer);
    d->fileNameEdit     = new QLineEdit(page);
    horizontalLayout->addWidget(d->fileNameEdit);
    verticalLayout->addLayout(horizontalLayout);

    d->pathCheckBox  = new QCheckBox(page);
    d->pathCheckBox->setText(i18n("Path filter:"));
    verticalLayout->addWidget(d->pathCheckBox);
    horizontalLayout = new QHBoxLayout();
    spacer           = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
    horizontalLayout->addItem(spacer);
    d->pathEdit      = new QLineEdit(page);
    horizontalLayout->addWidget(d->pathEdit);
    verticalLayout->addLayout(horizontalLayout);

    d->newFilesCheckBox = new QCheckBox(page);
    d->newFilesCheckBox->setText(i18n("Show only new files"));
    verticalLayout->addWidget(d->newFilesCheckBox);
    spacer              = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    verticalLayout->addItem(spacer);

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(page);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));

    connect(d->mimeCheckBox, SIGNAL(clicked(bool)),
            d->mimeButton, SLOT(setEnabled(bool)));

    connect(d->mimeButton, SIGNAL(clicked(bool)),
            this, SLOT(mimeButtonClicked()));

    connect(d->fileNameCheckBox, SIGNAL(clicked(bool)),
            d->fileNameEdit, SLOT(setEnabled(bool)));

    connect(d->pathCheckBox, SIGNAL(clicked(bool)),
            d->pathEdit, SLOT(setEnabled(bool)));

    connect(d->mimeCheckBox, SIGNAL(clicked(bool)),
            this, SLOT(mimeCheckBoxClicked()));

    connect(d->fileNameCheckBox, SIGNAL(clicked()),
            this, SLOT(fileNameCheckBoxClicked()));

    connect(d->pathCheckBox, SIGNAL(clicked()),
            this, SLOT(pathCheckBoxClicked()));

    adjustSize();
}

ImportFilters::~ImportFilters()
{
    delete d;
}

void ImportFilters::fileNameCheckBoxClicked()
{
    if (!d->fileNameCheckBox->isChecked())
    {
        d->fileNameEdit->clear();
    }
}

void ImportFilters::pathCheckBoxClicked()
{
    if (!d->pathCheckBox->isChecked())
    {
        d->pathEdit->clear();
    }
}

void ImportFilters::mimeCheckBoxClicked()
{
    if (!d->mimeCheckBox->isChecked())
    {
        d->mimeLabel->setAdjustedText();
    }
}

void ImportFilters::mimeButtonClicked()
{
    QString text     = i18n("Select the MimeTypes you want for this filter.");
    QStringList list = d->mimeLabel->adjustedText().split(QLatin1Char(';'), QString::SkipEmptyParts);
    KMimeTypeChooserDialog dlg(i18n("Select Mime Types"), text, list, QLatin1String("image"), this);

    if (dlg.exec() == QDialog::Accepted)
    {
        d->mimeLabel->setAdjustedText(dlg.chooser()->mimeTypes().join(QLatin1String(";")));
    }
}

void ImportFilters::setData(const Filter& filter)
{
    d->filterName->setText(filter.name);
    d->mimeCheckBox->setChecked(!filter.mimeFilter.isEmpty());
    d->mimeLabel->setAdjustedText(filter.mimeFilter);
    d->mimeButton->setEnabled(!filter.mimeFilter.isEmpty());
    d->fileNameCheckBox->setChecked(!filter.fileFilter.isEmpty());
    d->fileNameEdit->setText(filter.fileFilter.join(QLatin1String(";")));
    d->fileNameEdit->setEnabled(!filter.fileFilter.isEmpty());
    d->pathCheckBox->setChecked(!filter.pathFilter.isEmpty());
    d->pathEdit->setText(filter.pathFilter.join(QLatin1String(";")));
    d->pathEdit->setEnabled(!filter.pathFilter.isEmpty());
    d->newFilesCheckBox->setChecked(filter.onlyNew);
}

void ImportFilters::getData(Filter* const filter)
{
    filter->name       = d->filterName->text();
    filter->mimeFilter = d->mimeLabel->adjustedText();
    filter->fileFilter = d->fileNameEdit->text().split(QLatin1Char(';'), QString::SkipEmptyParts);
    filter->pathFilter = d->pathEdit->text().split(QLatin1Char(';'), QString::SkipEmptyParts);
    filter->onlyNew    = d->newFilesCheckBox->isChecked();
}

}  // namespace Digikam
