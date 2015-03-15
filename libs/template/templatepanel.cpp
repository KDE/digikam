/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-06
 * Description : metadata template settings panel.
 *
 * Copyright (C) 2009-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "templatepanel.h"

// Qt includes

#include <QLabel>
#include <QGridLayout>
#include <QFrame>

// KDE includes

#include <klocale.h>
#include <kdialog.h>
#include <kicon.h>
#include <klineedit.h>

// LibKExiv2 includes

#include <libkexiv2/altlangstredit.h>
#include <libkexiv2/countryselector.h>

// Local includes

#include "templatelist.h"
#include "subjectedit.h"

using namespace KExiv2Iface;

namespace Digikam
{

class TemplatePanel::Private
{

public:

    Private()
    {
        authorsEdit               = 0;
        authorsPositionEdit       = 0;
        creditEdit                = 0;
        sourceEdit                = 0;
        copyrightEdit             = 0;
        rightUsageEdit            = 0;
        instructionsEdit          = 0;

        locationCountryCodeEdit   = 0;
        locationProvinceStateEdit = 0;
        locationCityEdit          = 0;
        locationSublocationEdit   = 0;

        contactCityEdit           = 0;
        contactCountryEdit        = 0;
        contactAddressEdit        = 0;
        contactPostalCodeEdit     = 0;
        contactProvinceStateEdit  = 0;
        contactEmailEdit          = 0;
        contactPhoneEdit          = 0;
        contactWebUrlEdit         = 0;

        subjects                  = 0;
    }

    // Rights template information panel.
    KLineEdit*       authorsEdit;
    KLineEdit*       authorsPositionEdit;
    KLineEdit*       creditEdit;
    KLineEdit*       sourceEdit;
    KLineEdit*       instructionsEdit;

    AltLangStrEdit*  copyrightEdit;
    AltLangStrEdit*  rightUsageEdit;

    // Location template information panel.
    CountrySelector* locationCountryCodeEdit;
    KLineEdit*       locationProvinceStateEdit;
    KLineEdit*       locationCityEdit;
    KLineEdit*       locationSublocationEdit;

    // Contact template information panel.
    KLineEdit*       contactCityEdit;
    KLineEdit*       contactCountryEdit;
    KLineEdit*       contactAddressEdit;
    KLineEdit*       contactPostalCodeEdit;
    KLineEdit*       contactProvinceStateEdit;
    KLineEdit*       contactEmailEdit;
    KLineEdit*       contactPhoneEdit;
    KLineEdit*       contactWebUrlEdit;

