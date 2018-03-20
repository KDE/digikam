// Copyright (C) 2015  Davis E. King (davis@dlib.net)
// License: Boost Software License   See LICENSE.txt for the full license.
#ifndef DLIB_TeNSOR_TOOLS_H_
#define DLIB_TeNSOR_TOOLS_H_

#include "tensor.h"
#include "cpu_dlib.h"
#include "../dnn_base/rand_kernel_1.h"
#include <memory>

//namespace dlib
//{
/*
    static bool dnn_prefer_fastest_algorithms();
    static void set_dnn_prefer_fastest_algorithms();
    static void set_dnn_prefer_smallest_algorithms();
*/
//}

//namespace dlib {
namespace tt
{

// ----------------------------------------------------------------------------------------

    void inverse_norms (
        resizable_tensor& invnorms,
        const tensor& data,
        const double eps
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::inverse_norms(invnorms, data, eps);
#else
        invnorms = impl::reciprocal(sqrt(sum_cols(squared(mat(data))) + eps));
#endif
    }

    void dot_prods (
        resizable_tensor& out,
        const tensor& lhs,
        const tensor& rhs
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::dot_prods(out, lhs, rhs);
#else
        out = sum_cols(pointwise_multiply(mat(lhs), mat(rhs))); 
#endif
    }

    void scale_columns (
        tensor& out,
        const tensor& m,
        const tensor& v
    )
    {
        DLIB_CASSERT(have_same_dimensions(out,m));
        DLIB_CASSERT(is_vector(v));
        if (m.size() == 0 && v.size() == 0)
            return;
        DLIB_CASSERT(m.size() != 0);
        DLIB_CASSERT(m.size()/m.num_samples() == v.size());

#ifdef DLIB_USE_CUDA
        cuda::scale_columns(out, m, v);
#else
        DLIB_CASSERT(false, "shouldn't be called right now");
        out = scale_columns(mat(m), mat(v));
#endif
    }

    void scale_rows (
        tensor& out,
        const tensor& m,
        const tensor& v
    )
    {
        DLIB_CASSERT(have_same_dimensions(out,m));
        DLIB_CASSERT(is_vector(v));
        if (m.size() == 0 && v.size() == 0)
            return;
        DLIB_CASSERT(m.size() != 0);
        DLIB_CASSERT(m.num_samples() == (int)v.size());

#ifdef DLIB_USE_CUDA
        cuda::scale_rows(out, m, v);
#else
        out = scale_rows(mat(m), mat(v));
#endif
    }

    void scale_rows2 (
        float beta, 
        tensor& out,
        const tensor& m1,
        const tensor& m2,
        const tensor& v1,
        const tensor& v2
    )
    {
        DLIB_CASSERT(have_same_dimensions(out,m1));
        DLIB_CASSERT(have_same_dimensions(out,m2));
        DLIB_CASSERT(have_same_dimensions(v1,v2));
        DLIB_CASSERT(is_vector(mat(v1))); 
        DLIB_CASSERT((int)v1.size() == m1.num_samples());

#ifdef DLIB_USE_CUDA
        cuda::scale_rows2(beta, out, m1, m2, v1, v2);
#else
        if (beta == 0)
            out = scale_rows(mat(m1) - scale_rows(mat(m2),mat(v1)), mat(v2));
        else
            out = beta*mat(out) + scale_rows(mat(m1) - scale_rows(mat(m2),mat(v1)), mat(v2));
#endif
    }

// ----------------------------------------------------------------------------------------

