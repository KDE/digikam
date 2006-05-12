/* ============================================================
 * Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-23
 * Description : MRW RAW file metadata parser
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
#include <cstdio>
#include <cassert>

// Exiv2 includes.

#include <exiv2/types.hpp>
#include <exiv2/tags.hpp>
#include <exiv2/value.hpp>

// Local includes.

#include "mrwparser.h"

namespace Digikam
{

static const MRWParser::prd_version_t prd_version_info[] = 
{ 
    { "27730001", "D5"              },
    { "27660001", "D7"              },
    { "27660001", "D7u"             },
    { "27790001", "D7i"             },
    { "27780001", "D7Hi"            },
    { "27820001", "A1"              },
    { "27200001", "A2"              }, 
    { "27470002", "A200"            },
    { "21810002", "Dynax/Maxxum 7D" },
    { NULL      , NULL              }
};

MRWParser::MRWParser()
{
    err_counter               = 0; 
    warn_counter              = 0; 
    exif_start                = 0; 
    maker_note                = 0; 
    printim_start             = 0;
    camera_settings_pos_1     = 0; 
    camera_settings_size_1    = 0; 
    camera_settings_pos_3     = 0;
    camera_settings_size_3    = 0;
    camera_settings_pos_4     = 0;
    camera_settings_size_4    = 0;
    camera_settings_pos_0114  = 0;
    camera_settings_size_0114 = 0;    
}

MRWParser::~MRWParser()
{
}

bool MRWParser::parseMRW(const char *fname) 
{
    if (!load_file(fname))
       return false;
    
    prd.valid = 0; 
    
    parse_mrm_block(); 
    
    delete [] rawdata;
    
    if (err_counter > 0) 
        return false;
    
    return true;   
}

Exiv2::ExifData MRWParser::getExif()
{
    return m_exifMetadata;
}
    
// Load the file into memory

bool MRWParser::load_file(const char *fname) 
{
    rawfile = fname; 
  
    FILE *f = fopen( fname , "rb" ); 

    if (f==NULL) 
    {
        std::cerr << "Failed to open file " << fname << "\n";
        return false;
    }
    
    fseek(f, 0, SEEK_END);
    rawsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    if (rawsize > 400000) 
        rawsize = 400000;
    
    rawdata = new char[rawsize]; 
    
    if (!rawdata) 
    {
        std::cerr << "Failed to allocate " << rawsize << " bytes" << "\n";
        return false;
    }
    
    size_t sz = fread(rawdata, 1, rawsize, f);
    
    if (sz!=rawsize) 
    {
        std::cerr << "Failed to load " << rawsize << " bytes from file " 
                  << fname << ". Only read " << sz << "\n";
        delete [] rawdata;
        return false;
    }  

    return true;
}

void MRWParser::parse_ttf_block( off_t pos , uint32_t sz )
{
    int      i; 
    char     c1, c2; 
    uint16_t magic;
    uint32_t dir;
  
    
    tiff_base = pos;
    tiff_size = sz;
  
    if ( sz < 8 ) 
        std::cerr << "Illegal size " << sz << " for TTF block. Should be 8" << "\n";
    
    c1 = get_8_tiff(0); 
    c2 = get_8_tiff(1); 
  
    if ( c1 == 'M' &&  c2 == 'M' ) 
    {
        std::cerr << "Found MSB TIFF Header" << "\n";
        tiff_msb = 1;
    } 
    else if ( c1 == 'L' &&  c2 == 'L' ) 
    {
        /* should not happen in MRW but ... */
        std::cerr << "Found LSB TIFF Header" << "\n";
        tiff_msb = 0;
    }   
    else 
    {
        std::cerr << "Illegal header in TTF block" << "\n";
    }
  
    magic = get_16_tiff(2) ; 
  
    if ( magic != 42 ) 
        std::cerr << "Illegal magic number in TTF header" << "\n";
    
    /* Tiff offset of the 1st directory */
    dir = get_32_tiff(4) ; 
  
    do 
    {
        uint16_t nb = get_16_tiff(dir);
        std::cerr << "<IFD> with " << nb << " entries at offset " << dir << "\n";
  
        for (i=0 ; i < nb ; i++) 
        {
            off_t tag_pos = dir+2+i*12;
            dump_ttf_tag( tag_pos );
        }
  
        /* Tiff offset of the next directory */
        dir = get_32_tiff(dir+2+12*nb) ; 
  
    } 
    while (dir != 0);
  
    if ( exif_start != 0 )
    {
        uint16_t nb = get_16_tiff(exif_start);
        std::cerr << "<EXIF> with " << nb << " entries at offset " << exif_start << "\n";
        
        for (i=0 ; i < nb ; i++) 
        {
            off_t tag_pos = exif_start+2+i*12;
            dump_exif_tag( tag_pos );
        }  
    }
  
    if ( maker_note != 0 )
    {
        uint16_t nb = get_16_tiff(maker_note);
        std::cerr << "<MAKERNOTE> with " << nb << " entries at offset " << maker_note << "\n";
        
        for (i=0 ; i < nb ; i++) 
        {
            off_t tag_pos = maker_note+2+i*12;
            dump_maker_note_tag( tag_pos );
        }  
    }
  
    if ( camera_settings_pos_1 != 0 ) 
    {
      dumpCameraSettingsStd( camera_settings_pos_1 , camera_settings_size_1, "Exif.MinoltaCsOld." );
    }
  
    if ( camera_settings_pos_3 != 0 ) 
    {
      dumpCameraSettingsStd( camera_settings_pos_3 , camera_settings_size_3, "Exif.MinoltaCsNew." );
    }
  
    if ( camera_settings_pos_4 != 0 ) 
    {
      dumpCameraSettings7D( camera_settings_pos_4 , camera_settings_size_4 );
    }
      
    if ( camera_settings_pos_0114 != 0 ) 
    {
      dumpCameraSettings5D( camera_settings_pos_0114 , camera_settings_size_0114 );
    }
}

