PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE AlbumRoots
                            (id INTEGER PRIMARY KEY,
                            label TEXT,
                            status INTEGER NOT NULL,
                            type INTEGER NOT NULL,
                            identifier TEXT,
                            specificPath TEXT,
                            UNIQUE(identifier, specificPath));
CREATE TABLE Albums
                            (id INTEGER PRIMARY KEY,
                            albumRoot INTEGER NOT NULL,
                            relativePath TEXT NOT NULL,
                            date DATE,
                            caption TEXT,
                            collection TEXT,
                            icon INTEGER,
                            UNIQUE(albumRoot, relativePath));
CREATE TABLE Images
                            (id INTEGER PRIMARY KEY,
                            album INTEGER,
                            name TEXT NOT NULL,
                            status INTEGER NOT NULL,
                            category INTEGER NOT NULL,
                            modificationDate DATETIME,
                            fileSize INTEGER,
                            uniqueHash TEXT,
                            UNIQUE (album, name));
CREATE TABLE ImageHaarMatrix
                            (imageid INTEGER PRIMARY KEY,
                            modificationDate DATETIME,
                            uniqueHash TEXT,
                            matrix BLOB);
CREATE TABLE ImageInformation
                            (imageid INTEGER PRIMARY KEY,
                            rating INTEGER,
                            creationDate DATETIME,
                            digitizationDate DATETIME,
                            orientation INTEGER,
                            width INTEGER,
                            height INTEGER,
                            format TEXT,
                            colorDepth INTEGER,
                colorModel INTEGER);
CREATE TABLE ImageMetadata
                            (imageid INTEGER PRIMARY KEY,
                            make TEXT,
                            model TEXT,
                            lens TEXT,
                            aperture REAL,
                            focalLength REAL,
                            focalLength35 REAL,
                            exposureTime REAL,
                            exposureProgram INTEGER,
                            exposureMode INTEGER,
                            sensitivity INTEGER,
                            flash INTEGER,
                            whiteBalance INTEGER,
                            whiteBalanceColorTemperature INTEGER,
                            meteringMode INTEGER,
                            subjectDistance REAL,
                            subjectDistanceCategory INTEGER);
CREATE TABLE ImagePositions
                            (imageid INTEGER PRIMARY KEY,
                            latitude TEXT,
                            latitudeNumber REAL,
                            longitude TEXT,
                            longitudeNumber REAL,
                            altitude REAL,
                            orientation REAL,
                            tilt REAL,
                            roll REAL,
                            accuracy REAL,
                            description TEXT);
CREATE TABLE ImageComments
                            (id INTEGER PRIMARY KEY,
                            imageid INTEGER,
                            type INTEGER,
                            language TEXT,
                            author TEXT,
                            date DATETIME,
                            comment TEXT,
                            UNIQUE(imageid, type, language, author));
CREATE TABLE ImageCopyright
                            (id INTEGER PRIMARY KEY,
                            imageid INTEGER,
                            property TEXT,
                            value TEXT,
                            extraValue TEXT,
                            UNIQUE(imageid, property, value, extraValue));
CREATE TABLE Tags
                            (id INTEGER PRIMARY KEY,
                            pid INTEGER,
                            name TEXT NOT NULL,
                            icon INTEGER,
                            iconkde TEXT,
                            UNIQUE (name, pid));
CREATE TABLE TagsTree
                            (id INTEGER NOT NULL,
                            pid INTEGER NOT NULL,
                            UNIQUE (id, pid));
CREATE TABLE ImageTags
                            (imageid INTEGER NOT NULL,
                            tagid INTEGER NOT NULL,
                            UNIQUE (imageid, tagid));
CREATE TABLE ImageProperties
                            (imageid  INTEGER NOT NULL,
                            property TEXT    NOT NULL,
                            value    TEXT    NOT NULL,
                            UNIQUE (imageid, property));
CREATE TABLE Searches
                            (id INTEGER PRIMARY KEY,
                            type INTEGER,
                            name TEXT NOT NULL,
                            query TEXT NOT NULL);
CREATE TABLE DownloadHistory
                            (id  INTEGER PRIMARY KEY,
                            identifier TEXT,
                            filename TEXT,
                            filesize INTEGER,
                            filedate DATETIME,
                            UNIQUE(identifier, filename, filesize, filedate));
CREATE TABLE Settings
                            (keyword TEXT NOT NULL UNIQUE,
                            value TEXT);
CREATE TABLE ImageHistory
                            (imageid INTEGER PRIMARY KEY,
                             uuid TEXT,
                             history TEXT);
CREATE TABLE ImageRelations
                            (subject INTEGER,
                             object INTEGER,
                             type INTEGER,
                             UNIQUE(subject, object, type));
