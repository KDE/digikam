PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE Thumbnails
                            (id INTEGER PRIMARY KEY,
                            type INTEGER,
                            modificationDate DATETIME,
                            orientationHint INTEGER,
                            data BLOB);
CREATE TABLE UniqueHashes
                            (uniqueHash TEXT,
                            fileSize INTEGER,
                            thumbId INTEGER,
                            UNIQUE(uniqueHash, fileSize));
CREATE TABLE FilePaths
                            (path TEXT,
                            thumbId INTEGER,
                            UNIQUE(path));
CREATE TABLE CustomIdentifiers
                            (identifier TEXT,
                            thumbId INTEGER,
                            UNIQUE(identifier));
CREATE TABLE Settings
                            (keyword TEXT NOT NULL UNIQUE,
                            value TEXT);
COMMIT;
