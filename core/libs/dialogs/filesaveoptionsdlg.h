/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-01-14
 * Description : a dialog to display image file save options.
 *
 * Copyright (C) 2009 by David Eriksson <meldavid at acc umu se>
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef FILESAVEOPTIONSDLG_H
#define FILESAVEOPTIONSDLG_H

// Qt includes

#include <QDialog>

// Local includes

#include "digikam_export.h"
#include "filesaveoptionsbox.h"

class QWidget;

namespace Digikam
{

class DIGIKAM_EXPORT FileSaveOptionsDlg : public QDialog
{

public:

    FileSaveOptionsDlg(QWidget* const parent, FileSaveOptionsBox* const options);
    ~FileSaveOptionsDlg();

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
    static DImg::FORMAT discoverFormat(const QString& filename, DImg::FORMAT fallback = DImg::NONE);
};

}  // namespace Digikam

#endif /* FILESAVEOPTIONSDLG_H */
