/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-07-03
 * Description : dialog to edit and create digiKam xmp namespaces
 *
 * Copyright (C) 2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

#include "namespaceeditdlg.h"
#include <QGridLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QDialog>
#include <QPushButton>
#include <QApplication>
#include <QStyle>
#include <QPointer>
#include <QLineEdit>
#include <QCheckBox>
#include <QStandardPaths>
#include <QSpinBox>
#include <QDebug>

#include <KLocalizedString>


namespace Digikam {

class NamespaceEditDlg::Private
{
public:
    Private()
    {

    }
    QDialogButtonBox* buttons;
    bool create;
    QLabel* topLabel;
    QLineEdit* namespaceName;
    QLineEdit* nameSpaceSeparator;
    QCheckBox* isPath;
    QLineEdit* extraXml;

    QSpinBox* zeroStars;
    QSpinBox* oneStar;
    QSpinBox* twoStars;
    QSpinBox* threeStars;
    QSpinBox* fourStars;
    QSpinBox* fiveStars;
};

NamespaceEditDlg::NamespaceEditDlg(bool create, NamespaceEntry &entry, QWidget *parent)
    : QDialog(parent), d(new Private())
{
    setModal(true);

    d->buttons = new QDialogButtonBox(QDialogButtonBox::Help | QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);

    if (create)
    {
        setWindowTitle(i18n("New Xmp Namespace"));
    }
    else
    {
        setWindowTitle(i18n("Edit Xmp Namespace"));
    }

    d->create           = create;

    switch(entry.nsType)
    {
    case NamespaceEntry::TAGS:
        setupTagGui(entry);
        break;
    case NamespaceEntry::RATING:
        setupRatingGui(entry);
        break;
    case NamespaceEntry::COMMENT:
        setupCommentGui(entry);
    }

    // --------------------------------------------------------

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));

    connect(d->buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));

    // --------------------------------------------------------

    adjustSize();
}

NamespaceEditDlg::~NamespaceEditDlg()
{

}

bool NamespaceEditDlg::create(QWidget *parent, NamespaceEntry& entry)
{
    QPointer<NamespaceEditDlg> dlg = new NamespaceEditDlg(true,entry,parent);

    bool valRet = dlg->exec();

    if (valRet == QDialog::Accepted)
    {
        entry.namespaceName     = dlg->namespaceName();
        entry.separator   = dlg->nameSpaceSeparator();
        if(dlg->isTagPath())
            entry.tagPaths      = NamespaceEntry::TAGPATH;
        else
            entry.tagPaths      = NamespaceEntry::TAG;
        entry.extraXml    = dlg->extraXml();
    }

    delete dlg;
    return valRet;
}

bool NamespaceEditDlg::edit(QWidget *parent, NamespaceEntry &entry)
{
    QPointer<NamespaceEditDlg> dlg = new NamespaceEditDlg(false, entry, parent);

    bool valRet = dlg->exec();

    if (valRet == QDialog::Accepted)
    {
        entry.namespaceName     = dlg->namespaceName();
        entry.separator   = dlg->nameSpaceSeparator();
        if(dlg->isTagPath())
            entry.tagPaths      = NamespaceEntry::TAGPATH;
        else
            entry.tagPaths      = NamespaceEntry::TAG;
        entry.extraXml    = dlg->extraXml();
    }

    delete dlg;
    return valRet;
}

QString NamespaceEditDlg::namespaceName() const
{
    return d->namespaceName->text();
}

QString NamespaceEditDlg::nameSpaceSeparator() const
{
    return d->nameSpaceSeparator->text();
}

bool NamespaceEditDlg::isTagPath() const
{
    return d->isPath->isChecked();
}

QString NamespaceEditDlg::extraXml() const
{
    return d->extraXml->text();
}