    void gemm (
        float beta,
        tensor& dest,
        float alpha,
        const tensor& lhs,
        bool trans_lhs,
        const tensor& rhs,
        bool trans_rhs
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::gemm(beta, dest, alpha, lhs, trans_lhs, rhs, trans_rhs);
#else

        if (beta != 0)
        {
            if (trans_lhs && trans_rhs)
                dest = alpha*trans(mat(lhs))*trans(mat(rhs)) + beta*mat(dest);
            else if (!trans_lhs && trans_rhs)
                dest = alpha*mat(lhs)*trans(mat(rhs)) + beta*mat(dest);
            else if (trans_lhs && !trans_rhs)
                dest = alpha*trans(mat(lhs))*mat(rhs) + beta*mat(dest);
            else
                dest = alpha*mat(lhs)*mat(rhs) + beta*mat(dest);
        }
        else
        {
            if (trans_lhs && trans_rhs)
                dest = alpha*trans(mat(lhs))*trans(mat(rhs));
            else if (!trans_lhs && trans_rhs)
                dest = alpha*mat(lhs)*trans(mat(rhs));
            else if (trans_lhs && !trans_rhs)
                dest = alpha*trans(mat(lhs))*mat(rhs);
            else
                dest = alpha*mat(lhs)*mat(rhs);
        }

        /*
        if (beta != 0)
        {
            if (trans_lhs && trans_rhs)
            {
                dest = op_multi_float(alpha, trans(mat(lhs)))*trans(mat(rhs)) + op_multi_float(beta, mat(dest));
            }
            else if (!trans_lhs && trans_rhs)
            {
                dest = op_multi_float(alpha,mat(lhs))*trans(mat(rhs)) + op_multi_float(beta, mat(dest));
            }
            else if (trans_lhs && !trans_rhs)
            {
                dest = op_multi_float(alpha, trans(mat(lhs)))*mat(rhs) + op_multi_float(beta, mat(dest));
            }
            else
            {
                dest = op_multi_float(alpha, mat(lhs))*mat(rhs) + op_multi_float(beta, mat(dest));
            }
        }
        else
        {
            if (trans_lhs && trans_rhs)
                dest = op_multi_float(alpha, trans(mat(lhs)))*trans(mat(rhs));
            else if (!trans_lhs && trans_rhs)
                dest = op_multi_float(alpha,mat(lhs))*trans(mat(rhs));
            else if (trans_lhs && !trans_rhs)
                dest = op_multi_float(alpha, trans(mat(lhs)))*mat(rhs);
            else
                dest = op_multi_float(alpha, mat(lhs))*mat(rhs);
        }
        */
#endif
    }

// ----------------------------------------------------------------------------------------

    class tensor_rand
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is a tool for filling a tensor with random numbers.  

                Note that the sequence of random numbers output by this object is different
                when dlib is compiled with DLIB_USE_CUDA.  So you should not write code
                that depends on any specific sequence of numbers coming out of a
                tensor_rand.

        !*/

    public:
        // not copyable
        tensor_rand(const tensor_rand&) = delete;
        tensor_rand& operator=(const tensor_rand&) = delete;
        

        tensor_rand() : tensor_rand(0) {}
        tensor_rand(
            unsigned long long seed
        ) 
    #ifdef DLIB_USE_CUDA
        :rnd(seed){};
    #else
        {rnd.set_seed(cast_to_string(seed)); };
    #endif

        void fill_gaussian (
            tensor& data,
            float mean = 0,
            float stddev = 1
        );
        /*!
            requires
                - data.size()%2 == 0
            ensures
                - Fills data with random numbers drawn from a Gaussian distribution
                  with the given mean and standard deviation.
        !*/

        void fill_uniform (
            tensor& data
        );
        /*!
            ensures
                - Fills data with uniform random numbers in the range (0.0, 1.0].
        !*/

#ifdef DLIB_USE_CUDA
        cuda::curand_generator rnd;
#else
        drand rnd;
#endif
    };

// ----------------------------------------------------------------------------------------

    void multiply (
        bool add_to,
        tensor& dest,
        const tensor& src1,
        const tensor& src2
    )
    {
        DLIB_CASSERT(dest.k() == src1.k() && src1.k() == src2.k() &&
            dest.nr() == src1.nr() && src1.nr() == src2.nr() &&
            dest.nc() == src1.nc() && src1.nc() == src2.nc() );
        const long MD = std::max(std::max(dest.num_samples(),src1.num_samples()),src2.num_samples());
        DLIB_CASSERT((dest.num_samples()==1 || dest.num_samples()==MD) &&
                    (src1.num_samples()==1 || src1.num_samples()==MD) &&
                    (src2.num_samples()==1 || src2.num_samples()==MD) );
#ifdef DLIB_USE_CUDA
        cuda::multiply(add_to, dest, src1, src2);
#else
        cpu::multiply(add_to, dest, src1, src2);
#endif

    }

    void multiply_conv (
        bool add_to,
        tensor& dest,
        const tensor& src1,
        const tensor& src2
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::multiply_conv(add_to, dest, src1, src2);
#else
        cpu::multiply_conv(add_to, dest, src1, src2);
#endif
    }

