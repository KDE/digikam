/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-06-01
 * Description : Database element configuration
 *
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

// Qt includes

#include <QFile>
#include <QtXml/QXmlStreamReader>
#include <QTextStream> // for qout, remove later
#include <QtSql>
#include <QRegExp>
#include <QListIterator>
#include <QExplicitlySharedDataPointer>

// KDE includes

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <klocalizedstring.h>

// local includes
#include "read_QXmlStream.h"


QTextStream qout(stdout, QIODevice::WriteOnly);
QTextStream qerr(stderr, QIODevice::WriteOnly);

//////////////// database ////////////////////////////////////////////////////////////////

QSqlDatabase dbDigikamConn() {
    qout << "Starting connection" << endl;

    QString conname = QString::fromLatin1("digikamtest") ;
    QSqlDatabase db = QSqlDatabase::addDatabase(QString::fromLatin1("QMYSQL"));

    db.setHostName(QString::fromLatin1("localhost"));
    db.setPort(3306);
    db.setDatabaseName(QString::fromLatin1("digikamtest"));
    db.setUserName(QString::fromLatin1("digikam"));
    db.setPassword(QString::fromLatin1("digikam"));
    //db.setConnectOptions();
    db.database(conname, true);
    db.open();
    return db;
    /*
    if ( db.isValid() ) {
        return true;
    }
    return false;
    */
}

//////////////// XML ////////////////////////////////////////////////////////////////

void Playground::DatabaseActionElement::resetParams()
{
    /* LATER:
    for (QMap<QString, DatabaseParam>::iterator i = paramsByName.begin();
         i != paramsByName.constEnd();
         ++i)
    {
        i.value().reset();
    }
    */
}

void Playground::DatabaseActionElement::parse()
{
    parse_params();
    statement.squeeze();
}

void Playground::DatabaseActionElement::parse_params()
{
    // [[:param_name, default="123":]] -> [1:keyword, 2:default, 3:123]
    // [[:param_name:]]
    static const QString regexp = \
            QString("(?:\\[\\[:)(\\w+)(?:,\\s*)(default)(?:=\")([^\"]*)(?:\")(?:.*:\\]\\])");
    QRegExp rx(regexp, Qt::CaseSensitive, QRegExp::RegExp2);
    rx.setMinimal(true); // we really need non-gredy operators
    static const QString newparam = QString("?");

    int count = 0;
    int pos = 0;
    while ((pos = rx.indexIn(statement, pos)) != -1) {

        Q_ASSERT(rx.captureCount() == 1 || rx.captureCount() == 3);

        QExplicitlySharedDataPointer<DatabaseParam> p;
        if (paramsByName.contains(rx.cap(1))) {
            p = paramsByName.value(rx.cap(1));
        } else {
            p = new DatabaseParam;
            paramsByName.insert(rx.cap(1), p);
        }

        paramsByPos.insert(count, rx.cap(1));

        if ( p->defaultValue.isEmpty() \
                && rx.captureCount() == 3 \
                && rx.cap(2) == QLatin1String("default"))
        {
             p->defaultValue = rx.cap(3);
        }

        p->positions.append(count);
        //rm qout << "parse: -> rx0:" << rx.cap(0) << endl;
        //rm qout << "parse: " << count << " -> rx1:" << rx.cap(1) << "  rx2:" << rx.cap(2) << "  rx3:" << rx.cap(3) << endl;
        //rm qout << "def empty: " << p.defaultValue.isNull() << "'" << p.defaultValue << "'" << endl;
        //rm qout << "def cc: " << rx.captureCount() << endl;
        //rm qout << "def cc: " << rx.cap(2) << endl;

        // transform reference in a positional parameter
        statement.replace(statement.indexOf(rx.cap(0)), rx.cap(0).size(), newparam);

        pos += rx.matchedLength() - rx.cap(0).size() + newparam.size();
        count++;
    }
}

Playground::DatabaseStatements::DatabaseStatements(const QString& databaseType)
{
    wanted_backend = databaseType;
}

bool Playground::DatabaseStatements::read(QIODevice *device)
{
    xml.setDevice(device);

    if (xml.readNextStartElement()) {
        if (xml.name() == "databaseconfig") // && xml.attributes().value("version") == "1.0")
        {
            while (xml.name() != "dbactions")
            {
                xml.readNextStartElement();
            }

            readDbactions();

            for (QMap<QString, Playground::DatabaseAction>::const_iterator i = sqlStatements.constBegin();
                 i != sqlStatements.constEnd();
                 ++i)
            {
                qout << "key: " << i.key() << endl;
                qout << "     " << i.value().name << endl;
                qout << "     " << i.value().mode << endl;
                QListIterator<Playground::DatabaseActionElement> j(i.value().dbActionElements);
                while (j.hasNext()) {
                    Playground::DatabaseActionElement dbe = j.next();
                    qout << "         " << dbe.mode << endl;
                    qout << "         " << dbe.prepare << endl;
                    qout << "         " << dbe.statement << endl;

                    for (QMap<QString, QExplicitlySharedDataPointer<DatabaseParam> >::iterator i
                            = dbe.paramsByName.begin();
                         i != dbe.paramsByName.constEnd();
                         ++i)
                    {
                        qout << "       p:" << i.key() << endl;
                        qout << "       p:" << i.value()->defaultValue << endl;
                        qout << "       p:" << i.value()->positions[0] << endl;
                    }
                }
            }

        }
        else
        {
            qerr << "The file is not an databaseconfig file." << endl;
            xml.raiseError(QObject::tr("The file is not an databaseconfig file."));
        }
    }
    else
    {
        Q_ASSERT(false);
    }

    return !xml.error();
}

