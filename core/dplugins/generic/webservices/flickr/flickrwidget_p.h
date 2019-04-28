/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-07-07
 * Description : a tool to export images to Flickr web service
 *
 * Copyright (C) 2005-2008 by Vardhman Jain <vardhman at gmail dot com>
 * Copyright (C) 2008-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_FLICKR_WIDGET_PRIVATE_H
#define DIGIKAM_FLICKR_WIDGET_PRIVATE_H

#include "flickrwidget.h"

// Qt includes

#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QApplication>
#include <QStyle>
#include <QDialog>

// Local includes

#include "wscomboboxdelegate.h"
#include "wscomboboxintermediate.h"
#include "flickrlist.h"

using namespace Digikam;

namespace DigikamGenericFlickrPlugin
{

class Q_DECL_HIDDEN FlickrWidget::Private
{
public:

    explicit Private()
    {
        removeAccount             = nullptr;
        extendedTagsButton        = nullptr;
        extendedPublicationButton = nullptr;
        exportHostTagsCheckBox    = nullptr;
        stripSpaceTagsCheckBox    = nullptr;
        addExtraTagsCheckBox      = nullptr;
        familyCheckBox            = nullptr;
        friendsCheckBox           = nullptr;
        publicCheckBox            = nullptr;
        extendedTagsBox           = nullptr;
        extendedPublicationBox    = nullptr;
        tagsLineEdit              = nullptr;
        contentTypeComboBox       = nullptr;
        safetyLevelComboBox       = nullptr;
        imglst                    = nullptr;
    }

    QString                 serviceName;
    QPushButton*            removeAccount;
    QPushButton*            extendedTagsButton;
    QPushButton*            extendedPublicationButton;

    QCheckBox*              exportHostTagsCheckBox;
    QCheckBox*              stripSpaceTagsCheckBox;
    QCheckBox*              addExtraTagsCheckBox;
    QCheckBox*              familyCheckBox;
    QCheckBox*              friendsCheckBox;
    QCheckBox*              publicCheckBox;

    QGroupBox*              extendedTagsBox;
    QGroupBox*              extendedPublicationBox;

    QLineEdit*              tagsLineEdit;

    WSComboBoxIntermediate* contentTypeComboBox;
    WSComboBoxIntermediate* safetyLevelComboBox;

    FlickrList*             imglst;
};

} // namespace DigikamGenericFlickrPlugin

#endif // DIGIKAM_FLICKR_WIDGET_PRIVATE_H
