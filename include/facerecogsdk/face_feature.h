//
// Created by alvinzheng on 2020/03/04
//

#ifndef __FACE_RECOG_SDK_FEATURE_H__
#define __FACE_RECOG_SDK_FEATURE_H__

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
    class FaceFeature {
    public:
        FACE_RECOG_API FaceFeature(
                const std::string& model_path, 
                const std::string& config_file_name, 
                const int gpu_id = -1);

#if __ANDROID_API__ >= 9
        FACE_RECOG_API FaceFeature(
                AAssetManager* mgr, 
                const std::string& model_path, 
                const std::string& config_file_name, 
                const int gpu_id = -1);
#endif // __ANDROID_API__ >= 9
        FACE_RECOG_API ~FaceFeature();

        FACE_RECOG_API static void set_version(
                const std::string &model_path, 
                const std::string &config_file_name);

#if __ANDROID_API__ >= 9
        FACE_RECOG_API static void set_version(
                AAssetManager* mgr, 
                const std::string &model_path, 
                const std::string &config_file_name);
#endif // __ANDROID_API__ >= 9

        FACE_RECOG_API static std::string version();

        FACE_RECOG_API int ExtractRGB(
                const float* xy_5points, 
                unsigned char* rgb_data, 
                const int width, 
                const int height, 
                const int orientation, 
                float* feature);
        FACE_RECOG_API int ExtractBGR(
                const float* xy_5points, 
                unsigned char* bgr_data, 
                const int width, 
                const int height, 
                const int orientation, 
                float* feature);
        FACE_RECOG_API int ExtractYUV(
                const float* xy_5points, 
                unsigned char* yuv_data, 
                const int width, 
                const int height, 
                const int orientation, 
                float* feature);

    private:
        FaceFeature(const FaceFeature&) = delete;
        const FaceFeature &operator=(const FaceFeature&) = delete;

    private:
        void* impl_;
    };
} // end of namespace facerecogsdk

#endif  // __FACE_RECOG_SDK_FEATURE_H__
