//
// Created by alvinzheng on 2019/11/29
// Writed by junezhang on 2019/12/17
// Modified by alvinzheng on 2020/02/17
//

#ifndef __FACE_RECOG_SDK_LIVE_IR_H__
#define __FACE_RECOG_SDK_LIVE_IR_H__

#ifdef _MSC_VER
#ifdef FACE_RECOG_SDK_EXPORTS
#define FACE_RECOG_API __declspec(dllexport)
#else
#define FACE_RECOG_API 
#endif
#else
#define FACE_RECOG_API __attribute__ ((visibility("default")))
#endif


#if __ANDROID_API__ >= 9
#include <android/asset_manager.h>
#endif // __ANDROID_API__ >= 9

#include <string>


namespace facerecogsdk
{
    struct IRImage {
        int width;
        int height;
        unsigned char* data;  //IR (RGB format) image without rotation
    };

    class FaceLiveIR {
    public:
        FACE_RECOG_API FaceLiveIR(
                const std::string& model_path, 
                const std::string& config_file_name, 
                const int gpu_id = -1);

#if __ANDROID_API__ >= 9
        FACE_RECOG_API FaceLiveIR(
                AAssetManager* mgr, 
                const std::string& modelpath, 
                const std::string& config_file_name, 
                const int gpu_id = -1);
#endif // __ANDROID_API__ >= 9

        FACE_RECOG_API ~FaceLiveIR();

        FACE_RECOG_API static void set_version(
                const std::string& model_path, 
                const std::string& config_file_name);

#if __ANDROID_API__ >= 9
        FACE_RECOG_API static void set_version(
                AAssetManager* mgr, 
                const std::string& model_path, 
                const std::string& config_file_name);
#endif // __ANDROID_API__ >= 9

        FACE_RECOG_API static std::string version();

        FACE_RECOG_API float DetectRGB(
                const float* color_xy_5points, 
                unsigned char* color_rgb, 
                const int color_width, 
                const int color_height, 
                const int color_orientation,
                const float* ir_xy_5points, 
                unsigned char* ir_rgb, 
                const int ir_width, 
                const int ir_height, 
                const int ir_orientation, 
                IRImage* rectified_ir_image=nullptr);

        FACE_RECOG_API float DetectBGR(
                const float* color_xy_5points, 
                unsigned char* color_bgr, 
                const int color_width, 
                const int color_height, 
                const int color_orientation,
                const float* ir_xy_5points, 
                unsigned char* ir_bgr, 
                const int ir_width, 
                const int ir_height, 
                const int ir_orientation, 
                IRImage* rectified_ir_image = nullptr);

        FACE_RECOG_API float DetectYUV(
                const float* color_xy_5points, 
                unsigned char* color_yuv, 
                const int color_width, 
                const int color_height, 
                const int color_orientation,
                const float* ir_xy_5points, 
                unsigned char* ir_yuv, 
                const int ir_width, 
                const int ir_height, 
                const int ir_orientation, 
                IRImage* rectified_ir_image = nullptr);

        FACE_RECOG_API float __DetectBGR(
                unsigned char* ir_bgr, 
                const int ir_width, 
                const int ir_height, 
                const int ir_orientation); 

    private:
        FaceLiveIR(const FaceLiveIR&) = delete;
        const FaceLiveIR& operator=(const FaceLiveIR&) = delete;

    private:
        void* impl_;
    };

} // end of namespace facerecogsdk
#endif  // __FACE_RECOG_SDK_LIVE_IR_H__



