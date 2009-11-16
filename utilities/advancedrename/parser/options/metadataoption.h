/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an option to provide metadata information to the parser
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#ifndef METADATAOPTION_H
#define METADATAOPTION_H

// Qt includes

#include <QObject>
#include <QString>
#include <QStringList>

// KDE includes

#include <kdialog.h>

// Local includes

#include "option.h"

namespace Digikam
{

class MetadataOptionDialogPriv;

class MetadataOptionDialog : public KDialog
{
    Q_OBJECT

public:

    MetadataOptionDialog();
    ~MetadataOptionDialog();

    QStringList checkedTags() const;
    QString     separator()   const;

private:

    MetadataOptionDialogPriv* const d;
};

// --------------------------------------------------------

class MetadataOption : public Option
{
    Q_OBJECT

public:

    MetadataOption();
    ~MetadataOption() {};

protected:

    virtual void parseOperation(const QString& parseString, ParseInformation& info, ParseResults& results);

private Q_SLOTS:

    void slotTokenTriggered(const QString& token);

private:

    QString parseMetadata(const QString& token, ParseInformation& info);
};

} // namespace Digikam

#endif /* METADATAOPTION_H */
