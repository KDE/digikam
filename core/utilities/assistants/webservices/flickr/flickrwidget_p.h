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

namespace Digikam
{

class FlickrWidget::Private
{
public:

    Private()
    {
        removeAccount             = 0;
        extendedTagsButton        = 0;
        extendedPublicationButton = 0;
        exportHostTagsCheckBox    = 0;
        stripSpaceTagsCheckBox    = 0;
        addExtraTagsCheckBox      = 0;
        familyCheckBox            = 0;
        friendsCheckBox           = 0;
        publicCheckBox            = 0;
        extendedTagsBox           = 0;
        extendedPublicationBox    = 0;
        tagsLineEdit              = 0;
        contentTypeComboBox       = 0;
        safetyLevelComboBox       = 0;
        imglst                    = 0;
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

} // namespace Digikam
