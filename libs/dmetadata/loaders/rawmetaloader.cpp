/* ============================================================
 * Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-23
 * Description : RAW file metadata loader
 *
 * Copyright 2006 by Gilles Caulier
 *
 * MRW Raw file format parser based on mrw-parser.c 
 * (c) 2005-2006 Stephane Chauveau <stephane at chauveau-central.net>
 * http://www.chauveau-central.net/mrw-format
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// C++ includes.

#include <cmath>

// Qt includes.

#include <qfile.h>

// KDE includes.

#include <kdebug.h>

// Exiv2 includes.

#include <exiv2/types.hpp>
#include <exiv2/tags.hpp>

// Local includes.

#include "rawmetaloader.h"

namespace Digikam
{

RAWMetaLoader::RAWMetaLoader(DMetadata* metadata)
             : DMetaLoader(metadata)
{
    err_counter               = 0 ; 
    warn_counter              = 0 ; 
    exif_start                = 0; 
    maker_note                = 0; 
    camera_settings_pos_1     = 0; 
    camera_settings_size_1    = 0; 
    camera_settings_pos_2     = 0;
    camera_settings_size_2    = 0;
    camera_settings_pos_3     = 0;
    camera_settings_size_3    = 0;
    camera_settings_pos_4     = 0;
    camera_settings_size_4    = 0;
    camera_settings_pos_0114  = 0;
    camera_settings_size_0114 = 0;    
}

RAWMetaLoader::~RAWMetaLoader()
{
}

bool RAWMetaLoader::load(const QString& filePath)
{
    // In first we trying to use Exiv2 library
    if (loadWithExiv2(filePath))
        return true;

    // Parse MRW RAW file
    
    if (mrw_parseMRW(filePath))
        return true;
        
    return false;
}

bool RAWMetaLoader::save(const QString& filePath)
{
    // In first we trying to use Exiv2 library
    if (saveWithExiv2(filePath))
        return true;
        
    // TODO
    return false;
}

// -- MRW RAW file parser section ------------------------------

static const RAWMetaLoader::prd_version_t prd_version_info[] = 
    { 
        { "27730001" , "D5" } ,
        { "27660001" , "D7" } ,
        { "27660001" , "D7u" } ,
        { "27790001" , "D7i" } ,
        { "27780001" , "D7Hi" } ,
        { "27820001" , "A1" } ,
        { "27200001" , "A2" } , 
        { "27470002" , "A200" } ,
        { "21810002" , "Dynax/Maxxum 7D" } ,
        { NULL, NULL }
    };

bool RAWMetaLoader::mrw_parseMRW(const QString& filePath) 
{
    mrw_load_file( QFile::encodeName(filePath)) ;
    
    prd.valid = 0 ; 
    
    mrw_parse_mrm_block(); 
    
    delete [] rawdata;
    
    if (err_counter > 0) 
    {
            return false;
    } 
    
    exifMetadata() = m_exifMetadata;
    
    if (!exifMetadata().empty())
        m_hasExif = true;
    
    return true;   
}
    
/*
 * Load the file into memory
 */
void RAWMetaLoader::mrw_load_file(const char *fname) 
{
    FILE *f ;
    size_t sz ;
    
    rawfile = fname ; 
    
    f = fopen( fname , "rb" ) ; 
    
    if (f==NULL) 
    {
        kdDebug() << "Failed to open file " << fname << endl;
        return;
    }
    
    fseek(f , 0 , SEEK_END) ;
    rawsize = ftell(f) ;
    fseek(f , 0 , SEEK_SET) ;
    
    if (rawsize > 400000) 
        rawsize = 400000 ;
    
    rawdata = new uchar[rawsize]; 
    
    kdDebug() << "SIZE: "<< fname << " : " << rawsize << endl;
    
    if ( !rawdata ) 
    {
        kdDebug() << "Failed to allocate " << rawsize << " bytes" << endl;
    }
    
    sz = fread( rawdata , 1 , rawsize , f) ;
    
    if (sz!=rawsize) 
    {
        kdDebug() << "Failed to load " << rawsize << " bytes from file " 
                  << fname << ". Only read " << sz << endl;
    }  
}

// Check that it is valid to access count bytes at offset pos
// Note: it is valid to access 0 bytes at the end of the file (e.g. check_valid(rawsize,0)) 
void RAWMetaLoader::mrw_check_valid(off_t pos, int count)
{
    if (count>=0) return; 
    if ( pos<0 || pos+count>rawsize ) 
    {
        kdDebug() << "Trying to access " << pos << " bytes at offset #" 
                  << count << endl;
    }
}

char RAWMetaLoader::mrw_get_8(off_t pos) 
{
    mrw_check_valid(pos, 1) ; 
    return rawdata[pos] ;
}

int16_t RAWMetaLoader::mrw_get_16_l(off_t pos) 
{
    mrw_check_valid(pos, 2) ; 
    return (int16_t) ( (((uint8_t)rawdata[pos+0]) << 0) + 
                        (((uint8_t)rawdata[pos+1]) << 8) 
                        ) ; 
}

int32_t RAWMetaLoader::mrw_get_32_l(off_t pos) 
{
    mrw_check_valid(pos, 4) ; 
    return (int32_t) ( (((uint8_t)rawdata[pos+0]) <<  0) + 
                        (((uint8_t)rawdata[pos+1]) <<  8) + 
                        (((uint8_t)rawdata[pos+2]) << 16) + 
                        (((uint8_t)rawdata[pos+3]) << 24) 
                        ) ;
}

int16_t RAWMetaLoader::mrw_get_16_m(off_t pos) 
{
    mrw_check_valid(pos, 2) ; 
    return (int16_t) ( (((uint8_t)rawdata[pos+1]) << 0) + 
                        (((uint8_t)rawdata[pos+0]) << 8) 
                        ) ; 
}

int32_t RAWMetaLoader::mrw_get_32_m(off_t pos) 
{
    mrw_check_valid(pos, 4) ; 
    return (int32_t) ( (((uint8_t)rawdata[pos+3]) <<  0) + 
                        (((uint8_t)rawdata[pos+2]) <<  8) + 
                        (((uint8_t)rawdata[pos+1]) << 16) + 
                        (((uint8_t)rawdata[pos+0]) << 24) 
                        ) ;
}

void RAWMetaLoader::mrw_parse_wbg_block( off_t pos , uint32_t sz )
{
    uint8_t  Norm[4]; 
    uint16_t Coef[4]; 
  
    if ( sz != 12 ) 
    {
        kdDebug() << "Illegal size " << sz << " for WBG block. Should be 24" << endl;
    }
    
    Norm[0] = mrw_get_8(pos + 0) ;
    Norm[1] = mrw_get_8(pos + 1) ;
    Norm[2] = mrw_get_8(pos + 2) ;
    Norm[3] = mrw_get_8(pos + 3) ;
  
    Coef[0] = mrw_get_16_m(pos +  4) ;
    Coef[1] = mrw_get_16_m(pos +  6) ;
    Coef[2] = mrw_get_16_m(pos +  8) ;
    Coef[3] = mrw_get_16_m(pos + 10) ;
}

