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

#include "RoboRioPixy2USBJNI.h"
#include "libpixyusb2.h"
#include "get_raw_frame.h"

#include <cameraserver/CameraServer.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

Pixy2 pixy;

JNIEXPORT void JNICALL Java_frc_robot_vision_Pixy2USBJNI_pixy2USBInit(JNIEnv *env, jobject thisObj) {
   std::cout << "Hello World from pixy2 usb init" << std::endl;
   pixy.init();
   return;
}

JNIEXPORT void JNICALL Java_frc_robot_vision_Pixy2USBJNI_pixy2USBGetVersion(JNIEnv *env, jobject thisObj) {
   std::cout << "Hello World from pixy2 usb get version" << std::endl;
   pixy.version->print();
   return;
}

JNIEXPORT void JNICALL Java_frc_robot_vision_Pixy2USBJNI_pixy2USBLampOn(JNIEnv *env, jobject thisObj) {
   std::cout << "Hello World from pixy2 usb Lamp On" << std::endl;
   pixy.setLamp(0x01, 0x00);
   return;
}

JNIEXPORT void JNICALL Java_frc_robot_vision_Pixy2USBJNI_pixy2USBLampOff(JNIEnv *env, jobject thisObj) {
   std::cout << "Hello World from pixy2 usb Lamp Off" << std::endl;
   pixy.setLamp(0x00, 0x00);
   return;
}

JNIEXPORT void JNICALL Java_frc_robot_vision_Pixy2USBJNI_pixy2USBStartCameraServer(JNIEnv *env, jobject thisObj)
{
   std::cout << "Starting CameraServer..." << std::endl;
   uint8_t *bayerFrame;
   uint32_t rgbFrame[PIXY2_RAW_FRAME_WIDTH*PIXY2_RAW_FRAME_HEIGHT];
   // need to call stop() befroe calling getRawFrame().
   // Note, you can call getRawFrame multiple times after calling stop().
   // That is, you don't need to call stop() each time.
   pixy.m_link.stop();
   cs::CvSource outputStreamStd = frc::CameraServer::GetInstance()->PutVideo("Target Reticle", PIXY2_RAW_FRAME_WIDTH, PIXY2_RAW_FRAME_HEIGHT);
   cv::Mat output(PIXY2_RAW_FRAME_HEIGHT, PIXY2_RAW_FRAME_WIDTH, CV_8UC3);
   // cv::rectangle(output, cv::Point(160, 120), cv::Point(480, 360), cv::Scalar(0, 0, 255), 4);
   while(true) {
      // grab raw frame, BGGR Bayer format, 1 byte per pixel
      pixy.m_link.getRawFrame(&bayerFrame);
      // convert Bayer frame to RGB frame
      demosaic(PIXY2_RAW_FRAME_WIDTH, PIXY2_RAW_FRAME_HEIGHT, bayerFrame, rgbFrame);
      output.data = (uint8_t*)rgbFrame;
      outputStreamStd.PutFrame(output);
      // Call resume() to resume the current program, otherwise Pixy will be left
      // in "paused" state.  
      pixy.m_link.resume();
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
   }
}

