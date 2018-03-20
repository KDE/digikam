/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-07
 * Description : a tool to export images to Flickr web service
 *
 * Copyright (C) 2005-2008 by Vardhman Jain <vardhman at gmail dot com>
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009      by Luka Renko <lure at kubuntu dot org>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "flickrwidget_p.h"

namespace Digikam
{

FlickrWidget::FlickrWidget(QWidget* const parent,
                           DInfoInterface* const iface,
                           const QString& serviceName)
    : WSSettingsWidget(parent, iface, serviceName),
      d(new Private)
{
    d->serviceName = serviceName;

    //Adding Remove Account button
    d->removeAccount              = new QPushButton(getAccountBox());
    d->removeAccount->setText(i18n("Remove Account"));
    getAccountBoxLayout()->addWidget(d->removeAccount, 2, 0, 1, 4);

    // -- The image list --------------------------------------------------

    d->imglst               = new FlickrList(this, (serviceName == QString::fromLatin1("23")));

    // For figuring out the width of the permission columns.
    QHeaderView* const hdr = d->imglst->listView()->header();
    QFontMetrics hdrFont   = QFontMetrics(hdr->font());
    int permColWidth       = hdrFont.width(i18nc("photo permissions", "Public"));

    d->imglst->setAllowRAW(true);
    d->imglst->setIface(iface);
    d->imglst->loadImagesFromCurrentSelection();

    d->imglst->listView()->setWhatsThis(i18n("This is the list of images to upload to your Flickr account."));
    d->imglst->listView()->setColumn(static_cast<DImagesListView::ColumnType>(FlickrList::PUBLIC), i18nc("photo permissions", "Public"), true);

    // Handle extra tags per image.
    d->imglst->listView()->setColumn(static_cast<DImagesListView::ColumnType>(FlickrList::TAGS),
                                    i18n("Extra tags"), true);

    if (serviceName != QString::fromLatin1("23"))
    {
        int tmpWidth;

        if ((tmpWidth = hdrFont.width(i18nc("photo permissions", "Family"))) > permColWidth)
        {
            permColWidth = tmpWidth;
        }

        if ((tmpWidth = hdrFont.width(i18nc("photo permissions", "Friends"))) > permColWidth)
        {
            permColWidth = tmpWidth;
        }

        d->imglst->listView()->setColumn(static_cast<DImagesListView::ColumnType>(FlickrList::FAMILY),
                                        i18nc("photo permissions", "Family"), true);
        d->imglst->listView()->setColumn(static_cast<DImagesListView::ColumnType>(FlickrList::FRIENDS),
                                        i18nc("photo permissions", "Friends"), true);
        hdr->setSectionResizeMode(FlickrList::FAMILY,  QHeaderView::Interactive);
        hdr->setSectionResizeMode(FlickrList::FRIENDS, QHeaderView::Interactive);
        hdr->resizeSection(FlickrList::FAMILY,  permColWidth);
        hdr->resizeSection(FlickrList::FRIENDS, permColWidth);

        d->imglst->listView()->setColumn(static_cast<DImagesListView::ColumnType>(FlickrList::SAFETYLEVEL),
                                        i18n("Safety level"), true);
        d->imglst->listView()->setColumn(static_cast<DImagesListView::ColumnType>(FlickrList::CONTENTTYPE),
                                        i18n("Content type"), true);
        QMap<int, QString> safetyLevelItems;
        QMap<int, QString> contentTypeItems;
        safetyLevelItems[FlickrList::SAFE]       = i18nc("photo safety level", "Safe");
        safetyLevelItems[FlickrList::MODERATE]   = i18nc("photo safety level", "Moderate");
        safetyLevelItems[FlickrList::RESTRICTED] = i18nc("photo safety level", "Restricted");
        contentTypeItems[FlickrList::PHOTO]      = i18nc("photo content type", "Photo");
        contentTypeItems[FlickrList::SCREENSHOT] = i18nc("photo content type", "Screenshot");
        contentTypeItems[FlickrList::OTHER]      = i18nc("photo content type", "Other");
        ComboBoxDelegate* const safetyLevelDelegate = new ComboBoxDelegate(d->imglst, safetyLevelItems);
        ComboBoxDelegate* const contentTypeDelegate = new ComboBoxDelegate(d->imglst, contentTypeItems);
        d->imglst->listView()->setItemDelegateForColumn(static_cast<DImagesListView::ColumnType>(FlickrList::SAFETYLEVEL), safetyLevelDelegate);
        d->imglst->listView()->setItemDelegateForColumn(static_cast<DImagesListView::ColumnType>(FlickrList::CONTENTTYPE), contentTypeDelegate);
    }

    hdr->setSectionResizeMode(FlickrList::PUBLIC, QHeaderView::Interactive);
    hdr->resizeSection(FlickrList::PUBLIC, permColWidth);

    // -- Show upload original image CheckBox----------------------------------

    getOriginalCheckBox()->show();

    // -- Layout for the tags -------------------------------------------------

    QGroupBox* const tagsBox         = new QGroupBox(i18n("Tag options"), getSettingsBox());
    QGridLayout* const tagsBoxLayout = new QGridLayout(tagsBox);

    d->exportHostTagsCheckBox         = new QCheckBox(tagsBox);
    d->exportHostTagsCheckBox->setText(i18n("Use Host Application Tags"));

    d->extendedTagsButton             = new QPushButton(i18n("More tag options"));
    d->extendedTagsButton->setCheckable(true);
    // Initialize this button to checked, so extended options are shown.
    // FlickrWindow::readSettings can change this, but if checked is false it
    // cannot uncheck and subsequently hide the extended options (the toggled
    // signal won't be emitted).
    d->extendedTagsButton->setChecked(true);
    d->extendedTagsButton->setSizePolicy(QSizePolicy::Maximum,
                                        QSizePolicy::Preferred);

    d->extendedTagsBox               = new QGroupBox(QString::fromLatin1(""), getSettingsBox());
    d->extendedTagsBox->setFlat(true);
    QGridLayout* extendedTagsLayout = new QGridLayout(d->extendedTagsBox);

    QLabel* const tagsLabel         = new QLabel(i18n("Added Tags: "), d->extendedTagsBox);
    d->tagsLineEdit                  = new QLineEdit(d->extendedTagsBox);
    d->tagsLineEdit->setToolTip(i18n("Enter new tags here, separated by commas."));
    d->addExtraTagsCheckBox          = new QCheckBox(d->extendedTagsBox);
    d->addExtraTagsCheckBox->setText(i18n("Add tags per image"));
    d->addExtraTagsCheckBox->setToolTip(i18n("If checked, you can set extra tags for "
                                            "each image in the File List tab"));
    d->addExtraTagsCheckBox->setChecked(true);
    d->stripSpaceTagsCheckBox        = new QCheckBox(d->extendedTagsBox);
    d->stripSpaceTagsCheckBox->setText(i18n("Strip Spaces From Tags"));

    extendedTagsLayout->addWidget(tagsLabel,                0, 0);
    extendedTagsLayout->addWidget(d->tagsLineEdit,           0, 1);
    extendedTagsLayout->addWidget(d->stripSpaceTagsCheckBox, 1, 0, 1, 2);
    extendedTagsLayout->addWidget(d->addExtraTagsCheckBox,   2, 0, 1, 2);

    tagsBoxLayout->addWidget(d->exportHostTagsCheckBox, 0, 0);
    tagsBoxLayout->addWidget(d->extendedTagsButton,     0, 1);
    tagsBoxLayout->addWidget(d->extendedTagsBox,        1, 0, 1, 2);

    // -- Layout for the publication options ----------------------------------

    QGroupBox* const publicationBox         = new QGroupBox(i18n("Publication Options"), getSettingsBox());
    QGridLayout* const publicationBoxLayout = new QGridLayout;
    publicationBox->setLayout(publicationBoxLayout);

    d->publicCheckBox = new QCheckBox(publicationBox);
    d->publicCheckBox->setText(i18nc("As in accessible for people", "Public (anyone can see them)"));

    d->familyCheckBox = new QCheckBox(publicationBox);
    d->familyCheckBox->setText(i18n("Visible to Family"));

    d->friendsCheckBox = new QCheckBox(publicationBox);
    d->friendsCheckBox->setText(i18n("Visible to Friends"));

    // Extended publication settings
    d->extendedPublicationButton = new QPushButton(i18n("More publication options"));
    d->extendedPublicationButton->setCheckable(true);
    // Initialize this button to checked, so extended options are shown.
    // FlickrWindow::readSettings can change this, but if checked is false it
    // cannot uncheck and subsequently hide the extended options (the toggled
    // signal won't be emitted).
    d->extendedPublicationButton->setChecked(true);
    d->extendedPublicationButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    d->extendedPublicationBox                  = new QGroupBox(QString::fromLatin1(""), publicationBox);
    d->extendedPublicationBox->setFlat(true);
    QGridLayout* const extendedSettingsLayout = new QGridLayout(d->extendedPublicationBox);

    QLabel* const imageSafetyLabel = new QLabel(i18n("Safety level:"));
    d->safetyLevelComboBox          = new WSComboBoxIntermediate();
    d->safetyLevelComboBox->addItem(i18n("Safe"),       QVariant(FlickrList::SAFE));
    d->safetyLevelComboBox->addItem(i18n("Moderate"),   QVariant(FlickrList::MODERATE));
    d->safetyLevelComboBox->addItem(i18n("Restricted"), QVariant(FlickrList::RESTRICTED));

    QLabel* const imageTypeLabel = new QLabel(i18n("Content type:"));
    d->contentTypeComboBox        = new WSComboBoxIntermediate();
    d->contentTypeComboBox->addItem(i18nc("photo content type", "Photo"),      QVariant(FlickrList::PHOTO));
    d->contentTypeComboBox->addItem(i18nc("photo content type", "Screenshot"), QVariant(FlickrList::SCREENSHOT));
    d->contentTypeComboBox->addItem(i18nc("photo content type", "Other"),      QVariant(FlickrList::OTHER));

    extendedSettingsLayout->addWidget(imageSafetyLabel,      1, 0, Qt::AlignLeft);
    extendedSettingsLayout->addWidget(d->safetyLevelComboBox, 1, 1, Qt::AlignLeft);
    extendedSettingsLayout->addWidget(imageTypeLabel,        0, 0, Qt::AlignLeft);
    extendedSettingsLayout->addWidget(d->contentTypeComboBox, 0, 1, Qt::AlignLeft);
    extendedSettingsLayout->setColumnStretch(0, 0);
    extendedSettingsLayout->setColumnStretch(1, 1);

    publicationBoxLayout->addWidget(d->publicCheckBox,            0, 0);
    publicationBoxLayout->addWidget(d->familyCheckBox,            1, 0);
    publicationBoxLayout->addWidget(d->friendsCheckBox,           2, 0);
    publicationBoxLayout->addWidget(d->extendedPublicationButton, 2, 1);
    publicationBoxLayout->addWidget(d->extendedPublicationBox,    3, 0, 1, 2);

    // -- Add these extra widgets to settings box -------------------------------------------------

    addWidgetToSettingsBox(publicationBox);
    addWidgetToSettingsBox(tagsBox);

    // hiding widgets not required.
    getUploadBox()->hide();
    getSizeBox()->hide();

    // Removing DImagesList inherited from WSSettingsWidget and replacing it with more specific FlickrList
    replaceImageList(d->imglst);

    updateLabels();

    connect(d->imglst, SIGNAL(signalPermissionChanged(FlickrList::FieldType,Qt::CheckState)),
            this, SLOT(slotPermissionChanged(FlickrList::FieldType,Qt::CheckState)));

    connect(d->publicCheckBox, SIGNAL(stateChanged(int)),
            this, SLOT(slotMainPublicToggled(int)));

    connect(d->extendedTagsButton, SIGNAL(toggled(bool)),
            this, SLOT(slotExtendedTagsToggled(bool)));

    connect(d->addExtraTagsCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotAddExtraTagsToggled(bool)));

