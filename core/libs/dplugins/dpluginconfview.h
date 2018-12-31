/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-12-31
 * Description : configuration view for external plugin
 *
 * Copyright (C) 2018-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_DPLUGIN_CONF_VIEW_H
#define DIGIKAM_DPLUGIN_CONF_VIEW_H

// Qt includes

#include <QString>
#include <QTreeWidgetItem>
#include <QTreeWidget>

// Local includes

#include "dplugin.h"
#include "dpluginloader.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DPluginConfView : public QTreeWidget
{
    Q_OBJECT

public:

    /** Default constructor.
     */
    explicit DPluginConfView(QWidget* const parent=0);
    ~DPluginConfView();

    /** Apply all changes about plugins selected to be hosted in host application.
     */
    void apply();

    /** Return the number of plugins actived in the list.
     */
    int actived() const;

    /** Return the total number of plugins in the list.
     */
    int count()   const;

    /** Return the number of visible plugins in the list.
     */
    int visible() const;

    /** Select all plugins in the list.
     */
    void selectAll() Q_DECL_OVERRIDE;

    /** Clear all selected plugins in the list.
     */
    void clearAll();

    /** Set the string used to filter the plugins list. signalSearchResult() is emitted when all is done.
     */
    void setFilter(const QString& filter, Qt::CaseSensitivity cs);

    /** Return the current string used to filter the plugins list.
     */
    QString filter() const;

Q_SIGNALS:

    /** Signal emitted when filtering is done through slotSetFilter().
     *  Number of plugins found is sent when item relevant of filtering match the query.
     */
    void signalSearchResult(int);

private:

    class Private;
    Private* const d;
};
    
} // namespace Digikam

#endif // DIGIKAM_DPLUGIN_CONF_VIEW_H