void NamespaceEditDlg::setupTagGui(NamespaceEntry &entry)
{
    QWidget* const page = new QWidget(this);

    // --------------------------------------------------------

    QGridLayout* const grid = new QGridLayout(page);
    QLabel* const logo      = new QLabel(page);
    logo->setPixmap(QPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/logo-digikam.png")))
                    .scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    d->topLabel             = new QLabel(page);
    d->topLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->topLabel->setWordWrap(false);
    d->topLabel->setText(i18n("Add tag metadata namespace"));


    // --------------------------------------------------------

    QLabel* const titleLabel = new QLabel(page);
    titleLabel->setText(i18n("Name:"));

    d->namespaceName = new QLineEdit(this);


    QLabel* const tipLabel   = new QLabel(page);
    tipLabel->setTextFormat(Qt::RichText);
    tipLabel->setWordWrap(true);
    tipLabel->setText(i18n("<p>To create new namespaces, you need to specify paramters:</p>"
                           "<p><ul><li>Namespace name with dots.<br/>"
                           "Ex.: <i>\"Xmp.digiKam.TagsList\"</i></li>"
                           "<li>Separator parameter, used by tag paths <br/>"
                           "Ex.: \"City/Paris\" or \"City|Paris\"</li>"
                           "<li>Specify if only keyword or the whole path must be written.</li></ul></p>"
                          ));


    // --------------------------------------------------------

    QLabel* const separatorLabel = new QLabel(page);
    separatorLabel->setText(i18n("Separator:"));

    d->nameSpaceSeparator = new QLineEdit(this);

    // --------------------------------------------------------

    QLabel* const kscTextLabel = new QLabel(page);
    kscTextLabel->setText(i18n("Set Tags Path:"));

    d->isPath = new QCheckBox(this);

    QLabel* const extraXmlLabel = new QLabel(page);
    extraXmlLabel->setText(i18n("Extra XML"));
    d->extraXml = new QLineEdit(page);

    QLabel* const tipLabel2 = new QLabel(page);
    tipLabel2->setTextFormat(Qt::RichText);
    tipLabel2->setWordWrap(true);
    tipLabel2->setText(i18n("<p><b>Note</b>: Extra xml field can be used for namespaces with non standard wrapping.</p>"));

    // --------------------------------------------------------

    grid->addWidget(logo,                   0, 0, 1, 1);
    grid->addWidget(d->topLabel,            0, 2, 1, 4);
    grid->addWidget(tipLabel,               2, 0, 1, 6);
    grid->addWidget(titleLabel,             3, 0, 1, 2);
    grid->addWidget(d->namespaceName,       3, 2, 1, 4);
    grid->addWidget(separatorLabel,         4, 0, 1, 2);
    grid->addWidget(d->nameSpaceSeparator,  4, 2, 1, 4);
    grid->addWidget(kscTextLabel,           5, 0, 1, 2);
    grid->addWidget(d->isPath,              5, 2, 1, 3);
    grid->addWidget(extraXmlLabel,          6, 0, 1, 2);
    grid->addWidget(d->extraXml,            6, 2, 1, 4);
    grid->addWidget(tipLabel2,              7, 0, 1, 4);
    grid->setRowStretch(7, 10);
    grid->setColumnStretch(3, 10);
    grid->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultChildMargin));
    grid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    if(!d->create)
    {
        d->namespaceName->setText(entry.namespaceName);
        d->nameSpaceSeparator->setText(entry.separator);
        if(entry.tagPaths == NamespaceEntry::TAGPATH)
        {
            d->isPath->setChecked(true);
        }
        else
        {
            d->isPath->setChecked(false);
        }
        d->extraXml->setText(entry.extraXml);
    }

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(page);
    vbx->addWidget(d->buttons);
}

