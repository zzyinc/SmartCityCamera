#include <array>
#include <cstdio>
#include <ctime>
#include <map>
#include <string>
#include <vector>

#include <unistd.h>
#include <dirent.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include "plate_sdk.h"

#define LICENSE_PATH "./face_demo_license.key"
#define INPUT_IMAGES_PATH "../data/plates"
#define OUTPUT_IMAGES_PATH "../data/plates_result"

static void get_files(
        const char* dir, 
        std::vector<std::string>* names);

int main(int argc, char **argv) {
    PlateSDK plate_sdk(LICENSE_PATH);

    // 读取车牌照目录
    std::vector<std::string> plate_names;
    get_files(INPUT_IMAGES_PATH, &plate_names);

    printf("Scan plate dir... [file_nums=%zu]\n", plate_names.size());

    for (auto &plate_name : plate_names) {
        char file_name[128];
        snprintf(
                file_name, 
                sizeof(file_name), 
                "%s/%s",
                INPUT_IMAGES_PATH,
                plate_name.c_str());

        cv::Mat frame;
        std::vector<PlateResult> plate_results; 

        double t = clock(); 

        int res = plate_sdk.getPlateFile(
                file_name,
                &frame,
                &plate_results);

        if (0 != res) {
            printf(
                    "\tFailed to get plate. [file_name='%s' res=%d]\n",
                    file_name,
                    res);
            break;
        }

        printf(
                "File: %s (%d,%d) spent %.3fms\n", 
                file_name, 
                frame.size().width, 
                frame.size().height, 
                (clock()-t)/1000.0);
        for (auto& result : plate_results) {
            printf(
                    "\t%s [%d,%d,%d,%d] %.4f\n",
                    result.text.c_str(),
                    result.bbox[0],
                    result.bbox[1],
                    result.bbox[2],
                    result.bbox[3],
                    result.score);

            cv::rectangle(
                    frame,
                    cv::Point(result.bbox[0], result.bbox[1]),
                    cv::Point(result.bbox[2], result.bbox[3]),
                    cv::Scalar(187, 236, 93),
                    6 /* line width */);

            cv::putText(
                    frame,
                    result.text.c_str(),
                    cv::Point(result.bbox[0], result.bbox[1]),
                    cv::FONT_HERSHEY_SIMPLEX,
                    1, /* font scale */
                    cv::Scalar(0, 0, 255), /* font color */
                    2 /* font thickness */);
        }

        snprintf(
                file_name, 
                sizeof(file_name), 
                "%s/%s", 
                OUTPUT_IMAGES_PATH,
                plate_name.c_str());

        cv::imwrite(file_name, frame); 

    }

    return 0;
}

static void get_files(
        const char* folder_path, 
        std::vector<std::string>* names) {
    struct dirent* de = NULL;
    DIR* dir = NULL;

    dir = opendir(folder_path);
    if (dir == NULL) {
        printf("Failed to open folder. [path='%s']\n", folder_path);
        return;
    }
    
    while ((de = readdir(dir))) {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
            continue;
        names->push_back(de->d_name);
    }
    
    closedir(dir);
}

