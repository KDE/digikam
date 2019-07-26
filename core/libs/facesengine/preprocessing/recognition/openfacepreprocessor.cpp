/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-07-09
 * Description : Preprocessor for openface nn model
 *
 * Copyright (C) 2019 by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
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

#include "openfacepreprocessor.h"

// Qt includes

#include <QString>
#include <QFile>
#include <QDataStream>
#include <QStandardPaths>
#include <QTime>

// Local includes

#include "fullobjectdetection.h"
#include "digikam_debug.h"


#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

using namespace Digikam;

// --------------------------------------- Static global variables -----------------------------------

/** Template for face landmark to perform alignment with open face
  * This variable must be declared as static so that it is allocated as long as
  * dk is still running. We need that because this variable is the internal data 
  * for matrix faceTemplate below.
  */ 
static float FACE_TEMPLATE[3][2] =  { 
                                        {18.639072, 16.249624}, 
                                        {75.73048, 15.18443}, 
                                        {47.515285, 49.38637} 
                                    };

// ---------------------------------------------------------------------------------------------------

OpenfacePreprocessor::OpenfacePreprocessor()
  : outImageSize(cv::Size(96, 96)),
    faceTemplate(cv::Mat(3, 2, CV_32F, &FACE_TEMPLATE)),
    outerEyesNosePositions({36,45,33})
{
}

OpenfacePreprocessor::~OpenfacePreprocessor()
{
}

void OpenfacePreprocessor::init()
{
	// Load shapepredictor model for face alignment with 68 points of face landmark extraction

    QString spdata = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                           	QLatin1String("digikam/facesengine/shapepredictor.dat"));
    QFile model(spdata);
    redeye::ShapePredictor* const temp = new redeye::ShapePredictor();

    qCDebug(DIGIKAM_FACEDB_LOG) << "Start reading shape predictor file";

    if (model.open(QIODevice::ReadOnly))
    {
        QDataStream dataStream(&model);
        dataStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
        dataStream >> *temp;
        sp = *temp;
        model.close();
    }
    else
    {
        qCDebug(DIGIKAM_FACEDB_LOG) << "Error open file shapepredictor.dat\n";
        return;
    }

    delete temp;

    qCDebug(DIGIKAM_FACEDB_LOG) << "Finish reading shape predictor file";
}

cv::Mat OpenfacePreprocessor::process(const cv::Mat& image)
{
	int type = image.type();
    qCDebug(DIGIKAM_FACEDB_LOG) << "type: " << type;

    cv::Mat gray;

    if (type == CV_8UC3 || type == CV_16UC3)
    {
        cv::cvtColor(image, gray, CV_RGB2GRAY);   // 3 channels
    }
    else
    {
        cv::cvtColor(image, gray, CV_RGBA2GRAY);  // 4 channels
    }

    if (type == CV_16UC3 || type == CV_16UC4)
    {
        gray.convertTo(gray, CV_8UC1, 1 / 255.0);
    }

    cv::Rect new_rect(0, 0, image.cols, image.rows);
    cv::Mat landmarks(3,2,CV_32F);
    FullObjectDetection object = sp(gray, new_rect);
    for(int i = 0; i < outerEyesNosePositions.size(); i++)
    {
    	int index = outerEyesNosePositions[i];
        landmarks.at<float>(i,0) = object.part(index)[0];
        landmarks.at<float>(i,1) = object.part(index)[1];
        // qDebug() << "index = " << index << ", landmarks: (" << landmarks.at<float>(i,0) << ", " << landmarks.at<float>(i,1) << ")\n";
    }
    qCDebug(DIGIKAM_FACEDB_LOG) << "Full object detection and landmard computation finished";
    // qCDebug(DIGIKAM_FACEDB_LOG) << "Finish computing landmark in " << timer.restart() << " ms";

    cv::Mat affineTransformMatrix = cv::getAffineTransform(landmarks, faceTemplate);
    cv::Mat alignedFace;
    cv::warpAffine(image, alignedFace, affineTransformMatrix, outImageSize);

    if(alignedFace.empty())
    {
    	qCDebug(DIGIKAM_FACEDB_LOG) << "Face alignment failed!";
    	return image; 
    }
    else
    {
        qCDebug(DIGIKAM_FACEDB_LOG) << "Align face finished";
    }

    return alignedFace;
}