    // 23HQ doesn't support the Family and Friends concept.
    if (serviceName != QString::fromLatin1("23"))
    {
        connect(d->familyCheckBox, SIGNAL(stateChanged(int)),
                this, SLOT(slotMainFamilyToggled(int)));

        connect(d->friendsCheckBox, SIGNAL(stateChanged(int)),
                this, SLOT(slotMainFriendsToggled(int)));
    }
    else
    {
        d->familyCheckBox->hide();
        d->friendsCheckBox->hide();
    }

    // 23HQ don't support the Safety Level and Content Type concept.
    if (serviceName != QString::fromLatin1("23"))
    {
        connect(d->safetyLevelComboBox, SIGNAL(currentIndexChanged(int)),
                this, SLOT(slotMainSafetyLevelChanged(int)));

        connect(d->contentTypeComboBox, SIGNAL(currentIndexChanged(int)),
                this, SLOT(slotMainContentTypeChanged(int)));

        connect(d->extendedPublicationButton, SIGNAL(toggled(bool)),
                this, SLOT(slotExtendedPublicationToggled(bool)));

        connect(d->imglst, SIGNAL(signalSafetyLevelChanged(FlickrList::SafetyLevel)),
                this, SLOT(slotSafetyLevelChanged(FlickrList::SafetyLevel)));

        connect(d->imglst, SIGNAL(signalContentTypeChanged(FlickrList::ContentType)),
                this, SLOT(slotContentTypeChanged(FlickrList::ContentType)));
    }
    else
    {
        d->extendedPublicationBox->hide();
        d->extendedPublicationButton->hide();
        d->imglst->listView()->setColumnEnabled(static_cast<DImagesListView::ColumnType>(FlickrList::SAFETYLEVEL), false);
        d->imglst->listView()->setColumnEnabled(static_cast<DImagesListView::ColumnType>(FlickrList::CONTENTTYPE), false);
    }
}

