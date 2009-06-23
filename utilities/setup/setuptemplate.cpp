/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-07-04
 * Description : metadata template setup page.
 *
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setuptemplate.h"
#include "setuptemplate.moc"

// Qt includes

#include <QGroupBox>
#include <QLabel>
#include <QValidator>
#include <QGridLayout>
#include <QPushButton>

// KDE includes

#include <klocale.h>
#include <kdialog.h>
#include <klineedit.h>

// Local includes

#include "photographer.h"
#include "identitylist.h"

namespace Digikam
{

class SetupTemplatePriv
{
public:

    SetupTemplatePriv()
    {
        authorEdit         = 0;
        authorPositionEdit = 0;
        creditEdit         = 0;
        sourceEdit         = 0;
        copyrightEdit      = 0;
        rightUsageEdit     = 0;
        instructionsEdit   = 0;
        listView           = 0;
        addButton          = 0;
        delButton          = 0;
        repButton          = 0;
    }

    QPushButton   *addButton;
    QPushButton   *delButton;
    QPushButton   *repButton;

    KLineEdit     *authorEdit;
    KLineEdit     *authorPositionEdit;
    KLineEdit     *creditEdit;
    KLineEdit     *copyrightEdit;
    KLineEdit     *rightUsageEdit;
    KLineEdit     *sourceEdit;
    KLineEdit     *instructionsEdit;

