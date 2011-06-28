-- MySQL dump 10.13  Distrib 5.1.56, for pc-linux-gnu (x86_64)
--
-- Host: localhost    Database: digikam
-- ------------------------------------------------------
-- Server version	5.1.56-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `albumroots`
--

DROP TABLE IF EXISTS `albumroots`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `albumroots` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `label` longtext,
  `status` int(11) NOT NULL,
  `type` int(11) NOT NULL,
  `identifier` longtext,
  `specificPath` longtext,
  PRIMARY KEY (`id`),
  UNIQUE KEY `identifier` (`identifier`(127),`specificPath`(128))
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `albumroots`
--

LOCK TABLES `albumroots` WRITE;
/*!40000 ALTER TABLE `albumroots` DISABLE KEYS */;
INSERT INTO `albumroots` VALUES (1,NULL,0,1,'volumeid:?uuid=800b21c2-dadc-4930-829e-a96b04ce26fa','/vivo/digikam-devel/data/testimages/a1'),(2,'a2',0,1,'volumeid:?uuid=800b21c2-dadc-4930-829e-a96b04ce26fa','/vivo/digikam-devel/data/testimages/a2');
/*!40000 ALTER TABLE `albumroots` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `albums`
--

DROP TABLE IF EXISTS `albums`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `albums` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `albumRoot` int(11) NOT NULL,
  `relativePath` longtext NOT NULL,
  `date` date DEFAULT NULL,
  `caption` longtext,
  `collection` longtext,
  `icon` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `albumRoot` (`albumRoot`,`relativePath`(255))
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `albums`
--

LOCK TABLES `albums` WRITE;
/*!40000 ALTER TABLE `albums` DISABLE KEYS */;
INSERT INTO `albums` VALUES (1,1,'/','2011-06-28',NULL,NULL,NULL),(2,1,'/jpg','2011-06-28',NULL,NULL,NULL),(3,1,'/png','2011-06-28',NULL,NULL,NULL),(4,2,'/','2011-06-28',NULL,NULL,NULL),(5,2,'/pgf','2011-06-28',NULL,NULL,NULL),(6,2,'/pgf/link','2011-06-28',NULL,NULL,NULL);
/*!40000 ALTER TABLE `albums` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `customidentifiers`
--

DROP TABLE IF EXISTS `customidentifiers`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `customidentifiers` (
  `identifier` longtext,
  `thumbId` int(11) DEFAULT NULL,
  UNIQUE KEY `identifier` (`identifier`(255)),
  KEY `id_customIdentifiers` (`thumbId`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `customidentifiers`
--

LOCK TABLES `customidentifiers` WRITE;
/*!40000 ALTER TABLE `customidentifiers` DISABLE KEYS */;
INSERT INTO `customidentifiers` VALUES ('detail:/home/vivo/digikam-devel/data/testimages/a2/Martian_face_viking.jpg?rect=302,83-87x105',22),('detail:/home/vivo/digikam-devel/data/testimages/a2/Martian_face_viking.jpg?rect=232,13-227x245',23);
/*!40000 ALTER TABLE `customidentifiers` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `downloadhistory`
--

DROP TABLE IF EXISTS `downloadhistory`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `downloadhistory` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `identifier` longtext,
  `filename` longtext,
  `filesize` int(11) DEFAULT NULL,
  `filedate` datetime DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `identifier` (`identifier`(164),`filename`(165),`filesize`,`filedate`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `downloadhistory`
--

LOCK TABLES `downloadhistory` WRITE;
/*!40000 ALTER TABLE `downloadhistory` DISABLE KEYS */;
/*!40000 ALTER TABLE `downloadhistory` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `filepaths`
--

DROP TABLE IF EXISTS `filepaths`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `filepaths` (
  `path` longtext,
  `thumbId` int(11) DEFAULT NULL,
  UNIQUE KEY `path` (`path`(255)),
  KEY `id_filePaths` (`thumbId`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `filepaths`
--

LOCK TABLES `filepaths` WRITE;
/*!40000 ALTER TABLE `filepaths` DISABLE KEYS */;
INSERT INTO `filepaths` VALUES ('/home/vivo/digikam-devel/data/testimages/a2/big-image.png',14),('/home/vivo/digikam-devel/data/testimages/a2/pgf/link/otherlink.pgf',21),('/home/vivo/digikam-devel/data/testimages/a2/Martian_face_viking.jpg',18),('/home/vivo/digikam-devel/data/testimages/a2/icc-test-no-profile.jpg',17),('/home/vivo/digikam-devel/data/testimages/a2/icc-test-farbkreis_v1.png',5),('/home/vivo/digikam-devel/data/testimages/a2/icc-test-farbkreis.jpg',15),('/home/vivo/digikam-devel/data/testimages/a1/png/snap002.png',13),('/home/vivo/digikam-devel/data/testimages/a2/icc-test-farbkreis_v1.jpg',16),('/home/vivo/digikam-devel/data/testimages/a1/jpg/foto001.jpg',10),('/home/vivo/digikam-devel/data/testimages/a1/jpg/foto001bw.jpg',11),('/home/vivo/digikam-devel/data/testimages/a1/png/snap001.png',12);
/*!40000 ALTER TABLE `filepaths` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `imagecomments`
--

DROP TABLE IF EXISTS `imagecomments`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `imagecomments` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `imageid` int(11) DEFAULT NULL,
  `type` int(11) DEFAULT NULL,
  `language` varchar(128) DEFAULT NULL,
  `author` longtext,
  `date` datetime DEFAULT NULL,
  `comment` longtext,
  PRIMARY KEY (`id`),
  UNIQUE KEY `imageid` (`imageid`,`type`,`language`,`author`(202)),
  KEY `comments_imageid_index` (`imageid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `imagecomments`
--

LOCK TABLES `imagecomments` WRITE;
/*!40000 ALTER TABLE `imagecomments` DISABLE KEYS */;
INSERT INTO `imagecomments` VALUES (1,1,1,'x-default',NULL,NULL,'This file is a part of digiKam project\nhttp://www.digikam.org\nCopyright (C) 2011 by Francesco Riosa <francesco+kde at pnpitalia it>'),(2,2,1,'x-default',NULL,NULL,'This file is a part of digiKam project\nhttp://www.digikam.org\nCopyright (C) 2011 by Francesco Riosa <francesco+kde at pnpitalia it>'),(3,4,1,'x-default',NULL,NULL,'Test Caption');
/*!40000 ALTER TABLE `imagecomments` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `imagecopyright`
--

DROP TABLE IF EXISTS `imagecopyright`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `imagecopyright` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `imageid` int(11) DEFAULT NULL,
  `property` longtext,
  `value` longtext,
  `extraValue` longtext,
  PRIMARY KEY (`id`),
  UNIQUE KEY `imageid` (`imageid`,`property`(110),`value`(111),`extraValue`(111)),
  KEY `copyright_imageid_index` (`imageid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `imagecopyright`
--

LOCK TABLES `imagecopyright` WRITE;
/*!40000 ALTER TABLE `imagecopyright` DISABLE KEYS */;
INSERT INTO `imagecopyright` VALUES (1,4,'creator','DigiKam Developers',NULL),(2,4,'provider','Opensource world',NULL),(3,4,'copyrightNotice','Same as DigiKam','x-default'),(4,4,'source','Snapshot',NULL),(5,4,'creatorJobTitle','developers',NULL),(6,4,'instructions','',NULL),(7,4,'creatorContactInfo.city',NULL,NULL),(8,4,'creatorContactInfo.country',NULL,NULL),(9,4,'creatorContactInfo.address',NULL,NULL),(10,4,'creatorContactInfo.postalCode',NULL,NULL),(11,4,'creatorContactInfo.provinceState',NULL,NULL),(12,4,'creatorContactInfo.email',NULL,NULL),(13,4,'creatorContactInfo.phone',NULL,NULL),(14,4,'creatorContactInfo.webUrl',NULL,NULL);
/*!40000 ALTER TABLE `imagecopyright` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `imagehaarmatrix`
--

DROP TABLE IF EXISTS `imagehaarmatrix`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `imagehaarmatrix` (
  `imageid` int(11) NOT NULL,
  `modificationDate` datetime DEFAULT NULL,
  `uniqueHash` longtext,
  `matrix` longblob,
  PRIMARY KEY (`imageid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `imagehaarmatrix`
--

LOCK TABLES `imagehaarmatrix` WRITE;
/*!40000 ALTER TABLE `imagehaarmatrix` DISABLE KEYS */;
/*!40000 ALTER TABLE `imagehaarmatrix` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `imagehistory`
--

DROP TABLE IF EXISTS `imagehistory`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `imagehistory` (
  `imageid` int(11) NOT NULL,
  `uuid` varchar(128) DEFAULT NULL,
  `history` longtext,
  PRIMARY KEY (`imageid`),
  KEY `uuid_index` (`uuid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `imagehistory`
--

LOCK TABLES `imagehistory` WRITE;
/*!40000 ALTER TABLE `imagehistory` DISABLE KEYS */;
INSERT INTO `imagehistory` VALUES (6,'a2e5c19c4b9430082a5087dd55bf7511723417e0715e351b1f8dfb44ca9d47a5',NULL);
/*!40000 ALTER TABLE `imagehistory` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `imageinformation`
--

DROP TABLE IF EXISTS `imageinformation`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `imageinformation` (
  `imageid` int(11) NOT NULL,
  `rating` int(11) DEFAULT NULL,
  `creationDate` datetime DEFAULT NULL,
  `digitizationDate` datetime DEFAULT NULL,
  `orientation` int(11) DEFAULT NULL,
  `width` int(11) DEFAULT NULL,
  `height` int(11) DEFAULT NULL,
  `format` longtext,
  `colorDepth` int(11) DEFAULT NULL,
  `colorModel` int(11) DEFAULT NULL,
  PRIMARY KEY (`imageid`),
  KEY `creationdate_index` (`creationDate`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `imageinformation`
--

LOCK TABLES `imageinformation` WRITE;
/*!40000 ALTER TABLE `imageinformation` DISABLE KEYS */;
INSERT INTO `imageinformation` VALUES (1,-1,'2011-06-05 16:28:34','2011-06-05 16:28:34',1,720,479,'JPG',8,5),(2,-1,'2011-06-05 16:28:34','2011-06-05 16:28:34',1,720,479,'JPG',8,2),(3,-1,'2011-06-28 02:13:14',NULL,0,1538,852,'PNG',8,1),(4,-1,'2011-06-27 15:31:50','2011-06-27 15:31:50',0,802,1916,'PNG',8,4),(5,-1,'2011-06-28 02:13:14',NULL,0,2560,2560,'PNG',8,1),(6,-1,'2006-03-11 15:42:12','2006-03-11 15:42:12',1,400,400,'JPG',8,5),(9,-1,'2011-06-18 16:40:36','2011-06-18 16:40:36',0,640,472,'JPG',8,2),(10,-1,'2011-06-05 16:28:34','2011-06-05 16:28:34',1,538,358,'PGF',24,1),(11,-1,'2011-06-05 16:28:34','2011-06-05 16:28:34',1,538,358,'PGF',24,1),(12,-1,'2011-06-05 16:28:34','2011-06-05 16:28:34',1,538,358,'PGF',24,1),(14,-1,'2006-03-11 15:42:12','2006-03-11 15:42:12',1,400,400,'JPG',8,5),(15,-1,'2011-06-28 02:13:14',NULL,0,798,798,'JPG',8,5);
/*!40000 ALTER TABLE `imageinformation` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `imagemetadata`
--

DROP TABLE IF EXISTS `imagemetadata`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `imagemetadata` (
  `imageid` int(11) NOT NULL,
  `make` longtext,
  `model` longtext,
  `lens` longtext,
  `aperture` double DEFAULT NULL,
  `focalLength` double DEFAULT NULL,
  `focalLength35` double DEFAULT NULL,
  `exposureTime` double DEFAULT NULL,
  `exposureProgram` int(11) DEFAULT NULL,
  `exposureMode` int(11) DEFAULT NULL,
  `sensitivity` int(11) DEFAULT NULL,
  `flash` int(11) DEFAULT NULL,
  `whiteBalance` int(11) DEFAULT NULL,
  `whiteBalanceColorTemperature` int(11) DEFAULT NULL,
  `meteringMode` int(11) DEFAULT NULL,
  `subjectDistance` double DEFAULT NULL,
  `subjectDistanceCategory` int(11) DEFAULT NULL,
  PRIMARY KEY (`imageid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `imagemetadata`
--

LOCK TABLES `imagemetadata` WRITE;
/*!40000 ALTER TABLE `imagemetadata` DISABLE KEYS */;
INSERT INTO `imagemetadata` VALUES (1,'NIKON CORPORATION','NIKON D90','Nikon AF-S DX Nikkor 35mm f/1.8G',3.5,35,52,0.02,0,0,200,NULL,0,NULL,5,NULL,0),(2,'NIKON CORPORATION','NIKON D90','Nikon AF-S DX Nikkor 35mm f/1.8G',3.5,35,52,0.02,0,0,200,NULL,0,NULL,5,NULL,0),(10,'NIKON CORPORATION','NIKON D90','Nikon AF-S DX Nikkor 35mm f/1.8G',3.5,35,52,0.02,0,0,200,24,0,NULL,5,NULL,0),(11,'NIKON CORPORATION','NIKON D90','Nikon AF-S DX Nikkor 35mm f/1.8G',3.5,35,52,0.02,0,0,200,24,0,NULL,5,NULL,0),(12,'NIKON CORPORATION','NIKON D90','Nikon AF-S DX Nikkor 35mm f/1.8G',3.5,35,52,0.02,0,0,200,24,0,NULL,5,NULL,0);
/*!40000 ALTER TABLE `imagemetadata` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `imagepositions`
--

DROP TABLE IF EXISTS `imagepositions`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `imagepositions` (
  `imageid` int(11) NOT NULL,
  `latitude` longtext,
  `latitudeNumber` double DEFAULT NULL,
  `longitude` longtext,
  `longitudeNumber` double DEFAULT NULL,
  `altitude` double DEFAULT NULL,
  `orientation` double DEFAULT NULL,
  `tilt` double DEFAULT NULL,
  `roll` double DEFAULT NULL,
  `accuracy` double DEFAULT NULL,
  `description` longtext,
  PRIMARY KEY (`imageid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `imagepositions`
--

LOCK TABLES `imagepositions` WRITE;
/*!40000 ALTER TABLE `imagepositions` DISABLE KEYS */;
INSERT INTO `imagepositions` VALUES (1,'0,0.00000000N',0,'90,0.00000000E',90,200,NULL,NULL,NULL,NULL,NULL);
/*!40000 ALTER TABLE `imagepositions` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `imageproperties`
--

DROP TABLE IF EXISTS `imageproperties`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `imageproperties` (
  `imageid` int(11) NOT NULL,
  `property` longtext NOT NULL,
  `value` longtext NOT NULL,
  UNIQUE KEY `imageid` (`imageid`,`property`(255))
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `imageproperties`
--

LOCK TABLES `imageproperties` WRITE;
/*!40000 ALTER TABLE `imageproperties` DISABLE KEYS */;
INSERT INTO `imageproperties` VALUES (1,'country','Forever Iced'),(1,'city','Santa Claus'),(1,'location','North Pole'),(1,'provinceState','Icecream'),(4,'country',''),(4,'countryCode',''),(4,'city','Nowhere'),(4,'location',''),(4,'provinceState','Unknown');
/*!40000 ALTER TABLE `imageproperties` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `imagerelations`
--

DROP TABLE IF EXISTS `imagerelations`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `imagerelations` (
  `subject` int(11) DEFAULT NULL,
  `object` int(11) DEFAULT NULL,
  `type` int(11) DEFAULT NULL,
  UNIQUE KEY `subject` (`subject`,`object`,`type`),
  KEY `subject_relations_index` (`subject`),
  KEY `object_relations_index` (`object`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `imagerelations`
--

LOCK TABLES `imagerelations` WRITE;
/*!40000 ALTER TABLE `imagerelations` DISABLE KEYS */;
/*!40000 ALTER TABLE `imagerelations` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `images`
--

DROP TABLE IF EXISTS `images`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `images` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `album` int(11) DEFAULT NULL,
  `name` longtext NOT NULL,
  `status` int(11) NOT NULL,
  `category` int(11) NOT NULL,
  `modificationDate` datetime DEFAULT NULL,
  `fileSize` int(11) DEFAULT NULL,
  `uniqueHash` varchar(128) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `album` (`album`,`name`(255)),
  KEY `dir_index` (`album`),
  KEY `hash_index` (`uniqueHash`),
  KEY `image_name_index` (`name`(333))
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `images`
--

LOCK TABLES `images` WRITE;
/*!40000 ALTER TABLE `images` DISABLE KEYS */;
INSERT INTO `images` VALUES (1,2,'foto001.jpg',1,1,'2011-06-28 02:13:14',145538,'874f072fa8186c7e555c76c139ec3223'),(2,2,'foto001bw.jpg',1,1,'2011-06-28 02:13:14',105505,'62005ab09158c728652c972600f72b22'),(3,3,'snap001.png',1,1,'2011-06-28 02:13:14',173933,'1b9ef89b5b08b2866196e66435436bbc'),(4,3,'snap002.png',1,1,'2011-06-28 15:43:41',71591,'b35bdedfde2dde6c4c30e607ff29ce39'),(5,4,'big-image.png',1,1,'2011-06-28 02:13:14',114493,'c528d0080c44b6cadfb5f46202f57af7'),(6,4,'icc-test-farbkreis.jpg',1,1,'2011-06-28 02:13:14',75675,'723417e0715e351b1f8dfb44ca9d47a5'),(9,4,'Martian_face_viking.jpg',1,1,'2011-06-28 02:13:14',99416,'8c834a0330d63fd804d5966d2132bf1d'),(10,5,'foto001q5.pgf',1,1,'2011-06-28 02:13:14',89012,'90f2915b9c26374327e8b33b1893a480'),(11,6,'foto001q5.pgf',1,1,'2011-06-28 02:13:14',89012,'90f2915b9c26374327e8b33b1893a480'),(12,6,'otherlink.pgf',1,1,'2011-06-28 02:13:14',89012,'90f2915b9c26374327e8b33b1893a480'),(14,4,'icc-test-farbkreis_v1.jpg',1,1,'2011-06-28 17:42:03',27973,'7e2766d97c464bf69427b5e1d480c42a'),(15,4,'icc-test-no-profile.jpg',1,1,'2011-06-28 17:42:36',50331,'2c2fae970fe09d46a08e3bd663d0c464');
/*!40000 ALTER TABLE `images` ENABLE KEYS */;
UNLOCK TABLES;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8 */ ;
/*!50003 SET character_set_results = utf8 */ ;
/*!50003 SET collation_connection  = utf8_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = '' */ ;
DELIMITER ;;
/*!50003 CREATE*/ /*!50017 DEFINER=`digikam`@`%`*/ /*!50003 TRIGGER delete_image AFTER DELETE ON Images
                    FOR EACH ROW BEGIN
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
                    END */;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;