// ----------------------------------------------------------------------------------------

    void affine_transform(
        tensor& dest,
        const tensor& src,
        const float A,
        const float B
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::affine_transform(dest,src,A,B);
#else
        cpu::affine_transform(dest,src,A,B);
#endif
    }

    void affine_transform(
        tensor& dest,
        const tensor& src,
        const float A
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::affine_transform(dest,src,A);
#else
        cpu::affine_transform(dest,src,A,0);
#endif
    }

    void affine_transform(
        tensor& dest,
        const tensor& src1,
        const tensor& src2,
        const float A,
        const float B,
        const float C
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::affine_transform(dest,src1,src2,A,B,C);
#else
        cpu::affine_transform(dest,src1,src2,A,B,C);
#endif
    }

    void affine_transform(
        tensor& dest,
        const tensor& src1,
        const tensor& src2,
        const float A,
        const float B
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::affine_transform(dest,src1,src2,A,B);
#else
        cpu::affine_transform(dest,src1,src2,A,B,0);
#endif
    }

    void affine_transform(
        tensor& dest,
        const tensor& src1,
        const tensor& src2,
        const tensor& src3,
        const float A,
        const float B,
        const float C,
        const float D
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::affine_transform(dest,src1,src2,src3,A,B,C,D);
#else
        cpu::affine_transform(dest,src1,src2,src3,A,B,C,D);
#endif
    }

    void affine_transform_range(
        size_t begin,
        size_t end,
        tensor& dest,
        const tensor& src1,
        const tensor& src2,
        const tensor& src3,
        const float A,
        const float B,
        const float C
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::affine_transform_range(begin, end, dest,src1,src2,src3,A,B,C);
#else
        cpu::affine_transform_range(begin, end, dest,src1,src2,src3,A,B,C);
#endif
    }

    void affine_transform(
        tensor& dest,
        const tensor& src1,
        const tensor& src2,
        const tensor& src3,
        const float A,
        const float B,
        const float C
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::affine_transform_range(0,dest.size(),dest,src1,src2,src3,A,B,C);
#else
        cpu::affine_transform_range(0,dest.size(),dest,src1,src2,src3,A,B,C);
#endif
    }

// ----------------------------------------------------------------------------------------

    void affine_transform(
        tensor& dest,
        const tensor& src,
        const tensor& A,
        const tensor& B
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::affine_transform(dest,src,A,B);
#else
        cpu::affine_transform(dest,src,A,B);
#endif
    }

// ----------------------------------------------------------------------------------------

    void affine_transform_conv(
        tensor& dest,
        const tensor& src,
        const tensor& A,
        const tensor& B
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::affine_transform_conv(dest,src,A,B);
#else
        cpu::affine_transform_conv(dest,src,A,B);
#endif
    }

// ----------------------------------------------------------------------------------------

    void compute_adam_update (
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
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::compute_adam_update(begin, end, s, m, v, t, learning_rate, weight_decay, momentum1,
            momentum2, params, params_grad);
#else
        cpu::compute_adam_update(begin, end, s, m, v, t, learning_rate, weight_decay, momentum1,
            momentum2, params, params_grad);
#endif
    }

