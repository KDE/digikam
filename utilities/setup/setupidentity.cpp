/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2006-07-04
 * Description : default identity setup tab.
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

    QGroupBox *photographerIdGroup = new QGroupBox(i18n("Photographer Information"), this);
    QGridLayout* grid              = new QGridLayout(photographerIdGroup);

    QLabel *label1 = new QLabel(i18n("Author:"), photographerIdGroup);
    d->authorEdit  = new KLineEdit(photographerIdGroup);
    d->authorEdit->setMaxLength(32);
    label1->setBuddy(d->authorEdit);
    d->authorEdit->setWhatsThis( i18n("<p>Set the photographer name. This field is limited "
                                      "to 32 ASCII characters with IPTC."));

    QLabel *label2     = new QLabel(i18n("Author Title:"), photographerIdGroup);
    d->authorTitleEdit = new KLineEdit(photographerIdGroup);
    d->authorTitleEdit->setMaxLength(32);
    label2->setBuddy(d->authorTitleEdit);
    d->authorTitleEdit->setWhatsThis( i18n("<p>Set the photographer title. This field is limited "
                                           "to 32 ASCII characters with IPTC."));
        
    grid->addWidget(label1, 0, 0, 1, 1);
    grid->addWidget(d->authorEdit, 0, 1, 1, 1);
    grid->addWidget(label2, 1, 0, 1, 1);
    grid->addWidget(d->authorTitleEdit, 1, 1, 1, 1);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------

    QGroupBox *creditsGroup = new QGroupBox(i18n("Credit and Copyright"), this);
    QGridLayout* grid2      = new QGridLayout(creditsGroup);

    QLabel *label3 = new QLabel(i18n("Credit:"), creditsGroup);
    d->creditEdit  = new KLineEdit(creditsGroup);
    d->creditEdit->setMaxLength(32);
    label3->setBuddy(d->creditEdit);
    d->creditEdit->setWhatsThis( i18n("<p>Set the default provider identification of the image, "
                                      "not necessarily the owner/creator. This field is limited "
                                      "to 32 ASCII characters with IPTC."));

    QLabel *label4 = new QLabel(i18n("Source:"), creditsGroup);
    d->sourceEdit  = new KLineEdit(creditsGroup);
    d->sourceEdit->setMaxLength(32);
    label4->setBuddy(d->sourceEdit);
    d->sourceEdit->setWhatsThis( i18n("<p>Set the default original owner identification of the intellectual "
                                      "content of the image. This could be an agency, a member of an agency or "
                                      "an individual photographer name. This field is limited "
                                      "to 32 ASCII characters with IPTC."));

    QLabel *label5   = new QLabel(i18n("Copyright:"), creditsGroup);
    d->copyrightEdit = new KLineEdit(creditsGroup);
    d->copyrightEdit->setMaxLength(128);
    label5->setBuddy(d->copyrightEdit);
    d->copyrightEdit->setWhatsThis( i18n("<p>Set the default copyright notice of the images. "
                                         "This field is limited to 128 ASCII characters with IPTC."));

    grid2->addWidget(label3, 0, 0, 1, 1);
    grid2->addWidget(d->creditEdit, 0, 1, 1, 1);
    grid2->addWidget(label4, 1, 0, 1, 1);
    grid2->addWidget(d->sourceEdit, 1, 1, 1, 1);
    grid2->addWidget(label5, 2, 0, 1, 1);
    grid2->addWidget(d->copyrightEdit, 2, 1, 1, 1);
    grid2->setMargin(KDialog::spacingHint());
    grid2->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------

    QLabel *note = new QLabel(i18n("<b>Note: These informations are used to set "
                   "<b><a href='http://en.wikipedia.org/wiki/Extensible_Metadata_Platform'>XMP</a></b> "
                   "and <b><a href='http://en.wikipedia.org/wiki/IPTC'>IPTC</a></b> tags contents. "
                   "There is no limitation with XMP, but take a care than IPTC text tags "
			       "only support the printable <b><a href='http://en.wikipedia.org/wiki/Ascii'>ASCII</a></b>"
                   "characters set and limit strings size. "
				   "Use contextual help for details.</b>"), this);
    note->setOpenExternalLinks(true);
    note->setWordWrap(true);                       

    // --------------------------------------------------------
    
    layout->addWidget(photographerIdGroup);
    layout->addWidget(creditsGroup);
    layout->addWidget(note);
    layout->addStretch();
    layout->setMargin(0);
    layout->setSpacing(KDialog::spacingHint());
    
    readSettings();
}

SetupIdentity::~SetupIdentity()
{
    delete d;
}

void SetupIdentity::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;

    settings->setAuthor(d->authorEdit->text());
    settings->setAuthorTitle(d->authorTitleEdit->text());
    settings->setCredit(d->creditEdit->text());
    settings->setSource(d->sourceEdit->text());
    settings->setCopyright(d->copyrightEdit->text());
    settings->saveSettings();
}

void SetupIdentity::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;

    d->authorEdit->setText(settings->getAuthor());
    d->authorTitleEdit->setText(settings->getAuthorTitle());
    d->creditEdit->setText(settings->getCredit());
    d->sourceEdit->setText(settings->getSource());
    d->copyrightEdit->setText(settings->getCopyright());
}

}  // namespace Digikam
