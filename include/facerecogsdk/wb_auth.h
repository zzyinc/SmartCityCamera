//
// Created by alvinzheng on 2020/02/12
//

#ifndef __FACE_RECOG_SDK_AUTH_H__
#define __FACE_RECOG_SDK_AUTH_H__

#include <string>

#include "common.h"

namespace facerecogsdk {
    typedef struct {
        int status;
        std::string failed_resaon;
        long days_left;
    } WBLicenseInfo;

    FACE_RECOG_API std::string get_failed_reason(int error_code);
    FACE_RECOG_API std::string version();

    // for USE_ONLINE_LICENSE - ON
    FACE_RECOG_API int get_license_by_url(
            const std::string& license_path, 
            const std::string& pubkey_path, 
            const std::string& config_path);

    FACE_RECOG_API int license_init(
            const std::string& license_path, 
            const std::string& pubkey_path, 
            WBLicenseInfo* wb_license_info, 
            const std::string& project_name = "default");
    
    // for USE_ONLINE_LICENSE - OFF 
    FACE_RECOG_API int license_init_offline(
            const std::string& license_cipher_text_b64, 
            WBLicenseInfo* wb_license_info, 
            const std::string project_name = "default");


    FACE_RECOG_API bool is_license_authorized(WBLicenseInfo* wb_license_info);
} // end of namespace facerecogsdk

#endif // __FACE_RECOG_SDK_AUTH_H__