// ----------------------------------------------------------------------------------------

    void batch_normalize_inference (
        const double eps,
        resizable_tensor& dest,
        const tensor& src,
        const tensor& gamma, 
        const tensor& beta,
        const tensor& running_means,
        const tensor& running_variances
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::batch_normalize_inference(eps,dest,src,gamma,beta,running_means,running_variances);
#else
        cpu::batch_normalize_inference(eps,dest,src,gamma,beta,running_means,running_variances);
#endif
    }

    void batch_normalize (
        const double eps,
        resizable_tensor& dest,
        resizable_tensor& means,
        resizable_tensor& vars,
        const double averaging_factor,
        resizable_tensor& running_means,
        resizable_tensor& running_variances,
        const tensor& src,
        const tensor& gamma, 
        const tensor& beta 
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::batch_normalize(eps,dest,means,vars,averaging_factor,running_means,running_variances,src,gamma,beta);
#else
        cpu::batch_normalize(eps,dest,means,vars,averaging_factor,running_means,running_variances,src,gamma,beta);
#endif
    }

    void batch_normalize_gradient (
        const double eps,
            const tensor& gradient_input,
            const tensor& means,
            const tensor& invstds,
            const tensor& src,
            const tensor& gamma,
            tensor& src_grad,
            tensor& gamma_grad, 
            tensor& beta_grad 
    )
    {
             
#ifdef DLIB_USE_CUDA
        cuda::batch_normalize_gradient(eps,gradient_input, means, invstds, src, gamma, src_grad, gamma_grad, beta_grad);
#else
        cpu::batch_normalize_gradient(eps,gradient_input, means, invstds, src, gamma, src_grad, gamma_grad, beta_grad);
#endif
    }

// ----------------------------------------------------------------------------------------

    void batch_normalize_conv_inference (
        const double eps,
        resizable_tensor& dest,
        const tensor& src,
        const tensor& gamma, 
        const tensor& beta,
        const tensor& running_means,
        const tensor& running_variances
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::batch_normalize_conv_inference(eps,dest,src,gamma,beta,running_means,running_variances);
#else
        cpu::batch_normalize_conv_inference(eps,dest,src,gamma,beta,running_means,running_variances);
#endif
    }

    void batch_normalize_conv (
        const double eps,
        resizable_tensor& dest,
        resizable_tensor& means,
        resizable_tensor& vars,
        const double averaging_factor,
        resizable_tensor& running_means,
        resizable_tensor& running_variances,
        const tensor& src,
        const tensor& gamma, 
        const tensor& beta 
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::batch_normalize_conv(eps,dest,means,vars,averaging_factor,running_means,running_variances,src,gamma,beta);
#else
        cpu::batch_normalize_conv(eps,dest,means,vars,averaging_factor,running_means,running_variances,src,gamma,beta);
#endif
    }

    void batch_normalize_conv_gradient (
        const double eps,
        const tensor& gradient_input,
        const tensor& means,
        const tensor& invstds,
        const tensor& src,
        const tensor& gamma,
        tensor& src_grad,
        tensor& gamma_grad, 
        tensor& beta_grad 
    )
    {
             
#ifdef DLIB_USE_CUDA
        cuda::batch_normalize_conv_gradient(eps,gradient_input, means, invstds, src, gamma, src_grad, gamma_grad, beta_grad);
#else
        cpu::batch_normalize_conv_gradient(eps,gradient_input, means, invstds, src, gamma, src_grad, gamma_grad, beta_grad);
#endif
    }

// ----------------------------------------------------------------------------------------

    void threshold (
        tensor& data,
        float thresh
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::threshold(data,thresh);
#else
        cpu::threshold(data,thresh);
#endif
    }

    void dot (
        const tensor& a,
        const tensor& b,
        tensor& result,
        size_t idx
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::dot(a,b,result,idx);
#else
        cpu::dot(a,b,result,idx);
#endif
    }

// ----------------------------------------------------------------------------------------

    void add(
        float beta,
        tensor& dest,
        float alpha,
        const tensor& src
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::add(beta,dest,alpha,src);
#else
        cpu::add(beta,dest,alpha,src);
#endif
    }

// ----------------------------------------------------------------------------------------

    void add (
        tensor& dest,
        const tensor& src1,
        const tensor& src2
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::add(dest, src1, src2);
#else
        cpu::add(dest, src1, src2);
#endif
    }

