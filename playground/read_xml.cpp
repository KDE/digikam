// kate: encoding utf-8; eol unix;
// kate: indent-width 4; mixedindent off; replace-tabs on; remove-trailing-space on; space-indent on;
// kate: word-wrap-column 120; word-wrap off;
// uex: encoding=utf-8


// #include <QtSql>
// #include <QFile>
// 
// #include "db_alvise.h"
// 
// #include <QtXml>
// #include <QDebug>
// #include <QString>


// Qt includes

#include <QDir>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QDomNodeList>
#include <QFile>
#include <QIODevice>
#include <QTextStream>

// KDE includes

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <klocalizedstring.h>

// Local includes

#include "dbactiontype.h"
#include "databaseconfigelement.h"
#include "databasecorebackend.h" // getDBAction & DatabaseCoreBackend
//#include "dbconfigversion.h"
static const int dbconfig_xml_version = 1;


// #include <QtCore>
// //#include <QTextStream>
QTextStream qout(stdout, QIODevice::WriteOnly);
//QTextStream kError()(stderr, QIODevice::WriteOnly);


#define DATABASEPARAMETERS_DEBUG 1


QMap<QString, Digikam::DatabaseConfigElement> databaseConfigs;
QString errorMessage;

void readDBActions(QDomElement& sqlStatementElements, Digikam::DatabaseConfigElement& configElement)
{
    QDomElement dbActionElement =  sqlStatementElements.firstChildElement("dbaction");

    for ( ; !dbActionElement.isNull();  dbActionElement=dbActionElement.nextSiblingElement("dbaction"))
    {
        if (!dbActionElement.hasAttribute("name"))
        {
            kDebug() << "Missing statement attribute <name>.";
        }

        Digikam::DatabaseAction action;
        action.name = dbActionElement.attribute("name");
        //kDebug() << "Getting attribute " << dbActionElement.attribute("name");

        if (dbActionElement.hasAttribute("mode"))
        {
            action.mode = dbActionElement.attribute("mode");
        }


        QDomElement databaseElement = dbActionElement.firstChildElement("statement");

        for ( ; !databaseElement.isNull();  databaseElement=databaseElement.nextSiblingElement("statement"))
        {
            if (!databaseElement.hasAttribute("mode"))
            {
                kDebug() << "Missing statement attribute <mode>.";
            }

            Digikam::DatabaseActionElement actionElement;
            actionElement.mode      = databaseElement.attribute("mode");
            actionElement.statement = databaseElement.text();

            action.dbActionElements.append(actionElement);
        }

        configElement.sqlStatements.insert(action.name, action);
    }
}



Digikam::DatabaseConfigElement readDatabase(QDomElement& databaseElement)
{
    Digikam::DatabaseConfigElement configElement;
    configElement.databaseID="Unidentified";

    if (!databaseElement.hasAttribute("name"))
    {
        qout << "Missing statement attribute <name>.";
    }

    configElement.databaseID = databaseElement.attribute("name");

    QDomElement element =  databaseElement.namedItem("databaseName").toElement();

    if (element.isNull())
    {
        kDebug() << "Missing element <databaseName>.";
    }

    configElement.databaseName = element.text();

    element =  databaseElement.namedItem("userName").toElement();

    if (element.isNull())
    {
        kDebug() << "Missing element <userName>.";
    }

    configElement.userName = element.text();

    element =  databaseElement.namedItem("password").toElement();

    if (element.isNull())
    {
        kDebug() << "Missing element <password>.";
    }

    configElement.password = element.text();

    element =  databaseElement.namedItem("hostName").toElement();

    if (element.isNull())
    {
        kDebug() << "Missing element <hostName>.";
    }

    configElement.hostName = element.text();

    element =  databaseElement.namedItem("port").toElement();

    if (element.isNull())
    {
        kDebug() << "Missing element <port>.";
    }

    configElement.port = element.text();

    element =  databaseElement.namedItem("connectoptions").toElement();

    if (element.isNull())
    {
        kDebug() << "Missing element <connectoptions>.";
    }

    configElement.connectOptions = element.text();

    element =  databaseElement.namedItem("dbservercmd").toElement();

    if (element.isNull())
    {
        kDebug() << "Missing element <dbservercmd>.";
    }

    configElement.dbServerCmd = element.text();

    element =  databaseElement.namedItem("dbinitcmd").toElement();

    if (element.isNull())
    {
        kDebug() << "Missing element <dbinitcmd>.";
    }

    configElement.dbInitCmd = element.text();

    element =  databaseElement.namedItem("dbactions").toElement();

    if (element.isNull())
    {
        kDebug() << "Missing element <dbactions>.";
    }

    readDBActions(element, configElement);

    return configElement;
}

/*
void readDBActions(QDomElement& sqlStatementElements, Digikam::DatabaseConfigElement& configElement)
{
    QDomElement dbActionElement =  sqlStatementElements.firstChildElement("dbaction");

    for ( ; !dbActionElement.isNull();  dbActionElement=dbActionElement.nextSiblingElement("dbaction"))
    {
        if (!dbActionElement.hasAttribute("name"))
        {
            kDebug() << "Missing statement attribute <name>.";
        }

        Digikam::DatabaseAction action;
        action.name = dbActionElement.attribute("name");
        //kDebug() << "Getting attribute " << dbActionElement.attribute("name");

        if (dbActionElement.hasAttribute("mode"))
        {
            action.mode = dbActionElement.attribute("mode");
        }


        QDomElement databaseElement = dbActionElement.firstChildElement("statement");

        for ( ; !databaseElement.isNull();  databaseElement=databaseElement.nextSiblingElement("statement"))
        {
            if (!databaseElement.hasAttribute("mode"))
            {
                kDebug() << "Missing statement attribute <mode>.";
            }

            DatabaseActionElement actionElement;
            actionElement.mode      = databaseElement.attribute("mode");
            actionElement.statement = databaseElement.text();

            action.dbActionElements.append(actionElement);
        }

        configElement.sqlStatements.insert(action.name, action);
    }
}
*/

