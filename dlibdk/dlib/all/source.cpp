// Copyright (C) 2006  Davis E. King (davis@dlib.net)
// License: Boost Software License   See LICENSE.txt for the full license.
#ifndef DLIB_ALL_SOURCe_
#define DLIB_ALL_SOURCe_

#if defined(DLIB_ALGs_) || defined(DLIB_PLATFORm_)
#include "../dlib_basic_cpp_build_tutorial.txt"
#endif

// ISO C++ code
#include "../base64/base64_kernel_1.cpp"
#include "../bigint/bigint_kernel_1.cpp"
#include "../bigint/bigint_kernel_2.cpp"
//#include "../bit_stream/bit_stream_kernel_1.cpp"
//#include "../entropy_decoder/entropy_decoder_kernel_1.cpp"
#include "../entropy_decoder/entropy_decoder_kernel_2.cpp"
//#include "../entropy_encoder/entropy_encoder_kernel_1.cpp"
#include "../entropy_encoder/entropy_encoder_kernel_2.cpp"
//#include "../md5/md5_kernel_1.cpp"
#include "../tokenizer/tokenizer_kernel_1.cpp"
#include "../unicode/unicode.cpp"
//#include "../data_io/image_dataset_metadata.cpp"

#ifndef DLIB_ISO_CPP_ONLY
// Code that depends on OS specific APIs

// include this first so that it can disable the older version
// of the winsock API when compiled in windows.
//#include "../sockets/sockets_kernel_1.cpp"
//#include "../bsp/bsp.cpp"

//#include "../dir_nav/dir_nav_kernel_1.cpp"
//#include "../dir_nav/dir_nav_kernel_2.cpp"
//#include "../dir_nav/dir_nav_extensions.cpp"
//#include "../linker/linker_kernel_1.cpp"
#include "../logger/extra_logger_headers.cpp"
#include "../logger/logger_kernel_1.cpp"
#include "../logger/logger_config_file.cpp"
#include "../misc_api/misc_api_kernel_1.cpp"
#include "../misc_api/misc_api_kernel_2.cpp"
#include "../sockets/sockets_extensions.cpp"
#include "../sockets/sockets_kernel_2.cpp"
//#include "../sockstreambuf/sockstreambuf.cpp"
//#include "../sockstreambuf/sockstreambuf_unbuffered.cpp"
//#include "../server/server_kernel.cpp"
//#include "../server/server_iostream.cpp"
//#include "../server/server_http.cpp"
#include "../threads/multithreaded_object_extension.cpp"
#include "../threads/threaded_object_extension.cpp"
//#include "../threads/threads_kernel_1.cpp"
#include "../threads/threads_kernel_2.cpp"
#include "../threads/threads_kernel_shared.cpp"
//#include "../threads/thread_pool_extension.cpp"
#include "../timer/timer.cpp"
#include "../stack_trace.cpp"


#endif // DLIB_ISO_CPP_ONLY


#define DLIB_ALL_SOURCE_END

#endif // DLIB_ALL_SOURCe_

