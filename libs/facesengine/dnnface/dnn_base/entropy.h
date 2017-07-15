#ifndef DLIB_ENTROPY_
#define DLIB_ENTROPY_


#include "entropy_decoder_kernel_2.h"
#include "entropy_decoder_kernel_2.cpp"
#include "entropy_decoder_model_kernel_5.h"
#include "entropy_encoder_kernel_2.h"
#include "entropy_encoder_kernel_2.cpp"
#include "entropy_encoder_model_kernel_5.h"




	class entropy_decoder
    {
        entropy_decoder() {}


    public:

        typedef     entropy_decoder_kernel_2
                    kernel_2a;
    };

    template <
        unsigned long alphabet_size,
        typename entropy_decoder
        >
    class entropy_decoder_model
    {
        entropy_decoder_model() {}

    public:

        // kernel_5       
        typedef     entropy_decoder_model_kernel_5<alphabet_size,entropy_decoder,200000,4>
                    kernel_5a;

    };

    class entropy_encoder
    {
        entropy_encoder() {}


    public:

        typedef     entropy_encoder_kernel_2
                    kernel_2a;

    };

    template <
        unsigned long alphabet_size,
        typename entropy_encoder
        >
    class entropy_encoder_model
    {
        entropy_encoder_model() {}

    public:        
        typedef     entropy_encoder_model_kernel_5<alphabet_size,entropy_encoder,200000,4>
                    kernel_5a;


    
    };

#endif