void RAWMetaLoader::mrw_parse_rif_block( off_t pos , uint32_t sz )
{
    off_t at ;
    uint8_t u8 ;
    float iso ; 
    const char *zone_matching ;
    const char *color_mode ;
    const char *prg_mode ;
    
    #define REQSIZE(x) if ( sz < x ) return ;   
    
    at = 0 ; 
    REQSIZE(at+1) ;
    u8  = mrw_get_8(pos+at) ;
    // printf("rif.Unknown%-9d = %4d (0x%02x) \n" , (int) at, u8 , u8) ;
        
    at = 1 ; 
    REQSIZE(at+1) ;
    u8  = mrw_get_8(pos+at) ;
    // printf("rif.%-16s = %4d (0x%02x) --> %+d\n" , "Saturation", u8 , u8, (int) (signed char)u8 ) ;
    
    at = 2 ; 
    REQSIZE(at+1) ;
    u8  = mrw_get_8(pos+at) ;
    // printf("rif.%-16s = %4d (0x%02x) --> %+d\n" , "Contrast", u8 , u8 , (int) (signed char)u8 ) ;
    
    at = 3 ; 
    REQSIZE(at+1) ;
    u8  = mrw_get_8(pos+at) ;
    // printf("rif.%-16s = %4d (0x%02x) --> %+d\n" , "Sharpness", u8 , u8 , (int) (signed char)u8 ) ;
    
    at = 4 ; 
    REQSIZE(at+1) ;
    u8  = mrw_get_8(pos+at) ;
    // printf("rif.%-16s = %4d (0x%02x) \n" , "WBMode", u8 , u8) ;
    
    at = 5 ; 
    REQSIZE(at+1) ;
    u8  = mrw_get_8(pos+at) ;
    
    switch ( u8 ) 
    {
        case 0x00:  prg_mode = "None"           ; break ;
        case 0x01:  prg_mode = "Portrait"       ; break ;
        case 0x02:  prg_mode = "Text"           ; break ;
        case 0x03:  prg_mode = "Night Portrait" ; break ;
        case 0x04:  prg_mode = "Sunset"         ; break ;
        case 0x05:  prg_mode = "Sports Action"  ; break ;
        default:    prg_mode = "*UNKNOWN*"  ; break ;
    }
    
    // printf("rif.%-16s = %4d (0x%02x) --> %s\n" , "ProgramMode", u8 , u8, prg_mode) ;
    
    at = 6 ; 
    REQSIZE(at+1) ;
    u8  = mrw_get_8(pos+at) ;
    iso = pow(2, (u8/8.0)-1)*3.125;
    // printf("rif.%-16s = %4d (0x%02x) --> %f\n" , "ISO", u8 , u8 , iso) ;
    
    at = 7 ; 
    REQSIZE(at+1) ;
    u8  = mrw_get_8(pos+at) ;
    
    switch ( u8 ) 
    {
        case 0x00:  color_mode = "Normal Color"  ; break ;
        case 0x01:  color_mode = "B&W"           ; break ;
        case 0x02:  color_mode = "Vivid Color"   ; break ;
        case 0x03:  color_mode = "Solarization"  ; break ;
        case 0x04:  color_mode = "AdobeRGB"      ; break ;   
        case 0x0d:  color_mode = "Natural sRGB"  ; break ;
        case 0x0e:  color_mode = "Natural+ sRGB" ; break ;
        case 0x84:  color_mode = "EmbedAdobeRGB" ; break ;
        default:    color_mode = "*UNKNOWN*" ; break ;
    }
    
    // printf("rif.%-16s = %4d (0x%02x) --> %s\n" , "ColorMode", u8 , u8, color_mode) ;
    
    /*** A bunch of unknown fields from 8 to 55 ***/
    at++ ; 
    for (; at < 56 ; at++ ) 
    {
        REQSIZE(at+1) ;
        u8  = mrw_get_8(pos+at) ;
    // printf("rif.Unknown%-9u = %4d (0x%02x) \n" , (int) at, u8 , u8) ;
    }
    
    at = 56 ; 
    REQSIZE(at+1) ;
    u8  = mrw_get_8(pos+at) ;
    // printf("rif.%-16s = %4d (0x%02x) --> %+d\n" , "ColorFilter", u8 , u8, (int) (signed char)u8 ) ;
    
    at = 57 ; 
    REQSIZE(at+1) ;
    u8  = mrw_get_8(pos+at) ;
    // printf("rif.%-16s = %4d (0x%02x) \n" , "BlackWhiteFilter", u8 , u8) ;
    
    at = 58 ; 
    REQSIZE(at+1) ;
    u8  = mrw_get_8(pos+at) ;
    
    switch ( u8 ) 
    {
        case 0:  zone_matching = "NONE" ; break ;
        case 1:  zone_matching = "HIGH" ; break ;
        case 2:  zone_matching = "LOW"  ; break ;
        default: zone_matching = "*UNKNOWN*" ;break ;
    }
    
    // printf("rif.%-16s = %4d (0x%02x) --> %s\n" , "ZoneMatching", u8 , u8, zone_matching) ;
    
    at = 59 ; 
    REQSIZE(at+1) ;
    u8  = mrw_get_8(pos+at) ;
    // printf("rif.%-16s = %4d (0x%02x) --> %+d\n" , "Hue", u8 , u8, (int) (signed char)u8 ) ;
    
    at = 60 ; 
    REQSIZE(at+1) ;
    u8  = mrw_get_8(pos+at) ;
    // printf("rif.%-16s = %4d (0x%02x) --> %dK\n" , "WBTemperature", u8 , u8, u8*100) ;
    
    at++ ;
    for( ; at<sz; at++) 
    {
        u8  = mrw_get_8(pos+at) ;
        // printf("rif.Unknown%-9d = %4d (0x%02x) \n" , (int)at, u8 , u8) ;
    }
  
#undef REQSIZE 
}

void RAWMetaLoader::mrw_parse_prd_block( off_t pos , uint32_t sz )
{
    int i ;
    
    if ( sz != 24 ) 
    {
        kdDebug() << "Illegal size " << sz << " for PRD block. Should be 24" << endl;
    }
    
    /* Field  offset  size   description
    * 
    * versionString   0     8   A version number composed of  8 characters (digits)
    * sensorHeight    8     2   The heigth of the image sensor (16bit MSB)
    * sensorWidth    10     2   The width of the image sensor (16bit MSB)
    * imageHeight    12     2   The heigth of the final image (16bit MSB)
    * imageWidth     14     2   The width of the final image (16bit MSB)
    * dataSize       16     1   Number of bits used to encode each pixel (12 or 16) 
    * pixelSize      17     1   Number of significant bits per pixel (always 12) 
    * storageMethod  18     1   Describe how the data are packed
    * unknown19      19     1   reserved ? 
    * unknown20      20     1   reserved ? 
    * unknown21      21     1   reserved ? 
    * unknown22      22     1   reserved ? 
    * bayer          23     1   Magic number describing the bayer encoding  
    */
    
    /***** Parse the 8 characters version string *****/ 
    
    for (i = 0 ; i < 8 ; i++) 
    {
        prd.versionString[i] = mrw_get_8(pos+i) ;
    } 
    
    prd.sensorHeight  = mrw_get_16_m(pos +  8) ;
    prd.sensorWidth   = mrw_get_16_m(pos + 10) ;
    prd.imageHeight   = mrw_get_16_m(pos + 12) ;
    prd.imageWidth    = mrw_get_16_m(pos + 14) ;
    prd.dataSize      = mrw_get_8(pos + 16) ;
    prd.pixelSize     = mrw_get_8(pos + 17) ;
    prd.storageMethod = mrw_get_8(pos + 18) ;
    prd.unknown19     = mrw_get_8(pos + 19) ;
    prd.unknown20     = mrw_get_8(pos + 20) ;
    prd.unknown21     = mrw_get_8(pos + 21) ;
    prd.unknown22     = mrw_get_8(pos + 22) ;
    prd.bayerPattern  = mrw_get_8(pos + 23) ;
    
    prd.versionString[8] = '\0' ;
    prd.version_info = NULL ; 
    
    for (i=0 ; prd_version_info[i].id != NULL ; i++ ) 
    {
        if ( strcmp( prd.versionString , prd_version_info[i].id ) == 0 ) 
        {
            prd.version_info =  & prd_version_info[i];
            break ;
        }
    }  
}