bool MRWParser::parse_mrm_block() 
{
    off_t    pos = 0 ;
    /*
    * The MRM block is composed of 
    *   - 4*8bit    : the block identifier (\0 M R M)
    *   - 32bit MSB : the size of the MRM body in bytes
    */
  
    char     c1 = get_8(pos+0); 
    char     c2 = get_8(pos+1); 
    char     c3 = get_8(pos+2); 
    char     c4 = get_8(pos+3); 
    uint32_t sz = get_32_m(pos+4);    
  
    if ( c1!='\0' || c2!='M' ||  c3!='R' ||  c4!='M' ) 
        std::cerr << "MRM block not found" << "\n";
  
    /* The image data start immediately after the MRM block */ 
    image_start = pos + 8 + sz; 
    check_valid(image_start, 0); 
  
    pos += 8; 
  
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
  
        char c1 = get_8(pos+0); 
        char c2 = get_8(pos+1); 
        char c3 = get_8(pos+2); 
        char c4 = get_8(pos+3); 
  
        uint32_t sz = get_32_m(pos+4);  
  
        if ( c1=='\0' && c2=='P' && c3=='R' && c4=='D' )
        {
            /* Picture Raw Dimensions */
            parse_prd_block( pos+8 , sz);
        } 
        else if ( c1=='\0' && c2=='T' && c3=='T' && c4=='W' ) 
        {
            /* Tiff Tags 'Wonderland' */
            parse_ttf_block( pos+8 , sz);
        } 
        else if ( c1=='\0' && c2=='W' && c3=='B' && c4=='G' ) 
        {
            /* WBG = White Balance Gains */
            parse_wbg_block( pos+8 , sz);
        } 
        else if ( c1=='\0' && c2=='R' && c3=='I' && c4=='F' ) 
        {
            /* RIF = Requested Image Format */
            parse_rif_block( pos+8 , sz);
        } 
        else if ( c1=='\0' && c2=='P' && c3=='A' && c4=='D' ) 
        {
            /* A padding block. Its only purpose is to align the next block */ 
        } 
        else 
        {
            return false;
        }
          
        pos += 8;
        pos += sz;
    }
 
    return true;  
}

// Check that it is valid to access count bytes at offset pos
// Note: it is valid to access 0 bytes at the end of the file (e.g. check_valid(rawsize,0)) 
void MRWParser::check_valid(off_t pos, int count)
{
    if (count >= 0) return; 
    if ( pos<0 || pos+count>rawsize ) 
        std::cerr << "Trying to access " << pos << " bytes at offset #" << count << "\n";
}

char MRWParser::get_8(off_t pos) 
{
    check_valid(pos, 1); 
    return rawdata[pos];
}

int16_t MRWParser::get_16_l(off_t pos) 
{
    check_valid(pos, 2); 
    return (int16_t) ( (((uint8_t)rawdata[pos+0]) << 0) + 
                       (((uint8_t)rawdata[pos+1]) << 8) ); 
}

int32_t MRWParser::get_32_l(off_t pos) 
{
    check_valid(pos, 4); 
    return (int32_t) ( (((uint8_t)rawdata[pos+0]) <<   0) + 
                        (((uint8_t)rawdata[pos+1]) <<  8) + 
                        (((uint8_t)rawdata[pos+2]) << 16) + 
                        (((uint8_t)rawdata[pos+3]) << 24) );
}

int16_t MRWParser::get_16_m(off_t pos) 
{
    check_valid(pos, 2); 
    return (int16_t) ( (((uint8_t)rawdata[pos+1]) << 0) + 
                       (((uint8_t)rawdata[pos+0]) << 8) ); 
}

int32_t MRWParser::get_32_m(off_t pos) 
{
    check_valid(pos, 4); 
    return (int32_t) ( (((uint8_t)rawdata[pos+3]) <<   0) + 
                        (((uint8_t)rawdata[pos+2]) <<  8) + 
                        (((uint8_t)rawdata[pos+1]) << 16) + 
                        (((uint8_t)rawdata[pos+0]) << 24) );
}

void MRWParser::parse_wbg_block( off_t pos , uint32_t sz )
{
    uint8_t  Norm[4]; 
    uint16_t Coef[4]; 
  
    if ( sz != 12 ) 
        std::cerr << "Illegal size " << sz << " for WBG block. Should be 24" << "\n";
    
    Norm[0] = get_8(pos + 0);
    Norm[1] = get_8(pos + 1);
    Norm[2] = get_8(pos + 2);
    Norm[3] = get_8(pos + 3);
  
    Coef[0] = get_16_m(pos +  4);
    Coef[1] = get_16_m(pos +  6);
    Coef[2] = get_16_m(pos +  8);
    Coef[3] = get_16_m(pos + 10);
}

