#ifndef QSTANDARTPATHSWRAP_H_
#define QSTANDARTPATHSWRAP_H_

#include <QStandardPaths>

class QStandardPathsWrap{

public:
    static QString locate(QStandardPaths::StandardLocation type,
                          const QString & fileName,
                          QStandardPaths::LocateOptions options = QStandardPaths::LocateFile)
    {
        QString str;
        str = QStandardPaths::locate(type, fileName, options);
        if(str.isEmpty())
        {
            return QStandardPaths::locate(type, QString("apps/") +fileName, options);
        } else
            return str;
    }

    static QStringList locateAll(QStandardPaths::StandardLocation type,
                                 const QString & fileName,
                                 QStandardPaths::LocateOptions options = QStandardPaths::LocateFile)
    {
        QStringList strList;
        strList = QStandardPaths::locateAll(type, fileName, options);
        if(strList.isEmpty())
        {
            return QStandardPaths::locateAll(type, QString("apps/") +fileName, options);
        } else
            return strList;
    }
};


#endif