// ----------------------------------------------------------------------------------------

    void assign_conv_bias_gradient (
        tensor& grad,
        const tensor& gradient_input
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::assign_conv_bias_gradient(grad,gradient_input);
#else
        cpu::assign_conv_bias_gradient(grad,gradient_input);
#endif
    }

// ----------------------------------------------------------------------------------------

    void assign_bias_gradient (
        tensor& grad,
        const tensor& gradient_input
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::assign_bias_gradient(grad,gradient_input);
#else
        cpu::assign_bias_gradient(grad,gradient_input);
#endif
    }

// ----------------------------------------------------------------------------------------


// ----------------------------------------------------------------------------------------

    class tensor_conv
    {
    public:
        tensor_conv(const tensor_conv&) = delete;
        tensor_conv& operator=(const tensor_conv&) = delete;

        tensor_conv() {}

        void clear(
        ) { impl.clear(); }

        void operator() (
            resizable_tensor& output,
            const tensor& data,
            const tensor& filters,
            int stride_y,
            int stride_x,
            int padding_y,
            int padding_x
        ) { impl(output,data,filters,stride_y,stride_x,padding_y,padding_x); }
        /*!
            requires
                - stride_y > 0
                - stride_x > 0
                - 0 <= padding_y < filters.nr()
                - 0 <= padding_x < filters.nc()
                - is_same_object(output,data) == false
                - is_same_object(output,filters) == false
                - filters.k() == data.k()
                - filters.nr() <= src.nr() + 2*padding_y
                - filters.nc() <= src.nc() + 2*padding_x
            ensures
                - convolves filters over data.  
                - filters contains filters.num_samples() filters. 
                - #output.num_samples() == data.num_samples()
                - #output.k() == filters.num_samples()
                - #output.nr() == 1+(data.nr() + 2*padding_y - filters.nr())/stride_y
                - #output.nc() == 1+(data.nc() + 2*padding_x - filters.nc())/stride_x
        !*/

        void get_gradient_for_data (
            const tensor& gradient_input, 
            const tensor& filters,
            tensor& data_gradient
        ) { impl.get_gradient_for_data(gradient_input,filters,data_gradient); }
        /*!
            requires
                - filters has the same dimensions as the filters object given to the last
                  call to operator().
                - data_gradient has the same dimensions as the data object given to the last
                  call to operator().
                - gradient_input has the same dimensions as the last output of operator().
                - is_same_object(data_gradient,filters) == false
                - is_same_object(data_gradient,gradient_input) == false
            ensures
                - let OUT be the output of (*this)(OUT,data,filters,sx,sy).
                - let f(data,filters) == dot(OUT, gradient_input)
                - This function finds the gradient of f() with respect to data and adds
                  this gradient to data_gradient.
        !*/

        void get_gradient_for_filters (
            const tensor& gradient_input, 
            const tensor& data,
            tensor& filters_gradient
        ) { impl.get_gradient_for_filters(gradient_input,data,filters_gradient); }
        /*!
            requires
                - filters_gradient has the same dimensions as the filters object given to
                  the last call to operator().
                - data has the same dimensions as the data object given to the last call to
                  operator().
                - gradient_input has the same dimensions as the last output of operator().
                - is_same_object(filters_gradient,data) == false
                - is_same_object(filters_gradient,gradient_input) == false
            ensures
                - let OUT be the output of (*this)(OUT,data,filters,sx,sy).
                - let f(data,filters) == dot(OUT, gradient_input)
                - This function finds the gradient of f() with respect to filters and assigns 
                  this gradient to filters_gradient.
        !*/

    private:
#ifdef DLIB_USE_CUDA
        cuda::tensor_conv impl;
#else
        cpu::tensor_conv impl;
#endif

    };

