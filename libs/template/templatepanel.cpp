/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-06
 * Description : metadata template settings panel.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Local includes

#include "altlangstredit.h"
#include "templatelist.h"

namespace Digikam
{

class TemplatePanelPriv
{
public:

    enum TemplateTab
    {
        RIGHTS=0,
        LOCATION,
        CONTACT
    };

public:

    TemplatePanelPriv()
    {
        authorsEdit               = 0;
        authorsPositionEdit       = 0;
        creditEdit                = 0;
        sourceEdit                = 0;
        copyrightEdit             = 0;
        rightUsageEdit            = 0;
        instructionsEdit          = 0;

        locationCountryEdit       = 0;
        locationCountryCodeEdit   = 0;
        locationProvinceStateEdit = 0;
        locationCityEdit          = 0;
        locationEdit              = 0;

        contactCityEdit           = 0;
        contactCountryEdit        = 0;
        contactAddressEdit        = 0;
        contactPostalCodeEdit     = 0;
        contactProvinceStateEdit  = 0;
        contactEmailEdit          = 0;
        contactPhoneEdit          = 0;
        contactWebUrlEdit         = 0;
    }

    // Rights template informations panel.
    KLineEdit      *authorsEdit;
    KLineEdit      *authorsPositionEdit;
    KLineEdit      *creditEdit;
    KLineEdit      *sourceEdit;
    KLineEdit      *instructionsEdit;

    AltLangStrEdit *copyrightEdit;
    AltLangStrEdit *rightUsageEdit;

    // Location template informations panel.
    KLineEdit      *locationCountryEdit;
    KLineEdit      *locationCountryCodeEdit;
    KLineEdit      *locationProvinceStateEdit;
    KLineEdit      *locationCityEdit;
    KLineEdit      *locationEdit;

