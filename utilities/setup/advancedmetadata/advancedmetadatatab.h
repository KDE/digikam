/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-06-16
 * Description : Advanced Configuration tab for metadata.
 *
 * Copyright (C) 2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail.com>
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
#include <QWidget>
#include "dmetadatasettingscontainer.h"

class QStandardItemModel;
namespace Digikam
{


class AdvancedMetadataTab : public QWidget
{
    Q_OBJECT
public:
    AdvancedMetadataTab(QWidget *parent = 0);
    virtual ~AdvancedMetadataTab();

public Q_SLOTS:

    void slotResetView();
    void slotAddNewNamespace();
    void slotEditNamespace();

private Q_SLOTS:
    void slotUnifyChecked(bool value);
    void slotIndexChanged();

private:

    void connectButtons();
    void setModelData(QStandardItemModel* model, QList<NamespaceEntry> &container);
    void setUi();

    /**
     * @brief getModelIndex - the view can have up to 6 models
     *                        based on tags, comments, rating selection
     *                        and read/ write operation selected
     * @return              - return index of correct model in d->models
     */
    int getModelIndex();

    void setModels();

    class Private;
    Private* d;
};

}