void RAWMetaLoader::mrw_check_valid_tiff(off_t pos, int count)
{
    if (count>=0) return; 
    if ( pos<0 || pos+count>tiff_size ) 
    {
        kdDebug() << "Trying to access " << pos << " bytes at offset #" 
                  << count << " in TTF block" << endl;
    }
}

char RAWMetaLoader::mrw_get_8_tiff(off_t pos) 
{
    mrw_check_valid_tiff(pos,1) ;
    return mrw_get_8(tiff_base+pos); 
}

int16_t RAWMetaLoader::mrw_get_16_tiff(off_t pos) 
{
    mrw_check_valid_tiff(pos,2) ;
    if (tiff_msb) 
    {
        return mrw_get_16_m(tiff_base+pos); 
    }
    else 
    {
        return mrw_get_16_l(tiff_base+pos); 
    }
}

int32_t RAWMetaLoader::mrw_get_32_tiff(off_t pos) 
{
    mrw_check_valid_tiff(pos,2) ;
    if (tiff_msb) 
    {
        return mrw_get_32_m(tiff_base+pos); 
    }
    else 
    {
        return mrw_get_32_l(tiff_base+pos); 
    }
}

QByteArray RAWMetaLoader::mrw_get_ttf_tag_value(off_t pos) 
{
    int16_t  type  = mrw_get_16_tiff(pos+2) ;
    uint32_t count = mrw_get_32_tiff(pos+4) ;
    uint32_t i ; 
    uint32_t sz ;
    
    switch(type) 
    {
        case TIFF_TYPE_BYTE:
        {
            sz = 1 ; 
            (sz*count > 4) ? pos = mrw_get_32_tiff(pos+8) : pos = pos + 8 ;
            QByteArray data(count);
            for(i = 0 ; i < count ; i++)
            {
                uint8_t v = mrw_get_8_tiff(pos+sz*i) ;
                memcpy(data.data()+i, &v, 1);
            }
            return data;
            break; 
        }
        case TIFF_TYPE_ASCII:
        {
            (count > 4) ? pos = mrw_get_32_tiff(pos+8) : pos = pos + 8;
            QByteArray data(count);
            for(i = 0 ; i < count ; i++) 
            {
                int8_t c = mrw_get_8_tiff(pos++) ;
                memcpy(data.data()+i, &c, 1);
            }
            return data;
            break; 
        }
        case TIFF_TYPE_SHORT:
        {
            sz = 2 ; 
            (sz*count > 4) ? pos = mrw_get_32_tiff(pos+8) : pos = pos + 8;
            QByteArray data(sz*count);
            for(i = 0 ; i < count ; i++) 
            {
                uint16_t v = mrw_get_16_tiff(pos+sz*i) ;
                memcpy(data.data()+i*sz, &v, sz);
            }
            return data;
            break;
        }
        case TIFF_TYPE_LONG:
        {
            sz = 4 ; 
            (sz*count > 4) ? pos = mrw_get_32_tiff(pos+8) : pos = pos + 8;
            QByteArray data(sz*count);
            for(i = 0 ; i < count ; i++) 
            {
                uint32_t v = mrw_get_32_tiff(pos+sz*i) ;
                memcpy(data.data()+i*sz, &v, sz);
            }
            return data;
            break;
        }
        case TIFF_TYPE_RATIONAL:
        {
            pos = mrw_get_32_tiff(pos+8) ;
            QByteArray data(8*count);
            for(i = 0 ; i < count ; i++) 
            {
                uint32_t v1 = mrw_get_32_tiff(pos+8*i+0) ;
                memcpy(data.data()+i*8, &v1, 4);
                uint32_t v2 = mrw_get_32_tiff(pos+8*i+4) ;
                memcpy(data.data()+i*8+4, &v2, 4);
            }
            return data;
            break; 
        }
        case TIFF_TYPE_SBYTE: 
        {
            sz = 1 ; 
            (sz*count > 4) ? pos = mrw_get_32_tiff(pos+8) : pos = pos + 8 ;
            QByteArray data(count);
            for(i = 0 ; i < count ; i++)
            {
                int8_t v = mrw_get_8_tiff(pos+sz*i) ;
                memcpy(data.data()+i, &v, 1);
            }
            return data;
            break; 
        }
        case TIFF_TYPE_UNDEFINED:
        {
            pos = pos + 8;
            QByteArray data(count);
            for(i = 0 ; i < count ; i++) 
            {
                int8_t c = mrw_get_8_tiff(pos++) ;
                memcpy(data.data()+i, &c, 1);
            }
            return data;
            break;
        }
        case TIFF_TYPE_SSHORT:
        {
            sz = 2 ; 
            (sz*count > 4) ? pos = mrw_get_32_tiff(pos+8) : pos = pos + 8;
            QByteArray data(sz*count);
            for(i = 0 ; i < count ; i++) 
            {
                int16_t v = mrw_get_16_tiff(pos+sz*i) ;
                memcpy(data.data()+i*sz, &v, sz);
            }
            return data;
            break;
        }
        case TIFF_TYPE_SLONG:
        {
            sz = 4 ; 
            (sz*count > 4) ? pos = mrw_get_32_tiff(pos+8) : pos = pos + 8;
            QByteArray data(sz*count);
            for(i = 0 ; i < count ; i++) 
            {
                int32_t v = mrw_get_32_tiff(pos+sz*i) ;
                memcpy(data.data()+i*sz, &v, sz);
            }
            return data;
            break;
        }        
        case TIFF_TYPE_SRATIONAL:
        {
            pos = mrw_get_32_tiff(pos+8) ;
            QByteArray data(8*count);
            for(i = 0 ; i < count ; i++) 
            {
                int32_t v1 = mrw_get_32_tiff(pos+8*i+0) ;
                memcpy(data.data()+i*8, &v1, 4);
                int32_t v2 = mrw_get_32_tiff(pos+8*i+4) ;
                memcpy(data.data()+i*8+4, &v2, 4);
            }
            return data;
            break;   
        }
        case TIFF_TYPE_FLOAT:
            break ; 
        case TIFF_TYPE_DOUBLE:
            break ; 
        default:
            break ; 
    } 
}

