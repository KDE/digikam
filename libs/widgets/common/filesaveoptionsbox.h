/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-08-02
 * Description : a stack of widgets to set image file save 
 *               options into image editor.
 *
 * Copyright (C) 2007-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef FILESAVEOPTIONSBOX_H
#define FILESAVEOPTIONSBOX_H

// KDE includes

#include <QtGui/QStackedWidget>
#include <QtCore/QString>

// Local includes

#include "dimg.h"
#include "digikam_export.h"

namespace Digikam
{

class FileSaveOptionsBoxPriv;

class DIGIKAM_EXPORT FileSaveOptionsBox : public QStackedWidget
{
    Q_OBJECT

public:

    FileSaveOptionsBox(QWidget *parent=0);
    ~FileSaveOptionsBox();

    void applySettings();

    /**
     * Tries to discover a file format that has options to change based on a
     * filename.
     *
     * @param filename file name to discover the desired format from
     * @param fallback fallback format to return if no format could be
     *                 discovered based on the filename
     * @return file format guessed from the file name or the given fallback
     *         format if no format could be guessed based on the file name
     */
    DImg::FORMAT discoverFormat(const QString &filename,
    		                    DImg::FORMAT fallback = DImg::NONE);

public Q_SLOTS:

    void slotImageFileFormatChanged(const QString&);
    void slotImageFileSelected(const QString&);

private:

    void readSettings();

private:

    FileSaveOptionsBoxPriv* const d;
};

}  // namespace Digikam

#endif /* FILESAVEOPTIONSBOX_H */
