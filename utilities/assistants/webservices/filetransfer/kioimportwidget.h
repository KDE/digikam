/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 04.10.2009
 * Description : main widget of the import dialog
 *
 * Copyright (C) 2009 by Johannes Wienke <languitar at semipol dot de>
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

#ifndef KIO_IMPORT_WIDGET_H
#define KIO_IMPORT_WIDGET_H

// Qt includes

#include <QUrl>
#include <QWidget>

// Local includes

#include "dinfointerface.h"

namespace Digikam
{

class DImagesList;
    
/**
 * Helper widget for the import tool.
 */
class KioImportWidget: public QWidget
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent the parent widget
     * @param interface kipi interface to use
     */
    KioImportWidget(QWidget* const parent, DInfoInterface* const iface);

    /**
     * Destructor.
     */
    virtual ~KioImportWidget();

    /**
     * Returns the current list of source urls to import contained in the
     * image list.
     *
     * @return list of urls, potentially from remote system
     */
    QList<QUrl> sourceUrls() const;

    /**
     * Returns the image list used to manage images to import.
     *
     * @return pointer to the image list
     */
    DImagesList* imagesList() const;

    /**
     * Returns the upload widget for specifying the target location.
     *
     * @return pointer to the widget
     */
    QWidget* uploadWidget() const;

private:

    DImagesList* m_imageList;
    QWidget*     m_uploadWidget;
};

} // namespace Digikam

#endif // KIO_IMPORT_WIDGET_H
