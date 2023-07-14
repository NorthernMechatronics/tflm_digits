/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2023, Northern Mechatronics, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _ARDUCAM_H_
#define _ARDUCAM_H_

#ifdef __cplusplus
extern "C" {
#endif

#define TRUE  1
#define FALSE 0

struct CameraInfo {
    char* cameraId;                 /**<Model of camera module */
    int supportResolution;          /**<Resolution supported by the camera module */
    int supportSpecialEffects;      /**<Special effects supported by the camera
                                       module */
    unsigned long exposureValueMax; /**<Maximum exposure time supported by the
                                       camera module */
    unsigned int exposureValueMin;  /**<Minimum exposure time supported by the
                                       camera module */
    unsigned int gainValueMax;      /**<Maximum gain supported by the camera module */
    unsigned int gainValueMin;      /**<Minimum gain supported by the camera module */
    unsigned char supportFocus;     /**<Does the camera module support the focus function */
    unsigned char supportSharpness; /**<Does the camera module support the
                                       sharpening function */
    unsigned char deviceAddress;
};

/**
 * @enum CamStatus
 * @brief Camera status
 */
typedef enum {
    CAM_ERR_SUCCESS     = 0,  /**<Operation succeeded*/
    CAM_ERR_NO_CALLBACK = -1, /**< No callback function is registered*/
} CamStatus;

/**
 * @enum CAM_IMAGE_MODE
 * @brief Configure camera resolution
 */
typedef enum {
    CAM_IMAGE_MODE_QQVGA  = 0x00,  /**<160x120 */
    CAM_IMAGE_MODE_QVGA   = 0x01,  /**<320x240*/
    CAM_IMAGE_MODE_VGA    = 0x02,  /**<640x480*/
    CAM_IMAGE_MODE_SVGA   = 0x03,  /**<800x600*/
    CAM_IMAGE_MODE_HD     = 0x04,  /**<1280x720*/
    CAM_IMAGE_MODE_SXGAM  = 0x05,  /**<1280x960*/
    CAM_IMAGE_MODE_UXGA   = 0x06,  /**<1600x1200*/
    CAM_IMAGE_MODE_FHD    = 0x07,  /**<1920x1080*/
    CAM_IMAGE_MODE_QXGA   = 0x08,  /**<2048x1536*/
    CAM_IMAGE_MODE_WQXGA2 = 0x09,  /**<2592x1944*/
    CAM_IMAGE_MODE_96X96  = 0x0a,  /**<96x96*/
    CAM_IMAGE_MODE_128X128 = 0x0b, /**<128x128*/
    CAM_IMAGE_MODE_320X320 = 0x0c, /**<320x320*/
    /// @cond
    CAM_IMAGE_MODE_12      = 0x0d, /**<Reserve*/
    CAM_IMAGE_MODE_13      = 0x0e, /**<Reserve*/
    CAM_IMAGE_MODE_14      = 0x0f, /**<Reserve*/
    CAM_IMAGE_MODE_15      = 0x10, /**<Reserve*/
    CAM_IMAGE_MODE_NONE,
    /// @endcond
} CAM_IMAGE_MODE;

/**
 * @enum CAM_CONTRAST_LEVEL
 * @brief Configure camera contrast level
 */
typedef enum {
    CAM_CONTRAST_LEVEL_MINUS_3 = 6, /**<Level -3 */
    CAM_CONTRAST_LEVEL_MINUS_2 = 4, /**<Level -2 */
    CAM_CONTRAST_LEVEL_MINUS_1 = 2, /**<Level -1 */
    CAM_CONTRAST_LEVEL_DEFAULT = 0, /**<Level Default*/
    CAM_CONTRAST_LEVEL_1       = 1, /**<Level +1 */
    CAM_CONTRAST_LEVEL_2       = 3, /**<Level +2 */
    CAM_CONTRAST_LEVEL_3       = 5, /**<Level +3 */
} CAM_CONTRAST_LEVEL;

/**
 * @enum CAM_EV_LEVEL
 * @brief Configure camera EV level
 */
typedef enum {
    CAM_EV_LEVEL_MINUS_3 = 6, /**<Level -3 */
    CAM_EV_LEVEL_MINUS_2 = 4, /**<Level -2 */
    CAM_EV_LEVEL_MINUS_1 = 2, /**<Level -1 */
    CAM_EV_LEVEL_DEFAULT = 0, /**<Level Default*/
    CAM_EV_LEVEL_1       = 1, /**<Level +1 */
    CAM_EV_LEVEL_2       = 3, /**<Level +2 */
    CAM_EV_LEVEL_3       = 5, /**<Level +3 */
} CAM_EV_LEVEL;

/**
 * @enum CAM_STAURATION_LEVEL
 * @brief Configure camera stauration  level
 */
typedef enum {
    CAM_STAURATION_LEVEL_MINUS_3 = 6, /**<Level -3 */
    CAM_STAURATION_LEVEL_MINUS_2 = 4, /**<Level -2 */
    CAM_STAURATION_LEVEL_MINUS_1 = 2, /**<Level -1 */
    CAM_STAURATION_LEVEL_DEFAULT = 0, /**<Level Default*/
    CAM_STAURATION_LEVEL_1       = 1, /**<Level +1 */
    CAM_STAURATION_LEVEL_2       = 3, /**<Level +2 */
    CAM_STAURATION_LEVEL_3       = 5, /**<Level +3 */
} CAM_STAURATION_LEVEL;

/**
 * @enum CAM_BRIGHTNESS_LEVEL
 * @brief Configure camera brightness level
 */
typedef enum {
    CAM_BRIGHTNESS_LEVEL_MINUS_4 = 8, /**<Level -4 */
    CAM_BRIGHTNESS_LEVEL_MINUS_3 = 6, /**<Level -3 */
    CAM_BRIGHTNESS_LEVEL_MINUS_2 = 4, /**<Level -2 */
    CAM_BRIGHTNESS_LEVEL_MINUS_1 = 2, /**<Level -1 */
    CAM_BRIGHTNESS_LEVEL_DEFAULT = 0, /**<Level Default*/
    CAM_BRIGHTNESS_LEVEL_1       = 1, /**<Level +1 */
    CAM_BRIGHTNESS_LEVEL_2       = 3, /**<Level +2 */
    CAM_BRIGHTNESS_LEVEL_3       = 5, /**<Level +3 */
    CAM_BRIGHTNESS_LEVEL_4       = 7, /**<Level +4 */
} CAM_BRIGHTNESS_LEVEL;

/**
 * @enum CAM_SHARPNESS_LEVEL
 * @brief Configure camera Sharpness level
 */
typedef enum {
    CAM_SHARPNESS_LEVEL_AUTO = 0, /**<Sharpness Auto */
    CAM_SHARPNESS_LEVEL_1,        /**<Sharpness Level 1 */
    CAM_SHARPNESS_LEVEL_2,        /**<Sharpness Level 2 */
    CAM_SHARPNESS_LEVEL_3,        /**<Sharpness Level 3 */
    CAM_SHARPNESS_LEVEL_4,        /**<Sharpness Level 4 */
    CAM_SHARPNESS_LEVEL_5,        /**<Sharpness Level 5 */
    CAM_SHARPNESS_LEVEL_6,        /**<Sharpness Level 6 */
    CAM_SHARPNESS_LEVEL_7,        /**<Sharpness Level 7 */
    CAM_SHARPNESS_LEVEL_8,        /**<Sharpness Level 8 */
} CAM_SHARPNESS_LEVEL;

/**
 * @enum CAM_VIDEO_MODE
 * @brief Configure resolution in video streaming mode
 */
typedef enum {
    CAM_VIDEO_MODE_0 = 1, /**< 320x240 */
    CAM_VIDEO_MODE_1 = 2, /**< 640x480 */
} CAM_VIDEO_MODE;

/**
 * @enum CAM_IMAGE_PIX_FMT
 * @brief Configure image pixel format
 */
typedef enum {
    CAM_IMAGE_PIX_FMT_RGB565 = 0x02, /**< RGB565 format */
    CAM_IMAGE_PIX_FMT_JPG    = 0x01, /**< JPEG format */
    CAM_IMAGE_PIX_FMT_YUV    = 0x03, /**< YUV format */
    CAM_IMAGE_PIX_FMT_NONE,          /**< No defined format */
} CAM_IMAGE_PIX_FMT;

/**
 * @enum CAM_WHITE_BALANCE
 * @brief Configure white balance mode
 */
typedef enum {
    CAM_WHITE_BALANCE_MODE_DEFAULT = 0, /**< Auto */
    CAM_WHITE_BALANCE_MODE_SUNNY,       /**< Sunny */
    CAM_WHITE_BALANCE_MODE_OFFICE,      /**< Office */
    CAM_WHITE_BALANCE_MODE_CLOUDY,      /**< Cloudy*/
    CAM_WHITE_BALANCE_MODE_HOME,        /**< Home */
} CAM_WHITE_BALANCE;

/**
 * @enum CAM_COLOR_FX
 * @brief Configure special effects
 */
typedef enum {
    CAM_COLOR_FX_NONE = 0,      /**< no effect   */
    CAM_COLOR_FX_BLUEISH,       /**< cool light   */
    CAM_COLOR_FX_REDISH,        /**< warm   */
    CAM_COLOR_FX_BW,            /**< Black/white   */
    CAM_COLOR_FX_SEPIA,         /**<Sepia   */
    CAM_COLOR_FX_NEGATIVE,      /**<positive/negative inversion  */
    CAM_COLOR_FX_GRASS_GREEN,   /**<Grass green */
    CAM_COLOR_FX_OVER_EXPOSURE, /**<Over exposure*/
    CAM_COLOR_FX_SOLARIZE,      /**< Solarize   */
} CAM_COLOR_FX;

typedef enum {
    HIGH_QUALITY    = 0,
    DEFAULT_QUALITY = 1,
    LOW_QUALITY     = 2,
} IMAGE_QUALITY;

enum {
    SENSOR_5MP_1 = 0x81,
    SENSOR_3MP_1 = 0x82,
    SENSOR_5MP_2 = 0x83, /* 2592x1936 */
    SENSOR_3MP_2 = 0x84,
};

typedef uint8_t (*BUFFER_CALLBACK)(uint8_t* buffer, uint8_t lenght); /**<Callback function prototype  */
typedef void (*STOP_HANDLE)(void);                                   /**<Callback function prototype  */

/**
 * @struct ArducamCamera
 * @brief Camera drive interface and information
 */

typedef struct {
    int csPin;                                      /**< CS pin */
    uint32_t totalLength;                           /**< The total length of the picture */
    uint32_t receivedLength;                        /**< The remaining length of the picture */
    uint8_t blockSize;                              /**< The length of the callback function transmission */
    uint8_t cameraId;                               /**< Model of camera module */
    // uint8_t cameraDataFormat;                       /**< The currently set image pixel format */
    uint8_t burstFirstFlag;                         /**< Flag bit for reading data for the first time in
                                                       burst mode */
    uint8_t previewMode;                            /**< Stream mode flag */
    uint8_t currentPixelFormat;                     /**< The currently set image pixel format */
    uint8_t currentPictureMode;                     /**< Currently set resolution */
    struct CameraInfo myCameraInfo;                 /**< Basic information of the current camera */
    const struct CameraOperations* arducamCameraOp; /**< Camera function interface */
    BUFFER_CALLBACK callBackFunction;               /**< Camera callback function */
    STOP_HANDLE handle;
    uint8_t verDateAndNumber[4]; /**< Camera firmware version*/
    union SdkInfo* currentSDK;   /**< Current SDK version*/
} ArducamCamera;

#ifdef __cplusplus
}
#endif

#endif