void MRWParser::parse_rif_block( off_t pos , uint32_t sz )
{
    off_t       at;
    uint8_t     u8;
    float       iso; 
    const char *zone_matching;
    const char *color_mode;
    const char *prg_mode;
    
    #define REQSIZE(x) if ( sz < x ) return ;   
    
    at = 0; 
    REQSIZE(at+1);
    u8  = get_8(pos+at);
    // printf("rif.Unknown%-9d = %4d (0x%02x) \n" , (int) at, u8 , u8) ;
        
    at = 1; 
    REQSIZE(at+1);
    u8  = get_8(pos+at);
    // printf("rif.%-16s = %4d (0x%02x) --> %+d\n" , "Saturation", u8 , u8, (int) (signed char)u8 ) ;
    
    at = 2; 
    REQSIZE(at+1);
    u8  = get_8(pos+at);
    // printf("rif.%-16s = %4d (0x%02x) --> %+d\n" , "Contrast", u8 , u8 , (int) (signed char)u8 ) ;
    
    at = 3; 
    REQSIZE(at+1);
    u8  = get_8(pos+at);
    // printf("rif.%-16s = %4d (0x%02x) --> %+d\n" , "Sharpness", u8 , u8 , (int) (signed char)u8 ) ;
    
    at = 4; 
    REQSIZE(at+1);
    u8  = get_8(pos+at);
    // printf("rif.%-16s = %4d (0x%02x) \n" , "WBMode", u8 , u8) ;
    
    at = 5; 
    REQSIZE(at+1);
    u8  = get_8(pos+at);
    
    switch ( u8 ) 
    {
        case 0x00:  prg_mode = "None"           ; break;
        case 0x01:  prg_mode = "Portrait"       ; break;
        case 0x02:  prg_mode = "Text"           ; break;
        case 0x03:  prg_mode = "Night Portrait" ; break;
        case 0x04:  prg_mode = "Sunset"         ; break;
        case 0x05:  prg_mode = "Sports Action"  ; break;
        default:    prg_mode = "*UNKNOWN*"      ; break;
    }
    
    // printf("rif.%-16s = %4d (0x%02x) --> %s\n" , "ProgramMode", u8 , u8, prg_mode) ;
    
    at = 6; 
    REQSIZE(at+1);
    u8  = get_8(pos+at);
    iso = pow(2, (u8/8.0)-1)*3.125;
    // printf("rif.%-16s = %4d (0x%02x) --> %f\n" , "ISO", u8 , u8 , iso) ;
    
    at = 7; 
    REQSIZE(at+1);
    u8  = get_8(pos+at);
    
    switch ( u8 ) 
    {
        case 0x00:  color_mode = "Normal Color"  ; break;
        case 0x01:  color_mode = "B&W"           ; break;
        case 0x02:  color_mode = "Vivid Color"   ; break;
        case 0x03:  color_mode = "Solarization"  ; break;
        case 0x04:  color_mode = "AdobeRGB"      ; break;   
        case 0x0d:  color_mode = "Natural sRGB"  ; break;
        case 0x0e:  color_mode = "Natural+ sRGB" ; break;
        case 0x84:  color_mode = "EmbedAdobeRGB" ; break;
        default:    color_mode = "*UNKNOWN*"     ; break;
    }
    
    // printf("rif.%-16s = %4d (0x%02x) --> %s\n" , "ColorMode", u8 , u8, color_mode) ;
    
    /*** A bunch of unknown fields from 8 to 55 ***/
    at++; 
    for (; at < 56 ; at++ ) 
    {
        REQSIZE(at+1);
        u8  = get_8(pos+at);
    // printf("rif.Unknown%-9u = %4d (0x%02x) \n" , (int) at, u8 , u8) ;
    }
    
    at = 56; 
    REQSIZE(at+1);
    u8  = get_8(pos+at);
    // printf("rif.%-16s = %4d (0x%02x) --> %+d\n" , "ColorFilter", u8 , u8, (int) (signed char)u8 ) ;
    
    at = 57; 
    REQSIZE(at+1);
    u8  = get_8(pos+at);
    // printf("rif.%-16s = %4d (0x%02x) \n" , "BlackWhiteFilter", u8 , u8) ;
    
    at = 58; 
    REQSIZE(at+1);
    u8  = get_8(pos+at);
    
    switch ( u8 ) 
    {
        case 0:  zone_matching = "NONE"      ; break;
        case 1:  zone_matching = "HIGH"      ; break;
        case 2:  zone_matching = "LOW"       ; break;
        default: zone_matching = "*UNKNOWN*" ; break;
    }
    
    // printf("rif.%-16s = %4d (0x%02x) --> %s\n" , "ZoneMatching", u8 , u8, zone_matching) ;
    
    at = 59; 
    REQSIZE(at+1);
    u8  = get_8(pos+at);
    // printf("rif.%-16s = %4d (0x%02x) --> %+d\n" , "Hue", u8 , u8, (int) (signed char)u8 ) ;
    
    at = 60; 
    REQSIZE(at+1);
    u8  = get_8(pos+at);
    // printf("rif.%-16s = %4d (0x%02x) --> %dK\n" , "WBTemperature", u8 , u8, u8*100) ;
    
    at++;
    for( ; at<sz; at++) 
    {
        u8  = get_8(pos+at);
        // printf("rif.Unknown%-9d = %4d (0x%02x) \n" , (int)at, u8 , u8) ;
    }
  
#undef REQSIZE 
}

void MRWParser::parse_prd_block( off_t pos , uint32_t sz )
{
    int i;
    
    if ( sz != 24 ) 
        std::cerr << "Illegal size " << sz << " for PRD block. Should be 24" << "\n";
    
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
        prd.versionString[i] = get_8(pos+i) ;
    } 
    
    prd.sensorHeight  = get_16_m(pos +  8) ;
    prd.sensorWidth   = get_16_m(pos + 10) ;
    prd.imageHeight   = get_16_m(pos + 12) ;
    prd.imageWidth    = get_16_m(pos + 14) ;
    prd.dataSize      = get_8(pos + 16) ;
    prd.pixelSize     = get_8(pos + 17) ;
    prd.storageMethod = get_8(pos + 18) ;
    prd.unknown19     = get_8(pos + 19) ;
    prd.unknown20     = get_8(pos + 20) ;
    prd.unknown21     = get_8(pos + 21) ;
    prd.unknown22     = get_8(pos + 22) ;
    prd.bayerPattern  = get_8(pos + 23) ;
    
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

void MRWParser::check_valid_tiff(off_t pos, int count)
{
    if (count>=0) return; 
    if ( pos<0 || pos+count>tiff_size ) 
    {
        std::cerr << "Trying to access " << pos << " bytes at offset #" 
                  << count << " in TTF block" << "\n";
    }
}

char MRWParser::get_8_tiff(off_t pos) 
{
    check_valid_tiff(pos, 1) ;
    return get_8(tiff_base+pos); 
}

int16_t MRWParser::get_16_tiff(off_t pos) 
{
    check_valid_tiff(pos, 2) ;
    if (tiff_msb) 
    {
        return get_16_m(tiff_base+pos); 
    }
    else 
    {
        return get_16_l(tiff_base+pos); 
    }
}

int32_t MRWParser::get_32_tiff(off_t pos) 
{
    check_valid_tiff(pos, 2) ;
    if (tiff_msb) 
    {
        return get_32_m(tiff_base+pos); 
    }
    else 
    {
        return get_32_l(tiff_base+pos); 
    }
}

