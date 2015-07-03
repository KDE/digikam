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
};

NamespaceEditDlg::NamespaceEditDlg(bool create, QWidget *parent)
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
    QWidget* const page = new QWidget(this);

    // --------------------------------------------------------

    QGridLayout* const grid = new QGridLayout(page);
    QLabel* const logo      = new QLabel(page);
    logo->setPixmap(QPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/logo-digikam.png")))
                    .scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    d->topLabel             = new QLabel(page);
    d->topLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->topLabel->setWordWrap(false);
    d->topLabel->setText(i18n("Add metadata namespace"));

//    RLineWidget* const line  = new RLineWidget(Qt::Horizontal, page);

    // --------------------------------------------------------

    QLabel* const titleLabel = new QLabel(page);
    titleLabel->setText(i18n("&Name:"));

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
    separatorLabel->setText(i18n("&Separator:"));

    d->nameSpaceSeparator = new QLineEdit(this);

    // --------------------------------------------------------

    QLabel* const kscTextLabel = new QLabel(page);
    kscTextLabel->setText(i18n("&Set Tags Path:"));

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
    grid->addWidget(d->topLabel,            0, 1, 1, 4);
    grid->addWidget(tipLabel,               2, 0, 1, 4);
    grid->addWidget(titleLabel,             3, 0, 1, 1);
    grid->addWidget(d->namespaceName,       3, 1, 1, 3);
    grid->addWidget(separatorLabel,         4, 0, 1, 1);
    grid->addWidget(d->nameSpaceSeparator,  4, 1, 1, 3);
    grid->addWidget(kscTextLabel,           5, 0, 1, 1);
    grid->addWidget(d->isPath,              5, 1, 1, 3);
    grid->addWidget(extraXmlLabel,          6, 0, 1, 1);
    grid->addWidget(d->extraXml,            6, 1, 1, 3);
    grid->addWidget(tipLabel2,              7, 0, 1, 4);
    grid->setRowStretch(7, 10);
    grid->setColumnStretch(3, 10);
    grid->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultChildMargin));
    grid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(page);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

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
    QPointer<NamespaceEditDlg> dlg = new NamespaceEditDlg(true, parent);

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

}
