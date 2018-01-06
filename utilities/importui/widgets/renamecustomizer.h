/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-19
 * Description : a options group to set renaming files
 *               operations during camera downloading
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011      by Andi Clemens <andi dot clemens at gmail dot com>
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

#ifndef RENAMECUSTOMIZER_H
#define RENAMECUSTOMIZER_H

// Qt includes

#include <QMap>
#include <QRegExp>
#include <QWidget>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class AdvancedRenameManager;

class DIGIKAM_EXPORT RenameCustomizer : public QWidget
{
    Q_OBJECT

public:

    enum Case
    {
        NONE = 0,
        UPPER,
        LOWER
    };

    RenameCustomizer(QWidget* const parent, const QString& cameraTitle);
    ~RenameCustomizer();

    void    setUseDefault(bool val);
    bool    useDefault() const;
    QString newName(const QString& fileName) const;
    Case    changeCase() const;
    void    setChangeCase(Case val);

    int     startIndex() const;
    void    setStartIndex(int startIndex);
    void    reset();

    AdvancedRenameManager* renameManager() const;

Q_SIGNALS:

    void signalChanged();

private:

    void readSettings();
    void saveSettings();

private Q_SLOTS:

    void slotRadioButtonClicked(int);
    void slotRenameOptionsChanged();
    void slotCustomRenameChanged();
    void slotFileMetadataLinkUsed();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* RENAMECUSTOMIZER_H */
