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
#include <QFrame>

// KDE includes

#include <klocale.h>
#include <kdialog.h>
#include <klineedit.h>
#include <kmessagebox.h>

// Local includes

#include "altlangstredit.h"
#include "templatelist.h"

namespace Digikam
{

class SetupTemplatePriv
{
public:

    SetupTemplatePriv()
    {
        authorsEdit         = 0;
        authorsPositionEdit = 0;
        creditEdit          = 0;
        sourceEdit          = 0;
        copyrightEdit       = 0;
        rightUsageEdit      = 0;
        instructionsEdit    = 0;
        listView            = 0;
        addButton           = 0;
        delButton           = 0;
        repButton           = 0;
    }

    QPushButton    *addButton;
    QPushButton    *delButton;
    QPushButton    *repButton;

    KLineEdit      *titleEdit;
    KLineEdit      *authorsEdit;
    KLineEdit      *authorsPositionEdit;
    KLineEdit      *creditEdit;
    KLineEdit      *sourceEdit;
    KLineEdit      *instructionsEdit;

    AltLangStrEdit *copyrightEdit;
    AltLangStrEdit *rightUsageEdit;

    TemplateList   *listView;
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
    d->listView       = new TemplateList(panel);
    d->listView->setFixedHeight(100);

    // --------------------------------------------------------

    QLabel *label0 = new QLabel(i18n("Template Title:"), panel);
    d->titleEdit   = new KLineEdit(panel);
    d->titleEdit->setClearButtonShown(true);
    label0->setBuddy(d->titleEdit);
    d->titleEdit->setWhatsThis(i18n("<p>Enter here the metadata template title.</p>"));

    // --------------------------------------------------------

    QFrame *tview      = new QFrame(panel);
    QGridLayout* tgrid = new QGridLayout(tview);
    tview->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    tview->setLineWidth(1);
    tview->setFrameShape(QFrame::StyledPanel);

    QLabel *label1 = new QLabel(i18n("Authors:"), tview);
    d->authorsEdit = new KLineEdit(tview);
    d->authorsEdit->setClearButtonShown(true);
    label1->setBuddy(d->authorsEdit);
    d->authorsEdit->setWhatsThis(i18n("<p>This field should contain names of the persons who created the photograph. "
                                      "If it is not appropriate to add the name of the photographer (for example, if the identify of "
                                      "the photographer needs to be protected) the name of a company or organization can also be used. "
                                      "Once saved, this field should not be changed by anyone. "
                                      "<p>To enter more than one name, use <b>semi-colons as separators</b>.</p>"
                                      "<p>With IPTC, this field is limited to 32 ASCII characters.</p>"));

    // --------------------------------------------------------

    QLabel *label2         = new QLabel(i18n("Authors Position:"), tview);
    d->authorsPositionEdit = new KLineEdit(tview);
    d->authorsPositionEdit->setClearButtonShown(true);
    label2->setBuddy(d->authorsPositionEdit);
    d->authorsPositionEdit->setWhatsThis(i18n("<p>This field should contain the job title of the photographer. Examples might include "
                                              "titles such as: Staff Photographer, Freelance Photographer, or Independent Commercial "
                                              "Photographer. Since this is a qualifier for the Author field, the Author field must also "
                                              "be filled out.</p>"
                                              "<p>With IPTC, this field is limited to 32 ASCII characters.</p>"));

    // --------------------------------------------------------

    QLabel *label3 = new QLabel(i18n("Credit:"), tview);
    d->creditEdit  = new KLineEdit(tview);
    d->creditEdit->setClearButtonShown(true);
    label3->setBuddy(d->creditEdit);
    d->creditEdit->setWhatsThis(i18n("<p>(synonymous to Provider): Use the Provider field to identify who is providing the photograph. "
                                     "This does not necessarily have to be the author. If a photographer is working for a news agency "
                                     "such as Reuters or the Associated Press, these organizations could be listed here as they are "
                                     "\"providing\" the image for use by others. If the image is a stock photograph, then the group "
                                     "(agency) involved in supplying the image should be listed here.</p>"
                                     "<p>With IPTC, this field is limited to 32 ASCII characters.</p>"));

    // --------------------------------------------------------

