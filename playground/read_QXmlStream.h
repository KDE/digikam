/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-27
 * Description : Database element configuration
 *
 * Copyright (C) 2009-2011 by Holger Foerster <hamsi2k at freenet dot de>
 * Copyright (C) 2011 by Francesco Riosa <francesco+kde at pnpitalia it>
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

#ifndef READ_QXMLSTREAM_H_
#define READ_QXMLSTREAM_H_

// Qt includes

#include <QtGlobal>
#include <QMap>
#include <QString>
#include <QSharedData>

// Local includes

#include "digikam_export.h"

namespace Playground
{

class DIGIKAM_EXPORT DatabaseParam
: public QSharedData
{
public:

    inline void reset() { value = defaultValue; }

    QList<int> positions;
    QString defaultValue;
    // TODO: check if this is the right place for value, provided these classes
    // will be shared between threads, involve also all reset*() fx
    QVariant value;
};


class DIGIKAM_EXPORT DatabaseActionElement
{
public:

    DatabaseActionElement() : order(0) {}
    void resetParams();
    void parse();

    QString mode;
    QString prepare;
    int     order;
    QString statement;

    QMap<QString, QExplicitlySharedDataPointer<DatabaseParam> > paramsByName;
    QMap<int, QString> paramsByPos;
private:
    void parse_params();
};

class DIGIKAM_EXPORT DatabaseAction
{
public:

    QString                      name;
    QString                      mode;
    QList<DatabaseActionElement> dbActionElements;
};

class DIGIKAM_EXPORT DatabaseStatements
{
public:

    DatabaseStatements(const QString& databaseType);

    bool                        read(QIODevice *device);
    static bool                 checkReadyForUse();
    static QString              errorMessage();
    QString                     errorString() const;
    QString                     getSqlStatement(const QString key);

private:

    void readDbactions();
    void readDbaction();
    void readStatement(Playground::DatabaseAction* action);

    QString wanted_backend;
    QXmlStreamReader xml;
    QMap<QString, DatabaseAction> sqlStatements;
};

} // namespace Playground

#endif /* READ_QXMLSTREAM_H_ */

// kate: encoding utf-8; eol unix;
// kate: indent-width 4; mixedindent off; replace-tabs on; remove-trailing-space on; space-indent on;
// kate: word-wrap-column 120; word-wrap off;