bool readConfig(QString filepath)
{
    // QString filepath = KStandardDirs::locate("data", "digikam/database/dkstatements.xml");
    kDebug() << "Loading SQL code from config file" << filepath;
    QFile file(filepath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        errorMessage = i18n("Could not open xml file <filename>%1</filename>", filepath);
        return false;
    }

    QDomDocument doc("DBConfig");

    if (!doc.setContent(&file))
    {
        file.close();
        errorMessage = i18n("The XML in file <filename>%1</filename> is invalid and cannot be read.", filepath);
        return false;
    }

    file.close();

    QDomElement element = doc.namedItem("databaseconfig").toElement();

    if (element.isNull())
    {
        errorMessage = i18n("The XML in file <filename>%1</filename> "
                            "is missing the required element <icode>%1</icode>",
                            filepath, element.tagName());
        return false;
    }

    QDomElement defaultDB =  element.namedItem("defaultDB").toElement();

    if (defaultDB.isNull())
    {
        errorMessage = i18n("The XML in file <filename>%1</filename> "
                            "is missing the required element <icode>%1</icode>",
                            filepath, element.tagName());
        return false;
    }

    QDomElement versionElement = element.namedItem("version").toElement();
    int version = 0;

    kDebug() << versionElement.isNull() << versionElement.text() << versionElement.text().toInt() << dbconfig_xml_version;
    if (!versionElement.isNull())
    {
        version = versionElement.text().toInt();
    }

    if (version < dbconfig_xml_version)
    {
        errorMessage = i18n("An old version of the xml file <filename>%1</filename> "
                            "is found. Please ensure that the version released "
                            "with the running version of digiKam is installed. ",
                            filepath);
        return false;
    }

#ifdef DATABASEPARAMETERS_DEBUG
    kDebug() << "Default DB Node contains: " << defaultDB.text();
#endif

    QDomElement databaseElement =  element.firstChildElement("database");

    for ( ; !databaseElement.isNull();  databaseElement=databaseElement.nextSiblingElement("database"))
    {
        Digikam::DatabaseConfigElement l_DBCfgElement = readDatabase(databaseElement);
        databaseConfigs.insert(l_DBCfgElement.databaseID, l_DBCfgElement);
    }

#ifdef DATABASEPARAMETERS_DEBUG
    kDebug() << "Found entries: " << databaseConfigs.size();
    foreach (const Digikam::DatabaseConfigElement& configElement, databaseConfigs )
    {
        kDebug() << "DatabaseID: " << configElement.databaseID;
        kDebug() << "HostName: " << configElement.hostName;
        kDebug() << "DatabaseName: " << configElement.databaseName;
        kDebug() << "UserName: " << configElement.userName;
        kDebug() << "Password: " << configElement.password;
        kDebug() << "Port: " << configElement.port;
        kDebug() << "ConnectOptions: " << configElement.connectOptions;
        kDebug() << "Database Server CMD: " << configElement.dbServerCmd;
        kDebug() << "Database Init CMD: " << configElement.dbInitCmd;

        /*
        kDebug() << "Statements:";

        foreach (const QString actionKey, configElement.sqlStatements.keys())
        {
            QList<databaseActionElement> l_DBActionElement = configElement.sqlStatements[actionKey].dBActionElements;
            kDebug() << "DBAction [" << actionKey << "] has [" << l_DBActionElement.size() << "] actions";
            foreach (const databaseActionElement statement, l_DBActionElement)
            {
                kDebug() << "\tMode ["<< statement.mode <<"] Value ["<< statement.statement <<"]";
            }
        }
        */
    }
#endif

    return true;
}




QString getDBActionQuery(const Digikam::DatabaseAction& action)
{

    QString result;

    foreach (Digikam::DatabaseActionElement actionElement, action.dbActionElements)
    {
        result = result + actionElement.statement;
    }
    return result;
}


static QString DOC =
"<?xml version='1.0' encoding='ISO-8859-1'?>"
"<config>"
"  <blah>"
"    <![CDATA[blah1]]>"
"  </blah>"
"  <blah>"
"    <![CDATA[blah2]]>"
"  </blah>"
"</config>";

void printCDATA( QDomNode n )
{
    if ( n.isCDATASection() )
    qDebug() << n.toCDATASection().data();

    QDomNode c = n.firstChild();
    while ( !c.isNull() )
    {
        printCDATA( c );
        c = c.nextSibling();
    }
}

int main( int, char*[] )
{
    QDomDocument doc;
    doc.setContent( DOC );
    printCDATA( doc.documentElement() );

    QString filepath = KStandardDirs::locate("data", "digikam/database/dkstatements.xml");
    readConfig(filepath);

    qout << "Found entries: " << databaseConfigs.size() << endl;
    foreach (const Digikam::DatabaseConfigElement& configElement, databaseConfigs )
    {
        qout << "DatabaseID: " << configElement.databaseID << endl;
        Digikam::DatabaseAction action = configElement.sqlStatements.value(QString("ThumbnailDB::setSetting"));
        qout << action.name << endl;
        qout << action.mode << endl;
        qout << getDBActionQuery(action) << endl;
    }

    return 0;
}
