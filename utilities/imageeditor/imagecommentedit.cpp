/* ============================================================
 * File  : imagecommentedit.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-03-09
 * Description :
 *
 * Copyright 2003 by Renchi Raju
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include <klocale.h>

#include <qlabel.h>
#include <qtextedit.h>
#include <qlayout.h>
#include <qgroupbox.h>

#include "imagecommentedit.h"


ImageCommentEdit::ImageCommentEdit(const QString& itemName,
                             const QString& itemComments,
                             QWidget *parent)
    : KDialogBase( Plain, i18n("Image Comments"), Help|Ok|Cancel, Ok,
                   parent, 0, true, true )
{
    setHelp("imagedescedit.anchor", "digikam");
    mItemName = itemName;

    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(),
                                              0, spacingHint() );

    QGroupBox* groupBox = new QGroupBox( plainPage() );
    groupBox->setTitle(i18n("Image Comments"));
    topLayout->addWidget(groupBox);

    groupBox->setColumnLayout(0, Qt::Vertical );
    groupBox->layout()->setSpacing( 5 );
    groupBox->layout()->setMargin( 5 );

    QGridLayout *groupBoxLayout =
        new QGridLayout( groupBox->layout() );
    groupBoxLayout->setAlignment( Qt::AlignTop );

    // ------------------------------------------------

    QLabel* nameL = new QLabel(groupBox);
    nameL->setText(i18n("Name: ") + mItemName);
    groupBoxLayout->addWidget(nameL, 0, 0);

    QLabel* commentsL = new QLabel(groupBox);
    commentsL->setText(i18n("Comments: "));
    groupBoxLayout->addWidget(commentsL, 1, 0);

    mCommentsEdit = new QTextEdit(groupBox);
    groupBoxLayout->addWidget(mCommentsEdit, 2, 0);

    mCommentsEdit->setFocus();

    // -- Init Values ---------------------------------
    enableButtonOK(false);
    mCommentsEdit->setText(itemComments);

    // -- Setup Connections ---------------------------

    connect(mCommentsEdit, SIGNAL(textChanged()),
            this, SLOT(slot_textChanged()));
    
    resize(400, 300);
}

ImageCommentEdit::~ImageCommentEdit()
{
}


void ImageCommentEdit::slot_textChanged()
{
    enableButtonOK(true);
}


bool ImageCommentEdit::editComments(const QString& itemName,
                                 QString& itemComments,
                                 QWidget *parent
                                )
{
    ImageCommentEdit dlg(itemName, itemComments, parent);

    bool ok = dlg.exec() == QDialog::Accepted;
    itemComments = dlg.mCommentsEdit->text();
    return ok;
}

#include "imagecommentedit.moc"
