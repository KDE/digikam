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
COMMIT;
