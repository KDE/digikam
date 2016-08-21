/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date    2010-01-03
 * @brief   Class to perform faces detection.
 * @section DESCRIPTION
 *
 * @author Copyright (C) 2010 by Alex Jironkin
 *         <a href="alexjironkin at gmail dot com">alexjironkin at gmail dot com</a>
 * @author Copyright (C) 2010 by Aditya Bhatt
 *         <a href="adityabhatt at gmail dot com">adityabhatt at gmail dot com</a>
 * @author Copyright (C) 2010-2016 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 * @author Copyright (C) 2010-2013 by Marcel Wiesweg
 *         <a href="mailto:marcel dot wiesweg at gmx dot de">marcel dot wiesweg at gmx dot de</a>
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef FACESENGINE_OPENCVFACEDETECTOR_H
#define FACESENGINE_OPENCVFACEDETECTOR_H

// OpenCV library

#include "libopencv.h"

// Qt includes

#include <QImage>
#include <QList>
#include <QRect>
#include <QStringList>

// DigiKam includes

#include "dimg.h"

namespace FacesEngine
{

class Cascade;
class DetectObjectParameters;

class OpenCVFaceDetector
{
public:

    OpenCVFaceDetector(const QStringList& cascadeDirs);
    ~OpenCVFaceDetector();

    cv::Mat prepareForDetection(const QImage& inputImage) const;
    cv::Mat prepareForDetection(const Digikam::DImg& inputImage) const;
    QList<QRect> detectFaces(const cv::Mat& inputImage, const cv::Size& originalSize = cv::Size(0, 0));

    /**
     * Tunes the parameters.
     * There are two orthogonal dimensions to adjust:
     * - computation speed vs. accuracy (sensitivity and specificity)
     * - sensitivity vs. specificity
     * The value is in the interval [0;1], where 0 means
     * fastest operation and highest sensitivity while 1
     * means best accuracy (slow operation) and high specificity.
     */
    void setAccuracy(double speedVsAccuracy);
    void setSpecificity(double sensitivityVsSpecificity);

    double accuracy()    const;
    double specificity() const;

    /**
     * Returns the image size (one dimension)
     * recommended for face detection. If the image is considerably larger, it will be rescaled automatically.
     */
    static int recommendedImageSizeForDetection();

private:

    /**
     *  Detect faces in an image using a single cascade. Uses CANNY_PRUNING at present.
     *
     *  @param inputImage A pointer to the IplImage representing image of interest.
     *  @param casc The CvClassClassifierCascade pointer to be used for the detection
     *  @param params The parameters to be used for detection
     *  @return Returns a vector of Face objects. Each object hold information about 1 face.
     */
    QList<QRect> cascadeResult(const cv::Mat& inputImage, Cascade& cascade, const DetectObjectParameters& params) const;

    bool verifyFace(const cv::Mat& inputImage, const QRect& face) const;

    /**
     * Returns the faces from the detection results of multiple cascades
     *
     * @param combo A vector of a vector of faces, each component vector is the detection result of a single cascade
     * @param maxdist The maximum allowable distance between two duplicates, if two faces are further apart than this, they are not duplicates
     * @param mindups The minimum number of duplicate detections required for a face to qualify as genuine
     * @return The vector of the final faces
     */
    QList<QRect> mergeFaces(const cv::Mat& inputImage, const QList< QList<QRect> >& preliminaryResults) const;

    void updateParameters(const cv::Size& scaledSize, const cv::Size& originalSize);

private:

    class Private;
    Private* const d;
};

} // namespace FacesEngine

#endif /* FACESENGINE_OPENCVFACEDETECTOR_H */