FlickrWidget::~FlickrWidget()
{
    delete d;
}

void FlickrWidget::updateLabels(const QString& /*name*/, const QString& /*url*/)
{
    if (d->serviceName == QString::fromLatin1("23"))
    {
        getHeaderLbl()->setText(i18n("<b><h2><a href='http://www.23hq.com'>"
                                  "<font color=\"#7CD164\">23</font></a>"
                                  " Export"
                                  "</h2></b>"));
    }
    else
    {
        getHeaderLbl()->setText(i18n("<b><h2><a href='http://www.flickr.com'>"
                                  "<font color=\"#0065DE\">flick</font>"
                                  "<font color=\"#FF0084\">r</font></a>"
                                  " Export"
                                  "</h2></b>"));
    }
}

void FlickrWidget::slotPermissionChanged(FlickrList::FieldType checkbox, Qt::CheckState state)
{
    /* Slot for handling the signal from the FlickrList that the general
     * permissions have changed, considering the clicks in the checkboxes next
     * to each image. In response, the main permission checkboxes should be set
     * to the proper state.
     * The checkbox variable determines which of the checkboxes should be
     * changed. */

    // Select the proper checkbox.
    QCheckBox* currBox = 0;

    if (checkbox == FlickrList::PUBLIC)
    {
        currBox = d->publicCheckBox;
    }
    else if (checkbox == FlickrList::FAMILY)
    {
        currBox = d->familyCheckBox;
    }
    else
    {
        currBox = d->friendsCheckBox;
    }

    // If the checkbox should be set in the intermediate state, the tristate
    // property of the checkbox should be manually set to true, otherwise, it
    // has to be set to false so that the user cannot select it.
    currBox->setCheckState(state);

    if ((state == Qt::Checked) || (state == Qt::Unchecked))
    {
        currBox->setTristate(false);
    }
    else
    {
        currBox->setTristate(true);
    }
}

