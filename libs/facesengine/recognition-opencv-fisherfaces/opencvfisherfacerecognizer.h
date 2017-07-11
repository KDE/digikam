/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date    2017-06-10
 * @brief   <a href="http://docs.opencv.org/2.4/modules/contrib/doc/facerec/facerec_tutorial.html#Fisherfaces">Face Recognition based on Fisherfaces</a>
 *          Turk, Matthew A and Pentland, Alex P. "Face recognition using Fisherfaces." 
 *          Computer Vision and Pattern Recognition, 1991. Proceedings {CVPR'91.},
 *          {IEEE} Computer Society Conference on 1991.
 *
 * @section DESCRIPTION
 *
 * @author Copyright (C) 2017 by Yingjie Liu
 *         <a href="mailto:yingjiewudi at gmail dot come">yingjiewudi at gmail dot come</a>
 *
 * @section LICENSE
 *
 * Released to public domain under terms of the BSD Simplified license.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the organization nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 *   See <http://www.opensource.org/licenses/bsd-license>
 *
 * ============================================================ */

#ifndef OPENCV_FISHERFACES_RECOGNIZER_H
#define OPENCV_FISHERFACES_RECOGNIZER_H

// OpenCV library

#include "libopencv.h"

// Qt include

#include <QImage>

// Local includes

//#include "opencvmatdata.h"

namespace Digikam
{

class OpenCVFISHERFaceRecognizer
{

public:

    /**
     *  @brief FaceRecognizer:Master class to control entire recognition using Fisherfaces algorithm
     */
    OpenCVFISHERFaceRecognizer();
    ~OpenCVFISHERFaceRecognizer();

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
    void train(const std::vector<cv::Mat>& images, const std::vector<int>& labels, const QString& context);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // OPENCV_FISHERFACES_RECOGNIZER_H
