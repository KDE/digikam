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

// Qt includes.

#include <qlayout.h>
#include <qhgroupbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qvalidator.h>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <klineedit.h>
#include <kactivelabel.h>

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
    QVBoxLayout *layout = new QVBoxLayout( parent, 0, KDialog::spacingHint() );

    // --------------------------------------------------------

    // IPTC only accept printable Ascii char.
    QRegExp asciiRx("[\x20-\x7F]+$");
    QValidator *asciiValidator = new QRegExpValidator(asciiRx, this);

    QGroupBox *photographerIdGroup = new QGroupBox(0, Qt::Horizontal, i18n("Photographer and Copyright Information"), parent);
    QGridLayout* grid = new QGridLayout( photographerIdGroup->layout(), 1, 1, KDialog::spacingHint());

    QLabel *label1 = new QLabel(i18n("Author:"), photographerIdGroup);
    d->authorEdit  = new KLineEdit(photographerIdGroup);
    d->authorEdit->setValidator(asciiValidator);
    d->authorEdit->setMaxLength(32);
    label1->setBuddy(d->authorEdit);
    grid->addMultiCellWidget(label1, 0, 0, 0, 0);
    grid->addMultiCellWidget(d->authorEdit, 0, 0, 1, 1);
    QWhatsThis::add( d->authorEdit, i18n("<p>This field should contain your name, or the name of the person who created the photograph. "
                                         "If it is not appropriate to add the name of the photographer (for example, if the identify of "
                                         "the photographer needs to be protected) the name of a company or organization can also be used. "
                                         "Once saved, this field should not be changed by anyone. This field does not support the use of "
                                         "commas or semi-colons as separator. \nThis field is limited to 32 ASCII characters.</p>"));

    QLabel *label2 = new QLabel(i18n("Author Title:"), photographerIdGroup);
    d->authorTitleEdit = new KLineEdit(photographerIdGroup);
    d->authorTitleEdit->setValidator(asciiValidator);
    d->authorTitleEdit->setMaxLength(32);
    label2->setBuddy(d->authorTitleEdit);
    grid->addMultiCellWidget(label2, 1, 1, 0, 0);
    grid->addMultiCellWidget(d->authorTitleEdit, 1, 1, 1, 1);
    QWhatsThis::add( d->authorTitleEdit, i18n("<p>This field should contain the job title of the photographer. Examples might include "
                                              "titles such as: Staff Photographer, Freelance Photographer, or Independent Commercial "
                                              "Photographer. Since this is a qualifier for the Author field, the Author field must also "
                                              "be filled out. \nThis field is limited to 32 ASCII characters.</p>"));

    // --------------------------------------------------------

    QGroupBox *creditsGroup = new QGroupBox(0, Qt::Horizontal, i18n("Credit and Copyright"), parent);
    QGridLayout* grid2 = new QGridLayout( creditsGroup->layout(), 2, 1, KDialog::spacingHint());

    QLabel *label3 = new QLabel(i18n("Credit:"), creditsGroup);
    d->creditEdit = new KLineEdit(creditsGroup);
    d->creditEdit->setValidator(asciiValidator);
    d->creditEdit->setMaxLength(32);
    label3->setBuddy(d->creditEdit);
    grid2->addMultiCellWidget(label3, 0, 0, 0, 0);
    grid2->addMultiCellWidget(d->creditEdit, 0, 0, 1, 1);
    QWhatsThis::add( d->creditEdit, i18n("<p>(synonymous to Provider): Use the Provider field to identify who is providing the photograph. "
                                         "This does not necessarily have to be the author. If a photographer is working for a news agency "
                                         "such as Reuters or the Associated Press, these organizations could be listed here as they are "
                                         "\"providing\" the image for use by others. If the image is a stock photograph, then the group "
                                         "(agency) involved in supplying the image should be listed here. "
                                         "\nThis field is limited to 32 ASCII characters.</p>"));

    QLabel *label4 = new QLabel(i18n("Source:"), creditsGroup);
    d->sourceEdit = new KLineEdit(creditsGroup);
    d->sourceEdit->setValidator(asciiValidator);
    d->sourceEdit->setMaxLength(32);
    label4->setBuddy(d->sourceEdit);
    grid2->addMultiCellWidget(label4, 1, 1, 0, 0);
    grid2->addMultiCellWidget(d->sourceEdit, 1, 1, 1, 1);
    QWhatsThis::add( d->sourceEdit, i18n("<p>The Source field should be used to identify the original owner or copyright holder of the "
                                         "photograph. The value of this field should never be changed after the information is entered "
                                         "following the image's creation. While not yet enforced by the custom panels, you should consider "
                                         "this to be a \"write-once\" field. The source could be an individual, an agency, or a "
                                         "member of an agency. To aid in later searches, it is suggested to separate any slashes "
                                         "\"/\" with a blank space. Use the form \"photographer / agency\" rather than "
                                         "\"photographer/agency.\" Source may also be different from Creator and from the names "
                                         "listed in the Copyright Notice.\nThis field is limited to 32 ASCII characters.</p>"));

    QLabel *label5 = new QLabel(i18n("Copyright:"), creditsGroup);
    d->copyrightEdit = new KLineEdit(creditsGroup);
    d->copyrightEdit->setValidator(asciiValidator);
    d->copyrightEdit->setMaxLength(128);
    label5->setBuddy(d->copyrightEdit);
    grid2->addMultiCellWidget(label5, 2, 2, 0, 0);
    grid2->addMultiCellWidget(d->copyrightEdit, 2, 2, 1, 1);
    QWhatsThis::add( d->copyrightEdit, i18n("<p>The Copyright Notice should contain any necessary copyright notice for claiming the intellectual "
                                            "property, and should identify the current owner(s) of the copyright for the photograph. Usually, "
                                            "this would be the photographer, but if the image was done by an employee or as work-for-hire, "
                                            "then the agency or company should be listed. Use the form appropriate to your country. USA: "
                                            "&copy; {date of first publication} name of copyright owner, as in \"&copy;2005 John Doe.\" "
                                            "Note, the word \"copyright\" or the abbreviation \"copr\" may be used in place of the &copy; symbol. "
                                            "In some foreign countries only the copyright symbol is recognized and the abbreviation does not work. "
                                            "Furthermore the copyright symbol must be a full circle with a \"c\" inside; using something like (c) "
                                            "where the parentheses form a partial circle is not sufficient. For additional protection worldwide, "
                                            "use of the phrase, \"all rights reserved\" following the notice above is encouraged. \nIn Europe "
                                            "you would use: Copyright {Year} {Copyright owner}, all rights reserved. \nIn Japan, for maximum "
                                            "protection, the following three items should appear in the copyright field of the IPTC Core: "
                                            "(a) the word, Copyright; (b) year of the first publication; and (c) name of the author. "
                                            "You may also wish to include the phrase \"all rights reserved.\"\n"
                                            "This field is limited to 128 ASCII characters.</p>"));

    // --------------------------------------------------------

    KActiveLabel *note = new KActiveLabel(i18n("<b>Note: These informations are used to set "
                   "<b><a href='http://en.wikipedia.org/wiki/IPTC'>IPTC</a></b> tags contents. "
                   "IPTC text tags only support the printable "
                   "<b><a href='http://en.wikipedia.org/wiki/Ascii'>ASCII</a></b> "
                   "characters set and limit strings size. "
                   "Use contextual help for details.</b>"), parent);

    // --------------------------------------------------------

    layout->addWidget(photographerIdGroup);
    layout->addWidget(creditsGroup);
    layout->addWidget(note);
    layout->addStretch();

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

    settings->setIptcAuthor(d->authorEdit->text());
    settings->setIptcAuthorTitle(d->authorTitleEdit->text());
    settings->setIptcCredit(d->creditEdit->text());
    settings->setIptcSource(d->sourceEdit->text());
    settings->setIptcCopyright(d->copyrightEdit->text());
    settings->saveSettings();
}

void SetupIdentity::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;

    d->authorEdit->setText(settings->getIptcAuthor());
    d->authorTitleEdit->setText(settings->getIptcAuthorTitle());
    d->creditEdit->setText(settings->getIptcCredit());
    d->sourceEdit->setText(settings->getIptcSource());
    d->copyrightEdit->setText(settings->getIptcCopyright());
}

}  // namespace Digikam

