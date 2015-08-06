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
#include <QGroupBox>

#include <KLocalizedString>

#include "dxmlguiwindow.h"


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
    QGroupBox* ratingMappings;


    QSpinBox* zeroStars;
    QSpinBox* oneStar;
    QSpinBox* twoStars;
    QSpinBox* threeStars;
    QSpinBox* fourStars;
    QSpinBox* fiveStars;

    // Labels
    QLabel*    tagTipLabel;
    QLabel*    ratingTipLabel;
    QLabel*    commentTipLabel;
    QLabel*    subspaceLabel;
    QLabel*    titleLabel;
    QLabel*    specialOptsLabel;
    QLabel*    alternativeNameLabel;
    QLabel*    altspecialOptsLabel;
    QLabel*    isTagLabel;
    QLabel*    separatorLabel;

//    QLabel*    tipLabel2;
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

    if(!d->create)
    {
        populateFields(entry);
    }

    setType(entry.nsType);
    qDebug() << "Entry type" << entry.nsType << "subspace" << entry.subspace;
    adjustSize();
}

NamespaceEditDlg::~NamespaceEditDlg()
{

}

bool NamespaceEditDlg::create(QWidget *parent, NamespaceEntry& entry)
{
    QPointer<NamespaceEditDlg> dlg = new NamespaceEditDlg(true,entry,parent);

    qDebug() << "Name before save: " << entry.namespaceName;
    bool valRet = dlg->exec();

    if (valRet == QDialog::Accepted)
    {
        dlg->saveData(entry);
    }
    qDebug() << "Name after save: " << entry.namespaceName;
    delete dlg;
    return valRet;
}