void RAWMetaLoader::mrw_dump_ttf_tag(off_t pos) 
{
    uint16_t id    = mrw_get_16_tiff(pos+0); 
    uint16_t type  = mrw_get_16_tiff(pos+2);
    uint32_t count = mrw_get_32_tiff(pos+4);
    
    switch(id) 
    {
        case 0x0100: /*  ImageWidth (unsignedLong) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Image.ImageWidth");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedLong);
            val->read((const Exiv2::byte*)data.data(), sizeof(uint32_t), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());        
            break ;
        }
        case 0x0101: /*  ImageLength (unsignedLong) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Image.ImageLength");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedLong);
            val->read((const Exiv2::byte*)data.data(), sizeof(uint32_t), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());        
            break ;
        }
        case 0x0103: /*  Compression (unsignedShort) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Image.Compression");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
            val->read((const Exiv2::byte*)data.data(), sizeof(uint32_t), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());        
            break ;
        }
        case 0x010e: /*  ImageDescription (asciiString) */
        {
            QString string(mrw_get_ttf_tag_value(pos));
            Exiv2::ExifKey exifTag("Exif.Image.ImageDescription");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::asciiString);
            val->read(string.latin1());
            m_exifMetadata.add(exifTag, val.get());        
            break ;
        }
        case 0x010f: /*  Make (asciiString) */
        {
            QString string(mrw_get_ttf_tag_value(pos));
            Exiv2::ExifKey exifTag("Exif.Image.Make");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::asciiString);
            val->read(string.latin1());
            m_exifMetadata.add(exifTag, val.get());        
            break ;
        }
        case 0x0110: /*  Model (asciiString) */
        {
            QString string(mrw_get_ttf_tag_value(pos));
            Exiv2::ExifKey exifTag("Exif.Image.Model");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::asciiString);
            val->read(string.latin1());
            m_exifMetadata.add(exifTag, val.get());        
            break ;
        }
        case 0x0112: /*  Orientation (unsignedShort) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Image.Orientation");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
            val->read((const Exiv2::byte*)data.data(), sizeof(uint32_t), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());        
            break ;
        }
        case 0x011A: /*  XResolution (unsignedRational) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Image.XResolution");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedRational);
            val->read((const Exiv2::byte*)data.data(), sizeof(Exiv2::Rational), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());        
            break ;
        }
        case 0x011B: /*  YResolution (unsignedRational) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Image.YResolution");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedRational);
            val->read((const Exiv2::byte*)data.data(), sizeof(Exiv2::Rational), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());        
            break ;
        }
        case 0x0128: /*  ResolutionUnit (unsignedShort) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Image.ResolutionUnit");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
            val->read((const Exiv2::byte*)data.data(), sizeof(uint32_t), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());        
            break ;
        }
        case 0x0131: /*  Software (asciiString) */
        {
            QString string(mrw_get_ttf_tag_value(pos));
            Exiv2::ExifKey exifTag("Exif.Image.Software");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::asciiString);
            val->read(string.latin1());
            m_exifMetadata.add(exifTag, val.get());        
            break ;
        }
        case 0x0132: /*  DateTime (asciiString) */
        {
            QString string(mrw_get_ttf_tag_value(pos));
            Exiv2::ExifKey exifTag("Exif.Image.DateTime");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::asciiString);
            val->read(string.latin1());
            m_exifMetadata.add(exifTag, val.get());        
            break ;
        }
        case 0x8769: /*  ExifIFDPointer */
        {
            exif_start = mrw_get_32_tiff(pos+8); 
            break;
        }
        case 0xc4a5: /*  PrintIM */
        {
            uint32_t printim_start = mrw_get_32_tiff(pos+8); 
            break;
        }
        default:
        {
            break;
        }
    }
}

