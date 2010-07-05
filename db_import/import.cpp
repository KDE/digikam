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

// Qt includes

#include <qfiledialog.h>
#include <QSettings>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVector>

// Local includes

#include "import.h"

import::import()
{
}

import::~import()
{
    db_digikam.close();
    db_npp_node.close();
    db_npp_item.close();
}

QString import::NPP_insertTagsToDigikam(QString pid, QString name, QString icon)
{
    QSqlQuery insert_query(QSqlDatabase::database("digikam"));
    insert_query.prepare("INSERT INTO Tags (pid,name,icon) VALUES (:pid,:name,:icon)");
    insert_query.bindValue(":pid", pid);
    insert_query.bindValue(":name", name);
    insert_query.bindValue(":icon", icon);
    insert_query.exec();
    QVariant lastinsertid = insert_query.lastInsertId();

    return lastinsertid.toString();
}

bool import::connect_db_digikam(QString db_path)
{
    db_digikam = QSqlDatabase::addDatabase("QSQLITE","digikam");

    // QDir::cleanPath
    // QDir clean_native_path = QDir::toNativeSeparators();
    db_digikam.setDatabaseName(db_path);
    bool digikam_ok = db_digikam.open();

    return digikam_ok;
}

bool import::connect_db_npp(QString db_path)
{
    db_npp_node = QSqlDatabase::addDatabase("QODBC","npp_node");
    db_npp_node.setDatabaseName("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=" + db_path + "/Node.mdb");
    db_npp_node.setPassword("Nikon");
    bool db_npp_ok = db_npp_node.open();
    if ( db_npp_ok == FALSE ) { return db_npp_ok; }

    db_npp_item = QSqlDatabase::addDatabase("QODBC","npp_item");
    db_npp_item.setDatabaseName("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=" + db_path + "/Item.mdb");
    db_npp_item.setPassword("Nikon");
    db_npp_ok = db_npp_item.open();
    if ( db_npp_ok == FALSE ) { return db_npp_ok; }

    db_npp_source = QSqlDatabase::addDatabase("QODBC","npp_source");
    db_npp_source.setDatabaseName("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=" + db_path + "/Source.mdb");
    db_npp_source.setPassword("Nikon");
    db_npp_ok = db_npp_source.open();

    return db_npp_ok;
}

void import::NPP_import()
{
    QString command = "SELECT IDHi, IDLo FROM NodeTable WHERE Name='root';";
    QSqlQuery query(command,db_npp_node);
    query.first();
    emit updateStatusBar("Import in progress, please wait...");
    NPP_importTags(query.value(0).toString(),query.value(1).toString(),"0");
    emit updateStatusBar("Done.");
}

void import::NPP_importTags(QString pidhi_npp_node, QString pidlo_npp_node, QString pid_digikam)
{
    QString command = "SELECT IDHi, IDLo, Name, ParentIDHi, type FROM NodeTable WHERE ParentIDHi=" + pidhi_npp_node + " AND ParentIDLo=" + pidlo_npp_node + " ORDER BY IDLo;";
    QSqlQuery query( command,db_npp_node );
    QString icon = "0";
    QString lastdigikamid;

            while (query.next()) {
                if (query.value(0).toString()=="20481") {icon="folder-orange";}
                lastdigikamid = NPP_insertTagsToDigikam( pid_digikam, query.value(2).toString(), icon );
                NPP_addImagesToDigikamTags( query.value(0).toString() , query.value(1).toString() , lastdigikamid );
                NPP_importTags( query.value(0).toString(), query.value(1).toString(), lastdigikamid );
                }
}

void import::NPP_addImagesToDigikamTags(QString pidhi_npp_item, QString pidlo_npp_item, QString digikam_tagid)
{

    QMessageBox message;

    QString command = "SELECT SourceIDHi, SourceIDLo FROM ItemTable WHERE ParentIDHi=" + pidhi_npp_item + " AND ParentIDLo=" + pidlo_npp_item + ";";
    QSqlQuery query( command,db_npp_item );

    while (query.next()) {

    command = "SELECT Path FROM SourceTable WHERE IDHi=" + query.value(0).toString() + " AND IDLo=" + query.value(1).toString() + ";";
    QSqlQuery query2( command,db_npp_source );

        while (query2.next()) {

            command = "SELECT AlbumRoots.identifier ||  Albums.relativePath || '/' || images.name, Images.id FROM images INNER JOIN Albums ON Images.album = Albums.id INNER JOIN AlbumRoots ON Albums.albumRoot = AlbumRoots.id";
            QSqlQuery query3( command , db_digikam );

            while (query3.next()) {

                QDir npp_image_fullpath = QDir::fromNativeSeparators(query2.value(0).toString());

                if ( npp_image_fullpath.path() == readable_path(query3.value(0).toString()) )
                {

                QSqlQuery insert_query(QSqlDatabase::database("digikam"));
                insert_query.prepare("INSERT INTO ImageTags (imageid,tagid) VALUES (:imageid,:tagid)");
                insert_query.bindValue(":imageid", query3.value(1).toString());
                insert_query.bindValue(":tagid", digikam_tagid);
                insert_query.exec();
                insert_query.finish();

                }

            }
        }
    }
}

QString import::readable_path(QString path)
{
    path.replace("volumeid:?path=","");
    path.replace("%3A",":");
    path.replace("%2F","/");
    path.replace("//","/");
    return path;
}

QString import::clean_native_path(QString path)
{
    QDir clean_native_path;
    path = clean_native_path.toNativeSeparators(path);
    return clean_native_path.cleanPath(path);
}