QString Playground::DatabaseStatements::errorString() const
{
    return QObject::tr("%1\nLine %2, column %3")
            .arg(xml.errorString())
            .arg(xml.lineNumber())
            .arg(xml.columnNumber());
}

void Playground::DatabaseStatements::readDbactions()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "dbactions");

    while(!xml.atEnd() &&
      !xml.hasError()) {

        QXmlStreamReader::TokenType token = xml.readNext();

        /* If token is StartElement, we'll see if we can read it.*/
        if(token == QXmlStreamReader::StartElement) {
            /* "dbactions" is container, go to the next. */
            if(xml.name() == "dbactions") {
                continue;
            }
            /* If it's named dbaction, we'll dig the information from there.*/
            if(xml.name() == "dbaction") {
                readDbaction();
                //sqlStatements.insert(action.name, action);
            }
        }
    }
}

void Playground::DatabaseStatements::readDbaction()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "dbaction");
    QXmlStreamAttributes a = xml.attributes();
    Q_ASSERT(a.hasAttribute( QLatin1String("name")));
    //rm qout << a.value(QLatin1String("name")).toString() << endl;

    // initialize the action
    Playground::DatabaseAction action;
    action.name = a.value(QLatin1String("name")).toString();
    action.mode = a.value(QLatin1String("mode")).toString();

    // read statements for this backend
    QXmlStreamReader::TokenType token = xml.readNext();
    while(!xml.atEnd() && !xml.hasError() && token != QXmlStreamReader::StartElement) {
        token = xml.readNext();
    }
    while(xml.name() == "statement") {
        readStatement(&action);
        xml.readNextStartElement();
    }

    if ( action.dbActionElements.count() == 0) {
        action.mode = QString("noop");
    }

    sqlStatements.insert(action.name, action);

}

void Playground::DatabaseStatements::readStatement(Playground::DatabaseAction* action)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "statement");

    QXmlStreamAttributes a = xml.attributes();
    Q_ASSERT(a.hasAttribute( QLatin1String("backends")));

    const QString backends = a.value(QLatin1String("backends")).toString();

    QString sql = xml.readElementText(); // always read
    
    // search for desired backend
    for (int i = 0; ! backends.section(',', i, i).isNull(); i++)
    {
        if(0 == backends.section(',', i, i).trimmed().compare(wanted_backend, Qt::CaseInsensitive)) {

            // and populate action
            Playground::DatabaseActionElement actionElement;
            actionElement.mode      = a.value(QLatin1String("mode")).toString();
            if (a.hasAttribute( QLatin1String("prepare"))) {
                actionElement.prepare   = a.value(QLatin1String("prepare")).toString();
            }
            actionElement.statement = sql;
            actionElement.parse();

            action->dbActionElements.append(actionElement);
            
            break;
        }
    }
}

////////////////////////////////////////////////////////////////////////

int main( int, char*[] )
{
    // read xml

    QString filepath = KStandardDirs::locate("data", "digikam/database/dkstatements.xml");
    QFile* file = new QFile(filepath);

    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        qerr << "Could not open xml file <filename>" << filepath << "</filename>" << endl;
        return 1;
    }

    Playground::DatabaseStatements dbr(QString("mysql"));
    dbr.read(file);
    file->close();

    // test db

    QSqlDatabase db;
    db = dbDigikamConn();
    QSqlQuery *query = new QSqlQuery(db);

    query->exec("SET sql_mode = 'TRADITIONAL'");

    QString sql = "SELECT * FROM imageinformation WHERE :imageid = 1 LIMIT 2";
    qout << "sql\n" << sql << endl;

    query->prepare( sql );
    query->bindValue( ":imageid", 1 );
    //query->bindValue( ":imageid[1]", 2 );
    query->exec();
    qout << "sql exec\n" << query->executedQuery() << endl;

    //query->first();
    while (query->next()) {
        if ( query->isValid() ) {
            qout << "   " << query->value( 0 ).toString() \
                 << " - " << query->value( 1 ).toString() \
                 << " - " << query->value( 2 ).toString() \
                 << " - " << query->value( 3 ).toString() \
                 << " - " << query->value( 4 ).toString() \
                 << " - " << query->value( 5 ).toString() \
                 << " - " << query->value( 6 ).toString() \
                 << " - " << query->value( 7 ).toString() \
                 << " - " << query->value( 8 ).toString() \
                 << " - " << query->value( 9 ).toString() \
                 << endl;
        } else {
            qout << "lastError():       " << query->lastError().text() << endl;
            qout << "numRowsAffected(): " << query->numRowsAffected() << endl;
            qout << "last sql:          " << query->lastQuery() << endl;
            continue;
        }
    }
}

// kate: encoding utf-8; eol unix;
// kate: indent-width 4; mixedindent off; replace-tabs on; remove-trailing-space on; space-indent on;
// kate: word-wrap-column 120; word-wrap off;
// uex: encoding=utf-8