// ----------------------------------------------------------------------------------------

    class pooling
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                The pooling object is a tool for performing spatial pooling over a tensor.
                It can be configured to do either max or average pooling.
        !*/
    public:

        pooling(const pooling&) = delete;
        pooling& operator=(const pooling&) = delete;

        //pooling (
        //) = default;
        pooling(){}

        void clear(
        ) { impl.clear(); }

        void setup_max_pooling(
            int window_height,
            int window_width,
            int stride_y,
            int stride_x,
            int padding_y,
            int padding_x
        ) { impl.setup_max_pooling(window_height, window_width, stride_y, stride_x, padding_y, padding_x); }
        /*!
            requires
                - window_height > 0
                - window_width > 0
                - stride_y > 0
                - stride_x > 0
                - 0 <= padding_y < window_height
                - 0 <= padding_x < window_width
            ensures
                - When you call operator() it will do max pooling with the given
                  parameters.
        !*/

        void setup_avg_pooling(
            int window_height,
            int window_width,
            int stride_y,
            int stride_x,
            int padding_y,
            int padding_x
        ) { impl.setup_avg_pooling(window_height, window_width, stride_y, stride_x, padding_y, padding_x); }
        /*!
            requires
                - window_height > 0
                - window_width > 0
                - stride_y > 0
                - stride_x > 0
                - 0 <= padding_y < window_height
                - 0 <= padding_x < window_width
            ensures
                - When you call operator() it will do average pooling with the given
                  parameters.
        !*/

        bool does_max_pooling(
        ) const { return impl.does_max_pooling(); }

        void operator() (
            resizable_tensor& dest,
            const tensor& src
        ) { impl(dest, src); }
        /*!
            requires
                - is_same_object(dest,src) == false
                - either setup_max_pooling() or setup_avg_pooling() has been called.
                - window_width  <= src.nc() + 2*padding_x
                - window_height <= src.nr() + 2*padding_y
            ensures
                - #dest.num_samples() == src.num_samples()
                - #dest.k() == src.k()
                - #dest.nr() == 1 + (src.nr() + 2*padding_y - window_height)/stride_y
                - #dest.nc() == 1 + (src.nc() + 2*padding_x - window_width)/stride_x
                - WINDOW == centered_rect(x*stride_x + window_width/2 - padding_x,
                                          y*stride_y + window_height/2 - padding_y,
                                          window_width,
                                          window_height)
                - for all valid s, k, r, and c:
                    - if (does_max_pooling()) then
                        - image_plane(#dest,s,k)(r,c) == max(subm_clipped(image_plane(src,s,k),WINDOW(c,r)))
                    - else
                        - image_plane(#dest,s,k)(r,c) == mean(subm_clipped(image_plane(src,s,k),WINDOW(c,r)))
        !*/

        void get_gradient(
            const tensor& gradient_input, 
            const tensor& dest,
            const tensor& src,
            tensor& grad 
        ) { impl.get_gradient(gradient_input, dest, src, grad); }
        /*!
            requires
                - have_same_dimensions(gradient_input,dest) == true
                - have_same_dimensions(src,grad) == true
                - dest contains the result of calling (*this)(dest,src)
                - is_same_object(grad,gradient_input) == false
                - is_same_object(grad,dest) == false
                - is_same_object(grad,src) == false
            ensures
                - Recalling that dest is the output of (*this)(dest,src),
                  let f(src) == dot(gradient_input,dest)
                - Then this function computes the gradient of f() with respect to src and
                  adds it to grad.
        !*/

        private:
#ifdef DLIB_USE_CUDA
        cuda::pooling impl;
#else
        cpu::pooling impl;
#endif
    };

// ----------------------------------------------------------------------------------------

    // ----------------------------------------------------------------------------------------

    void softmax (
        tensor& dest,
        const tensor& src
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::softmax(dest,src);
#else
        cpu::softmax(dest,src);
#endif
    }

    void softmax_gradient (
        tensor& grad,
        const tensor& dest,
        const tensor& gradient_input
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::softmax_gradient(grad, dest, gradient_input);
#else
        cpu::softmax_gradient(grad, dest, gradient_input);
#endif
    }

// ----------------------------------------------------------------------------------------

    void sigmoid (
        tensor& dest,
        const tensor& src
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::sigmoid(dest,src);
#else
        cpu::sigmoid(dest,src);
#endif
    }

    void sigmoid_gradient (
        tensor& grad,
        const tensor& dest,
        const tensor& gradient_input
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::sigmoid_gradient(grad, dest, gradient_input);
#else
        cpu::sigmoid_gradient(grad, dest, gradient_input);
#endif
    }

