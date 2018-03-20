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

// Qt includes

#include <QWidget>

// Local includes

#include "dmetadatasettingscontainer.h"

class QStandardItemModel;
class QStandardItem;

namespace Digikam
{

class AdvancedMetadataTab : public QWidget
{
    Q_OBJECT

public:

    AdvancedMetadataTab(QWidget* const parent = 0);
    virtual ~AdvancedMetadataTab();

    void applySettings();

public Q_SLOTS:

    void slotResetToDefault();
    void slotRevertChanges();
    void slotAddNewNamespace();
    void slotEditNamespace();

private Q_SLOTS:

    void slotUnifyChecked(bool value);
    void slotIndexChanged();
    void slotRevertChangesAvailable();

private:

    /**
     * @brief The NsRoles enum will encode data from NamespaceEntry in
     *        model items, so we could retrieve and save it later
     */
    enum NsRoles
    {
        NAME_ROLE        = Qt::UserRole+1,
        ISTAG_ROLE       = Qt::UserRole+2,
        SEPARATOR_ROLE   = Qt::UserRole+3,
        NSTYPE_ROLE      = Qt::UserRole+5,

        ZEROSTAR_ROLE    = Qt::UserRole+6,
        ONESTAR_ROLE     = Qt::UserRole+7,
        TWOSTAR_ROLE     = Qt::UserRole+8,
        THREESTAR_ROLE   = Qt::UserRole+9,
        FOURSTAR_ROLE    = Qt::UserRole+10,
        FIVESTAR_ROLE    = Qt::UserRole+11,

        SPECIALOPTS_ROLE = Qt::UserRole+12,

        ALTNAME_ROLE     = Qt::UserRole+13,
        SUBSPACE_ROLE    = Qt::UserRole+14,
        ALTNAMEOPTS_ROLE = Qt::UserRole+15,

        ISDEFAULT_ROLE   = Qt::UserRole+16,
    };

    enum ModelNumbers
    {
        READ_TAGS      = 0,
        READ_RATINGS   = 1,
        READ_COMMENTS  = 2,
        WRITE_TAGS     = 3,
        WRITE_RATINGS  = 4,
        WRITE_COMMENTS = 5
    };

private:

    void connectButtons();
    /**
     * @brief setModelData for one model
     * @param model - model to be populated
     * @param container - namespace container to get data
     */
    void setModelData(QStandardItemModel* model, const QList<NamespaceEntry>& container);
    void setUi();

    void setDataToItem(QStandardItem* item, NamespaceEntry& entry);

    /**
     * @brief getModelIndex - the view can have up to 6 models
     *                        based on tags, comments, rating selection
     *                        and read/ write operation selected
     * @return              - return index of correct model in d->models
     */
    int getModelIndex();

    QList<NamespaceEntry>& getCurrentContainer();

    void setModels();

    void saveModelData(QStandardItemModel* model, QList<NamespaceEntry>& container);

private:

    class Private;
    Private* d;
};

} // namespace Digikam
