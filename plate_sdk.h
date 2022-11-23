#pragma once

#include <memory>
#include <string> 

// OpenCV
#include "opencv2/core.hpp"

// FaceRecogSDK
#include "plate_tracker.h"

struct PlateResult {
    int bbox[4];
    float score;
    std::string text;
};

/*
 * 针对7G需求二次封装的车牌SDK接口
 */
class PlateSDK : public std::enable_shared_from_this<PlateSDK> {

public:
    /*
     * 初始化 SDK
     *
     * @params [license_path] - 传入离线 License 文件地址
     */
    explicit PlateSDK(const std::string& license_path);
    ~PlateSDK();

    /*
     * 车牌检测及识别
     *
     * @params [frame] - OpenCV的图片矩阵。输入图片长宽若非4的整数倍时，需花费额外时间进行填充操作
     * @params [plate_result] - 返回车牌检测及特征结果
     *
     * @return - 0 表示正常返回，其余为错误
     */
    int getPlate( 
            const cv::Mat& frame,
            std::vector<PlateResult>* plate_result);

    /*
     * 车牌检测及识别
     *
     * @params [file_path] - 图片文件地址，支持 jpg/png 格式。输入图片长宽若非4的整数倍时，需花费额外时间进行填充操作
     * @params [frame] - 返回读入的图片
     * @params [plate_result] - 返回车牌检测及特征结果
     *
     * @return - 0 表示正常返回，其余为错误
     */
    int getPlateFile(
            const std::string& file_path,
            cv::Mat* frame,
            std::vector<PlateResult>* plate_result);

private:
    std::unique_ptr<facerecogsdk::PlateTracker> plate_tracker_;

};