CREATE TABLE TagProperties
                            (tagid INTEGER,
                             property TEXT,
                             value TEXT);
CREATE TABLE ImageTagProperties
                            (imageid INTEGER,
                             tagid INTEGER,
                             property TEXT,
                             value TEXT);
CREATE INDEX dir_index  ON Images (album);
CREATE INDEX hash_index ON Images (uniqueHash);
CREATE INDEX tag_index  ON ImageTags (tagid);
CREATE INDEX tag_id_index  ON ImageTags (imageid);
CREATE INDEX image_name_index ON Images (name);
CREATE INDEX creationdate_index ON ImageInformation (creationDate);
CREATE INDEX comments_imageid_index ON ImageComments (imageid);
CREATE INDEX copyright_imageid_index ON ImageCopyright (imageid);
CREATE INDEX uuid_index ON ImageHistory (uuid);
CREATE INDEX subject_relations_index ON ImageRelations (subject);
CREATE INDEX object_relations_index ON ImageRelations (object);
CREATE INDEX tagproperties_index ON TagProperties (tagid);
CREATE INDEX imagetagproperties_index ON ImageTagProperties (imageid, tagid);
CREATE INDEX imagetagproperties_imageid_index ON ImageTagProperties (imageid);
CREATE INDEX imagetagproperties_tagid_index ON ImageTagProperties (tagid);
CREATE TRIGGER delete_albumroot DELETE ON AlbumRoots
                BEGIN
                DELETE FROM Albums
                WHERE Albums.albumRoot = OLD.id;
                END;
CREATE TRIGGER delete_album DELETE ON Albums
                BEGIN
                DELETE FROM Images
                WHERE Images.album = OLD.id;
                END;
CREATE TRIGGER delete_image DELETE ON Images
                    BEGIN
                        DELETE FROM ImageTags          WHERE imageid=OLD.id;
                        DELETE From ImageHaarMatrix    WHERE imageid=OLD.id;
                        DELETE From ImageInformation   WHERE imageid=OLD.id;
                        DELETE From ImageMetadata      WHERE imageid=OLD.id;
                        DELETE From ImagePositions     WHERE imageid=OLD.id;
                        DELETE From ImageComments      WHERE imageid=OLD.id;
                        DELETE From ImageCopyright     WHERE imageid=OLD.id;
                        DELETE From ImageProperties    WHERE imageid=OLD.id;
                        DELETE From ImageHistory       WHERE imageid=OLD.id;
                        DELETE FROM ImageRelations     WHERE subject=OLD.id OR object=OLD.id;
                        DELETE FROM ImageTagProperties WHERE imageid=OLD.id;
                        UPDATE Albums SET icon=null    WHERE icon=OLD.id;
                        UPDATE Tags SET icon=null      WHERE icon=OLD.id;
                    END;
CREATE TRIGGER delete_tag DELETE ON Tags
                    BEGIN
                        DELETE FROM ImageTags WHERE tagid=OLD.id;
                        DELETE FROM TagProperties WHERE tagid=OLD.id;
                        DELETE FROM ImageTagProperties WHERE tagid=OLD.id;
                    END;
CREATE TRIGGER insert_tagstree AFTER INSERT ON Tags
                BEGIN
                INSERT INTO TagsTree
                    SELECT NEW.id, NEW.pid
                    UNION
                    SELECT NEW.id, pid FROM TagsTree WHERE id=NEW.pid;
                END;
CREATE TRIGGER delete_tagstree DELETE ON Tags
                BEGIN
                DELETE FROM Tags
                WHERE id  IN (SELECT id FROM TagsTree WHERE pid=OLD.id);
                DELETE FROM TagsTree
                WHERE id IN (SELECT id FROM TagsTree WHERE pid=OLD.id);
                DELETE FROM TagsTree
                    WHERE id=OLD.id;
                END;
CREATE TRIGGER move_tagstree UPDATE OF pid ON Tags
                BEGIN
                DELETE FROM TagsTree
                    WHERE
                    ((id = OLD.id)
                    OR
                    id IN (SELECT id FROM TagsTree WHERE pid=OLD.id))
                    AND
                    pid IN (SELECT pid FROM TagsTree WHERE id=OLD.id);
                INSERT INTO TagsTree
                    SELECT NEW.id, NEW.pid
                    UNION
                    SELECT NEW.id, pid FROM TagsTree WHERE id=NEW.pid
                    UNION
                    SELECT id, NEW.pid FROM TagsTree WHERE pid=NEW.id
                    UNION
                    SELECT A.id, B.pid FROM TagsTree A, TagsTree B
                    WHERE
                    A.pid = NEW.id AND B.id = NEW.pid;
                END;
COMMIT;
