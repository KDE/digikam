/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2017-08-08
 * Description : Base functions for dnn module, can be used for face recognition, 
 *               all codes are ported from dlib library (http://dlib.net/)
 *
 * Copyright (C) 2006-2016 by Davis E. King <davis at dlib dot net>
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

#ifndef DLIB_DNN_CPU_H_
#define DLIB_DNN_CPU_H_

// This file contains CPU implementations of the GPU based functions in cuda_dlib.h
// and cudnn_dlibapi.h

#include "tensor.h"

//namespace dlib
//{
    namespace cpu 
    {

    // -----------------------------------------------------------------------------------

        static void multiply (
            bool add_to,
            tensor& dest,
            const tensor& src1,
            const tensor& src2
        );

        static void multiply_conv (
            bool add_to,
            tensor& dest,
            const tensor& src1,
            const tensor& src2
        );

        static void add(
            float beta,
            tensor& dest,
            float alpha,
            const tensor& src
        );

        static void assign_bias_gradient (
            tensor& grad,
            const tensor& gradient_input
        );

        static void add (
            tensor& dest,
            const tensor& src1,
            const tensor& src2
        );

        static void assign_conv_bias_gradient (
            tensor& grad,
            const tensor& gradient_input
        );

    // -----------------------------------------------------------------------------------

        static void affine_transform(
            tensor& dest,
            const tensor& src,
            const float A,
            const float B
        );

        static void affine_transform(
            tensor& dest,
            const tensor& src1,
            const tensor& src2,
            const float A,
            const float B,
            const float C
        );

        static void affine_transform(
            tensor& dest,
            const tensor& src1,
            const tensor& src2,
            const tensor& src3,
            const float A,
            const float B,
            const float C,
            const float D
        );

        static void affine_transform_range(
            size_t begin,
            size_t end,
            tensor& dest,
            const tensor& src1,
            const tensor& src2,
            const tensor& src3,
            const float A,
            const float B,
            const float C
        );

    // -----------------------------------------------------------------------------------

        static void affine_transform(
            tensor& dest,
            const tensor& src,
            const tensor& A,
            const tensor& B
        );

    // -----------------------------------------------------------------------------------

        static void affine_transform_conv(
            tensor& dest,
            const tensor& src,
            const tensor& A,
            const tensor& B
        );

    // -----------------------------------------------------------------------------------

        static void compute_adam_update (
            size_t begin,
            size_t end,
            tensor& s,
            tensor& m,
            tensor& v,
            const float t,
            const float learning_rate,
            const float weight_decay,
            const float momentum1,
            const float momentum2,
            const tensor& params,
            const tensor& params_grad
        );

    // -----------------------------------------------------------------------------------

        static void batch_normalize_inference (
            const double eps,
            resizable_tensor& dest,
            const tensor& src,
            const tensor& gamma, 
            const tensor& beta,
            const tensor& running_means,
            const tensor& running_variances
        );

        static void batch_normalize (
            const double eps,
            resizable_tensor& dest,
            resizable_tensor& means,
            resizable_tensor& invstds,
            const double averaging_factor,
            resizable_tensor& running_means,
            resizable_tensor& running_variances,
            const tensor& src,
            const tensor& gamma, 
            const tensor& beta 
        );

        static void batch_normalize_gradient (
            const double eps,
            const tensor& gradient_input,
            const tensor& means,
            const tensor& invstds,
            const tensor& src,
            const tensor& gamma,
            tensor& src_grad,
            tensor& gamma_grad, 
            tensor& beta_grad 
        );

        static void batch_normalize_conv_inference (
            const double eps,
            resizable_tensor& dest,
            const tensor& src,
            const tensor& gamma, 
            const tensor& beta,
            const tensor& running_means,
            const tensor& running_variances
        );

        static void batch_normalize_conv (
            const double eps,
            resizable_tensor& dest,
            resizable_tensor& means,
            resizable_tensor& invstds,
            const double averaging_factor,
            resizable_tensor& running_means,
            resizable_tensor& running_variances,
            const tensor& src,
            const tensor& gamma, 
            const tensor& beta 
        );

        static void batch_normalize_conv_gradient (
            const double eps,
            const tensor& gradient_input,
            const tensor& means,
            const tensor& invstds,
            const tensor& src,
            const tensor& gamma,
            tensor& src_grad,
            tensor& gamma_grad, 
            tensor& beta_grad 
        );

    // -----------------------------------------------------------------------------------

        static void threshold (
            tensor& data,
            float thresh
        );

        static void dot (
            const tensor& a,
            const tensor& b,
            tensor& result,
            size_t idx
        );

    // -----------------------------------------------------------------------------------

        static void softmax (
            tensor& dest,
            const tensor& src
        );

        static void softmax_gradient (
            tensor& grad,
            const tensor& dest,
            const tensor& gradient_input
        );

    // ------------------------------------------------------------------------------------

        static void sigmoid (
            tensor& dest,
            const tensor& src
        );

        static void sigmoid_gradient (
            tensor& grad,
            const tensor& dest,
            const tensor& gradient_input
        );

    // ------------------------------------------------------------------------------------

        static void relu (
            tensor& dest,
            const tensor& src
        );

        static void relu_gradient (
            tensor& grad,
            const tensor& dest,
            const tensor& gradient_input
        );

    // ----------------------------------------------------------------------------------------

        static void prelu (
            tensor& dest,
            const tensor& src,
            const tensor& param
        );

        static void prelu_gradient (
            tensor& grad,
            const tensor& src,
            const tensor& gradient_input,
            const tensor& param,
            tensor& params_grad 
        );

    // ------------------------------------------------------------------------------------

        static void tanh (
            tensor& dest,
            const tensor& src
        );

        static void tanh_gradient (
            tensor& grad,
            const tensor& dest,
            const tensor& gradient_input
        );

    // -----------------------------------------------------------------------------------

        class pooling
        {
        public:

            pooling () : 
            window_height(0),window_width(0),stride_y(0),stride_x(0),padding_y(0),padding_x(0),do_max_pooling(true){};
            pooling(const pooling&) = delete;
            pooling& operator=(const pooling&) = delete;

            void clear()
            {
                window_height = 0;
                window_width = 0;
                stride_y = 0;
                stride_x = 0;
                padding_y = 0;
                padding_x = 0;
            }

            void setup_max_pooling(
                int window_height_,
                int window_width_,
                int stride_y_,
                int stride_x_,
                int padding_y_,
                int padding_x_
            )
            {
                DLIB_CASSERT(window_width_ > 0);
                DLIB_CASSERT(window_height_ > 0);
                DLIB_CASSERT(stride_y_ > 0);
                DLIB_CASSERT(stride_x_ > 0);
                DLIB_CASSERT(0 <= padding_y_ && padding_y_ < window_height_);
                DLIB_CASSERT(0 <= padding_x_ && padding_x_ < window_width_);

                window_height = window_height_;
                window_width = window_width_;
                stride_y = stride_y_;
                stride_x = stride_x_;
                padding_y = padding_y_;
                padding_x = padding_x_;
                do_max_pooling = true;
            };

            void setup_avg_pooling(
                int window_height_,
                int window_width_,
                int stride_y_,
                int stride_x_,
                int padding_y_,
                int padding_x_
            )
            {
                DLIB_CASSERT(window_width_ > 0);
                DLIB_CASSERT(window_height_ > 0);
                DLIB_CASSERT(stride_y_ > 0);
                DLIB_CASSERT(stride_x_ > 0);
                DLIB_CASSERT(0 <= padding_y_ && padding_y_ < window_height_);
                DLIB_CASSERT(0 <= padding_x_ && padding_x_ < window_width_);

                window_height = window_height_;
                window_width = window_width_;
                stride_y = stride_y_;
                stride_x = stride_x_;
                padding_y = padding_y_;
                padding_x = padding_x_;
                do_max_pooling = false;
            };

            bool does_max_pooling(
            ) const { return do_max_pooling; }

            void operator() (
                resizable_tensor& dest,
                const tensor& src
            )
            {
                DLIB_CASSERT(window_width > 0);
                DLIB_CASSERT(window_height > 0);
                DLIB_CASSERT(stride_y > 0);
                DLIB_CASSERT(stride_x > 0);
                DLIB_CASSERT(0 <= padding_y && padding_y < window_height);
                DLIB_CASSERT(0 <= padding_x && padding_x < window_width);
                DLIB_CASSERT(window_width  <= src.nc() + 2*padding_x,
                    "Pooling windows must be small enough to fit into the padded image.");
                DLIB_CASSERT(window_height <= src.nr() + 2*padding_y,
                    "Pooling windows must be small enough to fit into the padded image.");

                dest.set_size(
                     src.num_samples(),
                     src.k(),
                     1+(src.nr()+2*padding_y-window_height)/stride_y,
                     1+(src.nc()+2*padding_x-window_width)/stride_x
                    );

                if (src.size() == 0)
                {
                    dest = 0;
                    return;
                }


                auto d = dest.host();
                const long x_offset = window_width/2 - padding_x;
                const long y_offset = window_height/2 - padding_y;
                if (does_max_pooling())
                {
                    for (long n = 0; n < dest.num_samples(); ++n)
                    {
                        for (long k = 0; k < dest.k(); ++k)
                        {
                            auto simg = image_plane(src,n,k);
                            auto dimg = d + (n*dest.k() + k)*dest.nr()*dest.nc();

                            for (long r = 0; r < dest.nr(); ++r)
                            {
                                for (long c = 0; c < dest.nc(); ++c)
                                {
                                    auto win = centered_rect(c*stride_x+x_offset,
                                        r*stride_y+y_offset,
                                        window_width,
                                        window_height);
                                    dimg[r*dest.nc() + c] = max(subm_clipped(simg,win));
                                }
                            }
                        }
                    }
                }
                else
                {
                    for (long n = 0; n < dest.num_samples(); ++n)
                    {
                        for (long k = 0; k < dest.k(); ++k)
                        {
                            auto simg = image_plane(src,n,k);
                            auto dimg = d + (n*dest.k() + k)*dest.nr()*dest.nc();

                            for (long r = 0; r < dest.nr(); ++r)
                            {
                                for (long c = 0; c < dest.nc(); ++c)
                                {
                                    auto win = centered_rect(c*stride_x+x_offset,
                                        r*stride_y+y_offset,
                                        window_width,
                                        window_height);
                                    dimg[r*dest.nc() + c] = mean(subm_clipped(simg,win));
                                }
                            }
                        }
                    }
                }

            }

            void get_gradient(
                const tensor& gradient_input, 
                const tensor& dest,
                const tensor& src,
                tensor& grad 
            )
            {
                DLIB_CASSERT(have_same_dimensions(gradient_input,dest));
                DLIB_CASSERT(have_same_dimensions(src,grad));


                if (src.size() == 0)
                {
                    return;
                }


                auto gi = gradient_input.host();
                auto g = grad.host();
                const long x_offset = window_width/2 - padding_x;
                const long y_offset = window_height/2 - padding_y;
                if (does_max_pooling())
                {
                    for (long n = 0; n < dest.num_samples(); ++n)
                    {
                        for (long k = 0; k < dest.k(); ++k)
                        {
                            auto simg = image_plane(src,n,k);
                            auto gimg = g + (n*grad.k() + k)*grad.nr()*grad.nc();
                            auto giimg = gi + (n*dest.k() + k)*dest.nr()*dest.nc();
                            auto imgbox = get_rect(simg);

                            for (long r = 0; r < dest.nr(); ++r)
                            {
                                for (long c = 0; c < dest.nc(); ++c)
                                {
                                    auto win = centered_rect(c*stride_x+x_offset,
                                        r*stride_y+y_offset,
                                        window_width,
                                        window_height).intersect(imgbox);
                                    auto p = max_point(subm(simg,win))+win.tl_corner();
                                    gimg[p.y()*grad.nc()+p.x()] += giimg[r*dest.nc()+c];
                                }
                            }
                        }
                    }
                }
                else
                {
                    for (long n = 0; n < dest.num_samples(); ++n)
                    {
                        for (long k = 0; k < dest.k(); ++k)
                        {
                            auto simg = image_plane(src,n,k);
                            auto gimg = g + (n*grad.k() + k)*grad.nr()*grad.nc();
                            auto giimg = gi + (n*dest.k() + k)*dest.nr()*dest.nc();
                            auto imgbox = get_rect(simg);

                            for (long r = 0; r < dest.nr(); ++r)
                            {
                                for (long c = 0; c < dest.nc(); ++c)
                                {
                                    auto win = centered_rect(c*stride_x+x_offset,
                                        r*stride_y+y_offset,
                                        window_width,
                                        window_height).intersect(imgbox);
                                    const float delta = giimg[r*dest.nc()+c]/win.area();
                                    for (long y = win.top(); y <= win.bottom(); ++y)
                                    {
                                        for (long x = win.left(); x <= win.right(); ++x)
                                        {
                                            gimg[y*grad.nc()+x] += delta;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

            };

        private:
            int window_height;
            int window_width;
            int stride_y;
            int stride_x;
            int padding_y;
            int padding_x;
            bool do_max_pooling;

        };
        

    // -----------------------------------------------------------------------------------

        void img2col(
            matrix<float>& output,
            const tensor& data,
            long n,
            long filter_nr,
            long filter_nc,
            long stride_y,
            long stride_x,
            long padding_y,
            long padding_x
        )
        {
            const auto d = data.host() + data.k()*data.nr()*data.nc()*n;
            const rectangle boundary = get_rect(data);

            const long out_nr = 1+(data.nr()+2*padding_y-filter_nr)/stride_y;
            const long out_nc = 1+(data.nc()+2*padding_x-filter_nc)/stride_x;

            output.set_size(out_nr*out_nc, 
                            data.k()*filter_nr*filter_nc);
            DLIB_CASSERT(output.size() != 0);
            float* t = &output(0,0);

            // now fill in the Toeplitz output matrix for the n-th sample in data.  
            size_t cnt = 0;
            const long max_r = data.nr() + padding_y-(filter_nr-1);
            const long max_c = data.nc() + padding_x-(filter_nc-1);
            for (long r = -padding_y; r < max_r; r+=stride_y)
            {
                for (long c = -padding_x; c < max_c; c+=stride_x)
                {
                    for (long k = 0; k < data.k(); ++k)
                    {
                        for (long y = 0; y < filter_nr; ++y)
                        {
                            for (long x = 0; x < filter_nc; ++x)
                            {
                                DLIB_ASSERT(cnt < output.size());
                                long xx = c+x;
                                long yy = r+y;
                                if (boundary.contains(xx,yy))
                                    *t = d[(k*data.nr() + yy)*data.nc() + xx];
                                else
                                    *t = 0;
                                ++t;
                                ++cnt;
                            }
                        }
                    }
                }
            }
        }

        void col2img(
            const matrix<float>& output,
            tensor& data,
            long n,
            long filter_nr,
            long filter_nc,
            long stride_y,
            long stride_x,
            long padding_y,
            long padding_x
        )
        {
            const auto d = data.host() + data.k()*data.nr()*data.nc()*n;
            const rectangle boundary = get_rect(data);

            DLIB_CASSERT(output.size() != 0);
            const float* t = &output(0,0);

            // now fill in the Toeplitz output matrix for the n-th sample in data.  
            const long max_r = data.nr() + padding_y-(filter_nr-1);
            const long max_c = data.nc() + padding_x-(filter_nc-1);
            for (long r = -padding_y; r < max_r; r+=stride_y)
            {
                for (long c = -padding_x; c < max_c; c+=stride_x)
                {
                    for (long k = 0; k < data.k(); ++k)
                    {
                        for (long y = 0; y < filter_nr; ++y)
                        {
                            for (long x = 0; x < filter_nc; ++x)
                            {
                                long xx = c+x;
                                long yy = r+y;
                                if (boundary.contains(xx,yy))
                                    d[(k*data.nr() + yy)*data.nc() + xx] += *t;
                                ++t;
                            }
                        }
                    }
                }
            }
        }
    // -----------------------------------------------------------------------------------

        class tensor_conv
        {
        public:
            tensor_conv(const tensor_conv&) = delete;
            tensor_conv& operator=(const tensor_conv&) = delete;

            tensor_conv() {}

            void clear(
            ) {}

            void operator() (
                resizable_tensor& output,
                const tensor& data,
                const tensor& filters,
                int stride_y,
                int stride_x,
                int padding_y,
                int padding_x
            )
            {
                DLIB_CASSERT(is_same_object(output,data) == false);
                DLIB_CASSERT(is_same_object(output,filters) == false);
                DLIB_CASSERT(filters.k() == data.k());
                DLIB_CASSERT(stride_y > 0 && stride_x > 0);
                DLIB_CASSERT(0 <= padding_y && padding_y < filters.nr());
                DLIB_CASSERT(0 <= padding_x && padding_x < filters.nc());
                DLIB_CASSERT(filters.nr() <= data.nr() + 2*padding_y,
                    "Filter windows must be small enough to fit into the padded image.");
                DLIB_CASSERT(filters.nc() <= data.nc() + 2*padding_x,
                    "Filter windows must be small enough to fit into the padded image.");

                output.set_size(data.num_samples(),
                                filters.num_samples(),
                                1+(data.nr()+2*padding_y-filters.nr())/stride_y,
                                1+(data.nc()+2*padding_x-filters.nc())/stride_x);

                matrix<float> temp;
                //std::cout << "data.num_samples(): " << data.num_samples() << std::endl;
                
                for (long n = 0; n < data.num_samples(); ++n)
                {
                    img2col(temp, data, n, filters.nr(), filters.nc(), stride_y, stride_x, padding_y, padding_x);
                    output.set_sample(n, mat(filters)*trans(temp));
                }
                

                last_stride_y = stride_y;
                last_stride_x = stride_x;
                last_padding_y = padding_y;
                last_padding_x = padding_x;
            };

            void get_gradient_for_data (
                const tensor& gradient_input, 
                const tensor& filters,
                tensor& data_gradient
            )
            {
                matrix<float> temp;
                for (long n = 0; n < gradient_input.num_samples(); ++n)
                {
                    auto gi = mat(gradient_input.host()+gradient_input.k()*gradient_input.nr()*gradient_input.nc()*n,
                                  gradient_input.k(),
                                  gradient_input.nr()*gradient_input.nc());
                                        

                    temp = trans(gi)*mat(filters);
                    col2img(temp, data_gradient, n, filters.nr(), filters.nc(), last_stride_y, last_stride_x, last_padding_y, last_padding_x);
                }
            };

            void get_gradient_for_filters (
                const tensor& gradient_input, 
                const tensor& data,
                tensor& filters_gradient
            )
            {
                matrix<float> temp;
                for (long n = 0; n < gradient_input.num_samples(); ++n)
                {
                    auto gi = mat(gradient_input.host()+gradient_input.k()*gradient_input.nr()*gradient_input.nc()*n,
                                  gradient_input.k(),
                                  gradient_input.nr()*gradient_input.nc());


                    img2col(temp, data, n, filters_gradient.nr(), filters_gradient.nc(), last_stride_y, last_stride_x, last_padding_y, last_padding_x);
                    if (n == 0)
                        filters_gradient = gi*temp;
                    else
                        filters_gradient += gi*temp;
                }
            };

        private:

            long last_stride_y;
            long last_stride_x;
            long last_padding_y;
            long last_padding_x;
        };

    // -----------------------------------------------------------------------------------

        static void copy_tensor(
            tensor& dest,
            size_t dest_k_offset,
            const tensor& src,
            size_t src_k_offset,
            size_t count_k
        );

    // -----------------------------------------------------------------------------------

    } 
//}

//#ifdef NO_MAKEFILE
#include "cpu_dlib.cpp"
//#endif

#endif // DLIB_DNN_CPU_H_


