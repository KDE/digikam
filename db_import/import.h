/*

This file is part of digikam database import tool.

    digikam database import tool is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    digikam database import tool is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with digikam database import tool.  If not, see <http://www.gnu.org/licenses/>

*/

#ifndef IMPORT_H
#define IMPORT_H

// Qt includes

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlQuery>

class import : public QObject
{
    Q_OBJECT
public:
    import();
    ~import();
    bool connect_db_digikam(QString db_path);
    bool connect_db_npp(QString db_path);
    void NPP_import();

signals:
    void updateStatusBar(QString);

protected:

private:
    void NPP_importTags(QString, QString, QString);
    QString NPP_insertTagsToDigikam(QString, QString, QString);
    void NPP_addImagesToDigikamTags(QString, QString, QString);
    QSqlDatabase db_digikam;
    QSqlDatabase db_npp_node;
    QSqlDatabase db_npp_item;
    QSqlDatabase db_npp_source;
    QString readable_path(QString);
    QString clean_native_path(QString);
private slots:

};

#endif // IMPORT_H
