//    Copyright (C) 2010 Dirk Vanden Boer <dirk.vdb@gmail.com>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "imagewriter.h"
#include <iostream>


using namespace std;

namespace Digikam
{

ImageWriter::ImageWriter()
{
}

void ImageWriter::writeFrame(VideoFrame& frame,  QImage& image)
{
    QImage previewImage(frame.width, frame.height, QImage::Format_RGB888);
    for (quint32 y = 0; y < frame.height; y++) {
        // Copy each line ..
        memcpy(previewImage.scanLine(y), &frame.frameData[y*frame.lineSize], frame.width*3);
    }

    image = previewImage;
}
}