Exiv2::DataBuf MRWParser::get_ttf_tag_value(off_t pos) 
{
    try
    { 
        int16_t  type  = get_16_tiff(pos+2);
        uint32_t count = get_32_tiff(pos+4);
        uint32_t i; 
        uint32_t sz;
        
        switch(type) 
        {
            case TIFF_TYPE_BYTE:
            {
                sz = 1 ; 
                (sz*count > 4) ? pos = get_32_tiff(pos+8) : pos = pos + 8 ;
                Exiv2::DataBuf data(count);
                for(i = 0 ; i < count ; i++)
                {
                    uint8_t v = get_8_tiff(pos+sz*i) ;
                    memcpy(data.pData_+i, &v, 1);
                }
                return data;
                break; 
            }
            case TIFF_TYPE_ASCII:
            {
                (count > 4) ? pos = get_32_tiff(pos+8) : pos = pos + 8;
                Exiv2::DataBuf data(count);
                for(i = 0 ; i < count ; i++) 
                {
                    int8_t c = get_8_tiff(pos++) ;
                    memcpy(data.pData_+i, &c, 1);
                }
                return data;
                break; 
            }
            case TIFF_TYPE_SHORT:
            {
                sz = 2 ; 
                (sz*count > 4) ? pos = get_32_tiff(pos+8) : pos = pos + 8;
                Exiv2::DataBuf data(sz*count);
                for(i = 0 ; i < count ; i++) 
                {
                    uint16_t v = get_16_tiff(pos+sz*i) ;
                    memcpy(data.pData_+i*sz, &v, sz);
                }
                return data;
                break;
            }
            case TIFF_TYPE_LONG:
            {
                sz = 4 ; 
                (sz*count > 4) ? pos = get_32_tiff(pos+8) : pos = pos + 8;
                Exiv2::DataBuf data(sz*count);
                for(i = 0 ; i < count ; i++) 
                {
                    uint32_t v = get_32_tiff(pos+sz*i) ;
                    memcpy(data.pData_+i*sz, &v, sz);
                }
                return data;
                break;
            }
            case TIFF_TYPE_RATIONAL:
            {
                pos = get_32_tiff(pos+8) ;
                Exiv2::DataBuf data(8*count);
                for(i = 0 ; i < count ; i++) 
                {
                    uint32_t v1 = get_32_tiff(pos+8*i+0) ;
                    memcpy(data.pData_+i*8, &v1, 4);
                    uint32_t v2 = get_32_tiff(pos+8*i+4) ;
                    memcpy(data.pData_+i*8+4, &v2, 4);
                }
                return data;
                break; 
            }
            case TIFF_TYPE_SBYTE: 
            {
                sz = 1 ; 
                (sz*count > 4) ? pos = get_32_tiff(pos+8) : pos = pos + 8 ;
                Exiv2::DataBuf data(count);
                for(i = 0 ; i < count ; i++)
                {
                    int8_t v = get_8_tiff(pos+sz*i) ;
                    memcpy(data.pData_+i, &v, 1);
                }
                return data;
                break; 
            }
            case TIFF_TYPE_UNDEFINED:
            {
                pos = pos + 8;
                Exiv2::DataBuf data(count);
                for(i = 0 ; i < count ; i++) 
                {
                    int8_t c = get_8_tiff(pos++) ;
                    memcpy(data.pData_+i, &c, 1);
                }
                return data;
                break;
            }
            case TIFF_TYPE_SSHORT:
            {
                sz = 2 ; 
                (sz*count > 4) ? pos = get_32_tiff(pos+8) : pos = pos + 8;
                Exiv2::DataBuf data(sz*count);
                for(i = 0 ; i < count ; i++) 
                {
                    int16_t v = get_16_tiff(pos+sz*i) ;
                    memcpy(data.pData_+i*sz, &v, sz);
                }
                return data;
                break;
            }
            case TIFF_TYPE_SLONG:
            {
                sz = 4 ; 
                (sz*count > 4) ? pos = get_32_tiff(pos+8) : pos = pos + 8;
                Exiv2::DataBuf data(sz*count);
                for(i = 0 ; i < count ; i++) 
                {
                    int32_t v = get_32_tiff(pos+sz*i) ;
                    memcpy(data.pData_+i*sz, &v, sz);
                }
                return data;
                break;
            }        
            case TIFF_TYPE_SRATIONAL:
            {
                pos = get_32_tiff(pos+8) ;
                Exiv2::DataBuf data(8*count);
                for(i = 0 ; i < count ; i++) 
                {
                    int32_t v1 = get_32_tiff(pos+8*i+0) ;
                    memcpy(data.pData_+i*8, &v1, 4);
                    int32_t v2 = get_32_tiff(pos+8*i+4) ;
                    memcpy(data.pData_+i*8+4, &v2, 4);
                }
                return data;
                break;   
            }
            case TIFF_TYPE_FLOAT:
                break ; 
            case TIFF_TYPE_DOUBLE:
                break ; 
        } 
    }
    catch( Exiv2::Error &e )
    {
        std::cerr << "Cannot parse MRW tag data using Exiv2 (" 
                  << e.what() << ")\n";
    }                       
    
    return Exiv2::DataBuf();
}

void MRWParser::dump_ttf_tag(off_t pos) 
{
    try
    { 
        uint16_t id = get_16_tiff(pos+0); 
        
        switch(id) 
        {
            case 0x0100: /*  ImageWidth (unsignedLong) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Image.ImageWidth");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedLong);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedLong), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());        
                break;
            }
            case 0x0101: /*  ImageLength (unsignedLong) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Image.ImageLength");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedLong);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedLong), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());        
                break;
            }
            case 0x0103: /*  Compression (unsignedShort) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Image.Compression");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());        
                break;
            }
            case 0x010e: /*  ImageDescription (asciiString) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                std::string string(reinterpret_cast<char*>(data.pData_), data.size_);
                Exiv2::ExifKey exifTag("Exif.Image.ImageDescription");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::asciiString);
                val->read(string.c_str());
                m_exifMetadata.add(exifTag, val.get());        
                break;
            }
            case 0x010f: /*  Make (asciiString) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                std::string string(reinterpret_cast<char*>(data.pData_), data.size_);
                Exiv2::ExifKey exifTag("Exif.Image.Make");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::asciiString);
                val->read(string.c_str());
                m_exifMetadata.add(exifTag, val.get());        
                break;
            }
            case 0x0110: /*  Model (asciiString) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                std::string string(reinterpret_cast<char*>(data.pData_), data.size_);
                Exiv2::ExifKey exifTag("Exif.Image.Model");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::asciiString);
                val->read(string.c_str());
                m_exifMetadata.add(exifTag, val.get());        
                break;
            }
            case 0x0112: /*  Orientation (unsignedShort) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Image.Orientation");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());        
                break;
            }
            case 0x011A: /*  XResolution (unsignedRational) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Image.XResolution");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedRational);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedRational), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());        
                break;
            }
            case 0x011B: /*  YResolution (unsignedRational) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Image.YResolution");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedRational);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedRational), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());        
                break;
            }
            case 0x0128: /*  ResolutionUnit (unsignedShort) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Image.ResolutionUnit");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());        
                break;
            }
            case 0x0131: /*  Software (asciiString) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                std::string string(reinterpret_cast<char*>(data.pData_), data.size_);
                Exiv2::ExifKey exifTag("Exif.Image.Software");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::asciiString);
                val->read(string.c_str());
                m_exifMetadata.add(exifTag, val.get());        
                break;
            }
            case 0x0132: /*  DateTime (asciiString) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                std::string string(reinterpret_cast<char*>(data.pData_), data.size_);
                Exiv2::ExifKey exifTag("Exif.Image.DateTime");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::asciiString);
                val->read(string.c_str());
                m_exifMetadata.add(exifTag, val.get());        
                break;
            }
            case 0x8769: /*  ExifIFDPointer */
            {
                exif_start = get_32_tiff(pos+8); 
                break;
            }
            case 0xc4a5: /*  PrintIM */
            {
                printim_start = get_32_tiff(pos+8); 
                break;
            }
            default:
            {
                break;
            }
        }
    }
    catch( Exiv2::Error &e )
    {
        std::cerr << "Cannot parse MRW Exif data using Exiv2 (" 
                  << e.what() << ")\n";
    }                   
}

