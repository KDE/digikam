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
CREATE TABLE Settings
                            (keyword TEXT NOT NULL UNIQUE,
                            value TEXT);
CREATE TABLE CustomIdentifiers
                        (identifier TEXT,
                        thumbId INTEGER,
                        UNIQUE(identifier));
CREATE INDEX id_uniqueHashes ON UniqueHashes (thumbId);
CREATE INDEX id_filePaths ON FilePaths (thumbId);
CREATE INDEX id_customIdentifiers ON CustomIdentifiers (thumbId);
CREATE TRIGGER delete_thumbnails DELETE ON Thumbnails
                            BEGIN
                            DELETE FROM UniqueHashes WHERE UniqueHashes.thumbId = OLD.id;
                            DELETE FROM FilePaths WHERE FilePaths.thumbId = OLD.id;
                            DELETE FROM CustomIdentifiers WHERE CustomIdentifiers.thumbId = OLD.id;
                            END;
COMMIT;
