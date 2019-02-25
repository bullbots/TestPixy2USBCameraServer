//
// begin license header
//
// This file is part of Pixy CMUcam5 or "Pixy" for short
//
// All Pixy source code is provided under the terms of the
// GNU General Public License v2 (http://www.gnu.org/licenses/gpl-2.0.html).
// Those wishing to use Pixy source code, software and/or
// technologies under different licensing terms should contact us at
// cmucam@cs.cmu.edu. Such licensing terms are available for
// all portions of the Pixy codebase presented here.
//
// end license header
//
#include <jni.h>
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

#include "frc_robot_vision_Pixy2USBJNI.h"
#include "libpixyusb2.h"

#include <cscore.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

Pixy2 pixy;
uint8_t *bayerFrame;
cv::Mat bayerMat(PIXY2_RAW_FRAME_HEIGHT, PIXY2_RAW_FRAME_WIDTH, CV_8U);
cv::Mat output(PIXY2_RAW_FRAME_HEIGHT, PIXY2_RAW_FRAME_WIDTH, CV_8UC3);

JNIEXPORT jint JNICALL Java_frc_robot_vision_Pixy2USBJNI_pixy2USBInit(JNIEnv *env, jobject thisObj) {
   std::cout << "pixy2 usb init" << std::endl;
   return pixy.init();
}

JNIEXPORT void JNICALL Java_frc_robot_vision_Pixy2USBJNI_pixy2USBGetVersion(JNIEnv *env, jobject thisObj) {
   std::cout << "pixy2 usb get version" << std::endl;
   pixy.version->print();
   return;
}

JNIEXPORT void JNICALL Java_frc_robot_vision_Pixy2USBJNI_pixy2USBLampOn(JNIEnv *env, jobject thisObj) {
   std::cout << "pixy2 usb Lamp On" << std::endl;
   pixy.setLamp(0x01, 0x00);
   return;
}

JNIEXPORT void JNICALL Java_frc_robot_vision_Pixy2USBJNI_pixy2USBLampOff(JNIEnv *env, jobject thisObj) {
   std::cout << "pixy2 usb Lamp Off" << std::endl;
   pixy.setLamp(0x00, 0x00);
   return;
}

CS_Source source;

JNIEXPORT void JNICALL Java_frc_robot_vision_Pixy2USBJNI_pixy2USBInitCameraServer
  (JNIEnv *env, jobject, jint csHandle) {
     source = static_cast<CS_Source>(csHandle);
}

JNIEXPORT jint JNICALL Java_frc_robot_vision_Pixy2USBJNI_pixy2USBLoopCameraServer(JNIEnv *env, jobject thisObj)
{
   // need to call stop() before calling getRawFrame().
   // Note, you can call getRawFrame multiple times after calling stop().
   // That is, you don't need to call stop() each time.
   pixy.m_link.stop();

   // grab raw frame, BGGR Bayer format, 1 byte per pixel
   pixy.m_link.getRawFrame(&bayerFrame);
   // convert Bayer frame to RGB frame
   bayerMat.data = bayerFrame;

   // Using OpenCV for conversion to RGB
   cv::cvtColor(bayerMat, output, cv::COLOR_BayerBG2RGB);

   // Call resume() to resume the current program, otherwise Pixy will be left in "paused" state.
   pixy.m_link.resume();

   CS_Status status = 0;
   cs::PutSourceFrame(source, output, &status);
   return status;
}