void FlickrWidget::slotSafetyLevelChanged(FlickrList::SafetyLevel safetyLevel)
{
    /* Called when the general safety level of the FlickrList has changed,
     * considering the individual comboboxes next to the photos. Used to set the
     * main safety level combobox appropriately. */
    if (safetyLevel == FlickrList::MIXEDLEVELS)
    {
        d->safetyLevelComboBox->setIntermediate(true);
    }
    else
    {
        int index = d->safetyLevelComboBox->findData(QVariant(static_cast<int>(safetyLevel)));
        d->safetyLevelComboBox->setCurrentIndex(index);
    }
}

void FlickrWidget::slotContentTypeChanged(FlickrList::ContentType contentType)
{
    /* Called when the general content type of the FlickrList has changed,
     * considering the individual comboboxes next to the photos. Used to set the
     * main content type combobox appropriately. */
    if (contentType == FlickrList::MIXEDTYPES)
    {
        d->contentTypeComboBox->setIntermediate(true);
    }
    else
    {
        int index = d->contentTypeComboBox->findData(QVariant(static_cast<int>(contentType)));
        d->contentTypeComboBox->setCurrentIndex(index);
    }
}

void FlickrWidget::slotMainPublicToggled(int state)
{
    mainPermissionToggled(FlickrList::PUBLIC, static_cast<Qt::CheckState>(state));
}