bool NamespaceEditDlg::edit(QWidget *parent, NamespaceEntry &entry)
{
    QPointer<NamespaceEditDlg> dlg = new NamespaceEditDlg(false, entry, parent);

    qDebug() << "Name before save: " << entry.namespaceName;
    bool valRet = dlg->exec();

    if (valRet == QDialog::Accepted)
    {
        dlg->saveData(entry);
    }

    qDebug() << "Name before save: " << entry.namespaceName;
    delete dlg;
    return valRet;
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
    d->subspaceLabel = new QLabel(d->page);
    d->subspaceLabel->setText(i18n("Metadata Subspace"));

    d->subspaceCombo->addItem(QLatin1String("EXIV"),(int)NamespaceEntry::EXIV);
    d->subspaceCombo->addItem(QLatin1String("IPTC"),(int)NamespaceEntry::IPTC);
    d->subspaceCombo->addItem(QLatin1String("XMP"),(int)NamespaceEntry::XMP);
    d->subspaceCombo->setCurrentIndex((int)entry.subspace);

    qDebug() << "Enrty subspace" << (int)entry.subspace;

    // -------------------Tag Elements---------------------------------

    d->titleLabel = new QLabel(d->page);
    d->titleLabel->setText(i18n("Name:"));

    d->namespaceName = new QLineEdit(this);


    //----------------- Tip Labels --------------------------------------
    d->tagTipLabel   = new QLabel(d->page);
    d->tagTipLabel->setTextFormat(Qt::RichText);
    d->tagTipLabel->setWordWrap(true);
    d->tagTipLabel->setText(i18n("<p>To create new namespaces, you need to specify paramters:</p>"
                           "<p><ul><li>Namespace name with dots.<br/>"
                           "Ex.: <i>\"Xmp.digiKam.TagsList\"</i></li>"
                           "<li>Separator parameter, used by tag paths <br/>"
                           "Ex.: \"City/Paris\" or \"City|Paris\"</li>"
                           "<li>Specify if only keyword or the whole path must be written.</li></ul></p>"
                          ));

    d->ratingTipLabel   = new QLabel(d->page);
    d->ratingTipLabel->setTextFormat(Qt::RichText);
    d->ratingTipLabel->setWordWrap(true);
    d->ratingTipLabel->setText(i18n("<p>To create new rating namespaces, you need to specify paramters:</p>"
                           "<p><ul><li>Namespace name with dots.<br/>"
                           "Ex.: <i>\"Xmp.xmp.Rating\"</i></li>"
                           "<li>Rating mappings, if namespace need other values than 0-5 <br/>"
                           "Ex.: Microsoft uses 0 1 25 50 75 99</li>"
                           "<li>Select the correct namespace option from list.</li></ul></p>"
                          ));

    d->commentTipLabel   = new QLabel(d->page);
    d->commentTipLabel->setTextFormat(Qt::RichText);
    d->commentTipLabel->setWordWrap(true);
    d->commentTipLabel->setText(i18n("<p>To create new comment namespaces, you need to specify paramters:</p>"
                           "<p><ul><li>Namespace name with dots.<br/>"
                           "Ex.: <i>\"Xmp.xmp.Rating\"</i></li>"
                           "<li>Select the correct namespace option from list.</li></ul></p>"
                          ));
    // -------------------------------------------------------
    d->specialOptsLabel = new QLabel(d->page);
    d->specialOptsLabel->setText(i18n("Special Options"));
    d->specialOptsCombo = new QComboBox(d->page);
    d->specialOptsCombo->addItem(QLatin1String("NO_OPTS"), (int)NamespaceEntry::NO_OPTS);
    d->specialOptsCombo->addItem(QLatin1String("COMMENT_ALTLANG"), NamespaceEntry::COMMENT_ALTLANG);
    d->specialOptsCombo->addItem(QLatin1String("COMMENT_ALTLANGLIST"), NamespaceEntry::COMMENT_ATLLANGLIST);
    d->specialOptsCombo->addItem(QLatin1String("COMMENT_XMP"), NamespaceEntry::COMMENT_XMP);
    d->specialOptsCombo->addItem(QLatin1String("TAG_XMPBAG"), NamespaceEntry::TAG_XMPBAG);
    d->specialOptsCombo->addItem(QLatin1String("TAG_XMPSEQ"), NamespaceEntry::TAG_XMPSEQ);

    d->alternativeNameLabel = new QLabel(d->page);
    d->alternativeNameLabel->setText(i18n("Alternative name"));
    d->alternativeName = new QLineEdit(d->page);

    d->altspecialOptsLabel = new QLabel(d->page);
    d->altspecialOptsLabel->setText(i18n("Alternative special options"));
    d->altSpecialOptsCombo = new QComboBox(d->page);
    d->altSpecialOptsCombo->addItem(QLatin1String("NO_OPTS"), (int)NamespaceEntry::NO_OPTS);
    d->altSpecialOptsCombo->addItem(QLatin1String("COMMENT_ALTLANG"), NamespaceEntry::COMMENT_ALTLANG);
    d->altSpecialOptsCombo->addItem(QLatin1String("COMMENT_ALTLANGLIST"), NamespaceEntry::COMMENT_ATLLANGLIST);
    d->altSpecialOptsCombo->addItem(QLatin1String("COMMENT_XMP"), NamespaceEntry::COMMENT_XMP);
    d->altSpecialOptsCombo->addItem(QLatin1String("TAG_XMPBAG"), NamespaceEntry::TAG_XMPBAG);
    d->altSpecialOptsCombo->addItem(QLatin1String("TAG_XMPSEQ"), NamespaceEntry::TAG_XMPSEQ);
    // --------------------------------------------------------

    d->separatorLabel = new QLabel(d->page);
    d->separatorLabel->setText(i18n("Separator:"));

    d->nameSpaceSeparator = new QLineEdit(this);

    // --------------------------------------------------------

    d->isTagLabel = new QLabel(d->page);
    d->isTagLabel->setText(i18n("Set Tags Path:"));

    d->isPath = new QCheckBox(this);


//    d->tipLabel2 = new QLabel(d->page);
//    d->tipLabel2->setTextFormat(Qt::RichText);
//    d->tipLabel2->setWordWrap(true);
//    d->tipLabel2->setText(i18n("<p><b>Note</b>: Extra xml field can be used for namespaces with non standard wrapping.</p>"));

    // ----------------------Rating Elements----------------------------------
    d->ratingMappings = new QGroupBox(this);
    d->ratingMappings->setFlat(true);

    QGridLayout* ratingMappingsLayout = new QGridLayout(d->ratingMappings);

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

    ratingMappingsLayout->addWidget(ratingLabel,            0, 0, 1, 2);

    ratingMappingsLayout->addWidget(d->zeroStars,           1, 0, 1, 1);
    ratingMappingsLayout->addWidget(d->oneStar,             1, 1, 1, 1);
    ratingMappingsLayout->addWidget(d->twoStars,            1, 2, 1, 1);
    ratingMappingsLayout->addWidget(d->threeStars,          1, 3, 1, 1);
    ratingMappingsLayout->addWidget(d->fourStars,           1, 4, 1, 1);
    ratingMappingsLayout->addWidget(d->fiveStars,           1, 5, 1, 1);

    d->gridLayout->addWidget(d->logo,                0, 0, 1, 2);
    d->gridLayout->addWidget(d->topLabel,            0, 1, 1, 4);
    d->gridLayout->addWidget(d->tagTipLabel,            1, 0, 1, 6);
    d->gridLayout->addWidget(d->ratingTipLabel,         2, 0, 1, 6);
    d->gridLayout->addWidget(d->commentTipLabel,        3, 0, 1, 6);

    d->gridLayout->addWidget(d->subspaceLabel,          5, 0, 1, 2);
    d->gridLayout->addWidget(d->subspaceCombo,       5, 2, 1, 4);

    d->gridLayout->addWidget(d->titleLabel,             6, 0, 1, 2);
    d->gridLayout->addWidget(d->namespaceName,       6, 2, 1, 4);
    d->gridLayout->addWidget(d->specialOptsLabel,       7, 0, 1, 2);
    d->gridLayout->addWidget(d->specialOptsCombo,    7, 2, 1, 4);

    d->gridLayout->addWidget(d->alternativeNameLabel,   8, 0, 1, 2);
    d->gridLayout->addWidget(d->alternativeName,     8, 2, 1, 4);
    d->gridLayout->addWidget(d->altspecialOptsLabel,    9, 0, 1, 3);
    d->gridLayout->addWidget(d->altSpecialOptsCombo, 9, 3, 1, 3);

    d->gridLayout->addWidget(d->separatorLabel,         10, 0, 1, 2);
    d->gridLayout->addWidget(d->nameSpaceSeparator,  10, 2, 1, 4);
    d->gridLayout->addWidget(d->isTagLabel,           11, 0, 1, 2);
    d->gridLayout->addWidget(d->isPath,              11, 2, 1, 3);
//    d->gridLayout->addWidget(d->tipLabel2,              13, 0, 1, 4);

    d->gridLayout->addWidget(d->ratingMappings,      14, 0, 2, 6);

    d->gridLayout->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultChildMargin));
    d->gridLayout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));


    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(d->page);
    vbx->addWidget(d->buttons);
}

