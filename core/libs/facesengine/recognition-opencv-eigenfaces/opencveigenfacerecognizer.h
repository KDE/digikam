/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2017-05-22
 * Description : Face Recognition based on Eigenfaces
 *               http://docs.opencv.org/2.4/modules/contrib/doc/facerec/facerec_tutorial.html#eigenfaces
 *               Turk, Matthew A and Pentland, Alex P. "Face recognition using eigenfaces." 
 *               Computer Vision and Pattern Recognition, 1991. Proceedings {CVPR'91.},
 *               {IEEE} Computer Society Conference on 1991.
 *
 * Copyright (C) 2017      by Yingjie Liu <yingjiewudi at gmail dot com>
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef OPENCV_EIGENFACES_RECOGNIZER_H
#define OPENCV_EIGENFACES_RECOGNIZER_H

// OpenCV library

#include "libopencv.h"

// Qt include

#include <QImage>

// Local includes

namespace Digikam
{

class OpenCVEIGENFaceRecognizer
{

public:

    /**
     *  @brief FaceRecognizer:Master class to control entire recognition using Eigenfaces algorithm
     */
    explicit OpenCVEIGENFaceRecognizer();
    ~OpenCVEIGENFaceRecognizer();

    void setThreshold(float threshold) const;

    /**
     *  Returns a cvMat created from the inputImage, optimized for recognition
     */
    cv::Mat prepareForRecognition(const QImage& inputImage);

    /**
     *  Try to recognize the given image.
     *  Returns the identity id.
     *  If the identity cannot be recognized, returns -1.
     */
    int recognize(const cv::Mat& inputImage);

    /**
     *  Trains the given images, representing faces of the given matched identities.
     */
    void train(const std::vector<cv::Mat>& images, const std::vector<int>& labels, const QString& context, const std::vector<cv::Mat>& images_rgb);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // OPENCV_EIGENFACES_RECOGNIZER_H
