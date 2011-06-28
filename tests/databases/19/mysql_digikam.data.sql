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
  UNIQUE KEY `identifier` (`identifier`(167),`specificPath`(166))
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `albumroots`
--

LOCK TABLES `albumroots` WRITE;
/*!40000 ALTER TABLE `albumroots` DISABLE KEYS */;
INSERT INTO `albumroots` VALUES (1,NULL,0,1,'volumeid:?uuid=800b21c2-dadc-4930-829e-a96b04ce26fa','/vivo/digikam-devel/data/testimages/a1'),(3,'a2',0,1,'volumeid:?uuid=800b21c2-dadc-4930-829e-a96b04ce26fa','/vivo/digikam-devel/data/testimages/a2');
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
  UNIQUE KEY `albumRoot` (`albumRoot`,`relativePath`(332))
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `albums`
--

LOCK TABLES `albums` WRITE;
/*!40000 ALTER TABLE `albums` DISABLE KEYS */;
INSERT INTO `albums` VALUES (1,1,'/','2011-06-28',NULL,NULL,NULL),(2,1,'/jpg','2011-06-28',NULL,NULL,NULL),(3,1,'/png','2011-06-28',NULL,NULL,NULL),(4,2,'/','2011-06-28',NULL,NULL,NULL),(5,2,'/pgf','2011-06-28',NULL,NULL,NULL),(6,2,'/pgf/link','2011-06-28',NULL,NULL,NULL),(7,3,'/','2011-06-28',NULL,NULL,NULL),(8,3,'/pgf','2011-06-28',NULL,NULL,NULL),(9,3,'/pgf/link','2011-06-28',NULL,NULL,NULL);
/*!40000 ALTER TABLE `albums` ENABLE KEYS */;
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
  UNIQUE KEY `imageid` (`imageid`,`type`,`language`,`author`(202))
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
  UNIQUE KEY `imageid` (`imageid`,`property`(110),`value`(111),`extraValue`(111))
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
INSERT INTO `imagehaarmatrix` VALUES (1,'2011-06-28 02:13:14','64ba82087739998c8ca2b6fe795a27ef','\0\0\0?Ò‚ä5Šå\0¿…ÉC¦Ïe¿³&Tt*rÿÿÿ\0\0\0$ÿÿÿÛÿÿÿ÷\0\0ÿÿşüÿÿú}ÿÿù\0\0\0\0\0\0\0\0\0\0\0\0\0\0€\0\0\0ÿÿù€ÿÿÿ~\0\0»ÿÿÿÈÿÿÿyÿÿÿõ\0\0\0\0\n\0\0‡ÿÿÿıÿÿşóÿÿÿş\0\0\0\0\0\0ÿÿÿäÿÿÿùÿÿú€\0\0\0\rÿÿı|ÿÿúşÿÿü€\0\0ÿÿı~ÿÿş€ÿÿÿ\0\0\0”\0\0†ÿÿû\0\0’ÿÿıî\0\0	\0\0€\0\0\0ŒÿÿöP\0\0ÿÿûPÿÿÿ¢\0\0‚\0\0\0\0İÿÿı8ÿÿúÑÿÿû#ÿÿ÷\0ÿÿşúÿÿıÜ\0\0	ÿÿşô\0\0\0‰\0\0HÿÿöR\0\0\0\0\0„ÿÿÿ\0\0Œÿÿöÿ\0\0¯ÿÿöuÿÿÿõ\0\0\0	ÿÿÿÿÿÿş\0ÿÿıÿÿÿÿ\0ÿÿÿ€ÿÿşî\0\0	ÿÿûùÿÿıøÿÿ÷\0\0\0\0Œÿÿÿø\0\0Œÿÿşé\0\0\n\0\0H\0\0\0%\0\0\0	ÿÿıÜÿÿşíÿÿúşÿÿşõÿÿÿuÿÿş·\0\0%ÿÿûü\0\0ÿÿûÿÿı~\0\0\0ÿÿöÿ\0\0„ÿÿÿü\0\0ÿÿıüÿÿÿõ\0\0€ÿÿû€ÿÿÿÿÿÿıÿÿÿşÿÿÿÿ€ÿÿÿşÿÿş\0ÿÿÿ\0'),(2,'2011-06-28 02:13:14','faff1ab097816445d6c99618634c1054','\0\0\0?Ò³/rú},¼r@ÁaèÁE¼dòé:RÛ\0\0‚\0\0\0$ÿÿÿÛ\0\0ÿÿÿ÷ÿÿşüÿÿú}ÿÿù\0\0\0\0\0\0\0€\0\0ÿÿÿ~ÿÿù€\0\0\0\0\0\0ÿÿÿõ\0\0»ÿÿÿÈ\0\0ÿÿÿy\0\0\0\0\nÿÿÿş\0\0‡ÿÿşóÿÿÿı\0\0\0\0\0\0ÿÿÿäÿÿÿùÿÿú€\0\0\0\rÿÿı|ÿÿúşÿÿü€ÿÿı~\0\0ÿÿş€ÿÿÿ\0ÿÿÿeÿÿşãÿÿÏ\0\0\0Œÿÿòu\0\0€\0\0\0	\0\0-|ÿÿõEÿÿÿüÿÿë«\0\0ÿÿôùÿÿıÿÿÿú\0\0\0\r\0\0\0‡\0\0\08ÿÿõÿÿÿşôÿÿô\0\0\0\0ÿÿúõÿÿç€ÿÿÿå\0\0\nñ\0\0€ÿÿıy\0\0\0\0\0\0\0\0\0ÿÿÿúÿÿÿó\0\0\0\0€\0\0„ÿÿşÿ\0\0‚\0\0€\0\0\0ÿÿôñÿÿú\0\0-ƒÿÿıÿ\0\0w\0\0ş\0\0(«\0\0ÿÿÿe\0\0ÿÿıy\0\0\0‡\0\0¯\0\0,\0\0)ñ\0\0€\0\0\0‚\0\0\0ÿÿÛĞ\0\0([\0\0-ãÿÿÿú\0\0„ÿÿÒq\0\0\0\0\0rÿÿúõ\0\0\nñ\0\0€\0\0\0\0\n\0\0\rÿÿÿóÿÿıE\0\0\0\0\0\nò\0\0*òÿÿşÿ\0\0‚\0\0\0'),(3,'2011-06-28 02:13:14','b7f6e38ad04f1a8b4307cee7d4d144f2','\0\0\0?ëë÷?I¿€]…Nù5¿Héêg$ÿÿÿû\0\0\0\0\0\0\0\0\0\0ÿÿş~\0\0\0ˆÿÿİ\0ÿÿøx\0\0\0ÿÿø\0\0\0\0\0\0\0‚ÿÿòÿÿÿÖ\0\0\0~ÿÿò~ÿÿò€\0\0\0\0\0ÿÿüÿÿÿßÿÿÿ½\0\0\0ÿÿğ€\0\0\0 ÿÿşÿÿÿşÿÿà€\0\0\0@\0\0€ÿÿÿ\0ÿÿÿÿÿÿ÷€ÿÿÿü\0\0€\0\0€ÿÿş\0ÿÿÿøÿÿÿş\0\0ÿÿï{ÿÿûş\0\0\0\0\0ÿÿä~\0\0\0\0ÿÿú€ÿÿı]\0\0\0ÿÿ÷ûÿÿÿàÿÿÿìÿÿşï\0\0\0\0„ÿÿïúÿÿşû\0\0‚\0\0\0\0\0?ÿÿıû\0\0ÿÿÿÿ\0\0€ÿÿûûÿÿÿ€ÿÿÿ\0ÿÿà€\0\0 \0ÿÿß\0\0\0€\0\0€ÿÿï€\0\0\0ÿÿş\0ÿÿø\0ÿÿü\0ÿÿû¿ÿÿùõ\0\0‹\0\0\0\0\0\0‹\0\0ƒÿÿÿıÿÿşû\0\0\0ÿÿşÓ\0\0€ÿÿúıÿÿÿô\0\0€\0\0\0\0‚\0\0‹\0\0–\0\0–\0\0ƒÿÿü€ÿÿÿëÿÿÿúÿÿş€\0\0\0ÿÿÿ}\0\0\0\0\0€\0\0…ÿÿşê\0\0\0ÿÿğ\0\0\0\0-ÿÿşõ\0\0€\0\0\0\0\0\0\0\0\0\0\0\0'),(4,'2011-06-28 15:43:41','570d684473df126f54b8436a444b3ba1','\0\0\0?êYaºï¡?t6]×åÿ¿kFL\'öÿ}ÿÿú€ÿÿşş\0\0ÿÿÌ€ÿÿó\0\0€ÿÿë\0ÿÿùÿ\0\0€ÿÿòxÿÿä{ÿÿôû\0\0\0‚ÿÿÿÿÿÿÿ}\0\0\0\0\0…ÿÿæ\0\03ÿÿú}\0\0\0\0\r…ÿÿä€ÿÿäÿÿÈü\0\0ÿÿÌy\0\0€ÿÿÿ€ÿÿİ€ÿÿï\0\0\0\0ÿÿ÷€\0\0>€\0\0€\0\0€\0\0€ÿÿÿ\0ÿÿü\0ÿÿş\0\0\0\0\0\0ÿÿı}ÿÿÔ}ÿÿú{ÿÿëzÿÿêjÿÿéÿ\0\0\n‚ÿÿúı\0\0ÿÿê~\0\0€\0\0+Œ\0\0\0€\0\0ƒÿÿû\0\0\0\0\0\0ƒÿÿúÿÿõ}\0\0†\0\0\0ÿÿÓ\0ÿÿÒı\0\0\0\0ÿÿôı\0\0-€\0\0†\0\0\n…ÿÿõ€\0\0-ƒÿÿê{ÿÿõ\0ÿÿÿ\0ÿÿşı\0\0Œ\0\0€\0\0ƒ\0\0Œ\0\0ÿÿë{ÿÿúıÿÿêjÿÿéÿÿÿêıÿÿê~\0\0€\0\0\0€\0\0+Œ\0\0\0\0ƒÿÿı€ÿÿû\0\0\0\nÿÿúÿÿşıÿÿÓ\0ÿÿÒı\0\0\0\0ÿÿôı\0\0\n‚ÿÿÿ\0\0\0†\0\0-€\0\0-ƒÿÿõ\0\0\0†ÿÿı}ÿÿõzÿÿê{ÿÿêú\0\0Œ\0\0\n…ÿÿõ}ÿÿõ€\0\0€\0\0ƒ'),(5,'2011-06-28 02:13:14','73ded88d97601deb2d42dedac59a4786','\0\0\0?uî£¾-Å¿8<¿3ø¡[¾¹RÚºMªÿÿşu\0\0±ÿÿèÏ\0\0®\0\0\0\0\01\0\0\0\0Œ\0\0ÿÿúOÿÿÿû\0\0\0ÿÿçuÿÿèô\0\0Œ\0\0—\0\0ÿÿızÿÿú}ÿÿşôÿÿı€\0\0…\0\0\0\0‚ÿÿùûÿÿüõ\0\0\0\0\0\0\0ÿÿÿõ\0\0\0ÿÿúzÿÿıtÿÿú€\0\0‹\0\0…ÿÿùõ\0\0ÿÿút\0\0‹\0\0\0ÿÿçèÿÿú{ÿÿıuÿÿú\0ÿÿóıÿÿşhÿÿùúÿÿüôÿÿèP\0\0†\0\0˜\0\0\0\0Œ\0\0¯\0\0€\0\0\0\0ÿÿÿèÿÿô{\0\0\0\0\0ÿÿıiÿÿô\0ÿÿúuÿÿóúÿÿüèÿÿùô\0\0Œ\0\0˜\0\0\0\0ÿÿôuÿÿúiÿÿóôÿÿùè\0\0˜\0\0ÿÿôiÿÿóè\0\0/İ\0\0\0\0¯\0\00ÿÿÏĞÿÿĞiÿÿô!\0\0^ÿÿè\"ÿÿè ÿÿĞ\0\00`ÿÿĞO\0\0\0\00a\0\0/âÿÿú!\0\0àÿÿúi\0\0\0\01^ÿÿĞQÿÿôi\0\0Ş\0\0/`ÿÿôP\0\0b\0\0\0\0^ÿÿÏ£\0\0\0\0/0\0\0/^ÿÿÏÿÿÎŸÿÿÑ!ÿÿĞP\0\0âÿÿç¡ÿÿè#'),(6,'2011-06-28 02:13:14','6e6426a193df285022b616a563ce4ad6','\0\0\0?Èª0ı2ÅÏ¿l.Öoœı?Ou’˜Òeõÿÿısÿÿûyÿÿüò\0\0\0‚\0\0‡\0\0ÿÿşúÿÿı}\0\0‚ÿÿü\0\0\0‚ÿÿÿü\0\0\0ÿÿÿòÿÿù\0ÿÿıùÿÿÿû\0\0ƒ\0\0ÿÿüzÿÿüù\0\0ÿÿø~\0\0\0\0ƒÿÿøıÿÿşrÿÿşñ\0\0€\0\0‡ÿÿÿ\0\0\0€ÿÿş\0\0\0ÿÿÿş\0\0\0\0\0ÿÿÿÿÿÿş}\0\0\0\0\0‡\0\0\0ÿÿşwÿÿüüÿÿû{ÿÿûúÿÿøû\0\0\0\0€ÿÿøz\0\0…ÿÿıû\0\0†\0\0ÿÿü{\0\0€\0\0\0ÿÿø}\0\0ÿÿûı\0\0‚ÿÿÿ|ÿÿû~\0\0\0‡\0\0\0\0\0ƒÿÿÿ~\0\0ƒÿÿş\0ÿÿøş\0\0€ÿÿü€ÿÿü\0ÿÿø€ÿÿşş\0\0\0\0\0ƒ\0\0ÿÿş~\0\0\0€\0\0‚ÿÿø€\0\0€\0\0ÿÿıyÿÿşıÿÿüy\0\0\0	ÿÿşx\0\0‰\0\0\0\0\0\0\0\0\0\0\0\0†\0\0\0ÿÿşòÿÿıûÿÿû~\0\0\0\0ÿÿşq\0\0ÿÿıÿÿÿı|\0\0ÿÿşÿ\0\0ÿÿÿøÿÿş÷ÿÿÿñ\0\0ÿÿÿüÿÿÿù\0\0\0\0\0\0ÿÿşş\0\0\0€\0\0ƒ\0\0\0'),(8,'2011-06-28 17:42:36','6c335b8c4891b709969aaa5ec36ef097','\0\0\0?Æ×ÀãØ-Ü?a¨)¿_(¿YÖâAŞöÿÿıû\0\0\0	\0\0\0\0\0\0‚\0\0\0\0\0ÿÿü|\0\0\0\0\0ÿÿüz\0\0†ÿÿø{ÿÿÿ}\0\0\0‚ÿÿşx\0\0\0\0\0\0\0\0\0\0ÿÿÿ€ÿÿù\0ÿÿı|\0\0…ÿÿşşÿÿş\0\0\0ƒÿÿøı\0\0‰\0\0\0ÿÿş÷ÿÿø~ÿÿş}ÿÿÿø\0\0€\0\0ÿÿÿüÿÿÿ\0\0\0€\0\0‚ÿÿûş\0\0€ÿÿÿ}ÿÿşñ\0\0ÿÿş€\0\0\0\0\0ƒ\0\0‡ÿÿÿúÿÿÿûÿÿıüÿÿÿò\0\0\0\0\0€ÿÿÿ\0\0\0ÿÿşø\0\0ÿÿüzÿÿş\0\0‚\0\0ÿÿøıÿÿü\0\0\0\0\0\0\0„ÿÿüù\0\0	\0\0\0\0\0\0ÿÿÿş\0\0\0ÿÿşrÿÿÿı\0\0ÿÿş}ÿÿÿ€ÿÿÿÿ\0\0\0‡ÿÿÿ~ÿÿü€\0\0	ÿÿş\0ÿÿÿûÿÿşøÿÿûıÿÿıù\0\0‚ÿÿÿú\0\0„ÿÿÿ÷ÿÿø€ÿÿü\0ÿÿşñÿÿş\0\0ÿÿü{\0\0\0\0\0\0\0ƒÿÿü\0\0ˆ\0\0\0ÿÿøş\0\0\0\0‡ÿÿÿıÿÿüü\0\0\0\0\0\0\0\0\0\0\0ÿÿşwÿÿÿş\0\0ÿÿş~\0\0\0€ÿÿÿÿ'),(9,'2011-06-28 02:13:14','d77bfd4de3e8e4b2880a30cb03c824e6','\0\0\0?á/#I`¼€ó\nÚmû>¼s”ZÎ^\0\0\0\0„ÿÿı}ÿÿşè\0\0\0…\0\0\0Šÿÿı\0ÿÿñ€ÿÿÿg\0\0“ÿÿÿú\0\0‡ÿÿü|\0\0ÿÿør\0\0\0\0Šÿÿÿwÿÿÿzÿÿüı\0\0…ÿÿù\0ÿÿüó\0\0	ÿÿúzÿÿür\0\0\0ÿÿÿûÿÿÿñ\0\0ÿÿşwÿÿÿ\0\0€ÿÿüÿÿÿş~ÿÿÿù\0\0ÿÿÿ~ÿÿÿıÿÿÿÿÿÿñ}\0\0\0\0\0ÿÿâaÿÿááÿÿä2\0\0;º\0\0ıÿÿş\0\0†ÿÿşv\0\0‰ÿÿÄy\0\0;ôÿÿø\0\0\0\0\0\0\0\0\0\0\0\0\0wÿÿümÿÿÄÏÿÿøy\0\0ƒ\0\0\0ÿÿşç\0\0\0\0\0‰\0\0ÿÿøı\0\0\0ÿÿÄÿÿø€\0\0\0\0\0‚ÿÿüü\0\0\0‚\0\0\0\0\0\0\0\0ÿÿùt\0\0\0‰\0\0œ\0\0ÿÿüb\0\0\0ÿÿø€ÿÿıÿ\0\0€\0\0ÿÿÜÊ\0\0\0\0Ì\0\0\0\0¨\0\0	z\0\0\0„ÿÿúB\0\02\0\0wÿÿÿ\0ÿÿøq\0\0\0ÿÿışÿÿÃ\0\0„\0\0\0\0\0\0\0ƒ\0\0ƒ\0\0‰\0\0\0†\0\0ÿÿüü\0\0\0‚\0\0‚\0\0\0\0\0\0\0\0\0'),(10,'2011-06-28 02:13:14','891ade31c9d71cae9eefb599b467561e','\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0ÿÿÿÿÿÿÿıÿÿÿùÿÿÿñÿÿÿáÿÿÿØÿÿÿÙÿÿÿÚÿÿÿÛÿÿÿÜÿÿÿİÿÿÿŞÿÿÿßÿÿÿâÿÿÿàÿÿÿãÿÿÿäÿÿÿåÿÿÿæÿÿÿçÿÿÿèÿÿÿéÿÿÿêÿÿÿëÿÿÿìÿÿÿíÿÿÿîÿÿÿïÿÿÿòÿÿÿğÿÿÿóÿÿÿôÿÿÿõÿÿÿöÿÿÿ÷ÿÿÿúÿÿÿøÿÿÿûÿÿÿşÿÿÿüÿÿÿÿÿÿÿıÿÿÿùÿÿÿñÿÿÿáÿÿÿØÿÿÿÙÿÿÿÚÿÿÿÛÿÿÿÜÿÿÿİÿÿÿŞÿÿÿßÿÿÿâÿÿÿàÿÿÿãÿÿÿäÿÿÿåÿÿÿæÿÿÿçÿÿÿèÿÿÿéÿÿÿêÿÿÿëÿÿÿìÿÿÿíÿÿÿîÿÿÿïÿÿÿòÿÿÿğÿÿÿóÿÿÿôÿÿÿõÿÿÿöÿÿÿ÷ÿÿÿúÿÿÿøÿÿÿûÿÿÿşÿÿÿüÿÿÿÿÿÿÿıÿÿÿùÿÿÿñÿÿÿáÿÿÿØÿÿÿÙÿÿÿÚÿÿÿÛÿÿÿÜÿÿÿİÿÿÿŞÿÿÿßÿÿÿâÿÿÿàÿÿÿãÿÿÿäÿÿÿåÿÿÿæÿÿÿçÿÿÿèÿÿÿéÿÿÿêÿÿÿëÿÿÿìÿÿÿíÿÿÿîÿÿÿïÿÿÿòÿÿÿğÿÿÿóÿÿÿôÿÿÿõÿÿÿöÿÿÿ÷ÿÿÿúÿÿÿøÿÿÿûÿÿÿşÿÿÿü'),(11,'2011-06-28 02:13:14','891ade31c9d71cae9eefb599b467561e','\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0ÿÿÿÿÿÿÿıÿÿÿùÿÿÿñÿÿÿáÿÿÿØÿÿÿÙÿÿÿÚÿÿÿÛÿÿÿÜÿÿÿİÿÿÿŞÿÿÿßÿÿÿâÿÿÿàÿÿÿãÿÿÿäÿÿÿåÿÿÿæÿÿÿçÿÿÿèÿÿÿéÿÿÿêÿÿÿëÿÿÿìÿÿÿíÿÿÿîÿÿÿïÿÿÿòÿÿÿğÿÿÿóÿÿÿôÿÿÿõÿÿÿöÿÿÿ÷ÿÿÿúÿÿÿøÿÿÿûÿÿÿşÿÿÿüÿÿÿÿÿÿÿıÿÿÿùÿÿÿñÿÿÿáÿÿÿØÿÿÿÙÿÿÿÚÿÿÿÛÿÿÿÜÿÿÿİÿÿÿŞÿÿÿßÿÿÿâÿÿÿàÿÿÿãÿÿÿäÿÿÿåÿÿÿæÿÿÿçÿÿÿèÿÿÿéÿÿÿêÿÿÿëÿÿÿìÿÿÿíÿÿÿîÿÿÿïÿÿÿòÿÿÿğÿÿÿóÿÿÿôÿÿÿõÿÿÿöÿÿÿ÷ÿÿÿúÿÿÿøÿÿÿûÿÿÿşÿÿÿüÿÿÿÿÿÿÿıÿÿÿùÿÿÿñÿÿÿáÿÿÿØÿÿÿÙÿÿÿÚÿÿÿÛÿÿÿÜÿÿÿİÿÿÿŞÿÿÿßÿÿÿâÿÿÿàÿÿÿãÿÿÿäÿÿÿåÿÿÿæÿÿÿçÿÿÿèÿÿÿéÿÿÿêÿÿÿëÿÿÿìÿÿÿíÿÿÿîÿÿÿïÿÿÿòÿÿÿğÿÿÿóÿÿÿôÿÿÿõÿÿÿöÿÿÿ÷ÿÿÿúÿÿÿøÿÿÿûÿÿÿşÿÿÿü'),(12,'2011-06-28 02:13:14','891ade31c9d71cae9eefb599b467561e','\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0ÿÿÿÿÿÿÿıÿÿÿùÿÿÿñÿÿÿáÿÿÿØÿÿÿÙÿÿÿÚÿÿÿÛÿÿÿÜÿÿÿİÿÿÿŞÿÿÿßÿÿÿâÿÿÿàÿÿÿãÿÿÿäÿÿÿåÿÿÿæÿÿÿçÿÿÿèÿÿÿéÿÿÿêÿÿÿëÿÿÿìÿÿÿíÿÿÿîÿÿÿïÿÿÿòÿÿÿğÿÿÿóÿÿÿôÿÿÿõÿÿÿöÿÿÿ÷ÿÿÿúÿÿÿøÿÿÿûÿÿÿşÿÿÿüÿÿÿÿÿÿÿıÿÿÿùÿÿÿñÿÿÿáÿÿÿØÿÿÿÙÿÿÿÚÿÿÿÛÿÿÿÜÿÿÿİÿÿÿŞÿÿÿßÿÿÿâÿÿÿàÿÿÿãÿÿÿäÿÿÿåÿÿÿæÿÿÿçÿÿÿèÿÿÿéÿÿÿêÿÿÿëÿÿÿìÿÿÿíÿÿÿîÿÿÿïÿÿÿòÿÿÿğÿÿÿóÿÿÿôÿÿÿõÿÿÿöÿÿÿ÷ÿÿÿúÿÿÿøÿÿÿûÿÿÿşÿÿÿüÿÿÿÿÿÿÿıÿÿÿùÿÿÿñÿÿÿáÿÿÿØÿÿÿÙÿÿÿÚÿÿÿÛÿÿÿÜÿÿÿİÿÿÿŞÿÿÿßÿÿÿâÿÿÿàÿÿÿãÿÿÿäÿÿÿåÿÿÿæÿÿÿçÿÿÿèÿÿÿéÿÿÿêÿÿÿëÿÿÿìÿÿÿíÿÿÿîÿÿÿïÿÿÿòÿÿÿğÿÿÿóÿÿÿôÿÿÿõÿÿÿöÿÿÿ÷ÿÿÿúÿÿÿøÿÿÿûÿÿÿşÿÿÿü'),(13,'2011-06-28 02:13:14','73ded88d97601deb2d42dedac59a4786','\0\0\0?uî£¾-Å¿8<¿3ø¡[¾¹RÚºMªÿÿşu\0\0±ÿÿèÏ\0\0®\0\0\0\0\01\0\0\0\0Œ\0\0ÿÿúOÿÿÿû\0\0\0ÿÿçuÿÿèô\0\0Œ\0\0—\0\0ÿÿızÿÿú}ÿÿşôÿÿı€\0\0…\0\0\0\0‚ÿÿùûÿÿüõ\0\0\0\0\0\0\0ÿÿÿõ\0\0\0ÿÿúzÿÿıtÿÿú€\0\0‹\0\0…ÿÿùõ\0\0ÿÿút\0\0‹\0\0\0ÿÿçèÿÿú{ÿÿıuÿÿú\0ÿÿóıÿÿşhÿÿùúÿÿüôÿÿèP\0\0†\0\0˜\0\0\0\0Œ\0\0¯\0\0€\0\0\0\0ÿÿÿèÿÿô{\0\0\0\0\0ÿÿıiÿÿô\0ÿÿúuÿÿóúÿÿüèÿÿùô\0\0Œ\0\0˜\0\0\0\0ÿÿôuÿÿúiÿÿóôÿÿùè\0\0˜\0\0ÿÿôiÿÿóè\0\0/İ\0\0\0\0¯\0\00ÿÿÏĞÿÿĞiÿÿô!\0\0^ÿÿè\"ÿÿè ÿÿĞ\0\00`ÿÿĞO\0\0\0\00a\0\0/âÿÿú!\0\0àÿÿúi\0\0\0\01^ÿÿĞQÿÿôi\0\0Ş\0\0/`ÿÿôP\0\0b\0\0\0\0^ÿÿÏ£\0\0\0\0/0\0\0/^ÿÿÏÿÿÎŸÿÿÑ!ÿÿĞP\0\0âÿÿç¡ÿÿè#'),(14,'2011-06-28 02:13:14','6e6426a193df285022b616a563ce4ad6','\0\0\0?Èª0ı2ÅÏ¿l.Öoœı?Ou’˜Òeõÿÿısÿÿûyÿÿüò\0\0\0‚\0\0‡\0\0ÿÿşúÿÿı}\0\0‚ÿÿü\0\0\0‚ÿÿÿü\0\0\0ÿÿÿòÿÿù\0ÿÿıùÿÿÿû\0\0ƒ\0\0ÿÿüzÿÿüù\0\0ÿÿø~\0\0\0\0ƒÿÿøıÿÿşrÿÿşñ\0\0€\0\0‡ÿÿÿ\0\0\0€ÿÿş\0\0\0ÿÿÿş\0\0\0\0\0ÿÿÿÿÿÿş}\0\0\0\0\0‡\0\0\0ÿÿşwÿÿüüÿÿû{ÿÿûúÿÿøû\0\0\0\0€ÿÿøz\0\0…ÿÿıû\0\0†\0\0ÿÿü{\0\0€\0\0\0ÿÿø}\0\0ÿÿûı\0\0‚ÿÿÿ|ÿÿû~\0\0\0‡\0\0\0\0\0ƒÿÿÿ~\0\0ƒÿÿş\0ÿÿøş\0\0€ÿÿü€ÿÿü\0ÿÿø€ÿÿşş\0\0\0\0\0ƒ\0\0ÿÿş~\0\0\0€\0\0‚ÿÿø€\0\0€\0\0ÿÿıyÿÿşıÿÿüy\0\0\0	ÿÿşx\0\0‰\0\0\0\0\0\0\0\0\0\0\0\0†\0\0\0ÿÿşòÿÿıûÿÿû~\0\0\0\0ÿÿşq\0\0ÿÿıÿÿÿı|\0\0ÿÿşÿ\0\0ÿÿÿøÿÿş÷ÿÿÿñ\0\0ÿÿÿüÿÿÿù\0\0\0\0\0\0ÿÿşş\0\0\0€\0\0ƒ\0\0\0'),(15,'2011-06-28 17:42:36','6c335b8c4891b709969aaa5ec36ef097','\0\0\0?Æ×ÀãØ-Ü?a¨)¿_(¿YÖâAŞöÿÿıû\0\0\0	\0\0\0\0\0\0‚\0\0\0\0\0ÿÿü|\0\0\0\0\0ÿÿüz\0\0†ÿÿø{ÿÿÿ}\0\0\0‚ÿÿşx\0\0\0\0\0\0\0\0\0\0ÿÿÿ€ÿÿù\0ÿÿı|\0\0…ÿÿşşÿÿş\0\0\0ƒÿÿøı\0\0‰\0\0\0ÿÿş÷ÿÿø~ÿÿş}ÿÿÿø\0\0€\0\0ÿÿÿüÿÿÿ\0\0\0€\0\0‚ÿÿûş\0\0€ÿÿÿ}ÿÿşñ\0\0ÿÿş€\0\0\0\0\0ƒ\0\0‡ÿÿÿúÿÿÿûÿÿıüÿÿÿò\0\0\0\0\0€ÿÿÿ\0\0\0ÿÿşø\0\0ÿÿüzÿÿş\0\0‚\0\0ÿÿøıÿÿü\0\0\0\0\0\0\0„ÿÿüù\0\0	\0\0\0\0\0\0ÿÿÿş\0\0\0ÿÿşrÿÿÿı\0\0ÿÿş}ÿÿÿ€ÿÿÿÿ\0\0\0‡ÿÿÿ~ÿÿü€\0\0	ÿÿş\0ÿÿÿûÿÿşøÿÿûıÿÿıù\0\0‚ÿÿÿú\0\0„ÿÿÿ÷ÿÿø€ÿÿü\0ÿÿşñÿÿş\0\0ÿÿü{\0\0\0\0\0\0\0ƒÿÿü\0\0ˆ\0\0\0ÿÿøş\0\0\0\0‡ÿÿÿıÿÿüü\0\0\0\0\0\0\0\0\0\0\0ÿÿşwÿÿÿş\0\0ÿÿş~\0\0\0€ÿÿÿÿ'),(16,'2011-06-28 02:13:14','d77bfd4de3e8e4b2880a30cb03c824e6','\0\0\0?á/#I`¼€ó\nÚmû>¼s”ZÎ^\0\0\0\0„ÿÿı}ÿÿşè\0\0\0…\0\0\0Šÿÿı\0ÿÿñ€ÿÿÿg\0\0“ÿÿÿú\0\0‡ÿÿü|\0\0ÿÿør\0\0\0\0Šÿÿÿwÿÿÿzÿÿüı\0\0…ÿÿù\0ÿÿüó\0\0	ÿÿúzÿÿür\0\0\0ÿÿÿûÿÿÿñ\0\0ÿÿşwÿÿÿ\0\0€ÿÿüÿÿÿş~ÿÿÿù\0\0ÿÿÿ~ÿÿÿıÿÿÿÿÿÿñ}\0\0\0\0\0ÿÿâaÿÿááÿÿä2\0\0;º\0\0ıÿÿş\0\0†ÿÿşv\0\0‰ÿÿÄy\0\0;ôÿÿø\0\0\0\0\0\0\0\0\0\0\0\0\0wÿÿümÿÿÄÏÿÿøy\0\0ƒ\0\0\0ÿÿşç\0\0\0\0\0‰\0\0ÿÿøı\0\0\0ÿÿÄÿÿø€\0\0\0\0\0‚ÿÿüü\0\0\0‚\0\0\0\0\0\0\0\0ÿÿùt\0\0\0‰\0\0œ\0\0ÿÿüb\0\0\0ÿÿø€ÿÿıÿ\0\0€\0\0ÿÿÜÊ\0\0\0\0Ì\0\0\0\0¨\0\0	z\0\0\0„ÿÿúB\0\02\0\0wÿÿÿ\0ÿÿøq\0\0\0ÿÿışÿÿÃ\0\0„\0\0\0\0\0\0\0ƒ\0\0ƒ\0\0‰\0\0\0†\0\0ÿÿüü\0\0\0‚\0\0‚\0\0\0\0\0\0\0\0\0'),(17,'2011-06-28 02:13:14','891ade31c9d71cae9eefb599b467561e','\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0ÿÿÿÿÿÿÿıÿÿÿùÿÿÿñÿÿÿáÿÿÿØÿÿÿÙÿÿÿÚÿÿÿÛÿÿÿÜÿÿÿİÿÿÿŞÿÿÿßÿÿÿâÿÿÿàÿÿÿãÿÿÿäÿÿÿåÿÿÿæÿÿÿçÿÿÿèÿÿÿéÿÿÿêÿÿÿëÿÿÿìÿÿÿíÿÿÿîÿÿÿïÿÿÿòÿÿÿğÿÿÿóÿÿÿôÿÿÿõÿÿÿöÿÿÿ÷ÿÿÿúÿÿÿøÿÿÿûÿÿÿşÿÿÿüÿÿÿÿÿÿÿıÿÿÿùÿÿÿñÿÿÿáÿÿÿØÿÿÿÙÿÿÿÚÿÿÿÛÿÿÿÜÿÿÿİÿÿÿŞÿÿÿßÿÿÿâÿÿÿàÿÿÿãÿÿÿäÿÿÿåÿÿÿæÿÿÿçÿÿÿèÿÿÿéÿÿÿêÿÿÿëÿÿÿìÿÿÿíÿÿÿîÿÿÿïÿÿÿòÿÿÿğÿÿÿóÿÿÿôÿÿÿõÿÿÿöÿÿÿ÷ÿÿÿúÿÿÿøÿÿÿûÿÿÿşÿÿÿüÿÿÿÿÿÿÿıÿÿÿùÿÿÿñÿÿÿáÿÿÿØÿÿÿÙÿÿÿÚÿÿÿÛÿÿÿÜÿÿÿİÿÿÿŞÿÿÿßÿÿÿâÿÿÿàÿÿÿãÿÿÿäÿÿÿåÿÿÿæÿÿÿçÿÿÿèÿÿÿéÿÿÿêÿÿÿëÿÿÿìÿÿÿíÿÿÿîÿÿÿïÿÿÿòÿÿÿğÿÿÿóÿÿÿôÿÿÿõÿÿÿöÿÿÿ÷ÿÿÿúÿÿÿøÿÿÿûÿÿÿşÿÿÿü'),(18,'2011-06-28 02:13:14','891ade31c9d71cae9eefb599b467561e','\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0ÿÿÿÿÿÿÿıÿÿÿùÿÿÿñÿÿÿáÿÿÿØÿÿÿÙÿÿÿÚÿÿÿÛÿÿÿÜÿÿÿİÿÿÿŞÿÿÿßÿÿÿâÿÿÿàÿÿÿãÿÿÿäÿÿÿåÿÿÿæÿÿÿçÿÿÿèÿÿÿéÿÿÿêÿÿÿëÿÿÿìÿÿÿíÿÿÿîÿÿÿïÿÿÿòÿÿÿğÿÿÿóÿÿÿôÿÿÿõÿÿÿöÿÿÿ÷ÿÿÿúÿÿÿøÿÿÿûÿÿÿşÿÿÿüÿÿÿÿÿÿÿıÿÿÿùÿÿÿñÿÿÿáÿÿÿØÿÿÿÙÿÿÿÚÿÿÿÛÿÿÿÜÿÿÿİÿÿÿŞÿÿÿßÿÿÿâÿÿÿàÿÿÿãÿÿÿäÿÿÿåÿÿÿæÿÿÿçÿÿÿèÿÿÿéÿÿÿêÿÿÿëÿÿÿìÿÿÿíÿÿÿîÿÿÿïÿÿÿòÿÿÿğÿÿÿóÿÿÿôÿÿÿõÿÿÿöÿÿÿ÷ÿÿÿúÿÿÿøÿÿÿûÿÿÿşÿÿÿüÿÿÿÿÿÿÿıÿÿÿùÿÿÿñÿÿÿáÿÿÿØÿÿÿÙÿÿÿÚÿÿÿÛÿÿÿÜÿÿÿİÿÿÿŞÿÿÿßÿÿÿâÿÿÿàÿÿÿãÿÿÿäÿÿÿåÿÿÿæÿÿÿçÿÿÿèÿÿÿéÿÿÿêÿÿÿëÿÿÿìÿÿÿíÿÿÿîÿÿÿïÿÿÿòÿÿÿğÿÿÿóÿÿÿôÿÿÿõÿÿÿöÿÿÿ÷ÿÿÿúÿÿÿøÿÿÿûÿÿÿşÿÿÿü'),(19,'2011-06-28 02:13:14','891ade31c9d71cae9eefb599b467561e','\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0ÿÿÿÿÿÿÿıÿÿÿùÿÿÿñÿÿÿáÿÿÿØÿÿÿÙÿÿÿÚÿÿÿÛÿÿÿÜÿÿÿİÿÿÿŞÿÿÿßÿÿÿâÿÿÿàÿÿÿãÿÿÿäÿÿÿåÿÿÿæÿÿÿçÿÿÿèÿÿÿéÿÿÿêÿÿÿëÿÿÿìÿÿÿíÿÿÿîÿÿÿïÿÿÿòÿÿÿğÿÿÿóÿÿÿôÿÿÿõÿÿÿöÿÿÿ÷ÿÿÿúÿÿÿøÿÿÿûÿÿÿşÿÿÿüÿÿÿÿÿÿÿıÿÿÿùÿÿÿñÿÿÿáÿÿÿØÿÿÿÙÿÿÿÚÿÿÿÛÿÿÿÜÿÿÿİÿÿÿŞÿÿÿßÿÿÿâÿÿÿàÿÿÿãÿÿÿäÿÿÿåÿÿÿæÿÿÿçÿÿÿèÿÿÿéÿÿÿêÿÿÿëÿÿÿìÿÿÿíÿÿÿîÿÿÿïÿÿÿòÿÿÿğÿÿÿóÿÿÿôÿÿÿõÿÿÿöÿÿÿ÷ÿÿÿúÿÿÿøÿÿÿûÿÿÿşÿÿÿüÿÿÿÿÿÿÿıÿÿÿùÿÿÿñÿÿÿáÿÿÿØÿÿÿÙÿÿÿÚÿÿÿÛÿÿÿÜÿÿÿİÿÿÿŞÿÿÿßÿÿÿâÿÿÿàÿÿÿãÿÿÿäÿÿÿåÿÿÿæÿÿÿçÿÿÿèÿÿÿéÿÿÿêÿÿÿëÿÿÿìÿÿÿíÿÿÿîÿÿÿïÿÿÿòÿÿÿğÿÿÿóÿÿÿôÿÿÿõÿÿÿöÿÿÿ÷ÿÿÿúÿÿÿøÿÿÿûÿÿÿşÿÿÿü');
/*!40000 ALTER TABLE `imagehaarmatrix` ENABLE KEYS */;
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
  PRIMARY KEY (`imageid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `imageinformation`
--

LOCK TABLES `imageinformation` WRITE;
/*!40000 ALTER TABLE `imageinformation` DISABLE KEYS */;
INSERT INTO `imageinformation` VALUES (1,-1,'2011-06-05 16:28:34','2011-06-05 16:28:34',1,720,479,'JPG',8,5),(2,-1,'2011-06-05 16:28:34','2011-06-05 16:28:34',1,720,479,'JPG',8,2),(3,-1,'2011-06-28 02:13:14',NULL,0,1538,852,'PNG',8,1),(4,-1,'2011-06-27 15:31:50','2011-06-27 15:31:50',0,802,1916,'PNG',8,4),(5,-1,'2011-06-28 02:13:14',NULL,0,2560,2560,'PNG',8,1),(6,-1,'2006-03-11 15:42:12','2006-03-11 15:42:12',1,400,400,'JPG',8,5),(7,-1,'2006-03-11 15:42:12','2006-03-11 15:42:12',1,400,400,'JPG',8,5),(8,-1,'2011-06-28 17:42:36',NULL,0,798,798,'JPG',8,5),(9,-1,'2011-06-18 16:40:36','2011-06-18 16:40:36',0,640,472,'JPG',8,2),(10,-1,'2011-06-05 16:28:34','2011-06-05 16:28:34',1,538,358,'PGF',24,1),(11,-1,'2011-06-05 16:28:34','2011-06-05 16:28:34',1,538,358,'PGF',24,1),(12,-1,'2011-06-05 16:28:34','2011-06-05 16:28:34',1,538,358,'PGF',24,1),(13,-1,'2011-06-28 02:13:14',NULL,0,2560,2560,'PNG',8,1),(14,-1,'2006-03-11 15:42:12','2006-03-11 15:42:12',1,400,400,'JPG',8,5),(15,-1,'2011-06-28 17:42:36',NULL,0,798,798,'JPG',8,5),(16,-1,'2011-06-18 16:40:36','2011-06-18 16:40:36',0,640,472,'JPG',8,2),(17,-1,'2011-06-05 16:28:34','2011-06-05 16:28:34',1,538,358,'PGF',24,1),(18,-1,'2011-06-05 16:28:34','2011-06-05 16:28:34',1,538,358,'PGF',24,1),(19,-1,'2011-06-05 16:28:34','2011-06-05 16:28:34',1,538,358,'PGF',24,1);
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
  UNIQUE KEY `imageid` (`imageid`,`property`(332))
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
  `uniqueHash` varchar(32) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `album` (`album`,`name`(332)),
  KEY `dir_index` (`album`),
  KEY `hash_index` (`uniqueHash`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `images`
--

LOCK TABLES `images` WRITE;
/*!40000 ALTER TABLE `images` DISABLE KEYS */;
INSERT INTO `images` VALUES (1,2,'foto001.jpg',1,1,'2011-06-28 02:13:14',145538,'64ba82087739998c8ca2b6fe795a27ef'),(2,2,'foto001bw.jpg',1,1,'2011-06-28 02:13:14',105505,'faff1ab097816445d6c99618634c1054'),(3,3,'snap001.png',1,1,'2011-06-28 02:13:14',173933,'b7f6e38ad04f1a8b4307cee7d4d144f2'),(4,3,'snap002.png',1,1,'2011-06-28 15:43:41',71591,'570d684473df126f54b8436a444b3ba1'),(5,4,'big-image.png',1,1,'2011-06-28 02:13:14',114493,'73ded88d97601deb2d42dedac59a4786'),(6,4,'icc-test-farbkreis.jpg',1,1,'2011-06-28 02:13:14',75675,'6e6426a193df285022b616a563ce4ad6'),(8,4,'icc-test-no-profile.jpg',1,1,'2011-06-28 17:42:36',50331,'6c335b8c4891b709969aaa5ec36ef097'),(9,4,'Martian_face_viking.jpg',1,1,'2011-06-28 02:13:14',99416,'d77bfd4de3e8e4b2880a30cb03c824e6'),(10,5,'foto001q5.pgf',1,1,'2011-06-28 02:13:14',89012,'891ade31c9d71cae9eefb599b467561e'),(11,6,'foto001q5.pgf',1,1,'2011-06-28 02:13:14',89012,'891ade31c9d71cae9eefb599b467561e'),(12,6,'otherlink.pgf',1,1,'2011-06-28 02:13:14',89012,'891ade31c9d71cae9eefb599b467561e'),(13,7,'big-image.png',1,1,'2011-06-28 02:13:14',114493,'73ded88d97601deb2d42dedac59a4786'),(14,7,'icc-test-farbkreis.jpg',1,1,'2011-06-28 02:13:14',75675,'6e6426a193df285022b616a563ce4ad6'),(15,7,'icc-test-no-profile.jpg',1,1,'2011-06-28 17:42:36',50331,'6c335b8c4891b709969aaa5ec36ef097'),(16,7,'Martian_face_viking.jpg',1,1,'2011-06-28 02:13:14',99416,'d77bfd4de3e8e4b2880a30cb03c824e6'),(17,8,'foto001q5.pgf',1,1,'2011-06-28 02:13:14',89012,'891ade31c9d71cae9eefb599b467561e'),(18,9,'foto001q5.pgf',1,1,'2011-06-28 02:13:14',89012,'891ade31c9d71cae9eefb599b467561e'),(19,9,'otherlink.pgf',1,1,'2011-06-28 02:13:14',89012,'891ade31c9d71cae9eefb599b467561e');
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
            DELETE FROM ImageTags
            WHERE imageid=OLD.id;
            DELETE From ImageHaarMatrix
            WHERE imageid=OLD.id;
            DELETE From ImageInformation
            WHERE imageid=OLD.id;
            DELETE From ImageMetadata
            WHERE imageid=OLD.id;
            DELETE From ImagePositions
            WHERE imageid=OLD.id;
            DELETE From ImageComments
            WHERE imageid=OLD.id;
            DELETE From ImageCopyright
            WHERE imageid=OLD.id;
            DELETE From ImageProperties
            WHERE imageid=OLD.id;
            UPDATE Albums SET icon=null
            WHERE icon=OLD.id;
            UPDATE Tags SET icon=null
            WHERE icon=OLD.id;
            END */;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;

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
  KEY `tag_index` (`tagid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `imagetags`
--

LOCK TABLES `imagetags` WRITE;
/*!40000 ALTER TABLE `imagetags` DISABLE KEYS */;
INSERT INTO `imagetags` VALUES (4,1),(4,2),(4,3);
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
  UNIQUE KEY `keyword` (`keyword`(333))
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `settings`
--

LOCK TABLES `settings` WRITE;
/*!40000 ALTER TABLE `settings` DISABLE KEYS */;
INSERT INTO `settings` VALUES ('preAlpha010Update1','true'),('preAlpha010Update2','true'),('preAlpha010Update3','true'),('beta010Update1','true'),('beta010Update2','true'),('databaseImageFormats','jpg;jpeg;jpe;jp2;j2k;jpx;jpc;pgx;tif;tiff;png;xpm;ppm;pnm;pgf;gif;bmp;xcf;pcx;bay;bmq;cr2;crw;cs1;dc2;dcr;dng;erf;fff;hdr;k25;kdc;mdc;mos;mrw;nef;orf;pef;pxn;raf;raw;rdc;sr2;srf;x3f;arw;3fr;cine;ia;kc2;mef;nrw;qtk;rw2;sti;rwl;'),('databaseVideoFormats','mpeg;mpg;mpo;mpe;avi;mov;wmf;asf;mp4;3gp;wmv'),('databaseAudioFormats','ogg;mp3;wma;wav'),('FilterSettingsVersion','3'),('DcrawFilterSettingsVersion','3'),('DBVersion','0'),('databaseUserImageFormats',NULL),('databaseUserVideoFormats',NULL),('databaseUserAudioFormats',NULL),('DeleteRemovedCompleteScanCount','3'),('Scanned','2011-06-28T20:09:58'),('databaseUUID','{cb5e064b-a733-4466-a199-6d4022b5c932}');
/*!40000 ALTER TABLE `settings` ENABLE KEYS */;
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
INSERT INTO `tags` VALUES (1,8,'tagl2a',0,NULL,35,36),(2,15,'tl0a',0,NULL,1,34),(3,2,'tl0a0',0,NULL,32,33),(4,0,'tagl0a',0,NULL,30,31),(5,0,'tagl0b',0,NULL,6,29),(6,0,'tagl0c',0,NULL,4,5),(7,5,'tagl1a',0,NULL,27,28),(8,5,'tagl1b',0,NULL,9,26),(9,5,'tagl1c',0,NULL,7,8),(10,8,'tagl2b',0,NULL,14,25),(11,8,'tagl2c',0,NULL,12,13),(12,8,'tagl2d',0,NULL,10,11),(13,10,'tagl3a',0,NULL,23,24),(14,10,'tagl3b',0,NULL,21,22),(15,0,'tl0',0,NULL,15,20),(16,15,'tl0b',0,NULL,18,19),(17,15,'tagl1c',0,NULL,16,17),(18,2,'tloa1',0,NULL,2,3);
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
/*!50003 CREATE*/ /*!50017 DEFINER=`digikam`@`%`*/ /*!50003 TRIGGER move_tagstree AFTER UPDATE ON Tags
            FOR EACH ROW BEGIN
            DELETE FROM TagsTree;
            REPLACE INTO TagsTree
            SELECT node.id, parent.pid
            FROM Tags AS node, Tags AS parent
            WHERE node.lft BETWEEN parent.lft AND parent.rgt
            ORDER BY parent.lft;
            END */;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
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
                DELETE FROM ImageTags WHERE tagid=OLD.id;
                DELETE FROM TagsTree;
                REPLACE INTO TagsTree
                SELECT node.id, parent.pid
                FROM Tags AS node, Tags AS parent
                WHERE node.lft BETWEEN parent.lft AND parent.rgt
                ORDER BY parent.lft;
            END */;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;

--
-- Table structure for table `tagstree`
--

DROP TABLE IF EXISTS `tagstree`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `tagstree` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `pid` int(11) NOT NULL,
  UNIQUE KEY `id` (`id`,`pid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tagstree`
--

LOCK TABLES `tagstree` WRITE;
/*!40000 ALTER TABLE `tagstree` DISABLE KEYS */;
INSERT INTO `tagstree` VALUES (1,8),(2,15),(3,2),(3,15),(4,0),(4,15),(5,0),(5,15),(6,0),(6,15),(7,0),(7,5),(7,15),(8,0),(8,5),(8,15),(9,0),(9,5),(9,15),(10,0),(10,5),(10,8),(10,15),(11,0),(11,5),(11,8),(11,15),(12,0),(12,5),(12,8),(12,15),(13,0),(13,5),(13,8),(13,10),(13,15),(14,0),(14,5),(14,8),(14,10),(14,15),(15,0),(15,5),(15,8),(15,15),(16,0),(16,5),(16,8),(16,15),(17,0),(17,5),(17,8),(17,15);
/*!40000 ALTER TABLE `tagstree` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping routines for database 'digikam'
--
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