void FlickrWidget::slotMainFamilyToggled(int state)
{
    mainPermissionToggled(FlickrList::FAMILY, static_cast<Qt::CheckState>(state));
}

void FlickrWidget::slotMainFriendsToggled(int state)
{
    mainPermissionToggled(FlickrList::FRIENDS, static_cast<Qt::CheckState>(state));
}

void FlickrWidget::mainPermissionToggled(FlickrList::FieldType checkbox, Qt::CheckState state)
{
    /* Callback for when one of the main permission checkboxes is toggled.
     * checkbox specifies which of the checkboxes is toggled. */

    if (state != Qt::PartiallyChecked)
    {
        // Set the states for the image list.
        if (checkbox == FlickrList::PUBLIC)
        {
            d->imglst->setPublic(state);
        }
        else if (checkbox == FlickrList::FAMILY)
        {
            d->imglst->setFamily(state);
        }
        else if (checkbox == FlickrList::FRIENDS)
        {
            d->imglst->setFriends(state);
        }

        // Dis- or enable the family and friends checkboxes if the public
        // checkbox is clicked.
        if (checkbox == 0)
        {
            if (state == Qt::Checked)
            {
                d->familyCheckBox->setEnabled(false);
                d->friendsCheckBox->setEnabled(false);
            }
            else if (state == Qt::Unchecked)
            {
                d->familyCheckBox->setEnabled(true);
                d->friendsCheckBox->setEnabled(true);
            }
        }

        // Set the main checkbox tristate state to false, so that the user
        // cannot select the intermediate state.
        if (checkbox == FlickrList::PUBLIC)
        {
            d->publicCheckBox->setTristate(false);
        }
        else if (checkbox == FlickrList::FAMILY)
        {
            d->familyCheckBox->setTristate(false);
        }
        else if (checkbox == FlickrList::FRIENDS)
        {
            d->friendsCheckBox->setTristate(false);
        }
    }
}

void FlickrWidget::slotMainSafetyLevelChanged(int index)
{
    int currValue = (d->safetyLevelComboBox->itemData(index)).value<int>();
//     int currValue = qVariantValue<int>(d->safetyLevelComboBox->itemData(index));
    d->imglst->setSafetyLevels(static_cast<FlickrList::SafetyLevel>(currValue));
}

void FlickrWidget::slotMainContentTypeChanged(int index)
{
    int currValue = (d->contentTypeComboBox->itemData(index)).value<int>();
//     int currValue = qVariantValue<int>(d->contentTypeComboBox->itemData(index));
    d->imglst->setContentTypes(static_cast<FlickrList::ContentType>(currValue));
}

void FlickrWidget::slotExtendedPublicationToggled(bool status)
{
    // Show or hide the extended settings when the extended settings button
    // is toggled.
    d->extendedPublicationBox->setVisible(status);
    d->imglst->listView()->setColumnHidden(FlickrList::SAFETYLEVEL, !status);
    d->imglst->listView()->setColumnHidden(FlickrList::CONTENTTYPE, !status);

    if (status)
    {
        d->extendedPublicationButton->setText(i18n("Fewer publication options"));
    }
    else
    {
        d->extendedPublicationButton->setText(i18n("More publication options"));
    }
}

void FlickrWidget::slotExtendedTagsToggled(bool status)
{
    // Show or hide the extended tag settings when the extended tag option
    // button is toggled.
    d->extendedTagsBox->setVisible(status);

    if (!status)
    {
        d->imglst->listView()->setColumnHidden(FlickrList::TAGS, true);
        d->extendedTagsButton->setText(i18n("More tag options"));
    }
    else
    {
        d->imglst->listView()->setColumnHidden(FlickrList::TAGS, !d->addExtraTagsCheckBox->isChecked());
        d->extendedTagsButton->setText(i18n("Fewer tag options"));
    }
}

void FlickrWidget::slotAddExtraTagsToggled(bool status)
{
    if (d->extendedTagsButton->isChecked())
    {
        d->imglst->listView()->setColumnHidden(FlickrList::TAGS, !status);
    }
}

} // namespace Digikam