// ----------------------------------------------------------------------------------------

    void relu (
        tensor& dest,
        const tensor& src
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::relu(dest,src);
#else
        cpu::relu(dest,src);
#endif
    }

    void relu_gradient (
        tensor& grad,
        const tensor& dest,
        const tensor& gradient_input
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::relu_gradient(grad, dest, gradient_input);
#else
        cpu::relu_gradient(grad, dest, gradient_input);
#endif
    }

// ----------------------------------------------------------------------------------------

    void prelu (
        tensor& dest,
        const tensor& src,
        const tensor& param
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::prelu(dest, src, param);
#else
        cpu::prelu(dest, src, param);
#endif
    }

    void prelu_gradient (
        tensor& grad,
        const tensor& src,
        const tensor& gradient_input,
        const tensor& param,
        tensor& params_grad 
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::prelu_gradient(grad, src, gradient_input, param, params_grad);
#else
        cpu::prelu_gradient(grad, src, gradient_input, param, params_grad);
#endif
    }

// ----------------------------------------------------------------------------------------

    void tanh (
        tensor& dest,
        const tensor& src
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::tanh(dest,src);
#else
        cpu::tanh(dest,src);
#endif
    }

    void tanh_gradient (
        tensor& grad,
        const tensor& dest,
        const tensor& gradient_input
    )
    {
#ifdef DLIB_USE_CUDA
        cuda::tanh_gradient(grad, dest, gradient_input);
#else
        cpu::tanh_gradient(grad, dest, gradient_input);
#endif
    }


