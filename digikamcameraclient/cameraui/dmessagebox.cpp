/* ============================================================
 * File  : dmessagebox.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-02-22
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

#include <qlayout.h>
#include <qlabel.h>
#include <qtextedit.h>
#include <qpushbutton.h>
#include <qpixmap.h>
#include <qhbox.h>
#include <qapplication.h>

#include <klocale.h>
#include <kapplication.h>
#include <kiconloader.h>

#include "dmessagebox.h"

DMessageBox* DMessageBox::s_instance = 0;

DMessageBox::DMessageBox()
    : QWidget(0, 0, WShowModal | WStyle_DialogBorder| WDestructiveClose)
{
    setCaption(i18n("Error"));
    
    s_instance = this;
    count_ = 0;

    QGridLayout *grid = new QGridLayout(this, 1, 1, 6, 11);

    // ----------------------------------------------------

    QHBox *hbox = new QHBox(this);
    hbox->setSpacing(5);
    
    QPixmap pix = KApplication::kApplication()->iconLoader()->loadIcon("error",
                                                                       KIcon::NoGroup,
                                                                       KIcon::SizeMedium,
                                                                       KIcon::DefaultState,
                                                                       0, true);
    QLabel *pixLabel = new QLabel(hbox);
    pixLabel->setPixmap(pix);
    pixLabel->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,
                                        QSizePolicy::Minimum));
    
    msgBox_ = new QLabel(hbox);
    msgBox_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
                                       QSizePolicy::Minimum));

    grid->addMultiCellWidget(hbox, 0, 0, 0, 2);
    
    // ---------------------------------------------------

    extraMsgBox_ = new QTextEdit(this);
    extraMsgBox_->setReadOnly(true);
    grid->addMultiCellWidget(extraMsgBox_, 1, 1, 0, 2);

    extraMsgBox_->hide();

    // ---------------------------------------------------

    QPushButton *okButton = new QPushButton(i18n("&OK"), this);
    grid->addWidget(okButton, 2, 1);

    // ---------------------------------------------------

    grid->addItem( new QSpacerItem(5, 10,
                                   QSizePolicy::Expanding,
                                   QSizePolicy::Minimum),
                   2, 0);
    grid->addItem( new QSpacerItem(5, 10,
                                   QSizePolicy::Expanding,
                                   QSizePolicy::Minimum),
                   2, 2);
    // ---------------------------------------------------

    connect(okButton, SIGNAL(clicked()),
            this, SLOT(slotOkClicked()));

    int W=500, H=400;
    move(QApplication::desktop()->width ()/2-(W/2), QApplication::desktop()->height()/2-(H/2));
}

DMessageBox::~DMessageBox()
{
    s_instance = 0;
}

void DMessageBox::appendMsg(const QString& msg)
{
    if (count_ == 0) {
        mainMsg_ = msg;
        msgBox_->setText(msg);
    }
    else {
        QString text(i18n("More errors occurred and are shown below:"));
        msgBox_->setText(text);
        
        extraMsgBox_->append(msg);
        if (extraMsgBox_->isHidden())
            extraMsgBox_->show();
    }
    
    count_++;
}

void DMessageBox::slotOkClicked()
{
    close();    
}

void DMessageBox::showMsg(const QString& msg)
{
    DMessageBox* msgBox = DMessageBox::s_instance;
    if (!msgBox) {
        msgBox = new DMessageBox;
    }

    msgBox->appendMsg(msg);
    if (msgBox->isHidden())
        msgBox->show();
}