void NamespaceEditDlg::setupRatingGui(NamespaceEntry &entry)
{
    qDebug() << "Setting up rating page";
    QWidget* const page = new QWidget(this);

    // --------------------------------------------------------

    QGridLayout* const grid = new QGridLayout(page);
    QLabel* const logo      = new QLabel(page);
    logo->setPixmap(QPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/logo-digikam.png")))
                    .scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    d->topLabel             = new QLabel(page);
    d->topLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->topLabel->setWordWrap(false);
    d->topLabel->setText(i18n("Add tag metadata namespace"));


    // --------------------------------------------------------

    QLabel* const titleLabel = new QLabel(page);
    titleLabel->setText(i18n("Name:"));

    d->namespaceName = new QLineEdit(this);


    QLabel* const tipLabel   = new QLabel(page);
    tipLabel->setTextFormat(Qt::RichText);
    tipLabel->setWordWrap(true);
    tipLabel->setText(i18n("<p>To create new namespaces, you need to specify paramters:</p>"
                           "<p><ul><li>Namespace name with dots.<br/>"
                           "Ex.: <i>\"Xmp.digiKam.TagsList\"</i></li>"
                           "<li>Separator parameter, used by tag paths <br/>"
                           "Ex.: \"City/Paris\" or \"City|Paris\"</li>"
                           "<li>Specify if only keyword or the whole path must be written.</li></ul></p>"
                          ));


    // --------------------------------------------------------

    QLabel* const separatorLabel = new QLabel(page);
    separatorLabel->setText(i18n("Separator:"));

    d->nameSpaceSeparator = new QLineEdit(this);

    // --------------------------------------------------------

    QLabel* const kscTextLabel = new QLabel(page);
    kscTextLabel->setText(i18n("Set Tags Path:"));

    d->isPath = new QCheckBox(this);

    QLabel* const ratingLabel = new QLabel(page);
    ratingLabel->setText(i18n("Rating Mapping:"));

    d->zeroStars = new QSpinBox(this);
    d->zeroStars->setValue(0);

    d->oneStar = new QSpinBox(this);
    d->oneStar->setValue(1);

    d->twoStars = new QSpinBox(this);
    d->twoStars->setValue(2);

    d->threeStars = new QSpinBox(this);
    d->threeStars->setValue(3);

    d->fourStars = new QSpinBox(this);
    d->fourStars->setValue(4);

    d->fiveStars = new QSpinBox(this);
    d->fiveStars->setValue(5);

    QLabel* const tipLabel2 = new QLabel(page);
    tipLabel2->setTextFormat(Qt::RichText);
    tipLabel2->setWordWrap(true);
    tipLabel2->setText(i18n("<p><b>Note</b>: Extra xml field can be used for namespaces with non standard wrapping.</p>"));

    // --------------------------------------------------------

    grid->addWidget(logo,                   0, 0, 1, 1);
    grid->addWidget(d->topLabel,            0, 2, 1, 4);
    grid->addWidget(tipLabel,               2, 0, 1, 6);
    grid->addWidget(titleLabel,             3, 0, 1, 2);
    grid->addWidget(d->namespaceName,       3, 2, 1, 4);
    grid->addWidget(separatorLabel,         4, 0, 1, 2);
    grid->addWidget(d->nameSpaceSeparator,  4, 2, 1, 4);
    grid->addWidget(kscTextLabel,           5, 0, 1, 2);
    grid->addWidget(d->isPath,              5, 2, 1, 3);
    grid->addWidget(ratingLabel,            6, 0, 1, 2);

    grid->addWidget(d->zeroStars,           7, 0, 1, 1);
    grid->addWidget(d->oneStar,             7, 1, 1, 1);
    grid->addWidget(d->twoStars,            7, 2, 1, 1);
    grid->addWidget(d->threeStars,          7, 3, 1, 1);
    grid->addWidget(d->fourStars,           7, 4, 1, 1);
    grid->addWidget(d->fiveStars,           7, 5, 1, 1);

    grid->addWidget(tipLabel2,              8, 0, 1, 7);

    grid->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultChildMargin));
    grid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    if(!d->create)
    {
        d->namespaceName->setText(entry.namespaceName);
        d->nameSpaceSeparator->setText(entry.separator);
        if(entry.tagPaths == NamespaceEntry::TAGPATH)
        {
            d->isPath->setChecked(true);
        }
        else
        {
            d->isPath->setChecked(false);
        }
//        d->extraXml->setText(entry.extraXml);
    }

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(page);
    vbx->addWidget(d->buttons);
}

void NamespaceEditDlg::setupCommentGui(NamespaceEntry &entry)
{

}

}