    // Subjects template information panel.
    SubjectEdit*     subjects;
};

TemplatePanel::TemplatePanel(QWidget* const parent)
    : KTabWidget(parent), d(new Private)
{
    // -- Rights Template information panel -------------------------------------------------------------

    QWidget* page1     = new QWidget(this);
    QGridLayout* grid1 = new QGridLayout(page1);

    QLabel* label1     = new QLabel(i18n("Author Names:"), page1);
    d->authorsEdit     = new KLineEdit(page1);
    d->authorsEdit->setClearButtonShown(true);
    d->authorsEdit->setClickMessage(i18n("Enter the names of the photograph's creators. Use semi-colons as separator here."));
    label1->setBuddy(d->authorsEdit);
    d->authorsEdit->setWhatsThis(i18n("<p>This field should contain the names of the persons who created the photograph. "
                                      "If it is not appropriate to add the name of the photographer (for example, if the identity of "
                                      "the photographer needs to be protected) the name of a company or organization can also be used. "
                                      "Once saved, this field should not be changed by anyone. "
                                      "<p>To enter more than one name, use <b>semi-colons as separators</b>.</p>"
                                      "<p>With IPTC, this field is limited to 32 ASCII characters.</p>"));

    // --------------------------------------------------------

    QLabel* label2         = new QLabel(i18n("Authors' Positions:"), page1);
    d->authorsPositionEdit = new KLineEdit(page1);
    d->authorsPositionEdit->setClearButtonShown(true);
    d->authorsPositionEdit->setClickMessage(i18n("Enter the job titles of the authors here."));
    label2->setBuddy(d->authorsPositionEdit);
    d->authorsPositionEdit->setWhatsThis(i18n("<p>This field should contain the job titles of the authors. Examples might include "
                                         "titles such as: Staff Photographer, Freelance Photographer, or Independent Commercial "
                                         "Photographer. Since this is a qualifier for the Author field, the Author field must also "
                                         "be filled out.</p>"
                                         "<p>With IPTC, this field is limited to 32 ASCII characters.</p>"));

    // --------------------------------------------------------

    QLabel* label3 = new QLabel(i18n("Credit:"), page1);
    d->creditEdit  = new KLineEdit(page1);
    d->creditEdit->setClearButtonShown(true);
    d->creditEdit->setClickMessage(i18n("Enter the photograph credit here."));
    label3->setBuddy(d->creditEdit);
    d->creditEdit->setWhatsThis(i18n("<p>(synonymous to Provider): Use the Provider field to identify who is providing the photograph. "
                                     "This does not necessarily have to be the author. If a photographer is working for a news agency "
                                     "such as Reuters or the Associated Press, these organizations could be listed here as they are "
                                     "\"providing\" the image for use by others. If the image is a stock photograph, then the group "
                                     "(agency) involved in supplying the image should be listed here.</p>"
                                     "<p>With IPTC, this field is limited to 32 ASCII characters.</p>"));

    // --------------------------------------------------------

    d->copyrightEdit = new AltLangStrEdit(page1);
    d->copyrightEdit->setTitle(i18n("Copyright:"));
    d->copyrightEdit->setFixedHeight(75);
    d->copyrightEdit->setClickMessage(i18n("Enter the copyright notice to identify the current owner(s) of the copyright here."));
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

    d->rightUsageEdit = new AltLangStrEdit(page1);
    d->rightUsageEdit->setTitle(i18n("Right Usage Terms:"));
    d->rightUsageEdit->setFixedHeight(75);
    d->rightUsageEdit->setClickMessage(i18n("Enter the list of instructions on how a resource can be legally used here."));
    d->rightUsageEdit->setWhatsThis(i18n("<p>The Right Usage Terms field should be used to list instructions on how "
                                         "a resource can be legally used."
                                         "<p>With XMP, you can include more than one right usage terms string using "
                                         "different languages.</p>"
                                         "<p>This field does not exist with IPTC.</p>"));

    // --------------------------------------------------------

    QLabel* label6 = new QLabel(i18n("Source:"), page1);
    d->sourceEdit  = new KLineEdit(page1);
    d->sourceEdit->setClearButtonShown(true);
    d->sourceEdit->setClickMessage(i18n("Enter the original owner of the photograph here."));
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

    QLabel* label7      = new QLabel(i18n("Instructions:"), page1);
    d->instructionsEdit = new KLineEdit(page1);
    d->instructionsEdit->setClearButtonShown(true);
    d->instructionsEdit->setClickMessage(i18n("Enter the editorial notice here."));
    label7->setBuddy(d->instructionsEdit);
    d->instructionsEdit->setWhatsThis(i18n("<p>The Instructions field should be used to list editorial "
                                           "instructions concerning the use of photograph.</p>"
                                           "<p>With IPTC, this field is limited to 256 ASCII characters.</p>"));

    // --------------------------------------------------------

    grid1->setMargin(KDialog::spacingHint());
    grid1->setSpacing(KDialog::spacingHint());
    grid1->setAlignment(Qt::AlignTop);
    grid1->setColumnStretch(1, 10);
    grid1->addWidget(label1,                 0, 0, 1, 1);
    grid1->addWidget(d->authorsEdit,         0, 1, 1, 2);
    grid1->addWidget(label2,                 1, 0, 1, 1);
    grid1->addWidget(d->authorsPositionEdit, 1, 1, 1, 2);
    grid1->addWidget(label3,                 2, 0, 1, 1);
    grid1->addWidget(d->creditEdit,          2, 1, 1, 2);
    grid1->addWidget(d->copyrightEdit,       3, 0, 1, 2);
    grid1->addWidget(d->rightUsageEdit,      4, 0, 1, 2);
    grid1->addWidget(label6,                 5, 0, 1, 1);
    grid1->addWidget(d->sourceEdit,          5, 1, 1, 2);
    grid1->addWidget(label7,                 6, 0, 1, 1);
    grid1->addWidget(d->instructionsEdit,    6, 1, 1, 2);

    insertTab(RIGHTS, page1, KIcon("flag-red"), i18n("Rights"));

    // -- Location Template information panel -------------------------------------------------------------

    QWidget* page2 = new QWidget(this);

    // --------------------------------------------------------

    QLabel* label9      = new QLabel(i18n("City:"));
    d->locationCityEdit = new KLineEdit;
    d->locationCityEdit->setClearButtonShown(true);
    d->locationCityEdit->setClickMessage(i18n("Enter the city of contents here."));
    label9->setBuddy(d->locationCityEdit);
    d->locationCityEdit->setWhatsThis(i18n("<p>This field should contain the name of the city "
                                           "where the photograph was taken.</p>"));

    // --------------------------------------------------------

    QLabel* label10            = new QLabel(i18n("Sublocation:"));
    d->locationSublocationEdit = new KLineEdit;
    d->locationSublocationEdit->setClearButtonShown(true);
    d->locationSublocationEdit->setClickMessage(i18n("Enter the city sublocation of contents here."));
    label10->setBuddy(d->locationSublocationEdit);
    d->locationSublocationEdit->setWhatsThis(i18n("<p>This field should contain the sublocation of the city "
            "where the photograph was taken.</p>"));

    // --------------------------------------------------------

    QLabel* label11              = new QLabel(i18n("Province/State:"));
    d->locationProvinceStateEdit = new KLineEdit;
    d->locationProvinceStateEdit->setClearButtonShown(true);
    d->locationProvinceStateEdit->setClickMessage(i18n("Enter the province or state of contents here."));
    label11->setBuddy(d->locationProvinceStateEdit);
    d->locationProvinceStateEdit->setWhatsThis(i18n("<p>This field should contain the province or state "
            "where the photograph was taken.</p>"));

    // --------------------------------------------------------

    QLabel* label12            = new QLabel(i18n("Country:"));
    d->locationCountryCodeEdit = new CountrySelector(page2);
    label12->setBuddy(d->locationCountryCodeEdit);
    d->locationCountryCodeEdit->setWhatsThis(i18n("<p>Select here the country "
            "where the photograph was taken.</p>"));

    // --------------------------------------------------------

    QGridLayout* grid2 = new QGridLayout;
    grid2->addWidget(label9,                       0, 0, 1, 1);
    grid2->addWidget(d->locationCityEdit,          0, 1, 1, 2);
    grid2->addWidget(label10,                      1, 0, 1, 1);
    grid2->addWidget(d->locationSublocationEdit,   1, 1, 1, 2);
    grid2->addWidget(label11,                      2, 0, 1, 1);
    grid2->addWidget(d->locationProvinceStateEdit, 2, 1, 1, 2);
    grid2->addWidget(label12,                      3, 0, 1, 1);
    grid2->addWidget(d->locationCountryCodeEdit,   3, 1, 1, 2);
    grid2->setMargin(KDialog::spacingHint());
    grid2->setSpacing(KDialog::spacingHint());
    grid2->setAlignment(Qt::AlignTop);
    grid2->setColumnStretch(1, 10);
    grid2->setRowStretch(4, 10);
    page2->setLayout(grid2);

    page2->setTabOrder(d->locationCityEdit, d->locationSublocationEdit);
    page2->setTabOrder(d->locationSublocationEdit, d->locationProvinceStateEdit);
    page2->setTabOrder(d->locationProvinceStateEdit, d->locationCountryCodeEdit);

    insertTab(LOCATION, page2, KIcon("applications-internet"), i18n("Location"));

    // -- Contact Template information panel -------------------------------------------------------------

    QWidget* page3     = new QWidget(this);

    QLabel* label13    = new QLabel(i18n("City:"), page3);
    d->contactCityEdit = new KLineEdit(page3);
    d->contactCityEdit->setClearButtonShown(true);
    d->contactCityEdit->setClickMessage(i18n("Enter the city name of the lead author here."));
    label13->setBuddy(d->contactCityEdit);
    d->contactCityEdit->setWhatsThis(i18n("<p>This field should contain the city name "
                                          "where the lead author lives.</p>"));

    // --------------------------------------------------------

    QLabel* label14       = new QLabel(i18n("Country:"), page3);
    d->contactCountryEdit = new KLineEdit(page3);
    d->contactCountryEdit->setClearButtonShown(true);
    d->contactCountryEdit->setClickMessage(i18n("Enter the country name of the lead author here."));
    label14->setBuddy(d->contactCountryEdit);
    d->contactCountryEdit->setWhatsThis(i18n("<p>This field should contain the country name "
                                        "where the lead author lives.</p>"));

    // --------------------------------------------------------

    QLabel* label15       = new QLabel(i18n("Address:"), page3);
    d->contactAddressEdit = new KLineEdit(page3);
    d->contactAddressEdit->setClearButtonShown(true);
    d->contactAddressEdit->setClickMessage(i18n("Enter the address of the lead author here."));
    label15->setBuddy(d->contactAddressEdit);
    d->contactAddressEdit->setWhatsThis(i18n("<p>This field should contain the address "
                                        "where the lead author lives.</p>"));

    // --------------------------------------------------------

    QLabel* label16          = new QLabel(i18n("Postal Code:"), page3);
    d->contactPostalCodeEdit = new KLineEdit(page3);
    d->contactPostalCodeEdit->setClearButtonShown(true);
    d->contactPostalCodeEdit->setClickMessage(i18n("Enter the postal code of the lead author here."));
    label16->setBuddy(d->contactPostalCodeEdit);
    d->contactPostalCodeEdit->setWhatsThis(i18n("<p>This field should contain the postal code "
                                           "where the lead author lives.</p>"));

    // --------------------------------------------------------

    QLabel* label17             = new QLabel(i18n("Province:"), page3);
    d->contactProvinceStateEdit = new KLineEdit(page3);
    d->contactProvinceStateEdit->setClearButtonShown(true);
    d->contactProvinceStateEdit->setClickMessage(i18n("Enter the province of the lead author here."));
    label17->setBuddy(d->contactProvinceStateEdit);
    d->contactProvinceStateEdit->setWhatsThis(i18n("<p>This field should contain the province "
            "where the lead author lives.</p>"));

    // --------------------------------------------------------

    QLabel* label18     = new QLabel(i18n("Email:"), page3);
    d->contactEmailEdit = new KLineEdit(page3);
    d->contactEmailEdit->setClearButtonShown(true);
    d->contactEmailEdit->setClickMessage(i18n("Enter the email of the lead author here."));
    label18->setBuddy(d->contactEmailEdit);
    d->contactEmailEdit->setWhatsThis(i18n("<p>This field should contain the email "
                                           "of the lead author.</p>"));

    // --------------------------------------------------------

    QLabel* label19     = new QLabel(i18n("Phone:"), page3);
    d->contactPhoneEdit = new KLineEdit(page3);
    d->contactPhoneEdit->setClearButtonShown(true);
    d->contactPhoneEdit->setClickMessage(i18n("Enter the phone number of the lead author here."));
    label19->setBuddy(d->contactPhoneEdit);
    d->contactPhoneEdit->setWhatsThis(i18n("<p>This field should contain the phone number "
                                           "of the lead author.</p>"));

    // --------------------------------------------------------

    QLabel* label20      = new QLabel(i18n("URL:"), page3);
    d->contactWebUrlEdit = new KLineEdit(page3);
    d->contactWebUrlEdit->setClearButtonShown(true);
    d->contactWebUrlEdit->setClickMessage(i18n("Enter the web site URL of the lead author here."));
    label20->setBuddy(d->contactWebUrlEdit);
    d->contactWebUrlEdit->setWhatsThis(i18n("<p>This field should contain the web site URL "
                                            "of the lead author.</p>"));

    // --------------------------------------------------------

    QGridLayout* grid3 = new QGridLayout;
    grid3->addWidget(label15,                     0, 0, 1, 1);
    grid3->addWidget(d->contactAddressEdit,       0, 1, 1, 2);
    grid3->addWidget(label16,                     1, 0, 1, 1);
    grid3->addWidget(d->contactPostalCodeEdit,    1, 1, 1, 2);
    grid3->addWidget(label13,                     2, 0, 1, 1);
    grid3->addWidget(d->contactCityEdit,          2, 1, 1, 2);
    grid3->addWidget(label17,                     3, 0, 1, 1);
    grid3->addWidget(d->contactProvinceStateEdit, 3, 1, 1, 2);
    grid3->addWidget(label14,                     4, 0, 1, 1);
    grid3->addWidget(d->contactCountryEdit,       4, 1, 1, 2);
    grid3->addWidget(label19,                     5, 0, 1, 1);
    grid3->addWidget(d->contactPhoneEdit,         5, 1, 1, 2);
    grid3->addWidget(label18,                     6, 0, 1, 1);
    grid3->addWidget(d->contactEmailEdit,         6, 1, 1, 2);
    grid3->addWidget(label20,                     7, 0, 1, 1);
    grid3->addWidget(d->contactWebUrlEdit,        7, 1, 1, 2);
    grid3->setMargin(KDialog::spacingHint());
    grid3->setSpacing(KDialog::spacingHint());
    grid3->setAlignment(Qt::AlignTop);
    grid3->setColumnStretch(1, 10);
    grid3->setRowStretch(8, 10);
    page3->setLayout(grid3);

    page3->setTabOrder(d->contactAddressEdit, d->contactPostalCodeEdit);
    page3->setTabOrder(d->contactPostalCodeEdit, d->contactCityEdit);
    page3->setTabOrder(d->contactCityEdit, d->contactProvinceStateEdit);
    page3->setTabOrder(d->contactProvinceStateEdit, d->contactCountryEdit);
    page3->setTabOrder(d->contactCountryEdit, d->contactPhoneEdit);
    page3->setTabOrder(d->contactPhoneEdit, d->contactEmailEdit);
    page3->setTabOrder(d->contactEmailEdit, d->contactWebUrlEdit);

    insertTab(CONTACT, page3, KIcon("view-pim-contacts"), i18n("Contact"));

    // -- Subjects Template information panel -------------------------------------------------------------

    QWidget* page4     = new QWidget(this);
    QGridLayout* grid4 = new QGridLayout(page4);
    d->subjects        = new SubjectEdit(page4);

    grid4->setMargin(KDialog::spacingHint());
    grid4->setSpacing(KDialog::spacingHint());
    grid4->setAlignment(Qt::AlignTop);
    grid4->addWidget(d->subjects, 0, 0, 1, 1);
    grid4->setRowStretch(1, 10);

    insertTab(SUBJECTS, page4, KIcon("feed-subscribe"), i18n("Subjects"));
}

TemplatePanel::~TemplatePanel()
{
    delete d;
}

void TemplatePanel::setTemplate(const Template& t)
{
    d->authorsEdit->setText(t.authors().join(";"));
    d->authorsPositionEdit->setText(t.authorsPosition());
    d->creditEdit->setText(t.credit());
    d->copyrightEdit->setValues(t.copyright());
    d->rightUsageEdit->setValues(t.rightUsageTerms());
    d->sourceEdit->setText(t.source());
    d->instructionsEdit->setText(t.instructions());

    d->locationCountryCodeEdit->setCountry(t.locationInfo().countryCode);
    d->locationProvinceStateEdit->setText(t.locationInfo().provinceState);
    d->locationCityEdit->setText(t.locationInfo().city);
    d->locationSublocationEdit->setText(t.locationInfo().location);

    d->contactCityEdit->setText(t.contactInfo().city);
    d->contactCountryEdit->setText(t.contactInfo().country);
    d->contactAddressEdit->setText(t.contactInfo().address);
    d->contactPostalCodeEdit->setText(t.contactInfo().postalCode);
    d->contactProvinceStateEdit->setText(t.contactInfo().provinceState);
    d->contactEmailEdit->setText(t.contactInfo().email);
    d->contactPhoneEdit->setText(t.contactInfo().phone);
    d->contactWebUrlEdit->setText(t.contactInfo().webUrl);

    d->subjects->setSubjectsList(t.IptcSubjects());
}

Template TemplatePanel::getTemplate() const
{
    Template t;
    t.setAuthors(d->authorsEdit->text().split(';', QString::SkipEmptyParts));
    t.setAuthorsPosition(d->authorsPositionEdit->text());
    t.setCredit(d->creditEdit->text());
    t.setCopyright(d->copyrightEdit->values());
    t.setRightUsageTerms(d->rightUsageEdit->values());
    t.setSource(d->sourceEdit->text());
    t.setInstructions(d->instructionsEdit->text());

    IptcCoreLocationInfo inf1;
    d->locationCountryCodeEdit->country(inf1.countryCode, inf1.country);
    inf1.provinceState = d->locationProvinceStateEdit->text();
    inf1.city          = d->locationCityEdit->text();
    inf1.location      = d->locationSublocationEdit->text();
    t.setLocationInfo(inf1);

    IptcCoreContactInfo inf2;
    inf2.city          = d->contactCityEdit->text();
    inf2.country       = d->contactCountryEdit->text();
    inf2.address       = d->contactAddressEdit->text();
    inf2.postalCode    = d->contactPostalCodeEdit->text();
    inf2.provinceState = d->contactProvinceStateEdit->text();
    inf2.email         = d->contactEmailEdit->text();
    inf2.phone         = d->contactPhoneEdit->text();
    inf2.webUrl        = d->contactWebUrlEdit->text();
    t.setContactInfo(inf2);

    t.setIptcSubjects(d->subjects->subjectsList());

    return t;
}

void TemplatePanel::apply()
{
}

}  // namespace Digikam
