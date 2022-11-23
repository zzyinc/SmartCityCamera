#pragma once

#define PLATE_LANDMARK_NUM 4 

namespace facerecogsdk {

struct VechileDetStatus {
    float score;
    int class_id;
    int bbox[4];
    int width;
    int height;
};

struct PlateDetStatus {
    float score;
    int class_id;
    int bbox[4];
    int width;
    int height;
    float landmarks[PLATE_LANDMARK_NUM * 2];
};

struct PlateStatus {
    // vechile detect
    VechileDetStatus vechile;

    // vechile tracking
    int track_id;

    // plate detect
    PlateDetStatus plate;
    
    // plate recognize 
    std::string text;
};

struct PlateDetectParam {
    // plate detect
    int min_plate_size = 10;  //10
    int max_plate_size = 9999;  //1024
    bool bigger_plate_mode = false;  //whether only detect max plate in image
    bool align_plate = false; 
    bool street_view_mode = false;
};

} // end of namespace facerecogsdk

