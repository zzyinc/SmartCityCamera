#pragma once

#include <memory>
#include <string>
#include <vector>

// OpenCV
#include "opencv2/core.hpp"

// FaceRecogSDK
#include "face_tracker.h"
#include "face_feature.h"

// 人脸质量评估相关的阈值
#define FRS_RECOG_MIN_FACE_SIZE 96
#define FRS_RECOG_PITCH_THRESHOLD 20
#define FRS_RECOG_YAW_THRESHOLD 25
#define FRS_RECOG_ROLL_THRESHOLD 15

// 人脸特征长度，不可修改
#define FRS_FEATURE_LENGTH 512

// 人脸检测返回值
struct FaceDetectResult {
    int bbox[4];        // 人脸的 BBox，[0,1] 为左上角，[2,3] 为右下角
    int face_mask;      // 佩戴口罩的标记，0-佩戴口罩，1-未佩戴口罩
    int quality;        // 人脸质量综合打分（检测置信度、人脸像素、人脸角度），百分制
    float score;        // 人脸检测置信度
};

/*
 * 人脸SDK接口
 */
class FaceSDK {
public:

    /*
     * 初始化 SDK
     *
     * @params [license_path] - 传入离线 License 文件地址
     */
    explicit FaceSDK(const char* license_path);
    ~FaceSDK();

    /*
     * 载入图片 
     *
     * @params [bgr_data] - 输入 8bit 位图，色彩模式为 BGR，通道顺序为 HWC 
     * @params [image_width] - 输入图片的 Width
     * @params [image_height] - 输入图片的 Height
     * @params [bigger_face_mode] - True 表示输入为大头照，用于底库建库；False 表示输入为普通照片
     * @params face_num - 图片中人脸的数目
     *
     * @return - 0 表示正常返回，其余为错误
     */
    int loadRawImage( 
            const unsigned char* bgr_data,
            size_t image_width,
            size_t image_height,
            bool bigger_face_mode,
            int* face_num);

    /*
     * 载入图片 
     *
     * @params [jpg_data] - 输入 JPG/PNG 格式的图片
     * @params [jpg_size] - 输入图片的长度
     * @params [bigger_face_mode] - True 表示输入为大头照，用于底库建库；False 表示输入为普通照片
     * @params face_num - 图片中人脸的数目
     *
     * @return - 0 表示正常返回，其余为错误
     */
    int loadEncodedImage( 
            const unsigned char* jpg_data,
            size_t jpg_size,
            bool bigger_face_mode,
            int* face_num);

    /*
     * 获取载入图片的人脸检测结果
     *
     * @params [face_detects] - 人脸检测结果 FaceDetectResult 数组，空间由外部分
     *                          配，其长度至少应 到 load_image 时返回的 face_num 
     *                          人脸数量
     *
     * @return - >=0 表示正常返回，且返回值为人脸的数目；<0 为错误
     **/
    int getAllFaces(
            FaceDetectResult* face_detects);

    /*
     * 获取人脸的特征值
     *
     * @params [face_idx] - 人脸检测结果中对应的人脸序号 
     * @params [face_feature] - 人脸特征值数组，长度为 FRS_FEATURE_LENGTH 的 float
     *                          数组，空间由外部分配
     *
     * @return - 0 表示正常返回，其余为错误
     **/
    int getFaceFeature(
            int face_idx,
            float* face_feature);

    /*
     * 获取经过对齐的人脸图片
     *
     * @params [face_idx] - 人脸检测结果中对应的人脸序号 
     * @params [face_aligned] - 经过对齐的人脸图片
     *
     * @return - 0 表示正常返回，其余为错误
     **/
    int getFaceImage(
            int face_idx,
            cv::Mat* face_aligned);

    /*
     * 获取人脸相似度打分
     *
     * @params [face_feature0] - 人脸特征值数组，长度为 FRS_FEATURE_LENGTH 
     * @params [face_feature1] - 人脸特征值数组，长度为 FRS_FEATURE_LENGTH 
     * 
     * @return - 相似度分值，0~100 分
     */
    int faceSimilarity(
        const float* face_feature0,
        const float* face_feature1);

private:
    int loadCVImage_(
            bool bigger_face_mode,
            int* face_num);

    void alignFace_(
            const cv::Mat& src, 
            const float* xy_5points, 
            const int crop_size, 
            cv::Mat* dst);

    int calcFaceQuality_(
            const facerecogsdk::FaceStatus& face);

    facerecogsdk::FaceTracker* face_tracker_;
	facerecogsdk::FaceFeature* face_feature_;

    facerecogsdk::FaceDetectParam track_param_;

    cv::Mat image_;
    std::vector<facerecogsdk::FaceStatus> face_results_;
};
