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

#ifndef DIGIKAM_OPENFACE_PREPROCESSOR_H
#define DIGIKAM_OPENFACE_PREPROCESSOR_H

// C++ includes

#include <array>

// OpenCV includes

#include <opencv2/core.hpp>

// Local includes

#include "shapepredictor.h"

namespace Digikam
{

class OpenfacePreprocessor
{

public:

	OpenfacePreprocessor();
	~OpenfacePreprocessor();

	void init();

	cv::Mat process(const cv::Mat& image);

private:

	cv::Size				outImageSize;

	cv::Mat 				faceTemplate;
	std::array<int, 3> 		outerEyesNosePositions;

	redeye::ShapePredictor 	sp;

};

}; // namespace Digikam

#endif // DIGIKAM_OPENFACE_PREPROCESSOR_H