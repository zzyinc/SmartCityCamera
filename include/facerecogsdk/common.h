//
// Created by alvinzheng on 2019/12/03
//

#ifndef __FACE_RECOG_SDK_COMMON_H__
#define __FACE_RECOG_SDK_COMMON_H__

#ifdef _MSC_VER
#ifdef FACE_RECOG_SDK_EXPORTS
#define FACE_RECOG_API __declspec(dllexport)
#else
#define FACE_RECOG_API
#endif
#else
#define FACE_RECOG_API __attribute__ ((visibility("default")))
#endif

#include <cstring>
#include <string>
#include <vector>

#if __ARM_NEON
#include <arm_neon.h>
#endif

namespace facerecogsdk {
    // get now timestamp in ms
    FACE_RECOG_API double get_current_time();  // 毫秒

    FACE_RECOG_API int HiIVE_Resize(
            const unsigned char* srcImg, 
            const int srcWidth, 
            const int srcHeight, 
            const int srcChannels,
            const int dstWidth, 
            const int dstHeight, 
            unsigned char* dstImg, 
            const int mode);

    FACE_RECOG_API int HiIVE_cvtColor(
            const unsigned char* srcImg, 
            const int width, 
            const int height,
            unsigned char* dstImg, 
            const int mode);  // mode = 0 mean YUV2BGR, mode = 1 mean YUV2Gray

    FACE_RECOG_API std::vector<std::string> SplitByChar(const std::string &s, char delim); 

    // 简化版本的 Resize，只能进行缩小
    template <int channels>
    void resize_down_scaling_fast(
            uint8_t* input, 
            int height_in, 
            int width_in, 
            uint8_t* output,
            int height_out, 
            int width_out) {
        int32_t fl = (1 << 13); 

        int32_t step_width = (int32_t)(width_in / width_out * fl);
        int32_t step_height = (int32_t)(height_in / height_out * fl);

        for (int i = 0; i < height_out; ++i) {
            int32_t step_y = (int32_t)(step_height * i / fl);
            const uint8_t* src = input + step_y * width_in * channels;

            for (int j = 0; j < width_out; ++j) {
                int32_t step_x = (int32_t)(step_width * j / fl);
                const uint8_t* src_row = src + step_x * channels;

                memcpy(output, src_row, channels);

                output += channels;
            }
        }
    }

    // 简化版本的 Resize，只能进行缩小
    template <int channels>
    void resize_down_scaling(
            uint8_t* input, 
            int height_in, 
            int width_in, 
            uint8_t* output,
            int height_out, 
            int width_out) {
        float step_width = (float)((float)width_in / width_out);
        float step_height = (float)((float)height_in / height_out);

        for (int i = 0; i < height_out; ++i) {
            int32_t step_y = (int32_t)(step_height * i);
            const uint8_t* src = input + step_y * width_in * channels;

            for (int j = 0; j < width_out; ++j) {
                int32_t step_x = (int32_t)(step_width * j);
                const uint8_t* src_row = src + step_x * channels;

                memcpy(output, src_row, channels);

                output += channels;
            }
        }
    }

    FACE_RECOG_API void warpaffine_bilinear_c3(
            const uint8_t* src, 
            int srcw, 
            int srch, 
            uint8_t* dst, 
            int w, 
            int h, 
            const float* tm, 
            int type = 0, 
            unsigned int v = 0);   

}   // namespace facerecogsdk

#endif // __FACE_RECOG_SDK_COMMON_H__

