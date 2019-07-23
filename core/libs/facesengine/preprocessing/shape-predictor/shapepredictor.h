/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 16/08/2016
 * Description : A Shape predictor class that can predicts 68
 *               facial point including points surrounding faces
 *               eyes, that can be used for detecting human eyes
 *               positions, almost all codes are ported from dlib
 *               library (http://dlib.net/)
 *
 * Copyright (C) 2016 by Omar Amin <Omar dot moh dot amin at gmail dot com>
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

#ifndef DIGIKAM_SHAPE_PREDICTOR_H
#define DIGIKAM_SHAPE_PREDICTOR_H

// C++ includes

#include <vector>

// Local includes

#include "digikam_opencv.h"
#include "pointtransformaffine.h"
#include "vectoroperations.h"
#include "matrixoperations.h"
#include "fullobjectdetection.h"
#include "qdatastreamoverloads.h"

namespace Digikam
{

namespace redeye
{

struct SplitFeature
{
    unsigned long idx1;
    unsigned long idx2;
    float         thresh;
};

QDataStream& operator << (QDataStream& dataStream, const SplitFeature& sp);
QDataStream& operator >> (QDataStream& dataStream, SplitFeature& sp);

// a tree is just a std::vector<redeye::SplitFeature>.  We use this function to navigate the tree nodes.

/*!
    ensures
        - returns the index of the left child of the binary tree node idx
!*/
unsigned long left_child(unsigned long idx);

/*!
    ensures
        - returns the index of the left child of the binary tree node idx
!*/
unsigned long right_child(unsigned long idx);

// ----------------------------------------------------------------------------------------

struct RegressionTree
{
    std::vector<SplitFeature>        splits;
    std::vector<std::vector<float> > leaf_values;

    unsigned long num_leaves() const;

    /*!
        requires
            - All the index values in splits are less than feature_pixel_values.size()
            - leaf_values.size() is a power of 2.
                (i.e. we require a tree with all the levels fully filled out.
            - leaf_values.size() == splits.size()+1
                (i.e. there needs to be the right number of leaves given the number of splits in the tree)
        ensures
            - runs through the tree and returns the vector at the leaf we end up in.
            - #i == the selected leaf node index.
    !*/
    const std::vector<float>& operator()(const std::vector<float>& feature_pixel_values, unsigned long& i) const;
};

QDataStream& operator << (QDataStream& dataStream, const RegressionTree& regtree);
QDataStream& operator >> (QDataStream& dataStream, RegressionTree& regtree);

/*!
    requires
        - idx < shape.size()/2
        - shape.size()%2 == 0
    ensures
        - returns the idx-th point from the shape vector.
!*/
template<class T>
inline std::vector<T> location(const std::vector<T>& shape, unsigned long idx)
{
    std::vector<T> temp(2);
    temp[0] = shape[idx * 2];
    temp[1] = shape[idx * 2 + 1];

    return temp;
}

// ------------------------------------------------------------------------------------

unsigned long nearest_shape_point(const std::vector<float>& shape,
                                         const std::vector<float>& pt);

// ------------------------------------------------------------------------------------

/*!
    requires
        - shape.size()%2 == 0
        - shape.size() > 0
    ensures
        - #anchor_idx.size() == pixel_coordinates.size()
        - #deltas.size()     == pixel_coordinates.size()
        - for all valid i:
        - pixel_coordinates[i] == location(shape,#anchor_idx[i]) + #deltas[i]
!*/
void create_shape_relative_encoding(const std::vector<float>& shape,
                                    const std::vector<std::vector<float> >& pixel_coordinates,
                                    std::vector<unsigned long>& anchor_idx,
                                    std::vector<std::vector<float> >& deltas);

// ------------------------------------------------------------------------------------

PointTransformAffine find_tform_between_shapes(const std::vector<float>& from_shape,
                                               const std::vector<float>& to_shape);

// ------------------------------------------------------------------------------------

/*!
    ensures
        - returns a transform that maps rect.tl_corner() to (0, 0) and rect.br_corner()
            to (1,1).
!*/
PointTransformAffine normalizing_tform(const cv::Rect& rect);

// ------------------------------------------------------------------------------------

/*!
    ensures
        - returns a transform that maps (0, 0) to rect.tl_corner() and (1,1) to
            rect.br_corner().
!*/
PointTransformAffine unnormalizing_tform(const cv::Rect& rect);
bool pointContained(const cv::Rect& rect, const std::vector<float>& point);

// ------------------------------------------------------------------------------------

/*!
    requires
        - image_type == an image object that implements the interface defined in
            dlib/image_processing/generic_image.h
        - reference_pixel_anchor_idx.size() == reference_pixel_deltas.size()
        - current_shape.size() == reference_shape.size()
        - reference_shape.size()%2 == 0
        - max(mat(reference_pixel_anchor_idx)) < reference_shape.size()/2
    ensures
        - #feature_pixel_values.size() == reference_pixel_deltas.size()
        - for all valid i:
            - #feature_pixel_values[i] == the value of the pixel in img_ that
                corresponds to the pixel identified by reference_pixel_anchor_idx[i]
                and reference_pixel_deltas[i] when the pixel is located relative to
                current_shape rather than reference_shape.
!*/
void extract_feature_pixel_values(const cv::Mat& img_,
                                  const cv::Rect& rect,
                                  const std::vector<float>& current_shape,
                                  const std::vector<float>& reference_shape,
                                  const std::vector<unsigned long>& reference_pixel_anchor_idx,
                                  const std::vector<std::vector<float> >& reference_pixel_deltas,
                                  std::vector<float>& feature_pixel_values);

// ------------------------------------------------------------------------------------

class ShapePredictor
{
public:

    ShapePredictor();

    unsigned long num_parts() const;
    unsigned long num_features() const;

    FullObjectDetection operator()(const cv::Mat& img, const cv::Rect& rect) const;

public:

    std::vector<float>                                initial_shape;
    std::vector<std::vector<redeye::RegressionTree> > forests;
    std::vector<std::vector<unsigned long> >          anchor_idx;
    std::vector<std::vector<std::vector<float> > >    deltas;
};

QDataStream& operator << (QDataStream& dataStream, const ShapePredictor& shape);
QDataStream& operator >> (QDataStream& dataStream, ShapePredictor& shape);

} // namespace redeye

} // namespace Digikam

#endif // DIGIKAM_SHAPE_PREDICTOR_H