    d->copyrightEdit = new AltLangStrEdit(tview);
    d->copyrightEdit->setTitle(i18n("Copyright:"));
    d->copyrightEdit->setFixedHeight(75);
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
                                        "You may also wish to include the phrase \"all rights reserved\".</p>"
                                        "<p>With XMP, you can include more than one copyright string using different languages.</p>"
                                        "<p>With IPTC, this field is limited to 128 ASCII characters.</p>"));

    // --------------------------------------------------------

    d->rightUsageEdit = new AltLangStrEdit(tview);
    d->rightUsageEdit->setTitle(i18n("Right Usage Terms:"));
    d->rightUsageEdit->setFixedHeight(75);
    d->rightUsageEdit->setWhatsThis(i18n("<p>The Right Usage Terms field should be used to list instructions on how "
                                         "a resource can be legally used."
                                         "<p>With XMP, you can include more than one right usage terms string using "
                                         "different languages.</p>"
                                         "<p>This field do not exist with IPTC.</p>"));

    // --------------------------------------------------------

    QLabel *label6 = new QLabel(i18n("Source:"), tview);
    d->sourceEdit  = new KLineEdit(tview);
    d->sourceEdit->setClearButtonShown(true);
    label6->setBuddy(d->sourceEdit);
    d->sourceEdit->setWhatsThis(i18n("<p>The Source field should be used to identify the original owner or copyright holder of the "
                                     "photograph. The value of this field should never be changed after the information is entered "
                                     "following the image's creation. While not yet enforced by the custom panels, you should consider "
                                     "this to be a \"write-once\" field. The source could be an individual, an agency, or a "
                                     "member of an agency. To aid in later searches, it is suggested to separate any slashes "
                                     "\"/\" with a blank space. Use the form \"photographer / agency\" rather than "
                                     "\"photographer/agency.\" Source may also be different from Creator and from the names "
                                     "listed in the Copyright Notice.</p>"
                                     "<p>With IPTC, this field is limited to 32 ASCII characters.</p>"));

    // --------------------------------------------------------

    QLabel *label7      = new QLabel(i18n("Instructions:"), tview);
    d->instructionsEdit = new KLineEdit(tview);
    d->instructionsEdit->setClearButtonShown(true);
    label7->setBuddy(d->instructionsEdit);
    d->instructionsEdit->setWhatsThis(i18n("<p>The Instructions field should be used to list editorial "
                                           "instructions concerning the use of photograph.</p>"
                                           "<p>With IPTC, this field is limited to 256 ASCII characters.</p>"));

    // --------------------------------------------------------

    tgrid->setMargin(KDialog::spacingHint());
    tgrid->setSpacing(KDialog::spacingHint());
    tgrid->setAlignment(Qt::AlignTop);
    tgrid->setColumnStretch(1, 10);
    tgrid->addWidget(label1,                 0, 0, 1, 1);
    tgrid->addWidget(d->authorsEdit,         0, 1, 1, 2);
    tgrid->addWidget(label2,                 1, 0, 1, 1);
    tgrid->addWidget(d->authorsPositionEdit, 1, 1, 1, 2);
    tgrid->addWidget(label3,                 2, 0, 1, 1);
    tgrid->addWidget(d->creditEdit,          2, 1, 1, 2);
    tgrid->addWidget(d->copyrightEdit,       3, 0, 1, 2);
    tgrid->addWidget(d->rightUsageEdit,      4, 0, 1, 2);
    tgrid->addWidget(label6,                 5, 0, 1, 1);
    tgrid->addWidget(d->sourceEdit,          5, 1, 1, 2);
    tgrid->addWidget(label7,                 6, 0, 1, 1);
    tgrid->addWidget(d->instructionsEdit,    6, 1, 1, 2);

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

    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());
    grid->setAlignment(Qt::AlignTop);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(4, 10);
    grid->addWidget(d->listView,  0, 0, 4, 2);
    grid->addWidget(d->addButton, 0, 2, 1, 1);
    grid->addWidget(d->delButton, 1, 2, 1, 1);
    grid->addWidget(d->repButton, 2, 2, 1, 1);
    grid->addWidget(label0,       4, 0, 1, 1);
    grid->addWidget(d->titleEdit, 4, 1, 1, 1);
    grid->addWidget(tview,        5, 0, 1, 3);
    grid->addWidget(note,         6, 0, 1, 3);

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
}

void SetupTemplate::slotSelectionChanged()
{
    TemplateListItem *item = dynamic_cast<TemplateListItem*>(d->listView->currentItem());
    if (!item)
    {
        d->delButton->setEnabled(false);
        d->repButton->setEnabled(false);
        return;
    }
    d->delButton->setEnabled(true);
    d->repButton->setEnabled(true);
    populateTemplate(item->getTemplate());
}

void SetupTemplate::populateTemplate(const Template& t)
{
    d->titleEdit->setText(t.templateTitle());
    d->authorsEdit->setText(t.authors().join(";"));
    d->authorsPositionEdit->setText(t.authorsPosition());
    d->creditEdit->setText(t.credit());
    d->copyrightEdit->setValues(t.copyright());
    d->rightUsageEdit->setValues(t.rightUsageTerms());
    d->sourceEdit->setText(t.source());
    d->instructionsEdit->setText(t.instructions());
}

void SetupTemplate::slotAddTemplate()
{
    QString title = d->titleEdit->text();

    if (title.isEmpty())
    {
        KMessageBox::error(this, i18n("Cannot register new metadata template without title."));
        return;
    }

    if (d->listView->contains(title))
    {
        KMessageBox::error(this, i18n("A metadata template named '%1' already exist.", title));
        return;
    }

    Template t;
    t.setTemplateTitle(d->titleEdit->text());
    t.setAuthors(d->authorsEdit->text().split(";", QString::SkipEmptyParts));
    t.setAuthorsPosition(d->authorsPositionEdit->text());
    t.setCredit(d->creditEdit->text());
    t.setCopyright(d->copyrightEdit->values());
    t.setRightUsageTerms(d->rightUsageEdit->values());
    t.setSource(d->sourceEdit->text());
    t.setInstructions(d->instructionsEdit->text());
    new TemplateListItem(d->listView, t);
}

void SetupTemplate::slotDelTemplate()
{
    TemplateListItem *item = dynamic_cast<TemplateListItem*>(d->listView->currentItem());
    if (item) delete item;
}

void SetupTemplate::slotRepTemplate()
{
    QString title = d->titleEdit->text();

    if (title.isEmpty())
    {
        KMessageBox::error(this, i18n("Cannot register new metadata template without title."));
        return;
    }

    TemplateListItem *item = dynamic_cast<TemplateListItem*>(d->listView->currentItem());
    if (!item) return;

    Template t;
    t.setTemplateTitle(title);
    t.setAuthors(d->authorsEdit->text().split(";", QString::SkipEmptyParts));
    t.setAuthorsPosition(d->authorsPositionEdit->text());
    t.setCredit(d->creditEdit->text());
    t.setCopyright( d->copyrightEdit->values());
    t.setRightUsageTerms(d->rightUsageEdit->values());
    t.setSource(d->sourceEdit->text());
    t.setInstructions(d->instructionsEdit->text());
    item->setTemplate(t);
}

}  // namespace Digikam
