// kate: encoding utf-8; eol unix;
// kate: indent-width 4; mixedindent off; replace-tabs on; remove-trailing-space on; space-indent on;
// kate: word-wrap-column 120; word-wrap off;
// uex: encoding=utf-8

// Qt includes

#include <QFile>
#include <QtXml/QXmlStreamReader>
#include <QTextStream>

// KDE includes

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <klocalizedstring.h>

QTextStream qout(stdout, QIODevice::WriteOnly);
QTextStream qerr(stderr, QIODevice::WriteOnly);


class DbactionsReader
{
public:
    DbactionsReader();

    bool read(QIODevice *device);

    QString errorString() const;

private:
    void readDbactions();
    void readTitle();
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
    QString filepath = KStandardDirs::locate("data", "digikam/database/dkstatements.xml");
    QFile* file = new QFile(filepath);

    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        qerr << "Could not open xml file <filename>%1</filename>" << endl;
        return 1;
    }
    DbactionsReader dbr;
    dbr.read(file);
    file->close();

}