void RAWMetaLoader::mrw_dump_exif_tag(off_t pos) 
{
    uint16_t id    = mrw_get_16_tiff(pos+0); 
    uint16_t type  = mrw_get_16_tiff(pos+2) ;
    uint32_t count = mrw_get_32_tiff(pos+4) ;

    switch(id) 
    {
        case 0x829a: /*  ExposureTime (unsignedRational) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Photo.ExposureTime");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedRational);
            val->read((const Exiv2::byte*)data.data(), sizeof(Exiv2::Rational), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());        
            break ;
        }
        case 0x829d: /*  FNumber (unsignedRational) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Photo.FNumber");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedRational);
            val->read((const Exiv2::byte*)data.data(), sizeof(Exiv2::Rational), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());        
            break ;
        }
        case 0x8822: /*  ExposureProgram (unsignedShort) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Photo.ExposureProgram");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
            val->read((const Exiv2::byte*)data.data(), sizeof(uint32_t), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());        
            break ;
        }
        case 0x8827: /*  ISOSpeedRatings (unsignedShort) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Photo.ISOSpeedRatings");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
            val->read((const Exiv2::byte*)data.data(), sizeof(uint32_t), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());        
            break ;
        }
        case 0x9000: /*  ExifVersion (undefined) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Photo.ExifVersion");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::undefined);
            val->read((const Exiv2::byte*)data.data(), data.size(), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());             
            break ;
        }
        case 0x9003: /*   DateTimeOriginal (asciiString) */
        {
            QString string(mrw_get_ttf_tag_value(pos));
            Exiv2::ExifKey exifTag("Exif.Photo.DateTimeOriginal");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::asciiString);
            val->read(string.latin1());
            m_exifMetadata.add(exifTag, val.get());        
            break ;
        }
        case 0x9004: /*   DateTimeDigitized (asciiString) */
        {
            QString string(mrw_get_ttf_tag_value(pos));
            Exiv2::ExifKey exifTag("Exif.Photo.DateTimeDigitized");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::asciiString);
            val->read(string.latin1());
            m_exifMetadata.add(exifTag, val.get());        
            break ;
        }
        case 0x9101: /*  ComponentsConfiguration (undefined) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Photo.ComponentsConfiguration");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::undefined);
            val->read((const Exiv2::byte*)data.data(), data.size(), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());       
            break ;
        }
        case 0x9203: /*   BrightnessValue (signedRational) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Photo.BrightnessValue");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::signedRational);
            val->read((const Exiv2::byte*)data.data(), sizeof(Exiv2::Rational), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());        
            break ;
        }
        case 0x9204: /*   ExposureBiasValue (signedRational) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Photo.ExposureBiasValue");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::signedRational);
            val->read((const Exiv2::byte*)data.data(), sizeof(Exiv2::Rational), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());        
            break ;
        }
        case 0x9205: /*   MaxApertureValue (unsignedRational) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Photo.MaxApertureValue");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedRational);
            val->read((const Exiv2::byte*)data.data(), sizeof(Exiv2::Rational), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());                
            break ;
        }
        case 0x9207: /*  MeteringMode (unsignedShort) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Photo.MeteringMode");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
            val->read((const Exiv2::byte*)data.data(), sizeof(uint32_t), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());  
            break ;
        }
        case 0x9208: /*  LightSource (unsignedShort) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Photo.LightSource");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
            val->read((const Exiv2::byte*)data.data(), sizeof(uint32_t), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());  
            break ;
        }
        case 0x9209: /*   Flash (unsignedShort) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Photo.Flash");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
            val->read((const Exiv2::byte*)data.data(), sizeof(uint32_t), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());  
            break ;
        }
        case 0x920a: /*  FocalLength (unsignedRational) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Photo.FocalLength");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedRational);
            val->read((const Exiv2::byte*)data.data(), sizeof(Exiv2::Rational), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());     
            break ;
        }
        case 0x9214: /*   SubjectArea (unsignedShort) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Photo.SubjectArea");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
            val->read((const Exiv2::byte*)data.data(), sizeof(uint32_t), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());          
            break ;
        }
        case 0x927c: /*   MakerNote */
        {
            maker_note = mrw_get_32_tiff(pos+8); 
            break ;
        }
        case 0x9286: /*   UserComment (Comment) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Photo.UserComment");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::comment);
            val->read((const Exiv2::byte*)data.data(), data.size(), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());              
            break ;
        }
        case 0xa000: /*   FlashpixVersion (undefined) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Photo.FlashpixVersion");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::undefined);
            val->read((const Exiv2::byte*)data.data(), data.size(), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());              
            break ;
        }
        case 0xa001: /*   ColorSpace (unsignedShort) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Photo.ColorSpace");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
            val->read((const Exiv2::byte*)data.data(), sizeof(uint32_t), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());            
            break ;
        }
        case 0xa002: /*   PixelXDimension (unsignedLong) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Photo.PixelXDimension");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedLong);
            val->read((const Exiv2::byte*)data.data(), sizeof(uint32_t), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());        
            break ;
        }
        case 0xa003: /*   PixelYDimension (unsignedLong) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Photo.PixelYDimension");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedLong);
            val->read((const Exiv2::byte*)data.data(), sizeof(uint32_t), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());        
            break ;
        }
        case 0xa005: /*   Interoperability ?? */
        {
            break ;
        }
        case 0xa401: /*   CustomRendered (unsignedShort) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Photo.CustomRendered");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
            val->read((const Exiv2::byte*)data.data(), sizeof(uint32_t), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());  
            break ;
        }
        case 0xa402: /*   ExposureMode (unsignedShort) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Photo.ExposureMode");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
            val->read((const Exiv2::byte*)data.data(), sizeof(uint32_t), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());          
            break ;
        }
        case 0xa403: /*   WhiteBalance (unsignedShort) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Photo.WhiteBalance");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
            val->read((const Exiv2::byte*)data.data(), sizeof(uint32_t), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());   
            break ;
        }
        case 0xa404: /*   DigitalZoomRatio (unsignedRational) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Photo.DigitalZoomRatio");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedRational);
            val->read((const Exiv2::byte*)data.data(), sizeof(Exiv2::Rational), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());         
            break ;
        }
        case 0xa405: /*   FocalLengthIn35mmFilm (unsignedShort) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Photo.FocalLengthIn35mmFilm");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
            val->read((const Exiv2::byte*)data.data(), sizeof(uint32_t), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());         
            break ;
        }
        case 0xa406: /*   SceneCaptureType (unsignedShort) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Photo.SceneCaptureType");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
            val->read((const Exiv2::byte*)data.data(), sizeof(uint32_t), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());         
            break ;
        }
        case 0xa407: /*   GainControl (unsignedRational) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Photo.GainControl");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedRational);
            val->read((const Exiv2::byte*)data.data(), sizeof(Exiv2::Rational), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());             
            break ;
        }
        case 0xa408: /*  Contrast (unsignedShort) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Photo.Contrast");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
            val->read((const Exiv2::byte*)data.data(), sizeof(uint32_t), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());            
            break ;
        }
        case 0xa409: /*   Saturation (unsignedShort) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Photo.Saturation");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
            val->read((const Exiv2::byte*)data.data(), sizeof(uint32_t), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());             
            break ;
        }
        case 0xa40a: /*   Sharpness (unsignedShort) */
        {
            QByteArray data = mrw_get_ttf_tag_value(pos);
            Exiv2::ExifKey exifTag("Exif.Photo.Sharpness");
            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
            val->read((const Exiv2::byte*)data.data(), sizeof(uint32_t), Exiv2::littleEndian);
            m_exifMetadata.add(exifTag, val.get());             
            break ;
        }
        default:
        {
            break ;
        }
    }
}

// TODO  : added Makernote support when Exiv2 support properly Minolta tags

