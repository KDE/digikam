/* ============================================================
 * File  : tagcreatedlg.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-07-01
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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
#include <kiconloader.h>
#include <kicondialog.h>
#include <kapplication.h>

#include <qcombobox.h>
#include <qlabel.h>
#include <qframe.h>
#include <qlayout.h>
#include <qlineedit.h>

#include "album.h"
#include "tagcreatedlg.h"

TagCreateDlg::TagCreateDlg(TAlbum* parent)
    : KDialogBase( Plain, i18n("Create Tag"), Ok|Cancel, Ok,
                   0, 0, true, true )
{
    QVBoxLayout *topLayout = new QVBoxLayout(plainPage(), 0, spacingHint());

    QLabel *topLabel = new QLabel(plainPage());
    topLabel->setText( i18n("Create New Tag in '%1'").arg(parent->getURL()));
    topLayout->addWidget(topLabel);    

    // --------------------------------------------------------

    QFrame *topLine = new QFrame( plainPage() );
    topLine->setFrameShape( QFrame::HLine );
    topLine->setFrameShadow( QFrame::Sunken );
    topLayout->addWidget( topLine );

    // --------------------------------------------------------

    QGridLayout *gl = new QGridLayout(topLayout, spacingHint());

    QLabel *titleLabel = new QLabel(plainPage());
    titleLabel->setText(i18n("Title: "));
    gl->addWidget(titleLabel, 0, 0);

    m_titleEdit = new QLineEdit(plainPage());
    gl->addWidget(m_titleEdit, 0, 1);


    QLabel *iconTextLabel = new QLabel(plainPage());
    iconTextLabel->setText(i18n("Set Icon: "));
    gl->addWidget(iconTextLabel, 1, 0);

    m_iconButton = new QPushButton(plainPage());
    m_iconButton->setFixedSize(40, 40);
    gl->addWidget(m_iconButton, 1, 1);

    connect(m_iconButton, SIGNAL(clicked()),
            SLOT(slotIconChange()));

    // by default assign the icon of the parent (if not root)
    // to this new tag
    if (!parent->isRoot())
    {
        m_icon = parent->getIcon();
        m_iconButton->setIconSet( KApplication::kApplication()->iconLoader()
                                  ->loadIcon(m_icon, KIcon::NoGroup, 32,
                                             KIcon::DefaultState, 0, true));
    }
}

TagCreateDlg::~TagCreateDlg()
{
    
}

QString TagCreateDlg::title() const
{
    return m_titleEdit->text();    
}


QString TagCreateDlg::icon() const
{
    return m_icon;
}

void TagCreateDlg::slotIconChange()
{
    m_icon = KIconDialog::getIcon(KIcon::NoGroup, KIcon::Application, false, 32);
    m_iconButton->setIconSet( KApplication::kApplication()->iconLoader()
                              ->loadIcon(m_icon, KIcon::NoGroup, 32,
                                         KIcon::DefaultState, 0, true));
}

bool TagCreateDlg::tagCreate(TAlbum* parent, QString& title,
                             QString& icon)
{
    TagCreateDlg dlg(parent);
    
    bool ok = dlg.exec() == QDialog::Accepted;

    title    = dlg.title();
    icon     = dlg.icon();

    return ok;
}

// -------------------------------------------------------------------------------------

TagEditDlg::TagEditDlg(TAlbum* album)
    : KDialogBase( Plain, i18n("Edit Tag"), Ok|Cancel, Ok,
                   0, 0, true, true )
{
    QVBoxLayout *topLayout = new QVBoxLayout(plainPage(), 0, spacingHint());

    QLabel *topLabel = new QLabel(plainPage());
    topLabel->setText( i18n("Edit Tag '%1' properties").arg(album->getURL()));
    topLayout->addWidget(topLabel);    

    // --------------------------------------------------------

    QFrame *topLine = new QFrame( plainPage() );
    topLine->setFrameShape( QFrame::HLine );
    topLine->setFrameShadow( QFrame::Sunken );
    topLayout->addWidget( topLine );

    // --------------------------------------------------------

    QGridLayout *gl = new QGridLayout(topLayout, spacingHint());

    QLabel *titleLabel = new QLabel(plainPage());
    titleLabel->setText(i18n("Title: "));
    gl->addWidget(titleLabel, 0, 0);

    m_titleEdit = new QLineEdit(plainPage());
    m_titleEdit->setText(album->getTitle());
    gl->addWidget(m_titleEdit, 0, 1);


    QLabel *iconTextLabel = new QLabel(plainPage());
    iconTextLabel->setText(i18n("Set Icon: "));
    gl->addWidget(iconTextLabel, 1, 0);

    m_iconButton = new QPushButton(plainPage());
    m_iconButton->setFixedSize(40, 40);
    gl->addWidget(m_iconButton, 1, 1);

    connect(m_iconButton, SIGNAL(clicked()),
            SLOT(slotIconChange()));

    m_icon = album->getIcon();
    m_iconButton->setIconSet( KApplication::kApplication()->iconLoader()
                              ->loadIcon(m_icon, KIcon::NoGroup, 32,
                                         KIcon::DefaultState, 0, true));
}

TagEditDlg::~TagEditDlg()
{
    
}

QString TagEditDlg::title() const
{
    return m_titleEdit->text();    
}

QString TagEditDlg::icon() const
{
    return m_icon;
}

void TagEditDlg::slotIconChange()
{
    QString icon = KIconDialog::getIcon(KIcon::NoGroup, KIcon::Application, false, 32);
    if (icon.isEmpty() || icon == m_icon)
        return;

    m_icon = icon;
    m_iconButton->setIconSet( KApplication::kApplication()->iconLoader()
                              ->loadIcon(m_icon, KIcon::NoGroup, 32,
                                         KIcon::DefaultState, 0, true));
}

bool TagEditDlg::tagEdit(TAlbum* album, QString& title,
                         QString& icon)
{
    TagEditDlg dlg(album);
    
    bool ok = (dlg.exec() == QDialog::Accepted);

    title    = dlg.title();
    icon     = dlg.icon();

    return ok;
}


#include "tagcreatedlg.moc"
