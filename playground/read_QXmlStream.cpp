// kate: encoding utf-8; eol unix;
// kate: indent-width 4; mixedindent off; replace-tabs on; remove-trailing-space on; space-indent on;
// kate: word-wrap-column 120; word-wrap off;
// uex: encoding=utf-8

// Qt includes

#include <QFile>
#include <QtXml/QXmlStreamReader>
#include <QTextStream> // for qout, remove later
#include <QtSql>
#include <QRegExp>

// KDE includes

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <klocalizedstring.h>

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

/*
qint64 dbDigikamGetId(QSqlDatabase db, QString iden) {
    //QSqlQuery query = QSqlQuery::QSqlQuery(db); -fpermissive
    QSqlQuery *query = new QSqlQuery(db);
    query->exec( QString::fromLatin1("SELECT db_digikam_get_new_id(1, '") \
        % iden \
        % QString::fromLatin1("') AS id FROM DUAL;") );
    query->first();

    if(query->isValid()) {
        qint64 ret = query->value(0).toLongLong();
        delete query;
        return ret;
    } else {
        qout << "Error: dbDigikamGetId()" << endl;
        qout << "lastError():       " << query->lastError().text() << endl;
        qout << "numRowsAffected(): " << query->numRowsAffected() << endl;
        qout << "last sql:          " << query->lastQuery() << endl;
    }
    delete query;
    return -1;
}
*/

//////////////// XML ////////////////////////////////////////////////////////////////

class DbactionsReader
{
public:
    DbactionsReader();

    bool read(QIODevice *device);

    QString errorString() const;

private:
    void readDbactions();
    void readSeparator();
    void readDbaction();
    void readStatement();

    QXmlStreamReader xml;
};


DbactionsReader::DbactionsReader()
{
}

bool DbactionsReader::read(QIODevice *device)
{
    //rm qout << "DbactionsReader::read" << endl;
    xml.setDevice(device);

    if (xml.readNextStartElement()) {
        if (xml.name() == "databaseconfig") // && xml.attributes().value("version") == "1.0")
        {
            //rm qout << "read: " << xml.name().toString() << endl;
            while (xml.name() != "dbactions")
            {
                xml.readNextStartElement();
            }
            readDbactions();
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

QString DbactionsReader::errorString() const
{
    return QObject::tr("%1\nLine %2, column %3")
            .arg(xml.errorString())
            .arg(xml.lineNumber())
            .arg(xml.columnNumber());
}

void DbactionsReader::readDbactions()
{
    qout << "DbactionsReader::readDbactions -> ";
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
                token = xml.readNext();
                while(!xml.atEnd() && !xml.hasError() && token != QXmlStreamReader::StartElement) {
                    token = xml.readNext();
                }
                while(xml.name() == "statement") {
                    readStatement();
                    xml.readNextStartElement();
                }
            }
        }
    }
}

void DbactionsReader::readDbaction()
{
    qout << "DbactionsReader::readDbaction -> ";
    Q_ASSERT(xml.isStartElement() && xml.name() == "dbaction");
    QXmlStreamAttributes a = xml.attributes();
    Q_ASSERT(a.hasAttribute( QLatin1String("name")));
    qout << a.value(QLatin1String("name")).toString() << endl;
    //QString title = xml.readElementText();
}

void DbactionsReader::readStatement()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "statement");
    QXmlStreamAttributes a = xml.attributes();
    Q_ASSERT(a.hasAttribute( QLatin1String("backends")));
    qout << "  " << a.value(QLatin1String("backends")).toString() << endl;
    QString sql = xml.readElementText();
    qout << "  " << sql << endl;
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
    DbactionsReader dbr;
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