void NamespaceEditDlg::populateFields(NamespaceEntry &entry)
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
    d->specialOptsCombo->setCurrentIndex((int)entry.specialOpts);

    d->alternativeName->setText(entry.alternativeName);
    d->altSpecialOptsCombo->setCurrentIndex((int)entry.secondNameOpts);

    if(entry.convertRatio.size() == 6)
    {
        d->zeroStars->setValue(entry.convertRatio.at(0));
        d->oneStar->setValue(entry.convertRatio.at(1));
        d->twoStars->setValue(entry.convertRatio.at(2));
        d->threeStars->setValue(entry.convertRatio.at(3));
        d->fourStars->setValue(entry.convertRatio.at(4));
        d->fiveStars->setValue(entry.convertRatio.at(5));
    }
}

void NamespaceEditDlg::setType(NamespaceEntry::NamespaceType type)
{
    switch(type)
    {
    case NamespaceEntry::TAGS:
        qDebug() << "Setting up tags";
        d->ratingTipLabel->hide();
        d->commentTipLabel->hide();
        d->ratingMappings->hide();
        break;
    case NamespaceEntry::RATING:
        d->tagTipLabel->hide();
        d->commentTipLabel->hide();
        d->isPath->hide();
        d->isTagLabel->hide();
//        d->tipLabel2->hide();
        d->separatorLabel->hide();
        d->nameSpaceSeparator->hide();
        break;
    case NamespaceEntry::COMMENT:
        d->tagTipLabel->hide();
        d->ratingTipLabel->hide();
        d->isPath->hide();
        d->isTagLabel->hide();
//        d->tipLabel2->hide();
        d->separatorLabel->hide();
        d->nameSpaceSeparator->hide();
        d->ratingMappings->hide();
        break;
    }
}

void NamespaceEditDlg::saveData( NamespaceEntry &entry)
{
    entry.namespaceName     = d->namespaceName->text();
    entry.separator   = d->nameSpaceSeparator->text();
    if(d->isPath->isChecked())
        entry.tagPaths      = NamespaceEntry::TAGPATH;
    else
        entry.tagPaths      = NamespaceEntry::TAG;

    entry.alternativeName = d->alternativeName->text();
    entry.specialOpts    = (NamespaceEntry::SpecialOptions)d->specialOptsCombo->currentData().toInt();
    entry.secondNameOpts = (NamespaceEntry::SpecialOptions)d->altSpecialOptsCombo->currentData().toInt();
    entry.subspace       = (NamespaceEntry::NsSubspace)d->subspaceCombo->currentData().toInt();

    entry.convertRatio.clear();
    entry.convertRatio.append(d->zeroStars->value());
    entry.convertRatio.append(d->oneStar->value());
    entry.convertRatio.append(d->twoStars->value());
    entry.convertRatio.append(d->threeStars->value());
    entry.convertRatio.append(d->fourStars->value());
    entry.convertRatio.append(d->fiveStars->value());
}

void NamespaceEditDlg::slotHelp()
{
    DXmlGuiWindow::openHandbook(QLatin1String("namespacesadd.anchor"), QLatin1String("digikam"));
}

}
