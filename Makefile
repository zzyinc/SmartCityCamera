hide := @
ECHO := echo

GCC := aarch64-linux-gnu-gcc-8 
GXX := aarch64-linux-gnu-g++-8

CXXFLAGS := \
		  	-DLINUX \
			-I./ \
			-I./include/facerecogsdk \
			-I./3rdparty/include \
			 
LIB_PATHES := \
			 -L/usr/lib \
			 -L./3rdparty/lib64 \
			 -L./lib64 \
			 
LD_FLAGS := -lpthread -ldl \
			-lopencv_imgcodecs -lopencv_video -lopencv_imgproc -lopencv_core \
			-lopencv_highgui -lopencv_calib3d -lopencv_features2d -lopencv_flann \
    	 	-lrknn_api -ldrm -lrockchip_mpp -lrockx -lrga \
			-lz -lssl -lcrypto -lffi -lpcre \
			-ljpeg -lpng16 -ljasper -lgobject-2.0 -lglib-2.0

all:
	$(GXX) face_demo.cpp face_sdk.cpp similarity_transform.cpp \
		$(CXXFLAGS) $(LIB_PATHES) \
		-lFaceRecogSDKCommon -lFaceRecogSDKFaceFeature -lFaceRecogSDKFaceTracker \
		$(LD_FLAGS) \
		-o face_demo

	$(GXX) plate_demo.cpp plate_sdk.cpp similarity_transform.cpp \
		$(CXXFLAGS) $(LIB_PATHES) \
		-lFaceRecogSDKCommon -lFaceRecogSDKPlateTracker \
		$(LD_FLAGS) \
		-fexec-charset=UTF-8 -finput-charset=UTF-8 \
		-o plate_demo

	$(hide)$(ECHO) "Build Done ..."