--
-- Table structure for table `imagetagproperties`
--

DROP TABLE IF EXISTS `imagetagproperties`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `imagetagproperties` (
  `imageid` int(11) DEFAULT NULL,
  `tagid` int(11) DEFAULT NULL,
  `property` text,
  `value` longtext,
  KEY `imagetagproperties_index` (`imageid`,`tagid`),
  KEY `imagetagproperties_imageid_index` (`imageid`),
  KEY `imagetagproperties_tagid_index` (`tagid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `imagetagproperties`
--

LOCK TABLES `imagetagproperties` WRITE;
/*!40000 ALTER TABLE `imagetagproperties` DISABLE KEYS */;
INSERT INTO `imagetagproperties` VALUES (9,40,'tagRegion','<rect x=\"302\" y=\"83\" width=\"87\" height=\"105\"/>');
/*!40000 ALTER TABLE `imagetagproperties` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `imagetags`
--

DROP TABLE IF EXISTS `imagetags`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `imagetags` (
  `imageid` int(11) NOT NULL,
  `tagid` int(11) NOT NULL,
  UNIQUE KEY `imageid` (`imageid`,`tagid`),
  KEY `tag_index` (`tagid`),
  KEY `tag_id_index` (`imageid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `imagetags`
--

LOCK TABLES `imagetags` WRITE;
/*!40000 ALTER TABLE `imagetags` DISABLE KEYS */;
INSERT INTO `imagetags` VALUES (1,37),(3,14),(3,15),(3,16),(3,17),(3,27),(3,32),(3,33),(3,35),(4,1),(4,2),(4,3),(9,40);
/*!40000 ALTER TABLE `imagetags` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `searches`
--

DROP TABLE IF EXISTS `searches`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `searches` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `type` int(11) DEFAULT NULL,
  `name` longtext NOT NULL,
  `query` longtext NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `searches`
--

LOCK TABLES `searches` WRITE;
/*!40000 ALTER TABLE `searches` DISABLE KEYS */;
/*!40000 ALTER TABLE `searches` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `settings`
--

DROP TABLE IF EXISTS `settings`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `settings` (
  `keyword` longtext NOT NULL,
  `value` longtext,
  UNIQUE KEY `keyword` (`keyword`(255))
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `settings`
--

LOCK TABLES `settings` WRITE;
/*!40000 ALTER TABLE `settings` DISABLE KEYS */;
INSERT INTO `settings` VALUES ('preAlpha010Update1','true'),('preAlpha010Update2','true'),('preAlpha010Update3','true'),('beta010Update1','true'),('beta010Update2','true'),('uniqueHashVersion','2'),('databaseImageFormats','jpg;jpeg;jpe;jp2;j2k;jpx;jpc;pgx;tif;tiff;png;xpm;ppm;pnm;pgf;gif;bmp;xcf;pcx;bay;bmq;cr2;crw;cs1;dc2;dcr;dng;erf;fff;hdr;k25;kdc;mdc;mos;mrw;nef;orf;pef;pxn;raf;raw;rdc;sr2;srf;x3f;arw;3fr;cine;ia;kc2;mef;nrw;qtk;rw2;sti;rwl;srw;'),('databaseVideoFormats','mpeg;mpg;mpo;mpe;avi;mov;wmf;asf;mp4;3gp;wmv'),('databaseAudioFormats','ogg;mp3;wma;wav'),('FilterSettingsVersion','3'),('DcrawFilterSettingsVersion','4'),('DBVersion','6'),('DBVersionRequired','6'),('databaseUUID','{5d9bbc9c-62ce-4691-b7a2-1b2c44f50295}'),('Locale','UTF-8'),('DBThumbnailsVersion','2'),('DBThumbnailsVersionRequired','1'),('DeleteRemovedCompleteScanCount','3'),('Scanned','2011-06-28T17:34:39'),('databaseUserImageFormats',NULL),('databaseUserVideoFormats',NULL),('databaseUserAudioFormats',NULL),('RemovedItemsTime','2011-06-28T17:41:40');
/*!40000 ALTER TABLE `settings` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tagproperties`
--

DROP TABLE IF EXISTS `tagproperties`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `tagproperties` (
  `tagid` int(11) DEFAULT NULL,
  `property` text,
  `value` longtext,
  KEY `tagproperties_index` (`tagid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tagproperties`
--

LOCK TABLES `tagproperties` WRITE;
/*!40000 ALTER TABLE `tagproperties` DISABLE KEYS */;
INSERT INTO `tagproperties` VALUES (4,'internalTag',NULL),(5,'internalTag',NULL),(6,'internalTag',NULL),(8,'person',NULL),(8,'unknownPerson',NULL),(9,'internalTag',NULL),(10,'internalTag',NULL),(11,'internalTag',NULL),(12,'internalTag',NULL),(13,'tagKeyboardShortcut',''),(14,'tagKeyboardShortcut',''),(15,'tagKeyboardShortcut',''),(16,'tagKeyboardShortcut',''),(17,'internalTag',NULL),(18,'internalTag',NULL),(19,'internalTag',NULL),(20,'internalTag',NULL),(21,'internalTag',NULL),(22,'internalTag',NULL),(23,'internalTag',NULL),(24,'internalTag',NULL),(25,'internalTag',NULL),(26,'internalTag',NULL),(27,'internalTag',NULL),(28,'internalTag',NULL),(29,'internalTag',NULL),(30,'internalTag',NULL),(31,'tagKeyboardShortcut',''),(32,'tagKeyboardShortcut',''),(33,'tagKeyboardShortcut',''),(34,'tagKeyboardShortcut',''),(35,'tagKeyboardShortcut',''),(36,'tagKeyboardShortcut',''),(37,'tagKeyboardShortcut',''),(38,'tagKeyboardShortcut',''),(39,'tagKeyboardShortcut',''),(40,'tagKeyboardShortcut',''),(40,'person','LadyMars'),(40,'kfaceId','LadyMars');
/*!40000 ALTER TABLE `tagproperties` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tags`
--

DROP TABLE IF EXISTS `tags`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `tags` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `pid` int(11) DEFAULT NULL,
  `name` longtext NOT NULL,
  `icon` int(11) DEFAULT NULL,
  `iconkde` longtext,
  `lft` int(11) NOT NULL,
  `rgt` int(11) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tags`
--

LOCK TABLES `tags` WRITE;
/*!40000 ALTER TABLE `tags` DISABLE KEYS */;
INSERT INTO `tags` VALUES (0,-1,'_Digikam_root_tag_',0,NULL,1,82),(1,15,'tagl2a',0,NULL,80,81),(2,36,'tl0a',0,NULL,76,79),(3,2,'tl0a0',0,NULL,74,75),(4,0,'_Digikam_Internal_Tags_',0,NULL,32,73),(5,4,'Need Resolving History',0,NULL,71,72),(6,4,'Need Tagging History Graph',0,NULL,69,70),(7,0,'People',0,NULL,26,31),(8,7,'Unknown',0,NULL,29,30),(9,4,'Original Version',0,NULL,67,68),(10,4,'Intermediate Version',0,NULL,65,66),(11,4,'Current Version',0,NULL,63,64),(12,4,'Version Always Visible',0,NULL,61,62),(13,0,'tagl0a',0,NULL,8,25),(14,13,'tagl1a',0,NULL,23,24),(15,13,'tagl1b',0,NULL,11,22),(16,13,'tagl1c',0,NULL,9,10),(17,4,'Color Label None',0,NULL,59,60),(18,4,'Color Label Red',0,NULL,57,58),(19,4,'Color Label Orange',0,NULL,55,56),(20,4,'Color Label Yellow',0,NULL,53,54),(21,4,'Color Label Green',0,NULL,51,52),(22,4,'Color Label Blue',0,NULL,49,50),(23,4,'Color Label Magenta',0,NULL,47,48),(24,4,'Color Label Gray',0,NULL,45,46),(25,4,'Color Label Black',0,NULL,43,44),(26,4,'Color Label White',0,NULL,41,42),(27,4,'Pick Label None',0,NULL,39,40),(28,4,'Pick Label Rejected',0,NULL,37,38),(29,4,'Pick Label Pending',0,NULL,35,36),(30,4,'Pick Label Accepted',0,NULL,33,34),(31,15,'tagl2b',0,NULL,16,21),(32,15,'tagl2c',0,NULL,14,15),(33,15,'tagl2d',0,NULL,12,13),(34,31,'tagl3a',0,NULL,19,20),(35,31,'tagl3b',0,NULL,17,18),(36,0,'tl0',0,NULL,2,7),(37,2,'tl0a1',0,NULL,77,78),(38,36,'tl0b',0,NULL,5,6),(39,36,'tagl1c',0,NULL,3,4),(40,7,'LadyMars',0,NULL,27,28);
/*!40000 ALTER TABLE `tags` ENABLE KEYS */;
UNLOCK TABLES;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8 */ ;
/*!50003 SET character_set_results = utf8 */ ;
/*!50003 SET collation_connection  = utf8_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = '' */ ;
DELIMITER ;;
/*!50003 CREATE*/ /*!50017 DEFINER=`digikam`@`%`*/ /*!50003 TRIGGER delete_tag AFTER DELETE ON Tags
            FOR EACH ROW BEGIN
                DELETE FROM ImageTags          WHERE tagid=OLD.id;
                DELETE FROM TagProperties      WHERE tagid=OLD.id;
                DELETE FROM ImageTagProperties WHERE tagid=OLD.id;
            END */;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;

--
-- Table structure for table `thumbnails`
--

DROP TABLE IF EXISTS `thumbnails`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `thumbnails` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `type` int(11) DEFAULT NULL,
  `modificationDate` datetime DEFAULT NULL,
  `orientationHint` int(11) DEFAULT NULL,
  `data` longblob,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `thumbnails`
--

LOCK TABLES `thumbnails` WRITE;
/*!40000 ALTER TABLE `thumbnails` DISABLE KEYS */;
/*!40000 ALTER TABLE `thumbnails` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `uniquehashes`
--

DROP TABLE IF EXISTS `uniquehashes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `uniquehashes` (
  `uniqueHash` varchar(128) DEFAULT NULL,
  `fileSize` int(11) DEFAULT NULL,
  `thumbId` int(11) DEFAULT NULL,
  UNIQUE KEY `uniqueHash` (`uniqueHash`,`fileSize`),
  KEY `id_uniqueHashes` (`thumbId`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `uniquehashes`
--

LOCK TABLES `uniquehashes` WRITE;
/*!40000 ALTER TABLE `uniquehashes` DISABLE KEYS */;
INSERT INTO `uniquehashes` VALUES ('c528d0080c44b6cadfb5f46202f57af7',114493,14),('90f2915b9c26374327e8b33b1893a480',89012,21),('8c834a0330d63fd804d5966d2132bf1d',99416,18),('2c2fae970fe09d46a08e3bd663d0c464',50331,17),('49fc239f8d5cf5c4d256ae6bb098aee4',107932,5),('723417e0715e351b1f8dfb44ca9d47a5',75675,15),('b35bdedfde2dde6c4c30e607ff29ce39',71591,13),('7e2766d97c464bf69427b5e1d480c42a',27973,16),('874f072fa8186c7e555c76c139ec3223',145538,10),('62005ab09158c728652c972600f72b22',105505,11),('1b9ef89b5b08b2866196e66435436bbc',173933,12);
/*!40000 ALTER TABLE `uniquehashes` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping routines for database 'digikam'
--
/*!50003 DROP PROCEDURE IF EXISTS `create_index_if_not_exists` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8 */ ;
/*!50003 SET character_set_results = utf8 */ ;
/*!50003 SET collation_connection  = utf8_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = '' */ ;
DELIMITER ;;
/*!50003 CREATE*/ /*!50020 DEFINER=`digikam`@`%`*/ /*!50003 PROCEDURE `create_index_if_not_exists`(table_name_vc varchar(50), index_name_vc varchar(50), field_list_vc varchar(1024))
    SQL SECURITY INVOKER
BEGIN

                set @Index_cnt = (
                    SELECT COUNT(1) cnt
                    FROM INFORMATION_SCHEMA.STATISTICS
                    WHERE CONVERT(DATABASE() USING latin1) = CONVERT(TABLE_SCHEMA USING latin1)
                      AND CONVERT(table_name USING latin1) = CONVERT(table_name_vc USING latin1)
                      AND CONVERT(index_name USING latin1) = CONVERT(index_name_vc USING latin1)
                );

                IF IFNULL(@Index_cnt, 0) = 0 THEN
                    set @index_sql = CONCAT( 
                        CONVERT( 'ALTER TABLE ' USING latin1),
                        CONVERT( table_name_vc USING latin1),
                        CONVERT( ' ADD INDEX ' USING latin1),
                        CONVERT( index_name_vc USING latin1),
                        CONVERT( '(' USING latin1),
                        CONVERT( field_list_vc USING latin1),
                        CONVERT( ');' USING latin1)
                    );
                    PREPARE stmt FROM @index_sql;
                    EXECUTE stmt;
                    DEALLOCATE PREPARE stmt;
                END IF;
                END */;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