void MRWParser::dump_exif_tag(off_t pos) 
{
    try
    { 
        uint16_t id = get_16_tiff(pos+0); 
    
        switch(id) 
        {
            case 0x829a: /*  ExposureTime (unsignedRational) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Photo.ExposureTime");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedRational);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::Rational), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());        
                break;
            }
            case 0x829d: /*  FNumber (unsignedRational) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Photo.FNumber");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedRational);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::Rational), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());        
                break;
            }
            case 0x8822: /*  ExposureProgram (unsignedShort) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Photo.ExposureProgram");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());        
                break;
            }
            case 0x8827: /*  ISOSpeedRatings (unsignedShort) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Photo.ISOSpeedRatings");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());        
                break;
            }
            case 0x9000: /*  ExifVersion (undefined) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Photo.ExifVersion");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::undefined);
                val->read((const Exiv2::byte*)data.pData_, data.size_, Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());             
                break;
            }
            case 0x9003: /*   DateTimeOriginal (asciiString) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                std::string string(reinterpret_cast<char*>(data.pData_), data.size_);
                Exiv2::ExifKey exifTag("Exif.Photo.DateTimeOriginal");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::asciiString);
                val->read(string.c_str());
                m_exifMetadata.add(exifTag, val.get());        
                break;
            }
            case 0x9004: /*   DateTimeDigitized (asciiString) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                std::string string(reinterpret_cast<char*>(data.pData_), data.size_);
                Exiv2::ExifKey exifTag("Exif.Photo.DateTimeDigitized");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::asciiString);
                val->read(string.c_str());
                m_exifMetadata.add(exifTag, val.get());        
                break;
            }
            case 0x9101: /*  ComponentsConfiguration (undefined) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Photo.ComponentsConfiguration");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::undefined);
                val->read((const Exiv2::byte*)data.pData_, data.size_, Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());       
                break;
            }
            case 0x9203: /*   BrightnessValue (signedRational) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Photo.BrightnessValue");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::signedRational);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::Rational), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());        
                break;
            }
            case 0x9204: /*   ExposureBiasValue (signedRational) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Photo.ExposureBiasValue");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::signedRational);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::Rational), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());        
                break;
            }
            case 0x9205: /*   MaxApertureValue (unsignedRational) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Photo.MaxApertureValue");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedRational);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::Rational), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());                
                break;
            }
            case 0x9207: /*  MeteringMode (unsignedShort) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Photo.MeteringMode");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());  
                break;
            }
            case 0x9208: /*  LightSource (unsignedShort) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Photo.LightSource");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());  
                break;
            }
            case 0x9209: /*   Flash (unsignedShort) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Photo.Flash");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());  
                break;
            }
            case 0x920a: /*  FocalLength (unsignedRational) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Photo.FocalLength");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedRational);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::Rational), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());     
                break;
            }
            case 0x9214: /*   SubjectArea (unsignedShort) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Photo.SubjectArea");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());          
                break;
            }
            case 0x927c: /*   MakerNote */
            {
                maker_note = get_32_tiff(pos+8); 
                break;
            }
            case 0x9286: /*   UserComment (Comment) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Photo.UserComment");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::comment);
                val->read((const Exiv2::byte*)data.pData_, data.size_, Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());              
                break;
            }
            case 0xa000: /*   FlashpixVersion (undefined) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Photo.FlashpixVersion");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::undefined);
                val->read((const Exiv2::byte*)data.pData_, data.size_, Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());              
                break;
            }
            case 0xa001: /*   ColorSpace (unsignedShort) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Photo.ColorSpace");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());            
                break;
            }
            case 0xa002: /*   PixelXDimension (unsignedLong) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Photo.PixelXDimension");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedLong);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedLong), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());        
                break;
            }
            case 0xa003: /*   PixelYDimension (unsignedLong) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Photo.PixelYDimension");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedLong);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedLong), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());        
                break;
            }
            case 0xa005: /*   Interoperability ?? */
            {
                break;
            }
            case 0xa401: /*   CustomRendered (unsignedShort) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Photo.CustomRendered");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());  
                break;
            }
            case 0xa402: /*   ExposureMode (unsignedShort) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Photo.ExposureMode");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());          
                break;
            }
            case 0xa403: /*   WhiteBalance (unsignedShort) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Photo.WhiteBalance");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());   
                break;
            }
            case 0xa404: /*   DigitalZoomRatio (unsignedRational) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Photo.DigitalZoomRatio");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedRational);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedRational), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());         
                break;
            }
            case 0xa405: /*   FocalLengthIn35mmFilm (unsignedShort) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Photo.FocalLengthIn35mmFilm");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());         
                break;
            }
            case 0xa406: /*   SceneCaptureType (unsignedShort) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Photo.SceneCaptureType");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());         
                break;
            }
            case 0xa407: /*   GainControl (unsignedRational) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Photo.GainControl");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedRational);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedRational), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());             
                break;
            }
            case 0xa408: /*  Contrast (unsignedShort) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Photo.Contrast");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());            
                break;
            }
            case 0xa409: /*   Saturation (unsignedShort) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Photo.Saturation");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());             
                break;
            }
            case 0xa40a: /*   Sharpness (unsignedShort) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Photo.Sharpness");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());             
                break;
            }
            default:
            {
                break ;
            }
        }
    }
    catch( Exiv2::Error &e )
    {
        std::cerr << "Cannot parse MRW Exif data using Exiv2 (" 
                  << e.what() << ")\n";
    }                   
}

void MRWParser::dump_maker_note_tag(off_t pos) 
{
    try
    { 
        uint16_t id = get_16_tiff(pos+0); 
        
        switch(id) 
        {
            case 0x0000: /*   MakerNoteVersion (undefined) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Minolta.Version");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::undefined);
                val->read((const Exiv2::byte*)data.pData_, data.size_, Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());  
                break;
            }
            case 0x0001: /*   CameraSettingsStdOld (undefined) */
            {
                camera_settings_size_1 = get_32_tiff(pos+4); 
                camera_settings_pos_1  = get_32_tiff(pos+8); 
                break;
            }
            case 0x0003: /*   CameraSettingsStdNew (undefined) */
            {
                camera_settings_size_3 = get_32_tiff(pos+4); 
                camera_settings_pos_3  = get_32_tiff(pos+8); 
                break;
            }
            case 0x0004: /*   CameraSettings7D (undefined) */
            {
                camera_settings_size_4 = get_32_tiff(pos+4); 
                camera_settings_pos_4  = get_32_tiff(pos+8); 
                break;
            }
            case 0x0018: /*   ImageStabilizationData (undefined) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Minolta.ImageStabilizationData");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::undefined);
                val->read((const Exiv2::byte*)data.pData_, data.size_, Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());  
                break;
            }
            case 0x0040: /*   CompressedImageSize (unsignedLong) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Minolta.CompressedImageSize");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedLong);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedLong), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());
                break;
            }
            case 0x0081: /*   Thumbnail (undefined) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Minolta.Thumbnail");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::undefined);
                val->read((const Exiv2::byte*)data.pData_, data.size_, Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());
                break;
            }
            case 0x0088: /*   ThumbnailOffset (unsignedLong) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Minolta.ThumbnailOffset");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedLong);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedLong), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());
                break;
            }
            case 0x0089: /*   ThumbnailLength (unsignedLong) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Minolta.ThumbnailLength");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedLong);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedLong), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());
                break;
            }
            case 0x0101: /*   ColorMode (unsignedLong) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Minolta.ColorMode");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedLong);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedLong), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());
                break;
            }
            case 0x0102: /*   Quality (unsignedLong) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Minolta.Quality");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedLong);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedLong), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());
                break;
            }
            case 0x0103: /*   MinoltaImageSize/MinoltaQuality */
            {
                // TODO
                break;
            }
            case 0x0107: /*  ImageStabilization (unsignedLong) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Minolta.ImageStabilization");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedLong);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedLong), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());
                break;
            }
            case 0x010a: /*   ZoneMatching (unsignedLong) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Minolta.ZoneMatching");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedLong);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedLong), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());
                break;
            }
            case 0x010b: /*   ColorTemperature (unsignedLong) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Minolta.ColorTemperature");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedLong);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedLong), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());
                break;
            }
            case 0x010c: /*   LensID (unsignedLong) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Minolta.LensID");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedLong);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedLong), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get());
                break;
            }
            case 0x0114: /*   CameraSettings5D (undefined) */
            {
                camera_settings_size_0114 = get_32_tiff(pos+4); 
                camera_settings_pos_0114  = get_32_tiff(pos+8); 
                break;
            }
            case 0x0e00: /*   PIM_IFD (undefined) */
            {
                Exiv2::DataBuf data = get_ttf_tag_value(pos);
                Exiv2::ExifKey exifTag("Exif.Minolta.PIM_IFD");
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::undefined);
                val->read((const Exiv2::byte*)data.pData_, data.size_, Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get()); 
                break;
            }
            case 0x0f00: /*   CameraSettingsZ1 (undefined) */
            {
                break;
            }
            default:
            {
                break;  
            }
        }
    }
    catch( Exiv2::Error &e )
    {
        std::cerr << "Cannot parse MRW makernote data using Exiv2 (" 
                  << e.what() << ")\n";
    }                   
}