void RAWMetaLoader::mrw_dump_maker_note_tag(off_t pos) 
{
    uint16_t id    = mrw_get_16_tiff(pos+0); 
    uint16_t type  = mrw_get_16_tiff(pos+2) ;
    uint32_t count = mrw_get_32_tiff(pos+4) ;

    switch(id) 
    {
        case 0x0000: /*   MakerNoteVersion */
        {
/*            mrw_print_ttf_tag(pos,id,"MakerNoteVersion",type,count)  ;
            mrw_print_ttf_tag_value(pos) ; */
            break ;
        }
        case 0x0001: /*   MinoltaCameraSettings1  */
        {
            camera_settings_size_1 = mrw_get_32_tiff(pos+4); 
            camera_settings_pos_1  = mrw_get_32_tiff(pos+8); 
/*            mrw_print_ttf_tag(pos,id,"MinoltaCameraSettings1",type,count)  ;
            mrw_print_ttf_tag_value(pos) ; */
            break ;
        }
#if 0
        /* This one is not yet reported to exist but since we have 1, 3 and 4 ... */
        case 0x0002: /*   MinoltaCameraSettings2  */
        {
            camera_settings_size_2 = mrw_get_32_tiff(pos+4); 
            camera_settings_pos_2  = mrw_get_32_tiff(pos+8); 
            mrw_print_ttf_tag(pos,id,"MinoltaCameraSettings2",type,count)  ;
            mrw_print_ttf_tag_value(pos) ; 
            break ;
        }
#endif
        case 0x0003: /*   MinoltaCameraSettings3 */
        {
            camera_settings_size_3 = mrw_get_32_tiff(pos+4); 
            camera_settings_pos_3  = mrw_get_32_tiff(pos+8); 
/*            mrw_print_ttf_tag(pos,id,"MinoltaCameraSettings3",type,count)  ;
            mrw_print_ttf_tag_value(pos) ; */
            break ;
        }
        case 0x0004: /*   MinoltaCameraSettings4 */
        {
/*            camera_settings_size_4 = mrw_get_32_tiff(pos+4); 
            camera_settings_pos_4  = mrw_get_32_tiff(pos+8); 
            mrw_print_ttf_tag(pos,id,"MinoltaCameraSettings4",type,count)  ;
            mrw_print_ttf_tag_value(pos) ; */
            break ;
        }
        case 0x0010: /*   Unknown0010 */
        {
/*            mrw_print_ttf_tag(pos,id,"Unknown0010",type,count)  ;
            mrw_print_ttf_tag_value(pos) ; */
            break ;
        }
        case 0x0018: /*   ImageStabilization */
        {
/*            mrw_print_ttf_tag(pos,id,"ImageStabilization",type,count)  ;
            mrw_print_ttf_tag_value(pos) ; */
            break ;
        }
        case 0x0020: /*   Unknown0020 */
        {
/*            mrw_print_ttf_tag(pos,id,"Unknown0020",type,count)  ;
            mrw_print_ttf_tag_value(pos) ; */
            break ;
        }
        case 0x0040: /*   CompressedImageSize */
        {
/*            mrw_print_ttf_tag(pos,id,"CompressedImageSize",type,count)  ;
            mrw_print_ttf_tag_value(pos) ; */
            break ;
        }
        case 0x0081: /*   PreviewImage */
        {
/*            mrw_print_ttf_tag(pos,id,"PreviewImage",type,count)  ;
            mrw_print_ttf_tag_value(pos) ; */
            break ;
        }
        case 0x0088: /*   PreviewImageStart */
        {
/*            mrw_print_ttf_tag(pos,id,"PreviewImageStart",type,count)  ;
            mrw_print_ttf_tag_value(pos) ; */
            break ;
        }
        case 0x0089: /*   PreviewImageLength */
        {
/*            mrw_print_ttf_tag(pos,id,"PreviewImageLength",type,count)  ;
            mrw_print_ttf_tag_value(pos) ; */
            break ;
        }
        case 0x0100: /*   Unknown0100 */
        {
/*            mrw_print_ttf_tag(pos,id,"Unknown0100",type,count)  ;
            mrw_print_ttf_tag_value(pos) ; */
            break ;
        }
        case 0x0101: /*   ColorMode */
        {
/*            mrw_print_ttf_tag(pos,id,"ColorMode",type,count)  ;
            mrw_print_ttf_tag_value(pos) ; */
            break ;
        }
        case 0x0102: /*   MinoltaQuality */
        {
/*            mrw_print_ttf_tag(pos,id,"MinoltaQuality",type,count)  ;
            mrw_print_ttf_tag_value(pos) ; */
            break ;
        }
        case 0x0103: /*   MinoltaImageSize/MinoltaQuality */
        {
/*            mrw_print_ttf_tag(pos,id,"MinoltaImageSize",type,count)  ;
            mrw_print_ttf_tag_value(pos) ; */
            break ;
        }
        case 0x0104: /*   Unknown0104 */
        {
/*            mrw_print_ttf_tag(pos,id,"Unknown0104",type,count)  ;
            mrw_print_ttf_tag_value(pos) ; */
            break ;
        }
        case 0x0105: /*   Unknown0105 */
        {
/*            mrw_print_ttf_tag(pos,id,"Unknown0105",type,count)  ;
            mrw_print_ttf_tag_value(pos) ; */
            break ;
        }
        case 0x0106: /*   Unknown0106 */
        {
/*            mrw_print_ttf_tag(pos,id,"Unknown0106",type,count)  ;
            mrw_print_ttf_tag_value(pos) ; */
            break ;
        }
        case 0x0107: /*  Stabilization */
        {
/*            mrw_print_ttf_tag(pos,id,"Stabilization",type,count)  ;
            mrw_print_ttf_tag_value(pos) ; */
            break ;
        }
        case 0x010a: /*   ZoneMatching */
        {
/*            mrw_print_ttf_tag(pos,id,"ZoneMatching",type,count)  ;
            mrw_print_ttf_tag_value(pos) ; */
            break ;
        }
        case 0x010b: /*   WbTemperature */
        {
/*            mrw_print_ttf_tag(pos,id,"WbTemperature",type,count)  ;
            mrw_print_ttf_tag_value(pos) ; */
            break ;
        }
        case 0x010c: /*   LensID */
        {
/*            mrw_print_ttf_tag(pos,id,"LensID",type,count)  ;
            mrw_print_ttf_tag_value(pos) ; */
            break ;
        }
        case 0x010d: /*   Unknown010D */
        {
/*            mrw_print_ttf_tag(pos,id,"Unknown010D",type,count)  ;
            mrw_print_ttf_tag_value(pos) ; */
            break ;
        }
        case 0x0114: /*   MinoltaCameraSettings0114 */
        {
            camera_settings_size_0114 = mrw_get_32_tiff(pos+4); 
            camera_settings_pos_0114  = mrw_get_32_tiff(pos+8); 
/*            mrw_print_ttf_tag(pos,id,"MinoltaCameraSettings0114",type,count)  ;
            mrw_print_ttf_tag_value(pos) ; */
            break ;
        }
        case 0x0e00: /*   PrintIM */
        {
/*            mrw_print_ttf_tag(pos,id,"",type,count)  ;
            mrw_print_ttf_tag_value(pos) ; */
            break ;
        }
        case 0x0f00: /*   MinoltaCameraSettingsF00 */
        {
/*            mrw_print_ttf_tag(pos,id,"MinoltaCameraSettingsF00",type,count)  ;
            mrw_print_ttf_tag_value(pos) ; */
            break ;
        }
        default:
        {
/*            mrw_print_ttf_tag(pos,id,"**unknown**",type,count)  ;
            mrw_print_ttf_tag_value(pos) ; */
            break ;  
        }
    }
}

void RAWMetaLoader::mrw_dump_camera_settings_32bit( off_t pos , uint32_t size ,int mode )
{
    uint32_t i ; 
    uint32_t nb = size/4 ;
  
    //printf("<CameraSetting%04x> with %d entries at TFF offset %d\n", mode, (int)nb , (int)pos) ;
  
    for (i=0;i<nb;i++) 
    {
        uint32_t v = mrw_get_32_tiff(pos+i*4)  ;
        // printf("=== 0x%04lx = %08lx\n" ,(unsigned long) i , (unsigned long)  v) ;
    }
}

void RAWMetaLoader::mrw_dump_camera_settings_16bit( off_t pos , uint32_t size ,int mode )
{
    uint32_t i ; 
    uint32_t nb = size/2 ;
  
    // printf("<CameraSetting%04x> with %d entries at TFF offset %d\n", mode, (int)nb , (int)pos) ;
  
    for (i=0;i<nb;i++) 
    {
        uint16_t v = mrw_get_16_tiff(pos+i*2)  ;
        // printf("=== 0x%04lx = %04lx\n" ,(unsigned long) i , (unsigned long)  v) ;
    }
}

/* The CameraSetting4 is used by the Dynax/Maxxum 7D  */ 

