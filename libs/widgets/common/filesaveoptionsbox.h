/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-08-02
 * Description : a stack of widgets to set image file save
 *               options into image editor.
 *
 * Copyright (C) 2007-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QStackedWidget>
#include <QString>

// KDE includes

#include <kfiledialog.h>

// Local includes

#include "dimg.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT FileSaveOptionsBox : public QStackedWidget
{
    Q_OBJECT

public:

    /**
     * Constructor. Don't forget to call setDialog after creation of the dialog.
     *
     * @param parent parent for Qt's parent child mechanism
     */
    explicit FileSaveOptionsBox(QWidget* const parent=0);

    /**
     * Destructor.
     */
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
    DImg::FORMAT discoverFormat(const QString& filename, DImg::FORMAT fallback = DImg::NONE);

    /**
     * Call this method immediately after creation of the file dialog with this
     * options widget to enable signal handling on the dialog.
     *
     * @param dialog the file dialog this options widget is used for.
     */
    void setDialog(KFileDialog* const dialog);

    /**
     * Sets a filter used for the dialog that is used to automatically select
     * the format based on the filename provided by the user. If this is an
     * empty string, no automatic discovering of the file type is used.
     *
     * @param autoFilter filter string like it is used in the filter drop down
     *                   of the host dialog
     */
    void setAutoFilter(const QString& autoFilter);

public Q_SLOTS:

    void slotFilterChanged(const QString& newFilter);
    void slotImageFileFormatChanged(const QString&);
    void slotImageFileSelected(const QString&);

private:

    void readSettings();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* FILESAVEOPTIONSBOX_H */