/* The CameraSettings5D tag is used by the Dynax/Maxxum 5D */ 

void MRWParser::dumpCameraSettings5D( off_t pos , uint32_t size  )
{
    uint32_t i; 
    uint32_t nb = size/2;
    
    for (i=0 ; i < nb ; i++) 
    {
        try
        { 
            Exiv2::DataBuf data(2);
            uint16_t v = get_16_tiff(pos+i*2);
            memcpy(data.pData_, &v, 2);
          
            switch(i)
            {
                case 0x000A: /*  ExposureMode (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs5D.ExposureMode");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break; 
                }
                case 0x000C: /*  ImageSize (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs5D.ImageSize");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break; 
                }
                case 0x000D: /*  Quality (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs5D.Quality");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break; 
                }
                case 0x000E: /*  WhiteBalance (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs5D.WhiteBalance");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break;
                }
                case 0x001F: /*  Flash (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs5D.Flash");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break;
                }      
                case 0x0025: /*  MeteringMode (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs5D.MeteringMode");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break;
                }      
                case 0x0026: /*  ISOSpeed (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs5D.ISOSpeed");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break;
                }   
                case 0x0030: /*  Sharpness (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs5D.Sharpness");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break;
                } 
                case 0x0031: /*  Contrast (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs5D.Contrast");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break;
                } 
                case 0x0032: /*  Saturation (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs5D.Saturation");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break;
                } 
                case 0x0035: /*  ExposureTime (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs5D.ExposureTime");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break;
                } 
                case 0x0036: /*  FNumber (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs5D.FNumber");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break;
                } 
                case 0x0037: /*  FreeMemoryCardImages (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs5D.FreeMemoryCardImages");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break;
                } 
                case 0x0038: /*  ExposureRevision (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs5D.ExposureRevision");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break;
                } 
                case 0x0049: /*  ColorTemperature (signedShort) !!! WARNING: signed value here !!! */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs5D.ColorTemperature");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::signedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::signedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break;
                } 
                case 0x0050: /*  Rotation  (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs5D.Rotation");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break;
                } 
                case 0x0053: /*  ExposureCompensation  (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs5D.ExposureCompensation");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break;
                }                 
                case 0x0054: /*  FreeMemoryCardImages (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs5D.FreeMemoryCardImages");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break;
                }      
                case 0x00AE: /*  ImageNumber (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs5D.ImageNumber");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break;
                }   
                case 0x00B0: /*  NoiseReduction (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs5D.NoiseReduction");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break;
                }   
                case 0x00BD: /*  ImageStabilization (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs5D.ImageStabilization");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break;
                }   
                default:
                    break;
            }
        }
        catch( Exiv2::Error &e )
        {
            std::cerr << "Cannot parse MRW makernote data using Exiv2 (" 
                      << e.what() << ")\n";
        }           
    }
}

/* The CameraSettings7D tag is used by the Dynax/Maxxum 7D  */ 

void MRWParser::dumpCameraSettings7D( off_t pos , uint32_t size  )
{
    uint32_t i; 
    uint32_t nb = size/2;
  
    for (i=0 ; i < nb ; i++) 
    {
        try
        { 
            Exiv2::DataBuf data(2);
            uint16_t v = get_16_tiff(pos+i*2);
            memcpy(data.pData_, &v, 2);
      
            switch(i)
            {
                case 0x0000: /*  ExposureMode (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs7D.ExposureMode");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break; 
                }
                case 0x0002: /*  ImageSize (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs7D.ImageSize");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break; 
                }
                case 0x0003: /*  Quality (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs7D.Quality");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break; 
                }
                case 0x0004: /*  WhiteBalance (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs7D.WhiteBalance");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break; 
                }                
                case 0x000E: /*  FocusMode (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs7D.FocusMode");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break; 
                }                  
                case 0x0010: /*  AFPoints (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs7D.AFPoints");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break; 
                }                    
                case 0x0015: /*  Flash (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs7D.Flash");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break; 
                }                 
                case 0x0016: /*  FlashMode (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs7D.FlashMode");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break; 
                }                 
                case 0x001C: /*  ISOSpeed (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs7D.ISOSpeed");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break; 
                }     
                case 0x001E: /*  ExposureCompensation (signedShort) !!! WARNING: signed value here !!!*/
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs7D.ExposureCompensation");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::signedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::signedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break; 
                }                                      
                case 0x0025: /*  ColorSpace (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs7D.ColorSpace");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break; 
                }             
                case 0x0026: /*  Sharpness (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs7D.Sharpness");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break; 
                }                          
                case 0x0027: /*  Contrast (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs7D.Contrast");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break; 
                }  
                case 0x0028: /*  Saturation (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs7D.Saturation");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break; 
                }  
                case 0x002D: /*  FreeMemoryCardImages (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs7D.FreeMemoryCardImages");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break; 
                }
                case 0x003F: /*  ColorTemperature (signedShort) !!! WARNING: signed value here !!! */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs7D.ColorTemperature");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::signedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::signedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break; 
                }                              
                case 0x0040: /*  Hue (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs7D.Hue");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break; 
                }                
                case 0x0046: /*  Rotation (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs7D.Rotation");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break; 
                }                  
                case 0x0047: /*  FNumber (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs7D.FNumber");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break; 
                }                  
                case 0x0048: /*  ExposureTime (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs7D.ExposureTime");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break; 
                }                   
                case 0x004A: /*  FreeMemoryCardImages (unsignedShort) : dupplicate that 0x002D tags*/
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs7D.FreeMemoryCardImages");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break; 
                }                
                case 0x005E: /*  ImageNumber (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs7D.ImageNumber");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break; 
                }                  
                case 0x0060: /*  NoiseReduction (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs7D.NoiseReduction");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break; 
                }                  
                case 0x0062: /*  ImageNumber (unsignedShort) : dupplicate that 0x005E tags*/
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs7D.ImageNumber");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break; 
                }   
                case 0x0071: /*  ImageStabilization (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs7D.ImageStabilization");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break; 
                }                 
                case 0x0075: /*  ZoneMatchingOn (unsignedShort) */
                {
                    Exiv2::ExifKey exifTag("Exif.MinoltaCs7D.ZoneMatchingOn");
                    Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedShort);
                    val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedShort), Exiv2::littleEndian);
                    m_exifMetadata.add(exifTag, val.get()); 
                    break; 
                }                       
                default:
                    break;
            }
        }
        catch( Exiv2::Error &e )
        {
            std::cerr << "Cannot parse MRW makernote data using Exiv2 (" 
                      << e.what() << ")\n";
        }           
    }
}

