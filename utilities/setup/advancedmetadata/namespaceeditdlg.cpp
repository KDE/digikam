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
#include <QComboBox>

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
    QLabel* logo;
    QGridLayout* gridLayout;
    QWidget*    page;

    // NamespaceEntry variables
    QComboBox* subspaceCombo;
    QComboBox* specialOptsCombo;
    QComboBox* altSpecialOptsCombo;
    QLineEdit* namespaceName;
    QLineEdit* alternativeName;
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

    d->buttons = new QDialogButtonBox(QDialogButtonBox::Help
                                      | QDialogButtonBox::Ok
                                      | QDialogButtonBox::Cancel, this);

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

    setupTagGui(entry);

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
    d->page = new QWidget(this);
    d->gridLayout = new QGridLayout(d->page);
    d->logo      = new QLabel(d->page);
    d->logo->setPixmap(QPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                   QLatin1String("digikam/data/logo-digikam.png")))
                    .scaled(48, 48, Qt::KeepAspectRatio,
                            Qt::SmoothTransformation));

    d->topLabel             = new QLabel(d->page);
    d->topLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->topLabel->setWordWrap(false);
    d->topLabel->setText(i18n("Add metadata namespace"));

    d->subspaceCombo = new QComboBox(this);
    QLabel* const subspaceLabel = new QLabel(d->page);
    subspaceLabel->setText(i18n("Metadata Subspace"));

    d->subspaceCombo->addItem(QLatin1String("EXIV"),(int)NamespaceEntry::EXIV);
    d->subspaceCombo->addItem(QLatin1String("IPTC"),(int)NamespaceEntry::IPTC);
    d->subspaceCombo->addItem(QLatin1String("XMP"),(int)NamespaceEntry::XMP);
    d->subspaceCombo->setCurrentIndex((int)entry.subspace);

    qDebug() << "Enrty subspace" << (int)entry.subspace;

    // -------------------Tag Elements---------------------------------

    QLabel* const titleLabel = new QLabel(d->page);
    titleLabel->setText(i18n("Name:"));

    d->namespaceName = new QLineEdit(this);


    //----------------- Tip Labels --------------------------------------
    QLabel* const tagTipLabel   = new QLabel(d->page);
    tagTipLabel->setTextFormat(Qt::RichText);
    tagTipLabel->setWordWrap(true);
    tagTipLabel->setText(i18n("<p>To create new namespaces, you need to specify paramters:</p>"
                           "<p><ul><li>Namespace name with dots.<br/>"
                           "Ex.: <i>\"Xmp.digiKam.TagsList\"</i></li>"
                           "<li>Separator parameter, used by tag paths <br/>"
                           "Ex.: \"City/Paris\" or \"City|Paris\"</li>"
                           "<li>Specify if only keyword or the whole path must be written.</li></ul></p>"
                          ));

    QLabel* const ratingTipLabel   = new QLabel(d->page);
    ratingTipLabel->setTextFormat(Qt::RichText);
    ratingTipLabel->setWordWrap(true);
    ratingTipLabel->setText(i18n("<p>To create new rating namespaces, you need to specify paramters:</p>"
                           "<p><ul><li>Namespace name with dots.<br/>"
                           "Ex.: <i>\"Xmp.xmp.Rating\"</i></li>"
                           "<li>Rating mappings, if namespace need other values than 0-5 <br/>"
                           "Ex.: Microsoft uses 0 1 25 50 75 99</li>"
                           "<li>Select the correct namespace option from list.</li></ul></p>"
                          ));

    QLabel* const commentTipLabel   = new QLabel(d->page);
    commentTipLabel->setTextFormat(Qt::RichText);
    commentTipLabel->setWordWrap(true);
    commentTipLabel->setText(i18n("<p>To create new comment namespaces, you need to specify paramters:</p>"
                           "<p><ul><li>Namespace name with dots.<br/>"
                           "Ex.: <i>\"Xmp.xmp.Rating\"</i></li>"
                           "<li>Select the correct namespace option from list.</li></ul></p>"
                          ));
    // -------------------------------------------------------
    QLabel* const specialOptsLabel = new QLabel(d->page);
    specialOptsLabel->setText(i18n("Special Options"));
    d->specialOptsCombo = new QComboBox(d->page);
    d->specialOptsCombo->addItem(QLatin1String("NO_OPTS"), (int)NamespaceEntry::NO_OPTS);
    d->specialOptsCombo->addItem(QLatin1String("COMMENT_ALTLANG"), NamespaceEntry::COMMENT_ALTLANG);
    d->specialOptsCombo->addItem(QLatin1String("COMMENT_ALTLANGLIST"), NamespaceEntry::COMMENT_ATLLANGLIST);
    d->specialOptsCombo->addItem(QLatin1String("COMMENT_XMP"), NamespaceEntry::COMMENT_XMP);
    d->specialOptsCombo->addItem(QLatin1String("TAG_XMPBAG"), NamespaceEntry::TAG_XMPBAG);
    d->specialOptsCombo->addItem(QLatin1String("TAG_XMPSEQ"), NamespaceEntry::TAG_XMPSEQ);

    QLabel* alternativeNameLabel = new QLabel(d->page);
    alternativeNameLabel->setText(i18n("Alternative name"));
    d->alternativeName = new QLineEdit(d->page);

    QLabel* altSpecialOptsLabel = new QLabel(d->page);
    altSpecialOptsLabel->setText(i18n("Alternative special options"));
    d->altSpecialOptsCombo = new QComboBox(d->specialOptsCombo);
    // --------------------------------------------------------

    QLabel* const separatorLabel = new QLabel(d->page);
    separatorLabel->setText(i18n("Separator:"));

    d->nameSpaceSeparator = new QLineEdit(this);

    // --------------------------------------------------------

    QLabel* const kscTextLabel = new QLabel(d->page);
    kscTextLabel->setText(i18n("Set Tags Path:"));

    d->isPath = new QCheckBox(this);

    QLabel* const extraXmlLabel = new QLabel(d->page);
    extraXmlLabel->setText(i18n("Extra XML"));
    d->extraXml = new QLineEdit(d->page);

    QLabel* const tipLabel2 = new QLabel(d->page);
    tipLabel2->setTextFormat(Qt::RichText);
    tipLabel2->setWordWrap(true);
    tipLabel2->setText(i18n("<p><b>Note</b>: Extra xml field can be used for namespaces with non standard wrapping.</p>"));

    // ----------------------Rating Elements----------------------------------
    QLabel* const ratingLabel = new QLabel(d->page);
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

    d->gridLayout->addWidget(d->logo,                0, 0, 1, 2);
    d->gridLayout->addWidget(d->topLabel,            0, 1, 1, 4);
    d->gridLayout->addWidget(tagTipLabel,            1, 0, 1, 6);
    d->gridLayout->addWidget(ratingTipLabel,         2, 0, 1, 6);
    d->gridLayout->addWidget(commentTipLabel,        3, 0, 1, 6);

    d->gridLayout->addWidget(subspaceLabel,          5, 0, 1, 2);
    d->gridLayout->addWidget(d->subspaceCombo,       5, 2, 1, 4);

    d->gridLayout->addWidget(titleLabel,             6, 0, 1, 2);
    d->gridLayout->addWidget(d->namespaceName,       6, 2, 1, 4);
    d->gridLayout->addWidget(specialOptsLabel,       7, 0, 1, 2);
    d->gridLayout->addWidget(d->specialOptsCombo,    7, 2, 1, 4);

    d->gridLayout->addWidget(alternativeNameLabel,   8, 0, 1, 2);
    d->gridLayout->addWidget(d->alternativeName,     8, 2, 1, 4);
    d->gridLayout->addWidget(altSpecialOptsLabel,    9, 0, 1, 3);
    d->gridLayout->addWidget(d->altSpecialOptsCombo, 9, 3, 1, 3);

    d->gridLayout->addWidget(separatorLabel,         10, 0, 1, 2);
    d->gridLayout->addWidget(d->nameSpaceSeparator,  10, 2, 1, 4);
    d->gridLayout->addWidget(kscTextLabel,           11, 0, 1, 2);
    d->gridLayout->addWidget(d->isPath,              11, 2, 1, 3);
    d->gridLayout->addWidget(extraXmlLabel,          12, 0, 1, 2);
    d->gridLayout->addWidget(d->extraXml,            12, 2, 1, 4);
    d->gridLayout->addWidget(tipLabel2,              13, 0, 1, 4);

    d->gridLayout->addWidget(ratingLabel,            14, 0, 1, 2);

    d->gridLayout->addWidget(d->zeroStars,           15, 0, 1, 1);
    d->gridLayout->addWidget(d->oneStar,             15, 1, 1, 1);
    d->gridLayout->addWidget(d->twoStars,            15, 2, 1, 1);
    d->gridLayout->addWidget(d->threeStars,          15, 3, 1, 1);
    d->gridLayout->addWidget(d->fourStars,           15, 4, 1, 1);
    d->gridLayout->addWidget(d->fiveStars,           15, 5, 1, 1);


    d->gridLayout->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultChildMargin));
    d->gridLayout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

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
    vbx->addWidget(d->page);
    vbx->addWidget(d->buttons);
}



}
