/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-02
 * Description : validate / fix ui files
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmail dot com>
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

#ifndef UIFILEVALIDATOR_H
#define UIFILEVALIDATOR_H

// Qt includes

#include <QXmlDefaultHandler>
#include <QByteArray>

// Local includes

#include "digikam_export.h"

class QString;
class QFile;

namespace Digikam
{

class UiFileValidatorPriv;

class DIGIKAM_EXPORT UiFileValidator
{
public:

    explicit UiFileValidator(const QString& filename);
    ~UiFileValidator();

    bool isValid() const;

    QByteArray getFixedContent();
    bool       fixConfigFile();
    bool       fixConfigFile(const QString& destination);

private:

    bool isReadable(QFile& file) const;
    bool isWritable(QFile& file) const;

private:

    UiFileValidatorPriv* const d;
};

}  // namespace Digikam

#endif // UIFILEVALIDATOR_H
