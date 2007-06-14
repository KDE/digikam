/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2006-07-04
 * Description : default IPTC identity setup tab.
 * 
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// QT includes.

#include <qlayout.h>
#include <q3hgroupbox.h>
#include <q3groupbox.h>
#include <qlabel.h>

#include <qvalidator.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3VBoxLayout>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <klineedit.h>

// Local includes.

#include "albumsettings.h"
#include "setupidentity.h"
#include "setupidentity.moc"

namespace Digikam
{

class SetupIdentityPriv
{
public:

    SetupIdentityPriv()
    {
        authorEdit      = 0;
        authorTitleEdit = 0;
        creditEdit      = 0;
        sourceEdit      = 0;
        copyrightEdit   = 0;
    }

    KLineEdit *authorEdit;
    KLineEdit *authorTitleEdit;
    KLineEdit *creditEdit;
    KLineEdit *sourceEdit;
    KLineEdit *copyrightEdit;
};

SetupIdentity::SetupIdentity(QWidget* parent )
             : QWidget(parent)
{
    d = new SetupIdentityPriv;
    Q3VBoxLayout *layout = new Q3VBoxLayout( parent, 0, KDialog::spacingHint() );
    
    // --------------------------------------------------------

    // IPTC only accept printable Ascii char.
    QRegExp asciiRx("[\x20-\x7F]+$");
    QValidator *asciiValidator = new QRegExpValidator(asciiRx, this);

    Q3GroupBox *photographerIdGroup = new Q3GroupBox(0, Qt::Horizontal, i18n("Photographer Information"), parent);
    Q3GridLayout* grid = new Q3GridLayout( photographerIdGroup->layout(), 1, 1, KDialog::spacingHint());

    QLabel *label1 = new QLabel(i18n("Author:"), photographerIdGroup);
    d->authorEdit  = new KLineEdit(photographerIdGroup);
    d->authorEdit->setValidator(asciiValidator);
    d->authorEdit->setMaxLength(32);
    label1->setBuddy(d->authorEdit);
    grid->addMultiCellWidget(label1, 0, 0, 0, 0);
    grid->addMultiCellWidget(d->authorEdit, 0, 0, 1, 1);
    d->authorEdit->setWhatsThis( i18n("<p>Set the photographer name. This field is limited "
                                         "to 32 ASCII characters."));

    QLabel *label2 = new QLabel(i18n("Author Title:"), photographerIdGroup);
    d->authorTitleEdit = new KLineEdit(photographerIdGroup);
    d->authorTitleEdit->setValidator(asciiValidator);
    d->authorTitleEdit->setMaxLength(32);
    label2->setBuddy(d->authorTitleEdit);
    grid->addMultiCellWidget(label2, 1, 1, 0, 0);
    grid->addMultiCellWidget(d->authorTitleEdit, 1, 1, 1, 1);
    d->authorTitleEdit->setWhatsThis( i18n("<p>Set the photographer title. This field is limited "
                                              "to 32 ASCII characters."));
        
    // --------------------------------------------------------

    Q3GroupBox *creditsGroup = new Q3GroupBox(0, Qt::Horizontal, i18n("Credit and Copyright"), parent);
    Q3GridLayout* grid2 = new Q3GridLayout( creditsGroup->layout(), 2, 1, KDialog::spacingHint());

    QLabel *label3 = new QLabel(i18n("Credit:"), creditsGroup);
    d->creditEdit = new KLineEdit(creditsGroup);
    d->creditEdit->setValidator(asciiValidator);
    d->creditEdit->setMaxLength(32);
    label3->setBuddy(d->creditEdit);
    grid2->addMultiCellWidget(label3, 0, 0, 0, 0);
    grid2->addMultiCellWidget(d->creditEdit, 0, 0, 1, 1);
    d->creditEdit->setWhatsThis( i18n("<p>Set the default provider identification of the picture, "
                                         "not necessarily the owner/creator. This field is limited "
                                         "to 32 ASCII characters."));

    QLabel *label4 = new QLabel(i18n("Source:"), creditsGroup);
    d->sourceEdit = new KLineEdit(creditsGroup);
    d->sourceEdit->setValidator(asciiValidator);
    d->sourceEdit->setMaxLength(32);
    label4->setBuddy(d->sourceEdit);
    grid2->addMultiCellWidget(label4, 1, 1, 0, 0);
    grid2->addMultiCellWidget(d->sourceEdit, 1, 1, 1, 1);
    d->sourceEdit->setWhatsThis( i18n("<p>Set the default original owner identification of the intellectual "
                                         "content of the picture. This could be an agency, a member of an agency or "
                                         "an individual photographer name. This field is limited "
                                         "to 32 ASCII characters."));

    QLabel *label5 = new QLabel(i18n("Copyright:"), creditsGroup);
    d->copyrightEdit = new KLineEdit(creditsGroup);
    d->copyrightEdit->setValidator(asciiValidator);
    d->copyrightEdit->setMaxLength(128);
    label5->setBuddy(d->copyrightEdit);
    grid2->addMultiCellWidget(label5, 2, 2, 0, 0);
    grid2->addMultiCellWidget(d->copyrightEdit, 2, 2, 1, 1);
    d->copyrightEdit->setWhatsThis( i18n("<p>Set the default copyright notice of the pictures. "
                                            "This field is limited to 128 ASCII characters."));

    // --------------------------------------------------------

    QLabel *iptcNote = new QLabel(i18n("<b>Note: IPTC text tags only support the printable "
                                       "ASCII characters set.</b>"), parent);
                                         
    // --------------------------------------------------------
    
    layout->addWidget(photographerIdGroup);
    layout->addWidget(creditsGroup);
    layout->addWidget(iptcNote);
    layout->addStretch();
    
    readSettings();
}

SetupIdentity::~SetupIdentity()
{
    delete d;
}

void SetupIdentity::applySettings()
{
    AlbumSettings* settings = AlbumSettings::componentData();
    if (!settings) return;

    settings->setIptcAuthor(d->authorEdit->text());
    settings->setIptcAuthorTitle(d->authorTitleEdit->text());
    settings->setIptcCredit(d->creditEdit->text());
    settings->setIptcSource(d->sourceEdit->text());
    settings->setIptcCopyright(d->copyrightEdit->text());
    settings->saveSettings();
}

void SetupIdentity::readSettings()
{
    AlbumSettings* settings = AlbumSettings::componentData();
    if (!settings) return;

    d->authorEdit->setText(settings->getIptcAuthor());
    d->authorTitleEdit->setText(settings->getIptcAuthorTitle());
    d->creditEdit->setText(settings->getIptcCredit());
    d->sourceEdit->setText(settings->getIptcSource());
    d->copyrightEdit->setText(settings->getIptcCopyright());
}

}  // namespace Digikam

