//
// Created by alvinzheng on 2020/03/04
//

#ifndef __FACE_RECOG_SDK_QUALITY_H__
#define __FACE_RECOG_SDK_QUALITY_H__

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
    struct OutQuality {
        float blur;  // [0, 1]，模糊分数，越高代表越清晰
        float front;  // [0, 1], 正脸分数，越高代表脸越正
        float illumination; // [0, 1]，光照分数，越高代表光照越正常
    };

    class FaceQuality {
    public:
        FACE_RECOG_API FaceQuality(
                const std::string& model_path, 
                const std::string& config_file_name, 
                const int gpu_id = -1);

#if __ANDROID_API__ >= 9
        FACE_RECOG_API FaceQuality(
                AAssetManager* mgr, 
                const std::string& model_path, 
                const std::string& config_file_name, 
                const int gpu_id = -1);
#endif // __ANDROID_API__ >= 9

        FACE_RECOG_API ~FaceQuality();

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

        FACE_RECOG_API int EvaluateRGB(
                const int* bbox, 
                unsigned char* rgb_data, 
                const int width, 
                const int height, 
                const int orientation, 
                OutQuality& out_quality);
        FACE_RECOG_API int EvaluateBGR(
                const int* bbox, 
                unsigned char* bgr_data, 
                const int width, 
                const int height, 
                const int orientation, 
                OutQuality& out_quality);
        FACE_RECOG_API int EvaluateYUV(
                const int* bbox, 
                unsigned char* yuv_data, 
                const int width, 
                const int height, 
                const int orientation, 
                OutQuality& out_quality);

    private:
        FaceQuality(const FaceQuality&) = delete;
        const FaceQuality& operator=(const FaceQuality&) = delete;

    private:
        void* impl_;
    };
} // end of namespace facerecogsdk

#endif  // __FACE_RECOG_SDK_QUALITY_H__