void RAWMetaLoader::mrw_dump_camera_settings_4( off_t pos , uint32_t size  )
{
    uint32_t i ; 
    uint32_t nb = size/2 ;
  
    // printf("<CameraSetting4> with %d entries at TFF offset %d\n", (int)nb , (int)pos) ;
    
    for (i=0;i<nb;i++) 
    {
        uint16_t v = mrw_get_16_tiff(pos+i*2)  ;
      
        // printf("  CS ") ;
      
        switch(i)
        {
            case 0x0000: /*  ExposureMode  */
            {
                /*  For the 7D:  */
                const char *x = "**unknown**"; 
                switch(v) 
                {
                    case 0: x ="(P) Program" ; break ;
                    case 1: x ="(A) Aperture Priority" ; break ;
                    case 2: x ="(S) Shutter Priority" ; break ;
                    case 3: x ="(M) Manual" ; break ;
                    case 4: x ="(P/green) Auto" ; break ;
                    case 5: x ="(Pa) Program Shift-A" ; break ;
                    case 6: x ="(Ps) Program Shift-S" ; break ;
                }
                // printf("   0x%04x : %-20s = %6u (%04x) = '%s'\n" ,(int) i , "ExposureMode" ,(unsigned)  v,(unsigned)  v , x)  ; 
            }
            break ;
            case 0x0002: /*  ImageSize */
              /*  For the 7D: 0=3008x2000  1=2256x1496  2=1504x1000 */
        //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "ImageSize" ,(unsigned)  v,(unsigned)  v)  ; 
              break ;
            case 0x0003: /*  Quality */
              /*  For the 7D: 0=RAW  34=RAW+JPEG */
        //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "ImageSize" ,(unsigned)  v,(unsigned)  v)  ; 
              break ;
            case 0x0004: /*  WbMode */
        //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "WbMode?" ,(unsigned)  v,(unsigned)  v)  ; 
              break ;
            case 0x000E: /*  FocusDial */
              /*  For the 7D:  0=S 1= 2= 3=A 4=M */
        //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "FocusDial" ,(unsigned)  v,(unsigned)  v)  ; 
              break ;
            case 0x0010: /*  FocusPosition */
        //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "FocusPosition" ,(unsigned)  v,(unsigned)  v)  ; 
              break ;
            case 0x0015: /*  Flash */
              /*  For the 7D:  0=OFF 1=ON */
        //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "Flash" ,(unsigned)  v,(unsigned)  v)  ; 
              break ;
            case 0x0016: /*  FlashMode */
        //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "FlashMode" ,(unsigned)  v,(unsigned)  v)  ; 
              break ;
            case 0x001C: /*  ISO */
        //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "ISO?" ,(unsigned)  v,(unsigned)  v)  ; 
              break ;
            case 0x0025: /*  ColorMode */
              /*  On 7D:  0=NaturalsRGB 1=Naturals+RGB  4=EmbedAdobeRGB */
        //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "ColorMode" ,(unsigned)  v,(unsigned)  v)  ; 
              break ;
            case 0x003E: /*  FocusMode  */
        //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "FocusMode" ,(unsigned)  v,(unsigned)  v)  ; 
              break ;
            case 0x003F: /*  WBTemperature  */
        //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "WBTemperature" ,(unsigned)  v,(unsigned)  v)  ; 
              break ;
            case 0x0040: /*  Hue  ( 8(-2) , 9(-1) , 10(0) , 11(+1) , 12(+2) ) */
        //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "Hue" ,(unsigned)  v,(unsigned)  v)  ; 
              break ;
            case 0x0046: /*  CameraOrientation  */
            {
                const char * x = "Unknown" ;
                if (v == 72) x = "Horizontal"; 
                if (v == 82) x = "CounterClockWise"; 
                if (v == 76) x = "ClockWise"; 
            //	printf("   0x%04x : %-20s = %6u (%04x) = %s\n" ,(int) i , "CameraOrientation" ,(unsigned)  v,(unsigned)  v,x)  ;      
            }
            break ;
            case 0x0047: /*  Aperture */
        //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "Aperture?" ,(unsigned)  v,(unsigned)  v)  ; 
              break ;
            case 0x0048: /*  ShutterTime */
        //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "ShutterTime?" ,(unsigned)  v,(unsigned)  v)  ; 
              break ;
            case 0x002D: /*  MemCardSpace  (# of images that can still be stored on memory card) */
            case 0x004A: /*  is repeated here  */
        //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "MemCardSpace" ,(unsigned)  v,(unsigned)  v)  ; 
              break ;
            case 0x0060: /*  NoiseReduction */
        //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "NoiseReduction" ,(unsigned)  v,(unsigned)  v)  ; 
              break ;
            case 0x0062: /*  PictureNumber */
        //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "PictureNumber" ,(unsigned)  v,(unsigned)  v)  ; 
              break ;
            case 0x0071: /*  AntiShake */
        //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "AntiShake" ,(unsigned)  v,(unsigned)  v)  ; 
              break ;
            case 0x0075: /*  UseZoneMatching */
        //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "UseZoneMatching" ,(unsigned)  v,(unsigned)  v)  ; 
              break ;
            default:
        //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "????" ,(unsigned)  v,(unsigned)  v)  ; 
              break;
        }
    }  
}

/* The CameraSetting0114 is used by the Dynax/Maxxum 5D  */ 

void RAWMetaLoader::mrw_dump_camera_settings_0114( off_t pos , uint32_t size  )
{
    uint32_t i ; 
    uint32_t nb = size/2 ;
  
    // printf("<CameraSetting0114> with %d entries at TFF offset %d\n", (int)nb , (int)pos) ;
    
    for (i=0;i<nb;i++) 
    {
        uint16_t v = mrw_get_16_tiff(pos+i*2)  ;
      
        // printf("  CS ") ;
      
      switch(i)
      {
          case 0x000A: /*  ExposureMode  */
          {
              /*  For the 7D:  */
              const char *x = "**unknown**"; 
              switch(v) 
              {
                  case 0: x ="(P) Program" ; break ;
                  case 1: x ="(A) Aperture Priority" ; break ;
                  case 2: x ="(S) Shutter Priority" ; break ;
                  case 3: x ="(M) Manual" ; break ;
                  case 4: x ="(P/green) Auto" ; break ;
                  case 4131: x ="(?) connected copying" ; break ;
              }
              //printf("   0x%04x : %-20s = %6u (%04x) = '%s'\n" ,(int) i , "ExposureMode" ,(unsigned)  v,(unsigned)  v , x)  ; 
          }
          break ; 
          case 0x000D: /*  Quality */
      //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "Quality" ,(unsigned)  v,(unsigned)  v)  ; 
            break ;
          case 0x0025: /*  PhotometryMode */
      //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "PhotometryMode" ,(unsigned)  v,(unsigned)  v)  ; 
            break ;
          case 0x0026: /*  ISO */
      //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "ISO" ,(unsigned)  v,(unsigned)  v)  ; 
            break ;
          case 0x0030: /*  Sharpness */
      //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "Sharpness" ,(unsigned)  v,(unsigned)  v)  ; 
            break ;
          case 0x0031: /*  Contrast */
      //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "Contrast" ,(unsigned)  v,(unsigned)  v)  ; 
            break ;
          case 0x0032: /*  Saturation */
      //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "Saturation" ,(unsigned)  v,(unsigned)  v)  ; 
            break ;
          case 0x0037: /*  MemCardSpace1 */
      //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "MemCardSpace1" ,(unsigned)  v,(unsigned)  v)  ; 
            break ;
          case 0x0038: /*  ExposureRevision */
      //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "ExposureRevision" ,(unsigned)  v,(unsigned)  v)  ; 
            break ;
          case 0x0049: /*  WBFineControl */
      //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "WBFineControl" ,(unsigned)  v,(unsigned)  v)  ; 
            break ;
          case 0x0050: /*  CameraOrientation  */
          {
              const char * x = "unknown" ;
              if (v == 72) x = "Horizontal"; 
              if (v == 82) x = "CounterClockWise"; 
              if (v == 76) x = "ClockWise"; 
              // printf("   0x%04x : %-20s = %6u (%04x) = %s\n" ,(int) i , "CameraOrientation" ,(unsigned)  v,(unsigned) v , x)  ;      
          }
          break ;
          case 0x0054: /*  MemCardSpace2 */
      //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "MemCardSpace2" ,(unsigned)  v,(unsigned)  v)  ; 
            break ;
          case 0x00AE: /*  PictureNumber */
      //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "PictureNumber" ,(unsigned)  v,(unsigned)  v)  ; 
            break ;
          case 0x00B0: /*  NoiseReduction */
      //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "NoiseReduction" ,(unsigned)  v,(unsigned)  v)  ; 
            break ;
          case 0x00BD: /*  AntiShake */
      //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "AntiShake" ,(unsigned)  v,(unsigned)  v)  ; 
            break ;
          default:
      //      printf("   0x%04x : %-20s = %6u (%04x)\n" ,(int) i , "????" ,(unsigned)  v,(unsigned)  v)  ; 
            break;
        }
    }
}

