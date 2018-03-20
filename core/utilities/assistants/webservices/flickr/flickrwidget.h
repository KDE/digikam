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

#ifndef FLICKR_WIDGET_H
#define FLICKR_WIDGET_H

// Qt includes

#include <QWidget>
#include <QTabWidget>
#include <QLineEdit>
#include <QComboBox>

// Local includes

#include "wscomboboxintermediate.h"
#include "flickrlist.h"
#include "wssettingswidget.h"
#include "dinfointerface.h"

namespace Digikam
{

class FlickrWidget : public WSSettingsWidget
{
    Q_OBJECT

public:

    explicit FlickrWidget(QWidget* const parent,
                          DInfoInterface* const iface,
                          const QString& serviceName);
    ~FlickrWidget();

    void updateLabels(const QString& name = QString(), const QString& url = QString()) Q_DECL_OVERRIDE;

private Q_SLOTS:

    void slotPermissionChanged(FlickrList::FieldType, Qt::CheckState);
    void slotSafetyLevelChanged(FlickrList::SafetyLevel);
    void slotContentTypeChanged(FlickrList::ContentType);
    void slotMainPublicToggled(int);
    void slotMainFamilyToggled(int);
    void slotMainFriendsToggled(int);
    void slotMainSafetyLevelChanged(int);
    void slotMainContentTypeChanged(int);
    void slotExtendedPublicationToggled(bool);
    void slotExtendedTagsToggled(bool);
    void slotAddExtraTagsToggled(bool);

private:

    void mainPermissionToggled(FlickrList::FieldType, Qt::CheckState);

private:

    class Private;
    Private* const d;

    friend class FlickrWindow;
};

} // namespace Digikam

#endif // FLICKR_WIDGET_H
