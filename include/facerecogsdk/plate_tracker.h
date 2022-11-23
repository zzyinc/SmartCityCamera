#pragma once

#ifdef _MSC_VER
#ifdef FACE_RECOG_SDK_EXPORTS
#define FACE_RECOG_API __declspec(dllexport)
#else
#define FACE_RECOG_API 
#endif
#else
#define FACE_RECOG_API __attribute__ ((visibility("default")))
#endif


#include <string>
#include <vector>

#if __ANDROID_API__ >= 9
#include <android/asset_manager.h>
#endif // __ANDROID_API__ >= 9

#include "plate_status_param.h"

namespace facerecogsdk {

    class PlateTracker {
    public:
        FACE_RECOG_API PlateTracker(
                const std::string& model_path, 
                const std::string& config_file_name, 
                const PlateDetectParam& track_param, 
                const int gpu_id = -1);
#if __ANDROID_API__ >= 9
        FACE_RECOG_API PlateTracker(
                AAssetManager * mgr, 
                const std::string & model_path, 
                const std::string & config_file_name, 
                const PlateDetectParam& track_param, 
                const int gpu_id = -1);
#endif // __ANDROID_API__ >= 9
        FACE_RECOG_API ~PlateTracker();

#if __ANDROID_API__ >= 9
        FACE_RECOG_API static void set_version(
                AAssetManager * mgr, 
                const std::string & model_path, 
                const std::string & config_file_name);
#endif // __ANDROID_API__ >= 9
        FACE_RECOG_API static void set_version(
                const std::string& model_path, 
                const std::string& config_file_name);
        FACE_RECOG_API static std::string version();

        FACE_RECOG_API void set_track_param(const PlateDetectParam& track_param);
        FACE_RECOG_API PlateDetectParam track_param();

        FACE_RECOG_API int DetectRGB(
                unsigned char* rgb_data, 
                const int width, 
                const int height, 
                const int orientation, 
                const PlateDetectParam* detect_param,
                std::vector<PlateStatus>& final_detected_faces);
        FACE_RECOG_API int DetectBGR(
                unsigned char* bgr_data, 
                const int width, 
                const int height, 
                const int orientation, 
                const PlateDetectParam* detect_param, 
                std::vector<PlateStatus>& final_detected_faces);
        FACE_RECOG_API int DetectYUV(
                unsigned char* yuv_data, 
                const int width, 
                const int height, 
                const int orientation, 
                const PlateDetectParam* detect_param, 
                std::vector<PlateStatus>& final_detected_faces);

    private:
        PlateTracker(const PlateTracker&) = delete;
        const PlateTracker &operator=(const PlateTracker&) = delete;

    private:
        void* impl_;
    };

} // end of namespace facerecogsdk