    // Contact template informations panel.
    KLineEdit      *contactCityEdit;
    KLineEdit      *contactCountryEdit;
    KLineEdit      *contactAddressEdit;
    KLineEdit      *contactPostalCodeEdit;
    KLineEdit      *contactProvinceStateEdit;
    KLineEdit      *contactEmailEdit;
    KLineEdit      *contactPhoneEdit;
    KLineEdit      *contactWebUrlEdit;
};

TemplatePanel::TemplatePanel(QWidget* parent)
             : KTabWidget(parent), d(new TemplatePanelPriv)
{

    // -- Rights Template informations panel -------------------------------------------------------------

    QWidget *page1     = new QWidget(this);
    QGridLayout* grid1 = new QGridLayout(page1);

    QLabel *label1 = new QLabel(i18n("Author Names:"), page1);
    d->authorsEdit = new KLineEdit(page1);
    d->authorsEdit->setClearButtonShown(true);
    d->authorsEdit->setClickMessage(i18n("Enter here all creator name. Use semi-colons as separator."));
    label1->setBuddy(d->authorsEdit);
    d->authorsEdit->setWhatsThis(i18n("<p>This field should contain names of the persons who created the photograph. "
                                      "If it is not appropriate to add the name of the photographer (for example, if the identify of "
                                      "the photographer needs to be protected) the name of a company or organization can also be used. "
                                      "Once saved, this field should not be changed by anyone. "
                                      "<p>To enter more than one name, use <b>semi-colons as separators</b>.</p>"
                                      "<p>With IPTC, this field is limited to 32 ASCII characters.</p>"));

    // --------------------------------------------------------

    QLabel *label2         = new QLabel(i18n("Authors Position:"), page1);
    d->authorsPositionEdit = new KLineEdit(page1);
    d->authorsPositionEdit->setClearButtonShown(true);
    d->authorsPositionEdit->setClickMessage(i18n("Enter here the job title of authors."));
    label2->setBuddy(d->authorsPositionEdit);
    d->authorsPositionEdit->setWhatsThis(i18n("<p>This field should contain the job title of authors. Examples might include "
                                              "titles such as: Staff Photographer, Freelance Photographer, or Independent Commercial "
                                              "Photographer. Since this is a qualifier for the Author field, the Author field must also "
                                              "be filled out.</p>"
                                              "<p>With IPTC, this field is limited to 32 ASCII characters.</p>"));

    // --------------------------------------------------------

    QLabel *label3 = new QLabel(i18n("Credit:"), page1);
    d->creditEdit  = new KLineEdit(page1);
    d->creditEdit->setClearButtonShown(true);
    d->creditEdit->setClickMessage(i18n("Enter here the photograph credit."));
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
    d->rightUsageEdit->setWhatsThis(i18n("<p>The Right Usage Terms field should be used to list instructions on how "
                                         "a resource can be legally used."
                                         "<p>With XMP, you can include more than one right usage terms string using "
                                         "different languages.</p>"
                                         "<p>This field do not exist with IPTC.</p>"));

    // --------------------------------------------------------

    QLabel *label6 = new QLabel(i18n("Source:"), page1);
    d->sourceEdit  = new KLineEdit(page1);
    d->sourceEdit->setClearButtonShown(true);
    d->sourceEdit->setClickMessage(i18n("Enter here original owner of the photograph."));
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

    QLabel *label7      = new QLabel(i18n("Instructions:"), page1);
    d->instructionsEdit = new KLineEdit(page1);
    d->instructionsEdit->setClearButtonShown(true);
    d->instructionsEdit->setClickMessage(i18n("Enter here the editorial notice."));
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

    insertTab(TemplatePanelPriv::RIGHTS, page1, KIcon("flag-red"), i18n("Rights"));

    // -- Location Template informations panel -------------------------------------------------------------

    QWidget *page2     = new QWidget(this);
    QGridLayout* grid2 = new QGridLayout(page2);

    QLabel *label8         = new QLabel(i18n("Country Name:"), page2);
    d->locationCountryEdit = new KLineEdit(page2);
    d->locationCountryEdit->setClearButtonShown(true);
    d->locationCountryEdit->setClickMessage(i18n("Enter here country name of contents."));
    label8->setBuddy(d->locationCountryEdit);
    d->locationCountryEdit->setWhatsThis(i18n("<p>This field should contain country name "
                                              "where have been taken the photograph.</p>"));

    // --------------------------------------------------------

    QLabel *label9             = new QLabel(i18n("Country Code:"), page2);
    d->locationCountryCodeEdit = new KLineEdit(page2);
    d->locationCountryCodeEdit->setClearButtonShown(true);
    d->locationCountryCodeEdit->setClickMessage(i18n("Enter here country code of contents."));
    label9->setBuddy(d->locationCountryCodeEdit);
    d->locationCountryCodeEdit->setWhatsThis(i18n("<p>This field should contain country code "
                                                  "where have been taken the photograph.</p>"));

    // --------------------------------------------------------

    QLabel *label10              = new QLabel(i18n("Province State:"), page2);
    d->locationProvinceStateEdit = new KLineEdit(page2);
    d->locationProvinceStateEdit->setClearButtonShown(true);
    d->locationProvinceStateEdit->setClickMessage(i18n("Enter here province state of contents."));
    label10->setBuddy(d->locationProvinceStateEdit);
    d->locationProvinceStateEdit->setWhatsThis(i18n("<p>This field should contain province state "
                                                    "where have been taken the photograph.</p>"));

    // --------------------------------------------------------

    QLabel *label11 = new QLabel(i18n("City:"), page2);
    d->locationCityEdit = new KLineEdit(page2);
    d->locationCityEdit->setClearButtonShown(true);
    d->locationCityEdit->setClickMessage(i18n("Enter here city of contents."));
    label11->setBuddy(d->locationCityEdit);
    d->locationCityEdit->setWhatsThis(i18n("<p>This field should contain city name "
                                           "where have been taken the photograph.</p>"));

    // --------------------------------------------------------

    QLabel *label12 = new QLabel(i18n("Sublocation:"), page2);
    d->locationEdit = new KLineEdit(page2);
    d->locationEdit->setClearButtonShown(true);
    d->locationEdit->setClickMessage(i18n("Enter here sublocation place of contents."));
    label12->setBuddy(d->locationEdit);
    d->locationEdit->setWhatsThis(i18n("<p>This field should contain sublocation from the city "
                                       "where have been taken the photograph.</p>"));

    // --------------------------------------------------------

    grid2->setMargin(KDialog::spacingHint());
    grid2->setSpacing(KDialog::spacingHint());
    grid2->setAlignment(Qt::AlignTop);
    grid2->setColumnStretch(1, 10);
    grid2->setRowStretch(5, 10);
    grid2->addWidget(label8,                       0, 0, 1, 1);
    grid2->addWidget(d->locationCountryEdit,       0, 1, 1, 2);
    grid2->addWidget(label9,                       1, 0, 1, 1);
    grid2->addWidget(d->locationCountryCodeEdit,   1, 1, 1, 2);
    grid2->addWidget(label10,                      2, 0, 1, 1);
    grid2->addWidget(d->locationProvinceStateEdit, 2, 1, 1, 2);
    grid2->addWidget(label11,                      3, 0, 1, 1);
    grid2->addWidget(d->locationCityEdit,          3, 1, 1, 2);
    grid2->addWidget(label12,                      4, 0, 1, 1);
    grid2->addWidget(d->locationEdit,              4, 1, 1, 2);

    insertTab(TemplatePanelPriv::LOCATION, page2, KIcon("applications-internet"), i18n("Location"));

    // -- Contact Template informations panel -------------------------------------------------------------

    QWidget *page3     = new QWidget(this);
    QGridLayout* grid3 = new QGridLayout(page3);

    QLabel *label13    = new QLabel(i18n("City:"), page3);
    d->contactCityEdit = new KLineEdit(page3);
    d->contactCityEdit->setClearButtonShown(true);
    d->contactCityEdit->setClickMessage(i18n("Enter here city name of lead author."));
    label13->setBuddy(d->contactCityEdit);
    d->contactCityEdit->setWhatsThis(i18n("<p>This field should contain city name "
                                          "where lead author live.</p>"));

    // --------------------------------------------------------

    QLabel *label14       = new QLabel(i18n("Country:"), page3);
    d->contactCountryEdit = new KLineEdit(page3);
    d->contactCountryEdit->setClearButtonShown(true);
    d->contactCountryEdit->setClickMessage(i18n("Enter here country name of lead author."));
    label14->setBuddy(d->contactCountryEdit);
    d->contactCountryEdit->setWhatsThis(i18n("<p>This field should contain country name "
                                             "where lead author live.</p>"));

    // --------------------------------------------------------

    QLabel *label15       = new QLabel(i18n("Address:"), page3);
    d->contactAddressEdit = new KLineEdit(page3);
    d->contactAddressEdit->setClearButtonShown(true);
    d->contactAddressEdit->setClickMessage(i18n("Enter here address of lead author."));
    label15->setBuddy(d->contactAddressEdit);
    d->contactAddressEdit->setWhatsThis(i18n("<p>This field should contain address "
                                             "where lead author live.</p>"));

    // --------------------------------------------------------

    QLabel *label16          = new QLabel(i18n("Postal Code:"), page3);
    d->contactPostalCodeEdit = new KLineEdit(page3);
    d->contactPostalCodeEdit->setClearButtonShown(true);
    d->contactPostalCodeEdit->setClickMessage(i18n("Enter here postal code of lead author."));
    label16->setBuddy(d->contactPostalCodeEdit);
    d->contactPostalCodeEdit->setWhatsThis(i18n("<p>This field should contain postal code "
                                                "where lead author live.</p>"));

    // --------------------------------------------------------

    QLabel *label17             = new QLabel(i18n("Province:"), page3);
    d->contactProvinceStateEdit = new KLineEdit(page3);
    d->contactProvinceStateEdit->setClearButtonShown(true);
    d->contactProvinceStateEdit->setClickMessage(i18n("Enter here province of lead author."));
    label17->setBuddy(d->contactProvinceStateEdit);
    d->contactProvinceStateEdit->setWhatsThis(i18n("<p>This field should contain province "
                                                   "where lead author live.</p>"));

    // --------------------------------------------------------

    QLabel *label18     = new QLabel(i18n("Email:"), page3);
    d->contactEmailEdit = new KLineEdit(page3);
    d->contactEmailEdit->setClearButtonShown(true);
    d->contactEmailEdit->setClickMessage(i18n("Enter here email of lead author."));
    label18->setBuddy(d->contactEmailEdit);
    d->contactEmailEdit->setWhatsThis(i18n("<p>This field should contain email "
                                           "of lead author.</p>"));

    // --------------------------------------------------------

    QLabel *label19     = new QLabel(i18n("Phone:"), page3);
    d->contactPhoneEdit = new KLineEdit(page3);
    d->contactPhoneEdit->setClearButtonShown(true);
    d->contactPhoneEdit->setClickMessage(i18n("Enter here phone number of lead author."));
    label19->setBuddy(d->contactPhoneEdit);
    d->contactPhoneEdit->setWhatsThis(i18n("<p>This field should contain phone number "
                                           "of lead author.</p>"));

    // --------------------------------------------------------

    QLabel *label20      = new QLabel(i18n("Url:"), page3);
    d->contactWebUrlEdit = new KLineEdit(page3);
    d->contactWebUrlEdit->setClearButtonShown(true);
    d->contactWebUrlEdit->setClickMessage(i18n("Enter here Web site Url of lead author."));
    label20->setBuddy(d->contactWebUrlEdit);
    d->contactWebUrlEdit->setWhatsThis(i18n("<p>This field should contain web site Url "
                                            "of lead author.</p>"));

    // --------------------------------------------------------

    grid3->setMargin(KDialog::spacingHint());
    grid3->setSpacing(KDialog::spacingHint());
    grid3->setAlignment(Qt::AlignTop);
    grid3->setColumnStretch(1, 10);
    grid3->setRowStretch(8, 10);
    grid3->addWidget(label13,                     0, 0, 1, 1);
    grid3->addWidget(d->contactCityEdit,          0, 1, 1, 2);
    grid3->addWidget(label14,                     1, 0, 1, 1);
    grid3->addWidget(d->contactCountryEdit,       1, 1, 1, 2);
    grid3->addWidget(label15,                     2, 0, 1, 1);
    grid3->addWidget(d->contactAddressEdit,       2, 1, 1, 2);
    grid3->addWidget(label16,                     3, 0, 1, 1);
    grid3->addWidget(d->contactPostalCodeEdit,    3, 1, 1, 2);
    grid3->addWidget(label17,                     4, 0, 1, 1);
    grid3->addWidget(d->contactProvinceStateEdit, 4, 1, 1, 2);
    grid3->addWidget(label18,                     5, 0, 1, 1);
    grid3->addWidget(d->contactEmailEdit,         5, 1, 1, 2);
    grid3->addWidget(label19,                     6, 0, 1, 1);
    grid3->addWidget(d->contactPhoneEdit,         6, 1, 1, 2);
    grid3->addWidget(label20,                     7, 0, 1, 1);
    grid3->addWidget(d->contactWebUrlEdit,        7, 1, 1, 2);

    insertTab(TemplatePanelPriv::CONTACT, page3, KIcon("view-pim-contacts"), i18n("Contact"));
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

    d->locationCountryEdit->setText(t.locationInfo().country);
    d->locationCountryCodeEdit->setText(t.locationInfo().countryCode);
    d->locationProvinceStateEdit->setText(t.locationInfo().provinceState);
    d->locationCityEdit->setText(t.locationInfo().city);
    d->locationEdit->setText(t.locationInfo().location);

    d->contactCityEdit->setText(t.contactInfo().city);
    d->contactCountryEdit->setText(t.contactInfo().country);
    d->contactAddressEdit->setText(t.contactInfo().address);
    d->contactPostalCodeEdit->setText(t.contactInfo().postalCode);
    d->contactProvinceStateEdit->setText(t.contactInfo().provinceState);
    d->contactEmailEdit->setText(t.contactInfo().email);
    d->contactPhoneEdit->setText(t.contactInfo().phone);
    d->contactWebUrlEdit->setText(t.contactInfo().webUrl);
}

Template TemplatePanel::getTemplate() const
{
    Template t;
    t.setAuthors(d->authorsEdit->text().split(";", QString::SkipEmptyParts));
    t.setAuthorsPosition(d->authorsPositionEdit->text());
    t.setCredit(d->creditEdit->text());
    t.setCopyright(d->copyrightEdit->values());
    t.setRightUsageTerms(d->rightUsageEdit->values());
    t.setSource(d->sourceEdit->text());
    t.setInstructions(d->instructionsEdit->text());

    IptcCoreLocationInfo inf1;
    inf1.country       = d->locationCountryEdit->text();
    inf1.countryCode   = d->locationCountryCodeEdit->text();
    inf1.provinceState = d->locationProvinceStateEdit->text();
    inf1.city          = d->locationCityEdit->text();
    inf1.location      = d->locationEdit->text();
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

    return t;
}

void TemplatePanel::apply()
{
    d->copyrightEdit->apply();
    d->rightUsageEdit->apply();
}

}  // namespace Digikam
