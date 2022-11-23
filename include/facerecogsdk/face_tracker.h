//
// Created by alvinzheng on 2020/03/03
//

#ifndef __FACE_RECOG_SDK_TRACKER_H__
#define __FACE_RECOG_SDK_TRACKER_H__

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

#include "face_status_param.h"

namespace facerecogsdk
{
    class FaceTracker {
    public:
        FACE_RECOG_API FaceTracker(
                const std::string& model_path, 
                const std::string& config_file_name, 
                const FaceDetectParam& track_param, 
                const int gpu_id = -1);
#if __ANDROID_API__ >= 9
        FACE_RECOG_API FaceTracker(
                AAssetManager * mgr, 
                const std::string & model_path, 
                const std::string & config_file_name, 
                const FaceDetectParam& track_param, 
                const int gpu_id = -1);
#endif // __ANDROID_API__ >= 9
        FACE_RECOG_API ~FaceTracker();

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

        FACE_RECOG_API void set_track_param(const FaceDetectParam& track_param);
        FACE_RECOG_API FaceDetectParam track_param();

        FACE_RECOG_API int DetectRGB(
                unsigned char* rgb_data, 
                const int width, 
                const int height, 
                const int orientation, 
                const FaceDetectParam* detect_param, 
                Image* rectified_data, 
                std::vector<FaceStatus>& final_detected_faces);
        FACE_RECOG_API int DetectBGR(
                unsigned char* bgr_data, 
                const int width, 
                const int height, 
                const int orientation, 
                const FaceDetectParam* detect_param, 
                Image* rectified_data, 
                std::vector<FaceStatus>& final_detected_faces);
        FACE_RECOG_API int DetectYUV(
                unsigned char* yuv_data, 
                const int width, 
                const int height, 
                const int orientation, 
                const FaceDetectParam* detect_param, 
                Image* rectified_data, 
                std::vector<FaceStatus>& final_detected_faces);

        FACE_RECOG_API int TrackRGB(
                unsigned char* rgb_data, 
                const int width, 
                const int height, 
                const int orientation, 
                Image* rectified_data, 
                std::vector<FaceStatus>& final_detected_faces);
        FACE_RECOG_API int TrackBGR(
                unsigned char* bgr_data, 
                const int width, 
                const int height, 
                const int orientation, 
                Image* rectified_data, 
                std::vector<FaceStatus>& final_detected_faces);
        FACE_RECOG_API int TrackYUV(
                unsigned char* yuv_data, 
                const int width, 
                const int height, 
                const int orientation, 
                Image* rectified_data, 
                std::vector<FaceStatus>& final_detected_faces);

    private:
        FaceTracker(const FaceTracker&) = delete;
        const FaceTracker &operator=(const FaceTracker&) = delete;

    private:
        void* impl_;
    };
} // end of namespace facerecogsdk

#endif  //__FACE_RECOG_SDK_TRACKER_H__