// ----------------------------------------------------------------------------------------

    class enable_peer_access
        {
        public:
            enable_peer_access() = delete;
            enable_peer_access(const enable_peer_access&) = delete;
            enable_peer_access& operator=(const enable_peer_access&) = delete;
            enable_peer_access( int, int ){}
            enable_peer_access( const tensor&, const tensor& ) {}
        };
    inline void set_device (
                int id
            )
        {
                DLIB_CASSERT(id == 0, " cuda::set_device(id) called with an invalid device id.");
        }

    inline int get_device ()
    {
        return 0;
    }

    class raii_set_device
        {
        public:
            raii_set_device() = delete;
            raii_set_device(const raii_set_device&) = delete;
            raii_set_device& operator=(const raii_set_device&) = delete;

            raii_set_device(int dev)
            {
                prev_dev = get_device();
                set_device(dev);
            }

            raii_set_device(const tensor& dev)
            {
                prev_dev = get_device();
                set_device(dev.device_id());
            }

            void operator() (int dev)
            {
                set_device(dev);
            }

            void operator() (const tensor& dev)
            {
                set_device(dev.device_id());
            }

            ~raii_set_device() noexcept(false)
            {
                set_device(prev_dev);
            }

        private:
            int prev_dev;
        };

    class multi_device_tensor_averager
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This object is a tool for very quickly averaging a bunch of tensors
                together.
        !*/
    public:

        multi_device_tensor_averager(const multi_device_tensor_averager&) = delete;
        multi_device_tensor_averager& operator=(const multi_device_tensor_averager&) = delete;

        multi_device_tensor_averager() = default;

        inline bool can_access_peer (int , int )
        { return false; }
        inline bool can_access_peer (const tensor& , const tensor& )
        { return false; }

        void set(
            std::vector<tensor*> items
        )
        /*!
            requires
                - All the tensors in items are the same size
            ensures
                - When you call average() we will average the tensors in items.
                - It's important that the tensors already be allocated to their devices
                  before you call set().  This is because set() will setup the types of
                  between device transfers now and use them when you call average().  
        !*/
        {
            using namespace  cuda;
            accessible_groups.clear();
            epa.clear();
            if (items.size() < 1)
                return;

            scale = 1.0/items.size();

            // split item into groups of accessible devices
            std::vector<tensor*> group, unused;
            while(items.size() > 0)
            {
                group.push_back(items[0]);
                for(size_t i = 1; i < items.size(); ++i)
                {
                    if (can_access_peer(*items[0], *items[i]))
                        group.push_back(items[i]);
                    else
                        unused.push_back(items[i]);
                }
                accessible_groups.push_back(group);
                unused.swap(items);
                unused.clear();
                group.clear();
            }
            for (auto&& g : accessible_groups)
            {
                for (size_t i = 1; i < g.size(); ++i)
                {
                    epa.emplace_back(new enable_peer_access(*g[0], *g[i]));
                }
            }
        }

        size_t num_device_groups(
        ) const { return accessible_groups.size(); }
        /*!
            ensures
                - The devices given to set() are grouped together when they can directly
                  access each other using GPUDirect.  This function returns the number of
                  such groups.  For example, if all devices can directly access each other
                  then the number of groups is 1.
        !*/

        void average()
        /*!
            requires
                - All the devices have stopped writing to the tensors given to set().  So
                  you should probably call cudaDeviceSynchronize() on each of the relevant
                  devices before calling average().
            ensures
                - Computes the average of all the tensors given to set() and then sets them
                  all equal to the average.
        !*/
        {
            using namespace  cuda;


            // First we average things within each group
            for (auto&& g : accessible_groups)
            {
                raii_set_device set_dev(*g[0]);
                if (g.size() == 1)
                    tt::affine_transform(*g[0], *g[0], scale);
                else 
                    tt::affine_transform(*g[0], *g[0], *g[1], scale, scale);

                for (size_t i = 2; i < g.size(); ++i)
                    tt::affine_transform(*g[0], *g[0], *g[i], 1, scale);
            }

            if (accessible_groups.size() > 1)
            {
                tensor& total_avg = *accessible_groups[0][0];
                raii_set_device set_dev(total_avg);
                accum_buffer.copy_size(total_avg);
                // now we need to average things across groups
                for (size_t i = 1; i < accessible_groups.size(); ++i)
                {
                    memcpy(accum_buffer, *accessible_groups[i][0]);
                    tt::add(total_avg, total_avg, accum_buffer);
                }

                // Now total_avg has the final average in it.  So we need to send
                // copies of it back to each of the groups.
                for (size_t i = 1; i < accessible_groups.size(); ++i)
                {
                    memcpy(*accessible_groups[i][0], total_avg);
                }
            }


            // Now propagate averages back out to each element using point to point
            // communication inside a group.
            for (auto&& g : accessible_groups)
            {
                raii_set_device set_dev(*g[0]);
                for (size_t i = 1; i < g.size(); ++i)
                    memcpy(*g[i], *g[0]); 
            }
        }

    private:
        std::vector<std::unique_ptr< enable_peer_access>> epa;
        std::vector<std::vector<tensor*>> accessible_groups;
        float scale;

        resizable_tensor accum_buffer;
    };
    // ----------------------------------------------------------------------------------------

        void copy_tensor(
                tensor& dest,
                size_t dest_k_offset,
                const tensor& src,
                size_t src_k_offset,
                size_t count_k
        )
        {
#ifdef DLIB_USE_CUDA
            cuda::copy_tensor(dest, dest_k_offset, src, src_k_offset, count_k);
#else
            cpu::copy_tensor(dest, dest_k_offset, src, src_k_offset, count_k);
#endif
        };
        /*!
            requires
                - dest.nc() == src.nc()
                - dest.nr() == src.nr()
                - dest.num_samples() == src.num_samples()
                - dest.k() - dest_k_offset >= count_k
                - src.k() - src_k_offset >= count_k
                - is_same_object(dest,src) == false
            ensures
                - performs: dest[i, k + dest_k_offset, r, c] = src[i, k + src_k_offset, r, c], where k in [0..count_k]
                  Copies content of each sample from src in to corresponding place of sample at dest.
        !*/

// ----------------------------------------------------------------------------------------

}//}

//#ifdef NO_MAKEFILE
#include "tensor_tools.cpp"
//#endif

#endif // DLIB_TeNSOR_TOOLS_H_


