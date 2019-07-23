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

#include "shapepredictor.h"

namespace Digikam
{

namespace redeye
{

QDataStream& operator << (QDataStream& dataStream, const SplitFeature& sp)
{
    dataStream << sp.idx1 << sp.idx2 << sp.thresh;
    return dataStream;
}

QDataStream& operator >> (QDataStream& dataStream, SplitFeature& sp)
{
    dataStream >> sp.idx1 >> sp.idx2 >> sp.thresh;
    return dataStream;
}

unsigned long left_child(unsigned long idx)
{
    return 2*idx + 1;
}

unsigned long right_child(unsigned long idx)
{
    return 2*idx + 2;
}

// ----------------------------------------------------------------------------------------

unsigned long RegressionTree::num_leaves() const
{
    return leaf_values.size();
}

const std::vector<float>& RegressionTree::operator()(const std::vector<float>& feature_pixel_values, unsigned long& i) const
{
    i = 0;

    while (i < splits.size())
    {
        if (feature_pixel_values[splits[i].idx1] - feature_pixel_values[splits[i].idx2] > splits[i].thresh)
            i = left_child(i);
        else
            i = right_child(i);
    }

    i = i - splits.size();

    return leaf_values[i];
}

QDataStream& operator << (QDataStream& dataStream, const RegressionTree& regtree)
{
    dataStream << (unsigned int)regtree.splits.size();

    for (unsigned int i = 0 ; i < regtree.splits.size() ; ++i)
    {
        dataStream << regtree.splits[i];
    }

    dataStream << (unsigned int)regtree.leaf_values.size();
    dataStream << (unsigned int)regtree.leaf_values[0].size();

    for (unsigned int i = 0 ; i < regtree.leaf_values.size() ; ++i)
    {
        for (unsigned int j = 0 ; j < regtree.leaf_values[i].size() ; ++j)
        {
            dataStream << regtree.leaf_values[i][j];
        }

    }

    return dataStream;
}

QDataStream& operator >> (QDataStream& dataStream, RegressionTree& regtree)
{
    unsigned int size;
    dataStream >> size;
    regtree.splits.resize(size);

    for (unsigned int i = 0 ; i < regtree.splits.size() ; ++i)
    {
        dataStream >> regtree.splits[i];
    }

    dataStream >> size;
    regtree.leaf_values.resize(size);
    dataStream >> size;

    for (unsigned int i = 0 ; i < regtree.leaf_values.size() ; ++i)
    {
        regtree.leaf_values[i].resize(size);

        for (unsigned int j = 0 ; j < regtree.leaf_values[i].size() ; ++j)
        {
            dataStream >> regtree.leaf_values[i][j];
        }
    }

    return dataStream;
}

// ------------------------------------------------------------------------------------

unsigned long nearest_shape_point(const std::vector<float>& shape,
                                  const std::vector<float>& pt)
{
    // find the nearest part of the shape to this pixel
    float best_dist                     = std::numeric_limits<float>::infinity();
    const unsigned long num_shape_parts = shape.size()/2;
    unsigned long best_idx              = 0;

    for (unsigned long j = 0 ; j < num_shape_parts ; ++j)
    {
        const float dist = length_squared(location(shape, j) - pt);

        if (dist < best_dist)
        {
            best_dist = dist;
            best_idx  = j;
        }
    }

    return best_idx;
}

// ------------------------------------------------------------------------------------

void create_shape_relative_encoding(const std::vector<float>& shape,
                                    const std::vector<std::vector<float> >& pixel_coordinates,
                                    std::vector<unsigned long>& anchor_idx,
                                    std::vector<std::vector<float> >& deltas)
{
    anchor_idx.resize(pixel_coordinates.size());
    deltas.resize(pixel_coordinates.size());

    for (unsigned long i = 0 ; i < pixel_coordinates.size() ; ++i)
    {
        anchor_idx[i] = nearest_shape_point(shape, pixel_coordinates[i]);
        deltas[i]     = pixel_coordinates[i] - location(shape, anchor_idx[i]);
    }
}

// ------------------------------------------------------------------------------------

PointTransformAffine find_tform_between_shapes(const std::vector<float>& from_shape,
                                               const std::vector<float>& to_shape)
{
    assert(from_shape.size() == to_shape.size() && (from_shape.size()%2) == 0 && from_shape.size() > 0);

    std::vector<std::vector<float> > from_points, to_points;
    const unsigned long num = from_shape.size()/2;
    from_points.reserve(num);
    to_points.reserve(num);

    if (num == 1)
    {
        // Just use an identity transform if there is only one landmark.
        return PointTransformAffine();
    }

    for (unsigned long i = 0 ; i < num ; ++i)
    {
        from_points.push_back(location(from_shape, i));
        to_points.push_back(location(to_shape, i));
    }

    return find_similarity_transform(from_points, to_points);
}

// ------------------------------------------------------------------------------------

PointTransformAffine normalizing_tform(const cv::Rect& rect)
{
    std::vector<std::vector<float> > from_points, to_points;
    std::vector<float> tlcorner(2);
    tlcorner[0] = rect.x;
    tlcorner[1] = rect.y;
    std::vector<float> trcorner(2);
    tlcorner[0] = rect.x + rect.width;
    trcorner[1] = rect.y;
    std::vector<float> brcorner(2);
    tlcorner[0] = rect.x + rect.width;
    brcorner[1] = rect.y + rect.height;

    std::vector<float> pt1(2);
    pt1[0] = 0;
    pt1[1] = 0;
    std::vector<float> pt2(2);
    pt2[0] = 1;
    pt2[1] = 0;
    std::vector<float> pt3(2);
    pt3[0] = 1;
    pt3[1] = 1;
    from_points.push_back(tlcorner);
    to_points.push_back(pt1);
    from_points.push_back(trcorner);
    to_points.push_back(pt2);
    from_points.push_back(brcorner);
    to_points.push_back(pt3);
    return find_affine_transform(from_points, to_points);
}

// ------------------------------------------------------------------------------------

PointTransformAffine unnormalizing_tform(const cv::Rect& rect)
{
    std::vector<std::vector<float> > from_points, to_points;
    std::vector<float> tlcorner(2);
    tlcorner[0] = rect.x;
    tlcorner[1] = rect.y;
    std::vector<float> trcorner(2);
    trcorner[0] = rect.x + rect.width;
    trcorner[1] = rect.y;
    std::vector<float> brcorner(2);
    brcorner[0] = rect.x + rect.width;
    brcorner[1] = rect.y + rect.height;

    std::vector<float> pt1(2);
    pt1[0] = 0;
    pt1[1] = 0;
    std::vector<float> pt2(2);
    pt2[0] = 1;
    pt2[1] = 0;
    std::vector<float> pt3(2);
    pt3[0] = 1;
    pt3[1] = 1;

    to_points.push_back(tlcorner);
    from_points.push_back(pt1);
    to_points.push_back(trcorner);
    from_points.push_back(pt2);
    to_points.push_back(brcorner);
    from_points.push_back(pt3);
    return find_affine_transform(from_points, to_points);
}

bool pointContained(const cv::Rect& rect, const std::vector<float>& point)
{
    int x = std::round(point[0]);
    int y = std::round(point[1]);

    if (x >= 0 && x < rect.width   &&
        y >= 0 && y < rect.height)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// ------------------------------------------------------------------------------------

void extract_feature_pixel_values(const cv::Mat& img_,
                          		  const cv::Rect& rect,
                                  const std::vector<float>& current_shape,
                                  const std::vector<float>& reference_shape,
                                  const std::vector<unsigned long>& reference_pixel_anchor_idx,
                                  const std::vector<std::vector<float> >& reference_pixel_deltas,
                                  std::vector<float>& feature_pixel_values)
{
    const std::vector<std::vector<float> > tform = find_tform_between_shapes(reference_shape, current_shape).get_m();
    const PointTransformAffine tform_to_img      = unnormalizing_tform(rect);
    const cv::Rect area                          = cv::Rect(0, 0, img_.size().width, img_.size().height);
    cv::Mat img(img_);
    feature_pixel_values.resize(reference_pixel_deltas.size());

    for (unsigned long i = 0 ; i < feature_pixel_values.size() ; ++i)
    {
        // Compute the point in the current shape corresponding to the i-th pixel and
        // then map it from the normalized shape space into pixel space.
        std::vector<float> p = tform_to_img(tform*reference_pixel_deltas[i] + location(current_shape, reference_pixel_anchor_idx[i]));

        if (pointContained(area, p))
        {
            feature_pixel_values[i] = img.at<unsigned char>(std::round(p[1]), std::round(p[0]));
        }
        else
        {
            feature_pixel_values[i] = 0;
        }
    }
}

// ------------------------------------------------------------------------------------

ShapePredictor::ShapePredictor()
{
}

unsigned long ShapePredictor::num_parts() const
{
    return initial_shape.size() / 2;
}

unsigned long ShapePredictor::num_features() const
{
    unsigned long num = 0;

    for (unsigned long iter = 0 ; iter < forests.size() ; ++iter)
    {
        for (unsigned long i = 0 ; i < forests[iter].size() ; ++i)
        {
            num += forests[iter][i].num_leaves();
        }
    }

    return num;
}

FullObjectDetection ShapePredictor::operator()(const cv::Mat& img, const cv::Rect& rect) const
{
    using namespace redeye;
    std::vector<float> current_shape = initial_shape;
    std::vector<float> feature_pixel_values;

    for (unsigned long iter = 0 ; iter < forests.size() ; ++iter)
    {
        extract_feature_pixel_values(img, rect, current_shape, initial_shape,
                                        anchor_idx[iter], deltas[iter], feature_pixel_values);
        unsigned long leaf_idx;

        // evaluate all the trees at this level of the cascade.

        for (unsigned long i = 0 ; i < forests[iter].size() ; ++i)
        {
            current_shape = current_shape + forests[iter][i](feature_pixel_values, leaf_idx);
        }
    }

    // convert the current_shape into a full_object_detection
    const PointTransformAffine tform_to_img = unnormalizing_tform(rect);
    std::vector<std::vector<float> > parts(current_shape.size() / 2);

    for (unsigned long i = 0 ; i < parts.size() ; ++i)
    {
        parts[i] = tform_to_img(location(current_shape, i));
    }

    return FullObjectDetection(rect, parts);
}

// ------------------------------------------------------------------------------------

QDataStream& operator << (QDataStream& dataStream, const ShapePredictor& shape)
{
    dataStream << (unsigned int)shape.initial_shape.size();

    for (unsigned int i = 0 ; i < shape.initial_shape.size() ; ++i)
    {
        dataStream << shape.initial_shape[i];
    }

    dataStream<<(unsigned int)shape.forests.size();
    dataStream<<(unsigned int)shape.forests[0].size();

    for (unsigned int i = 0 ; i < shape.forests.size() ; ++i)
    {
        for (unsigned int j = 0 ; j < shape.forests[i].size() ; ++j)
        {
            dataStream << shape.forests[i][j];
        }
    }

    dataStream << (unsigned int)shape.anchor_idx.size();
    dataStream << (unsigned int)shape.anchor_idx[0].size();

    for (unsigned int i = 0 ; i < shape.anchor_idx.size() ; ++i)
    {
        for (unsigned int j = 0 ; j < shape.anchor_idx[i].size() ; ++j)
        {
            dataStream << shape.anchor_idx[i][j];
        }

    }

    dataStream << (unsigned int)shape.deltas.size();
    dataStream << (unsigned int)shape.deltas[0].size();

    for (unsigned int i = 0 ; i < shape.deltas.size() ; ++i)
    {
        for (unsigned int j = 0 ; j < shape.deltas[i].size() ; ++j)
        {
            dataStream << shape.deltas[i][j][0];
            dataStream << shape.deltas[i][j][1];
        }
    }

    return dataStream;
}

QDataStream& operator >> (QDataStream& dataStream, ShapePredictor& shape)
{
    unsigned int size;
    dataStream >> size;
    shape.initial_shape.resize(size);

    for (unsigned int i = 0 ; i < shape.initial_shape.size() ; ++i)
    {
        dataStream >> shape.initial_shape[i];
    }

    dataStream >> size;
    shape.forests.resize(size);
    dataStream >> size;

    for (unsigned int i = 0 ; i < shape.forests.size() ; ++i)
    {
        shape.forests[i].resize(size);

        for (unsigned int j = 0 ; j < shape.forests[i].size() ; ++j)
        {
            dataStream >> shape.forests[i][j];
        }
    }

    dataStream >> size;
    shape.anchor_idx.resize(size);
    dataStream >> size;

    for (unsigned int i = 0 ; i < shape.anchor_idx.size() ; ++i)
    {
        shape.anchor_idx[i].resize(size);

        for (unsigned int j = 0 ; j < shape.anchor_idx[i].size() ; ++j)
        {
            dataStream >> shape.anchor_idx[i][j];
        }

    }

    dataStream >> size;
    shape.deltas.resize(size);
    dataStream >> size;

    for (unsigned int i = 0 ; i < shape.deltas.size() ; ++i)
    {
        shape.deltas[i].resize(size);

        for (unsigned int j = 0 ; j < shape.deltas[i].size() ; ++j)
        {
            shape.deltas[i][j].resize(2);
            dataStream >> shape.deltas[i][j][0];
            dataStream >> shape.deltas[i][j][1];
        }
    }

    return dataStream;
}

}; // namespace redeye

}; // namespace Digikam