void RAWMetaLoader::mrw_parse_ttf_block( off_t pos , uint32_t sz )
{
    int i ; 
    char     c1, c2 ; 
    uint16_t magic ;
    uint32_t dir ;
  
    
    tiff_base = pos ;
    tiff_size = sz ;
  
    if ( sz < 8 ) 
    {
        kdDebug() << "Illegal size " << sz << " for TTF block. Should be 8" << endl;
    }
    
    c1 = mrw_get_8_tiff(0) ; 
    c2 = mrw_get_8_tiff(1) ; 
  
    if ( c1 == 'M' &&  c2 == 'M' ) 
    {
        kdDebug() << "Found MSB TIFF Header" << endl;
        tiff_msb = 1;
    } 
    else if ( c1 == 'L' &&  c2 == 'L' ) 
    {
        /* should not happen in MRW but ... */
        kdDebug() << "Found LSB TIFF Header" << endl;
        tiff_msb = 0;
    }   
    else 
    {
        kdDebug() << "Illegal header in TTF block" << endl;
    }
  
    magic = mrw_get_16_tiff(2) ; 
  
    if ( magic != 42 ) 
    {
        kdDebug() << "Illegal magic number in TTF header" << endl;
    }
    
    /* Tiff offset of the 1st directory */
    dir = mrw_get_32_tiff(4) ; 
  
    do 
    {
        uint16_t nb ;
        nb = mrw_get_16_tiff(dir) ;
        kdDebug() << "<IFD> with " << nb << " entries at offset " << dir << endl;
  
        for ( i=0;i<nb;i++) 
        {
            off_t    tag_pos   = dir+2+i*12 ;
            mrw_dump_ttf_tag( tag_pos );
        }
  
        /* Tiff offset of the next directory */
        dir = mrw_get_32_tiff(dir+2+12*nb) ; 
  
    } 
    while (dir != 0);
  
    if ( exif_start != 0 )
    {
        uint16_t nb ;
        nb = mrw_get_16_tiff(exif_start) ;
        kdDebug() << "<EXIF> with " << nb << " entries at offset " << exif_start << endl;
        
        for ( i=0;i<nb;i++) 
        {
            off_t    tag_pos   = exif_start+2+i*12 ;
            mrw_dump_exif_tag( tag_pos )  ;
        }  
    }
  
    if ( maker_note != 0 )
    {
        uint16_t nb ;
        nb = mrw_get_16_tiff(maker_note) ;
        kdDebug() << "<MAKERNOTE> with " << nb << " entries at offset " << maker_note << endl;
        
        for ( i=0;i<nb;i++) 
        {
            off_t    tag_pos   = maker_note+2+i*12 ;
            mrw_dump_maker_note_tag( tag_pos )  ;
        }  
    }
  
    if ( camera_settings_pos_1 != 0 ) 
    {
      mrw_dump_camera_settings_32bit( camera_settings_pos_1 , camera_settings_size_1 , 1 );
    }
  
    if ( camera_settings_pos_2 != 0 ) 
    {
      mrw_dump_camera_settings_32bit( camera_settings_pos_2 , camera_settings_size_2 , 2 );
    }
  
    if ( camera_settings_pos_3 != 0 ) 
    {
      mrw_dump_camera_settings_32bit( camera_settings_pos_3 , camera_settings_size_3 , 3 );
    }
  
    if ( camera_settings_pos_4 != 0 ) 
    {
      mrw_dump_camera_settings_4( camera_settings_pos_4 , camera_settings_size_4 );
    }
      
    if ( camera_settings_pos_0114 != 0 ) 
    {
      mrw_dump_camera_settings_0114( camera_settings_pos_0114 , camera_settings_size_0114 );
    }
}

bool RAWMetaLoader::mrw_parse_mrm_block() 
{
    off_t    pos = 0 ;
    /*
    * The MRM block is composed of 
    *   - 4*8bit    : the block identifier (\0 M R M)
    *   - 32bit MSB : the size of the MRM body in bytes
    */
  
    char     c1 = mrw_get_8(pos+0); 
    char     c2 = mrw_get_8(pos+1); 
    char     c3 = mrw_get_8(pos+2); 
    char     c4 = mrw_get_8(pos+3); 
    uint32_t sz = mrw_get_32_m(pos+4);    
  
    if ( c1!='\0' || c2!='M' ||  c3!='R' ||  c4!='M' ) 
    {
        kdDebug() << "MRM block not found" << endl;
    }
  
    /* The image data start immediately after the MRM block */ 
    image_start = pos + 8 + sz ; 
    mrw_check_valid(image_start,0) ; 
  
    pos += 8 ; 
  
    while ( pos < image_start ) 
    {
       /* The MRM block contains a sequence of blocks .
        * They are described using the same principle as for the MRM:
        *
        *   - 4*8bit    : the block identifier (\0 ? ? ?)
        *   - 32bit MSB : the size of the block body in bytes
        *   - ... 
        *
        */
  
        char c1     = mrw_get_8(pos+0) ; 
        char c2     = mrw_get_8(pos+1) ; 
        char c3     = mrw_get_8(pos+2) ; 
        char c4     = mrw_get_8(pos+3) ; 
  
        uint32_t sz = mrw_get_32_m(pos+4) ;  
  
        if ( c1=='\0' && c2=='P' && c3=='R' && c4=='D' )
        {
            /* Picture Raw Dimensions */
            mrw_parse_prd_block( pos+8 , sz) ;
        } 
        else if ( c1=='\0' && c2=='T' && c3=='T' && c4=='W' ) 
        {
            /* Tiff Tags 'Wonderland' */
            mrw_parse_ttf_block( pos+8 , sz) ;
        } 
        else if ( c1=='\0' && c2=='W' && c3=='B' && c4=='G' ) 
        {
            /* WBG = White Balance Gains */
            mrw_parse_wbg_block( pos+8 , sz) ;
        } 
        else if ( c1=='\0' && c2=='R' && c3=='I' && c4=='F' ) 
        {
            /* RIF = Requested Image Format */
            mrw_parse_rif_block( pos+8 , sz) ;
        } 
        else if ( c1=='\0' && c2=='P' && c3=='A' && c4=='D' ) 
        {
            /* A padding block. Its only purpose is to align the next block */ 
        } 
        else 
        {
            return false;
        }
          
        pos += 8 ;
        pos += sz ;
    }
 
    return true ;  
}

}  // NameSpace Digikam
