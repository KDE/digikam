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

#include <QGroupBox>
#include <QLabel>
#include <QValidator>
#include <QGridLayout>
#include <QVBoxLayout>

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
    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setSpacing( KDialog::spacingHint() );
    
    // --------------------------------------------------------

    // IPTC only accept printable Ascii char.
    QRegExp asciiRx("[\x20-\x7F]+$");
    QValidator *asciiValidator = new QRegExpValidator(asciiRx, this);

    QGroupBox *photographerIdGroup = new QGroupBox(i18n("Photographer Information"), this);
    QGridLayout* grid              = new QGridLayout();
    grid->setSpacing(KDialog::spacingHint());

    QLabel *label1 = new QLabel(i18n("Author:"), photographerIdGroup);
    d->authorEdit  = new KLineEdit(photographerIdGroup);
    d->authorEdit->setValidator(asciiValidator);
    d->authorEdit->setMaxLength(32);
    label1->setBuddy(d->authorEdit);
    grid->addWidget(label1, 0, 0, 0, 0);
    grid->addWidget(d->authorEdit, 0, 0, 1, 1);
    d->authorEdit->setWhatsThis( i18n("<p>Set the photographer name. This field is limited "
                                      "to 32 ASCII characters."));

    QLabel *label2     = new QLabel(i18n("Author Title:"), photographerIdGroup);
    d->authorTitleEdit = new KLineEdit(photographerIdGroup);
    d->authorTitleEdit->setValidator(asciiValidator);
    d->authorTitleEdit->setMaxLength(32);
    label2->setBuddy(d->authorTitleEdit);
    grid->addWidget(label2, 1, 1, 0, 0);
    grid->addWidget(d->authorTitleEdit, 1, 1, 1, 1);
    d->authorTitleEdit->setWhatsThis( i18n("<p>Set the photographer title. This field is limited "
                                           "to 32 ASCII characters."));
        
    photographerIdGroup->setLayout(grid);

    // --------------------------------------------------------

    QGroupBox *creditsGroup = new QGroupBox(i18n("Credit and Copyright"), this);
    QGridLayout* grid2      = new QGridLayout();
    grid2->setSpacing(KDialog::spacingHint());

    QLabel *label3 = new QLabel(i18n("Credit:"), creditsGroup);
    d->creditEdit = new KLineEdit(creditsGroup);
    d->creditEdit->setValidator(asciiValidator);
    d->creditEdit->setMaxLength(32);
    label3->setBuddy(d->creditEdit);
    grid2->addWidget(label3, 0, 0, 0, 0);
    grid2->addWidget(d->creditEdit, 0, 0, 1, 1);
    d->creditEdit->setWhatsThis( i18n("<p>Set the default provider identification of the picture, "
                                      "not necessarily the owner/creator. This field is limited "
                                      "to 32 ASCII characters."));

    QLabel *label4 = new QLabel(i18n("Source:"), creditsGroup);
    d->sourceEdit  = new KLineEdit(creditsGroup);
    d->sourceEdit->setValidator(asciiValidator);
    d->sourceEdit->setMaxLength(32);
    label4->setBuddy(d->sourceEdit);
    grid2->addWidget(label4, 1, 1, 0, 0);
    grid2->addWidget(d->sourceEdit, 1, 1, 1, 1);
    d->sourceEdit->setWhatsThis( i18n("<p>Set the default original owner identification of the intellectual "
                                      "content of the picture. This could be an agency, a member of an agency or "
                                      "an individual photographer name. This field is limited "
                                      "to 32 ASCII characters."));

    QLabel *label5   = new QLabel(i18n("Copyright:"), creditsGroup);
    d->copyrightEdit = new KLineEdit(creditsGroup);
    d->copyrightEdit->setValidator(asciiValidator);
    d->copyrightEdit->setMaxLength(128);
    label5->setBuddy(d->copyrightEdit);
    grid2->addWidget(label5, 2, 2, 0, 0);
    grid2->addWidget(d->copyrightEdit, 2, 2, 1, 1);
    d->copyrightEdit->setWhatsThis( i18n("<p>Set the default copyright notice of the pictures. "
                                         "This field is limited to 128 ASCII characters."));

    creditsGroup->setLayout(grid2);

    // --------------------------------------------------------

    QLabel *iptcNote = new QLabel(i18n("<b>Note: IPTC text tags only support the printable "
                                       "ASCII characters set.</b>"), this);
                                         
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

