/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-02-23
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju
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

// C Ansi includes.

extern "C" 
{
#include <string.h>
}
 
// C++ includes.
 
#include <fstream>
  
// Qt includes.

#include <qstring.h>
#include <qfile.h>

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "exifrestorer.h"

using namespace std;

ExifRestorer::ExifRestorer()
{
    jpegSections_.setAutoDelete(true);
    imageData_ = 0;
    exifData_ = 0;
    hasExif_ = false;
}

ExifRestorer::~ExifRestorer()
{
    clear();
}

void ExifRestorer::clear()
{
    jpegSections_.clear();
    if (exifData_)
        delete exifData_;
    exifData_ = 0;
    if (imageData_)
        delete imageData_;
    imageData_ = 0;
    hasExif_ = false;
}


int ExifRestorer::readFile(const QString& filename, ReadMode mode)
{
    clear();

    std::ifstream filestream;

    filestream.open(QFile::encodeName(filename), ios::binary | ios::in);
    if (!filestream.good()) {
        kdWarning() << "Failed to Open file" << endl;
        return -1;
    }

    unsigned char header[2];
    filestream.read((char*)header, 2);
    if (filestream.gcount() != 2) {
        kdWarning() << "Failed to read header" << endl;
        return -1;
    }

    unsigned char marker;

    if ((header[0] == 0xFF) && (header[1] == M_SOI)) {

        // Smells like a jpeg file

        bool found_SOSorEOI = false;

        while (!filestream.eof()) {

            // Padding bytes between sections
            for (int i=0; i<7; i++){
                filestream.read((char*)&marker, 1);
                if (marker != 0xFF) break;
                if (i >= 6){
                    // 0xff is legal padding, but if we get
                    // that many times, something's wrong.
                    kdWarning() << "Too many padding bytes" << endl;
                    return -1;
                }
            }

            JpegSection *section = new JpegSection;

            // read data size
            unsigned char lh, ll;
            filestream.read((char*)&lh, 1);
            filestream.read((char*)&ll, 1);
            section->size = (lh << 8) | ll;
            if (section->size < 2) {
                kdWarning() << "Invalid Marker found" << endl;
                return -1;
            }

            section->data = new unsigned char[section->size];
            section->data[0] =  lh;
            section->data[1] =  ll;
            filestream.read((char*)&(section->data[2]), section->size-2);
            if ((section->size-2) != (unsigned int)filestream.gcount()) {
                kdWarning() << "Premature File End" << endl;
                return -1;
            }

            section->type = marker;
            jpegSections_.append(section);

            //cout << (dec) << section->size << " ";
            //cout << (hex) << int(section->type) << endl;

            switch (marker) {

            case M_SOS: {

                int currPos, endPos;
                found_SOSorEOI=true;

                if (mode == EntireImage) {

                    currPos = filestream.tellg();
                    filestream.seekg(0,ios::end);
                    endPos = filestream.tellg();
                    filestream.seekg(currPos);

                    imageData_ = new JpegSection;
                    imageData_->size = endPos - currPos;
                    imageData_->data =
                        new unsigned char[imageData_->size];

                    filestream.read((char*)imageData_->data,
                                    imageData_->size);
                    imageData_->type = marker;
                }

                break;
            }

            case M_EOI: {

                found_SOSorEOI=true;
                break;
            }

            case M_COM: {

                break;
            }

            case M_JFIF: {

                break;
            }

            case M_EXIF: {
                if (section->data[2] == 'E' &&
                    section->data[3] == 'x'  &&
                    section->data[4] == 'i' &&
                    section->data[5] == 'f') {
                    hasExif_ = true;
                    exifData_ = new JpegSection;
                    exifData_->size = section->size;
                    exifData_->type = section->type;
                    exifData_->data =
                        new unsigned char[section->size];
                    memcpy(exifData_->data, section->data,
                           section->size);
                }
                break;
            }

            default: {
                break;
            }

            }

            if (found_SOSorEOI) {
                if (mode == ExifOnly)
                    jpegSections_.clear();
                return 0;
            }

        }

        kdWarning() << "End of file without SOS or EOI" << endl;
        return -1;

    }

    kdWarning() << "Not a jpeg file" << endl;
    return -1;

}

int ExifRestorer::writeFile(const QString& filename)
{
    std::ofstream outStream (QFile::encodeName(filename), ios::binary | ios::out);

    if (!outStream) {
        kdWarning() << "Error in opening output file" << endl;
        return -1;
    }

    outStream.put(0xff);
    outStream.put(0xd8);

    for (unsigned int i=0; i<jpegSections_.count(); i++) {
        outStream.put(0xff);
        outStream.put(jpegSections_.at(i)->type);
        if (!outStream.write((char*)jpegSections_.at(i)->data,
                             jpegSections_.at(i)->size))
            return -1;
    }

    if (!outStream.write((char*)imageData_->data, imageData_->size)) {
        kdWarning() << "Error in writing to file" << endl;
        return -1;
    }

    outStream.close();

    return 0;
}

void ExifRestorer::insertExifData(JpegSection *exifSection)
{
    QPtrList<JpegSection> newSections;
    newSections.setAutoDelete(false);

    // Check if the first section of jpegSections is a JFIF
    // If so, add this to newSections
    if (jpegSections_.at(0)->type == M_JFIF) {
        newSections.append(jpegSections_.at(0));
    }


    // Now add the new exif section
    JpegSection* newExifSection = new JpegSection;
    newExifSection->type = exifSection->type;
    newExifSection->size = exifSection->size;
    newExifSection->data = new unsigned char[exifSection->size];
    memcpy(newExifSection->data, exifSection->data,
           exifSection->size);

    newSections.append(newExifSection);

    // Check if image already has exif section. if so replace it
    for (JpegSection* section = jpegSections_.first(); section;
         section = jpegSections_.next()) {
        if (section->type == M_EXIF) {
            jpegSections_.remove(section);
        }
    }


    // Add the rest of the sections;
    for (unsigned int i=1; i<jpegSections_.count(); i++) {
        newSections.append(jpegSections_.at(i));
    }

    jpegSections_.setAutoDelete(false);
    jpegSections_.clear();

    for (unsigned int i=0; i<newSections.count(); i++) {
        jpegSections_.append(newSections.at(i));
    }

    jpegSections_.setAutoDelete(true);
}
