/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-12-16
 * Description : Import filters configuration dialog
 *
 * Copyright (C) 2010 by Petri Damstén <petri.damsten@iki.fi>
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

#include "importfilters.moc"

// KDE includes

#include <KComboBox>
#include <KLocale>
#include <KDebug>
#include <KSqueezedTextLabel>

// Qt includes

#include <QtGui/QCheckBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QLabel>
#include <QtGui/QToolButton>
#include <QtGui/QPainter>

// Local includes

#include "importfilters.h"
#include "filtercombo.h"

namespace Digikam
{

// ----------------------------------------------------------------------------------------

ImportFilters::ImportFilters(QWidget* parent) 
    : KDialog(parent) 
{
    setButtons(KDialog::Cancel | KDialog::Ok);
    setDefaultButton(KDialog::Ok);
    resize(QSize(400, 200));
    setWindowTitle(i18n("Edit Import Filters"));

    QVBoxLayout* verticalLayout   = new QVBoxLayout(this->mainWidget());
    QHBoxLayout* horizontalLayout = 0;
    QSpacerItem* spacer           = 0;
    QLabel* label                 = 0;

    label            = new QLabel(this);
    label->setText(i18n("Name:"));
    verticalLayout->addWidget(label);
    filterName       = new QLineEdit(this);
    verticalLayout->addWidget(filterName);

    mimeCheckBox     = new QCheckBox(this);
    mimeCheckBox->setText(i18n("Mime filter:"));
    verticalLayout->addWidget(mimeCheckBox);
    horizontalLayout = new QHBoxLayout();
    spacer           = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
    horizontalLayout->addItem(spacer);
    mimeLabel        = new KSqueezedTextLabel(this);
    horizontalLayout->addWidget(mimeLabel);
    mimeButton       = new QToolButton(this);
    mimeButton->setText("...");
    horizontalLayout->addWidget(mimeButton);
    verticalLayout->addLayout(horizontalLayout);

    fileNameCheckBox = new QCheckBox(this);
    fileNameCheckBox->setText(i18n("File name filter:"));
    verticalLayout->addWidget(fileNameCheckBox);
    horizontalLayout = new QHBoxLayout();
    spacer           = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
    horizontalLayout->addItem(spacer);
    fileNameEdit     = new QLineEdit(this);
    horizontalLayout->addWidget(fileNameEdit);
    verticalLayout->addLayout(horizontalLayout);

    pathCheckBox     = new QCheckBox(this);
    pathCheckBox->setText(i18n("Path filter:"));
    verticalLayout->addWidget(pathCheckBox);
    horizontalLayout = new QHBoxLayout();
    spacer           = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
    horizontalLayout->addItem(spacer);
    pathEdit         = new QLineEdit(this);
    horizontalLayout->addWidget(pathEdit);
    verticalLayout->addLayout(horizontalLayout);

    newFilesCheckBox = new QCheckBox(this);
    newFilesCheckBox->setText(i18n("Show only new files"));
    verticalLayout->addWidget(newFilesCheckBox);
    spacer           = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    verticalLayout->addItem(spacer);

    connect(mimeCheckBox, SIGNAL(clicked(bool)), 
            mimeButton, SLOT(setEnabled(bool)));

    connect(mimeButton, SIGNAL(clicked(bool)),
            this, SLOT(mimeButtonClicked()));

    connect(fileNameCheckBox, SIGNAL(clicked(bool)),
            fileNameEdit, SLOT(setEnabled(bool)));

    connect(pathCheckBox, SIGNAL(clicked(bool)),
            pathEdit, SLOT(setEnabled(bool)));

    connect(mimeCheckBox, SIGNAL(clicked(bool)),
            this, SLOT(mimeCheckBoxClicked()));

    connect(fileNameCheckBox, SIGNAL(clicked()),
            this, SLOT(fileNameCheckBoxClicked()));

    connect(pathCheckBox, SIGNAL(clicked()),
            this, SLOT(pathCheckBoxClicked()));
}

ImportFilters::~ImportFilters() 
{
}

void ImportFilters::fileNameCheckBoxClicked()
{
    if (!fileNameCheckBox->isChecked())
    {
        fileNameEdit->clear();
    }
}

void ImportFilters::pathCheckBoxClicked()
{
    if (!pathCheckBox->isChecked())
    {
        pathEdit->clear();
    }
}

void ImportFilters::mimeCheckBoxClicked()
{
    if (!mimeCheckBox->isChecked())
    {
        mimeLabel->clear();
    }
}

void ImportFilters::mimeButtonClicked()
{
    QString text     = i18n("Select the MimeTypes you want for this filter.");
    QStringList list = mimeLabel->text().split(';', QString::SkipEmptyParts);
    KMimeTypeChooserDialog dlg(i18n("Select Mime Types"), text, list, "image", this);

    if (dlg.exec() == KDialog::Accepted)
    {
        mimeLabel->setText(dlg.chooser()->mimeTypes().join(";"));
    }
}

void ImportFilters::setData(const Filter& filter)
{
    filterName->setText(filter.name);
    mimeCheckBox->setChecked(!filter.mimeFilter.isEmpty());
    mimeLabel->setText(filter.mimeFilter);
    mimeButton->setEnabled(!filter.mimeFilter.isEmpty());
    fileNameCheckBox->setChecked(!filter.fileFilter.isEmpty());
    fileNameEdit->setText(filter.fileFilter.join(";"));
    fileNameEdit->setEnabled(!filter.fileFilter.isEmpty());
    pathCheckBox->setChecked(!filter.pathFilter.isEmpty());
    pathEdit->setText(filter.pathFilter.join(";"));
    pathEdit->setEnabled(!filter.pathFilter.isEmpty());
    newFilesCheckBox->setChecked(filter.onlyNew);
}

void ImportFilters::getData(Filter* filter)
{
    filter->name       = filterName->text();
    filter->mimeFilter = mimeLabel->text();
    filter->fileFilter = fileNameEdit->text().split(';', QString::SkipEmptyParts);
    filter->pathFilter = pathEdit->text().split(';', QString::SkipEmptyParts);
    filter->onlyNew    = newFilesCheckBox->isChecked();
}

}  // namespace Digikam