    IndentityList *listView;
};

SetupTemplate::SetupTemplate(QWidget* parent)
             : QScrollArea(parent), d(new SetupTemplatePriv)
{
    QWidget *panel = new QWidget(viewport());
    panel->setAutoFillBackground(false);
    setWidget(panel);
    setWidgetResizable(true);
    viewport()->setAutoFillBackground(false);

    QGridLayout* grid = new QGridLayout(panel);
    d->listView       = new IndentityList(panel);

    // --------------------------------------------------------

    QLabel *label1 = new QLabel(i18n("Author:"), panel);
    d->authorEdit  = new KLineEdit(panel);
    d->authorEdit->setClearButtonShown(true);
    d->authorEdit->setMaxLength(32);
    label1->setBuddy(d->authorEdit);
    d->authorEdit->setWhatsThis(i18n("<p>This field should contain your name, or the name of the person who created the photograph. "
                                     "If it is not appropriate to add the name of the photographer (for example, if the identify of "
                                     "the photographer needs to be protected) the name of a company or organization can also be used. "
                                     "Once saved, this field should not be changed by anyone. This field does not support the use of "
                                     "commas or semi-colons as separators. \nThis field is limited to 32 ASCII characters.</p>"));

    // --------------------------------------------------------

    QLabel *label2        = new QLabel(i18n("Author Position:"), panel);
    d->authorPositionEdit = new KLineEdit(panel);
    d->authorPositionEdit->setClearButtonShown(true);
    d->authorPositionEdit->setMaxLength(32);
    label2->setBuddy(d->authorPositionEdit);
    d->authorPositionEdit->setWhatsThis(i18n("<p>This field should contain the job title of the photographer. Examples might include "
                                             "titles such as: Staff Photographer, Freelance Photographer, or Independent Commercial "
                                             "Photographer. Since this is a qualifier for the Author field, the Author field must also "
                                             "be filled out. \nThis field is limited to 32 ASCII characters.</p>"));

    // --------------------------------------------------------

    QLabel *label3 = new QLabel(i18n("Credit:"), panel);
    d->creditEdit  = new KLineEdit(panel);
    d->creditEdit->setClearButtonShown(true);
    d->creditEdit->setMaxLength(32);
    label3->setBuddy(d->creditEdit);
    d->creditEdit->setWhatsThis(i18n("<p>(synonymous to Provider): Use the Provider field to identify who is providing the photograph. "
                                     "This does not necessarily have to be the author. If a photographer is working for a news agency "
                                     "such as Reuters or the Associated Press, these organizations could be listed here as they are "
                                     "\"providing\" the image for use by others. If the image is a stock photograph, then the group "
                                     "(agency) involved in supplying the image should be listed here. "
                                     "\nThis field is limited to 32 ASCII characters.</p>"));

    // --------------------------------------------------------

    QLabel *label4   = new QLabel(i18n("Copyright:"), panel);
    d->copyrightEdit = new KLineEdit(panel);
    d->copyrightEdit->setClearButtonShown(true);
    d->copyrightEdit->setMaxLength(128);
    label4->setBuddy(d->copyrightEdit);
    d->copyrightEdit->setWhatsThis(i18n("<p>The Copyright Notice should contain any necessary copyright notice for claiming the intellectual "
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

    QLabel *label5    = new QLabel(i18n("Right Usage Terms:"), panel);
    d->rightUsageEdit = new KLineEdit(panel);
    d->rightUsageEdit->setClearButtonShown(true);
    label5->setBuddy(d->rightUsageEdit);
//TODO    d->rightUsageEdit->setWhatsThis(i18n("<p></p>"));

    // --------------------------------------------------------

    QLabel *label6 = new QLabel(i18n("Source:"), panel);
    d->sourceEdit  = new KLineEdit(panel);
    d->sourceEdit->setClearButtonShown(true);
    d->sourceEdit->setMaxLength(32);
    label6->setBuddy(d->sourceEdit);
    d->sourceEdit->setWhatsThis(i18n("<p>The Source field should be used to identify the original owner or copyright holder of the "
                                     "photograph. The value of this field should never be changed after the information is entered "
                                     "following the image's creation. While not yet enforced by the custom panels, you should consider "
                                     "this to be a \"write-once\" field. The source could be an individual, an agency, or a "
                                     "member of an agency. To aid in later searches, it is suggested to separate any slashes "
                                     "\"/\" with a blank space. Use the form \"photographer / agency\" rather than "
                                     "\"photographer/agency.\" Source may also be different from Creator and from the names "
                                     "listed in the Copyright Notice.\nThis field is limited to 32 ASCII characters.</p>"));

    // --------------------------------------------------------

    QLabel *label7      = new QLabel(i18n("Instructions:"), panel);
    d->instructionsEdit = new KLineEdit(panel);
    d->instructionsEdit->setClearButtonShown(true);
    d->instructionsEdit->setMaxLength(256);
    label5->setBuddy(d->instructionsEdit);
//TODO    d->instructionsEdit->setWhatsThis(i18n("<p></p>"));

    // --------------------------------------------------------

    QLabel *note = new QLabel(i18n("<b>Note: These information are used to set "
                   "<b><a href='http://en.wikipedia.org/wiki/Extensible_Metadata_Platform'>XMP</a></b> "
                   "and <b><a href='http://en.wikipedia.org/wiki/IPTC'>IPTC</a></b> tag contents. "
                   "There is no limitation with XMP, but note that IPTC text tags "
                   "only support the printable <b><a href='http://en.wikipedia.org/wiki/Ascii'>ASCII</a></b> "
                   "character set, and tag sizes are limited. "
                   "Use contextual help for details.</b>"), panel);
    note->setOpenExternalLinks(true);
    note->setWordWrap(true);
    note->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    // -------------------------------------------------------------

    d->addButton = new QPushButton(panel);
    d->delButton = new QPushButton(panel);
    d->repButton = new QPushButton(panel);

    d->addButton->setText(i18n("&Add..."));
    d->addButton->setIcon(SmallIcon("list-add"));
    d->delButton->setText(i18n( "&Remove"));
    d->delButton->setIcon(SmallIcon("list-remove"));
    d->repButton->setText(i18n("&Replace..."));
    d->repButton->setIcon(SmallIcon("view-refresh"));
    d->delButton->setEnabled(false);
    d->repButton->setEnabled(false);

    // -------------------------------------------------------------

    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());
    grid->setAlignment(Qt::AlignTop);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(4, 10);
    grid->addWidget(d->listView,           0,  0, 4, 2);
    grid->addWidget(d->addButton,          0,  2, 1, 1);
    grid->addWidget(d->delButton,          1,  2, 1, 1);
    grid->addWidget(d->repButton,          2,  2, 1, 1);
    grid->addWidget(label1,                4,  0, 1, 1);
    grid->addWidget(d->authorEdit,         4,  1, 1, 2);
    grid->addWidget(label2,                5,  0, 1, 1);
    grid->addWidget(d->authorPositionEdit, 5,  1, 1, 2);
    grid->addWidget(label3,                6,  0, 1, 1);
    grid->addWidget(d->creditEdit,         6,  1, 1, 2);
    grid->addWidget(label4,                7,  0, 1, 1);
    grid->addWidget(d->copyrightEdit,      7,  1, 1, 2);
    grid->addWidget(label5,                8,  0, 1, 1);
    grid->addWidget(d->rightUsageEdit,     8,  1, 1, 2);
    grid->addWidget(label6,                9,  0, 1, 1);
    grid->addWidget(d->sourceEdit,         9,  1, 1, 2);
    grid->addWidget(label7,                10, 0, 1, 1);
    grid->addWidget(d->instructionsEdit,   10, 1, 1, 2);
    grid->addWidget(note,                  11, 0, 1, 3);

    // --------------------------------------------------------

    connect(d->listView, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(d->addButton, SIGNAL(clicked()),
            this, SLOT(slotAddTemplate()));

    connect(d->delButton, SIGNAL(clicked()),
            this, SLOT(slotDelTemplate()));

    connect(d->repButton, SIGNAL(clicked()),
            this, SLOT(slotRepTemplate()));

    // --------------------------------------------------------

    readSettings();
}

SetupTemplate::~SetupTemplate()
{
    delete d;
}

void SetupTemplate::applySettings()
{
    d->listView->applySettings();
}

void SetupTemplate::readSettings()
{
    d->listView->readSettings();

/*
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;

    d->authorEdit->setText(settings->getAuthor());
    d->authorTitleEdit->setText(settings->getAuthorTitle());
    d->creditEdit->setText(settings->getCredit());
    d->sourceEdit->setText(settings->getSource());
    d->copyrightEdit->setText(settings->getCopyright());
*/
}

void SetupTemplate::slotSelectionChanged()
{
    IndentityListItem *item = dynamic_cast<IndentityListItem*>(d->listView->currentItem());
    if (!item)
    {
        d->delButton->setEnabled(false);
        d->repButton->setEnabled(false);
        return;
    }

    Photographer *photographer = item->photographer();
    d->authorEdit->setText(photographer->author());
    d->authorPositionEdit->setText(photographer->authorPosition());
    d->creditEdit->setText(photographer->credit());
    d->copyrightEdit->setText(photographer->copyright());
    d->rightUsageEdit->setText(photographer->rightUsageTerms());
    d->sourceEdit->setText(photographer->source());
    d->instructionsEdit->setText(photographer->instructions());

    d->delButton->setEnabled(true);
    d->repButton->setEnabled(true);
}

void SetupTemplate::slotAddTemplate()
{
    Photographer photographer;
    photographer.setAuthor(d->authorEdit->text());
    photographer.setAuthorPosition(d->authorPositionEdit->text());
    photographer.setCredit(d->creditEdit->text());
    photographer.setCopyright(d->copyrightEdit->text());
    photographer.setRightUsageTerms(d->rightUsageEdit->text());
    photographer.setSource(d->sourceEdit->text());
    photographer.setInstructions(d->instructionsEdit->text());
    new IndentityListItem(d->listView, &photographer);
}

void SetupTemplate::slotDelTemplate()
{
    IndentityListItem *item = dynamic_cast<IndentityListItem*>(d->listView->currentItem());
    if (item) delete item;
}

void SetupTemplate::slotRepTemplate()
{
    IndentityListItem *item = dynamic_cast<IndentityListItem*>(d->listView->currentItem());
    if (!item) return;

    Photographer photographer;
    photographer.setAuthor(d->authorEdit->text());
    photographer.setAuthorPosition(d->authorPositionEdit->text());
    photographer.setCredit(d->creditEdit->text());
    photographer.setCopyright(d->copyrightEdit->text());
    photographer.setRightUsageTerms(d->rightUsageEdit->text());
    photographer.setSource(d->sourceEdit->text());
    photographer.setInstructions(d->instructionsEdit->text());
    item->setPhotographer(&photographer);
}

}  // namespace Digikam