/* The CameraSettingsStd tags are used by the D7u, D7i, D7hi, D5, D7, S304, and S404 */ 

void MRWParser::dumpCameraSettingsStd( off_t pos , uint32_t size, const std::string& csSection )
{
    uint32_t i; 
    uint32_t nb = size/4;
    
    for (i=0 ; i < nb ; i++) 
    {
        try
        { 
            Exiv2::DataBuf data(4);
            uint32_t v = get_32_tiff(pos+i*4);
            memcpy(data.pData_, &v, 4);
            std::string key;
            
            switch(i)
            {
                case 0x0001: /*  ExposureMode (unsignedLong) */
                {
                    key = csSection + std::string("ExposureMode");
                    break; 
                }
                case 0x0002: /*  FlashMode (unsignedLong) */
                {
                    key = csSection + std::string("FlashMode");
                    break; 
                }
                case 0x0003: /*  WhiteBalance (unsignedLong) */
                {
                    key = csSection + std::string("WhiteBalance");
                    break; 
                }
                case 0x0004: /*  ImageSize (unsignedLong) */
                {
                    key = csSection + std::string("ImageSize");
                    break; 
                }
                case 0x0005: /*  Quality (unsignedLong) */
                {
                    std::string key = csSection + std::string("Quality");
                    break; 
                }
                case 0x0006: /*  DriveMode (unsignedLong) */
                {
                    key = csSection + std::string("DriveMode");
                    break; 
                }
                case 0x0007: /*  MeteringMode (unsignedLong) */
                {
                    key = csSection + std::string("MeteringMode");
                    break; 
                }                                
                case 0x0008: /*  ExposureSpeed (unsignedLong) */
                {
                    key = csSection + std::string("ExposureSpeed");
                    break; 
                }   
                case 0x0009: /*  ExposureTime (unsignedLong) */
                {
                    key = csSection + std::string("ExposureTime");
                    break; 
                }                 
                case 0x000A: /*  FNumber (unsignedLong) */
                {
                    key = csSection + std::string("FNumber");
                    break; 
                }
                case 0x000B: /*  MacroMode (unsignedLong) */
                {
                    key = csSection + std::string("MacroMode");
                    break; 
                }                
                case 0x000C: /*  DigitalZoom (unsignedLong) */
                {
                    key = csSection + std::string("DigitalZoom");
                    break; 
                }                     
                case 0x000D: /*  ExposureCompensation (unsignedLong) */
                {
                    key = csSection + std::string("ExposureCompensation");
                    break; 
                }                        
                case 0x000E: /*  BracketStep (unsignedLong) */
                {
                    key = csSection + std::string("BracketStep");
                    break; 
                }                   
                case 0x0010: /*  IntervalLength (unsignedLong) */
                {
                    key = csSection + std::string("IntervalLength");
                    break; 
                } 
                case 0x0011: /*  IntervalNumber (unsignedLong) */
                {
                    key = csSection + std::string("IntervalNumber");
                    break; 
                } 
                case 0x0012: /*  FocalLength (unsignedLong) */
                {
                    key = csSection + std::string("FocalLength");
                    break; 
                }                
                case 0x0013: /*  FocusDistance (unsignedLong) */
                {
                    key = csSection + std::string("FocusDistance");
                    break; 
                }                 
                case 0x0014: /*  Flash (unsignedLong) */
                {
                    key = csSection + std::string("Flash");
                    break; 
                }                    
                case 0x0015: /*  MinoltaDate (unsignedLong) */
                {
                    key = csSection + std::string("MinoltaDate");
                    break; 
                }                    
                case 0x0016: /*  MinoltaTime (unsignedLong) */
                {
                    key = csSection + std::string("MinoltaTime");
                    break; 
                }                  
                case 0x0017: /*  MaxAperture (unsignedLong) */
                {
                    key = csSection + std::string("MaxAperture");
                    break; 
                }                 
                case 0x001A: /*  FileNumberMemory (unsignedLong) */
                {
                    key = csSection + std::string("FileNumberMemory");
                    break; 
                }                       
                case 0x001B: /*  ImageNumber (unsignedLong) */
                {
                    key = csSection + std::string("ImageNumber");
                    break; 
                }                       
                case 0x001C: /*  ColorBalanceRed (unsignedLong) */
                {
                    key = csSection + std::string("ColorBalanceRed");
                    break; 
                }                  
                case 0x001D: /*  ColorBalanceGreen (unsignedLong) */
                {
                    key = csSection + std::string("ColorBalanceGreen");
                    break; 
                }                 
                case 0x001E: /*  ColorBalanceBlue (unsignedLong) */
                {
                    key = csSection + std::string("ColorBalanceBlue");
                    break; 
                }                  
                case 0x001F: /*  Saturation (unsignedLong) */
                {
                    key = csSection + std::string("Saturation");
                    break; 
                }                  
                case 0x0020: /*  Contrast (unsignedLong) */
                {
                    key = csSection + std::string("Contrast");
                    break; 
                }                   
                case 0x0021: /*  Sharpness (unsignedLong) */
                {
                    key = csSection + std::string("Sharpness");
                    break; 
                }
                case 0x0022: /*  SubjectProgram (unsignedLong) */
                {
                    key = csSection + std::string("SubjectProgram");
                    break; 
                }                
                case 0x0023: /*  FlashExposureComp (unsignedLong) */
                {
                    key = csSection + std::string("FlashExposureComp");
                    break; 
                }                 
                case 0x0024: /*  ISOSpeed (unsignedLong) */
                {
                    key = csSection + std::string("ISOSpeed");
                    break; 
                }                 
                case 0x0025: /*  MinoltaModel (unsignedLong) */
                {
                    key = csSection + std::string("MinoltaModel");
                    break; 
                }                  
                case 0x0026: /*  IntervalMode (unsignedLong) */
                {
                    key = csSection + std::string("IntervalMode");
                    break; 
                }                    
                case 0x0027: /*  FolderName (unsignedLong) */
                {
                    key = csSection + std::string("FolderName");
                    break; 
                }                  
                case 0x0028: /*  ColorMode (unsignedLong) */
                {
                    key = csSection + std::string("ColorMode");
                    break; 
                }                  
                case 0x0029: /*  ColorFilter (unsignedLong) */
                {
                    key = csSection + std::string("ColorFilter");
                    break; 
                }                
                case 0x002A: /*  BWFilter (unsignedLong) */
                {
                    key = csSection + std::string("BWFilter");
                    break; 
                }                    
                case 0x002B: /*  InternalFlash (unsignedLong) */
                {
                    key = csSection + std::string("InternalFlash");
                    break; 
                }                    
                case 0x002C: /*  Brightness (unsignedLong) */
                {
                    key = csSection + std::string("Brightness");
                    break; 
                }                    
                case 0x002D: /*  SpotFocusPointX (unsignedLong) */
                {
                    key = csSection + std::string("SpotFocusPointX");
                    break; 
                }                    
                case 0x002E: /*  SpotFocusPointY (unsignedLong) */
                {
                    key = csSection + std::string("SpotFocusPointY");
                    break; 
                }                    
                case 0x002F: /*  WideFocusZone (unsignedLong) */
                {
                    key = csSection + std::string("WideFocusZone");
                    break; 
                }                    
                case 0x0030: /*  FocusMode (unsignedLong) */
                {
                    key = csSection + std::string("FocusMode");
                    break; 
                }                    
                case 0x0031: /*  FocusArea (unsignedLong) */
                {
                    key = csSection + std::string("FocusArea");
                    break; 
                }                    
                case 0x0032: /*  DECPosition (unsignedLong) */
                {
                    key = csSection + std::string("DECPosition");
                    break; 
                }                    
                case 0x0033: /*  ColorProfile (unsignedLong) */
                {
                    key = csSection + std::string("ColorProfile");
                    break; 
                }                 
                case 0x0034: /*  DataImprint (unsignedLong) */
                {
                    key = csSection + std::string("DataImprint");
                    break; 
                }      
                case 0x003F: /*  FlashMetering (unsignedLong) */
                {
                    key = csSection + std::string("FlashMetering");
                    break; 
                }                 
                default:
                    break;
            }
        
            if (!key.empty())
            {
                Exiv2::ExifKey exifTag(key);
                Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::unsignedLong);
                val->read((const Exiv2::byte*)data.pData_, sizeof(Exiv2::unsignedLong), Exiv2::littleEndian);
                m_exifMetadata.add(exifTag, val.get()); 
            }
        
        }
        catch( Exiv2::Error &e )
        {
            std::cerr << "Cannot parse MRW makernote data using Exiv2 (" 
                      << e.what() << ")\n";
        }           
    }
}

void MRWParser::dump_camera_settings_32bit( off_t /*pos*/ , uint32_t /*size*/ ,int /*mode*/ )
{
/*
    uint32_t i; 
    uint32_t nb = size/4;
  
    printf("<CameraSetting%04x> with %d entries at TFF offset %d\n", mode, (int)nb , (int)pos) ;
  
    for (i=0 ; i < nb ; i++) 
    {
        uint32_t v = get_32_tiff(pos+i*4);
        printf("=== 0x%04lx = %08lx\n" ,(unsigned long) i , (unsigned long)  v);
    }
*/    
}

}  // NameSpace Digikam
