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
#include <map>

#include "RoboRioPixy2USBJNI.h"
#include "libpixyusb2.h"

#include <cameraserver/CameraServer.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

Pixy2 pixy;
cs::UsbCamera camera;
cs::UsbCamera camera1;
uint8_t *bayerFrame;
cs::CvSource outputStreamStd;
cv::Mat bayerMat(PIXY2_RAW_FRAME_HEIGHT, PIXY2_RAW_FRAME_WIDTH, CV_8U);
cv::Mat output(PIXY2_RAW_FRAME_HEIGHT, PIXY2_RAW_FRAME_WIDTH, CV_8UC3);
std::map <int, cv::Scalar> colorMap = {{1, cv::Scalar(0, 0, 255)}, 
                                       {2, cv::Scalar(165, 0, 255)},
                                       {3, cv::Scalar(255, 0 ,255)},
                                       {4, cv::Scalar(0, 255, 0)},
                                       {5, cv::Scalar(255, 255, 0)},
                                       {6, cv::Scalar(255, 0, 0)},
                                       {7, cv::Scalar(238, 130, 238)}};
const float MAX_FONT_IMAGE_HEIGHT = 100.0;

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

JNIEXPORT jstring JNICALL Java_frc_robot_vision_Pixy2USBJNI_pixy2USBGetBlocks(JNIEnv *env, jobject thisObj)
{
   int  Block_Index;

   // Query Pixy for blocks //
   int result = pixy.ccc.getBlocks();
   // std::cout << "Get blocks result: " << result << std::endl;

   std::stringstream ss;

   // Were blocks detected? //
   if (result >= 0) {
      if (pixy.ccc.numBlocks)
      {
         // Blocks detected - print them! //
         // std::cout << "Getting num blocks: " << (int) pixy.ccc.numBlocks << std::endl;

         for (Block_Index = 0; Block_Index < pixy.ccc.numBlocks; ++Block_Index)
         {
            // printf ("  Block %d: ", Block_Index + 1);
            ss << "block " << Block_Index + 1 << " : ";
            ss << pixy.ccc.blocks[Block_Index].str();
            if (Block_Index < pixy.ccc.numBlocks-1) {
                  ss << std::endl;
            }
            //   pixy.ccc.blocks[Block_Index].print();
         }
      }
   } else if (result == -2) {
      ss << result << std::endl;
   }

   return env->NewStringUTF(ss.str().c_str());
}

JNIEXPORT void JNICALL Java_frc_robot_vision_Pixy2USBJNI_pixy2USBStartCameraServer(JNIEnv *env, jobject thisObj)
{
   std::cout << "Starting CameraServer..." << std::endl;
   // Uncomment these to get more regular USB cameras
//   camera = frc::CameraServer::GetInstance()->StartAutomaticCapture(0);
//   camera.SetResolution(640, 480);
//   camera1 = frc::CameraServer::GetInstance()->StartAutomaticCapture(1);
//   camera1.SetResolution(640, 480);
   
   outputStreamStd = frc::CameraServer::GetInstance()->PutVideo("Target Reticle", PIXY2_RAW_FRAME_WIDTH, PIXY2_RAW_FRAME_HEIGHT);
}

JNIEXPORT void JNICALL Java_frc_robot_vision_Pixy2USBJNI_pixy2USBLoopCameraServer(JNIEnv *env, jobject thisObj, jobjectArray objArr)
{
   // need to call stop() before calling getRawFrame().
   // Note, you can call getRawFrame multiple times after calling stop().
   // That is, you don't need to call stop() each time.
   pixy.m_link.stop();

   // grab raw frame, BGGR Bayer format, 1 byte per pixel
   pixy.m_link.getRawFrame(&bayerFrame);

   // convert Bayer frame to RGB frame
   bayerMat.data = bayerFrame;

   // Reticle overlay example using OpenCV
   cv::cvtColor(bayerMat, output, cv::COLOR_BayerBG2RGB);

   // If they exist, draw object boxes on top of frame
   jsize len = env->GetArrayLength(objArr);

   for (int i=0; i<len; ++i) {
      jintArray arr = (jintArray) env->GetObjectArrayElement(objArr, i);
      jsize innerLen = env->GetArrayLength(arr);
      jint* vals = env->GetIntArrayElements(arr, NULL);

      int sig = vals[0];
      int center_x = vals[1];
      int center_y = vals[2];
      int width = vals[3];
      int height = vals[4];
      int index = vals[5];
      int age = vals[6];

      cv::Point top_left(center_x - width / 2, center_y - height / 2);
      cv::Point bottom_right(center_x + width / 2, center_y + height / 2);

      cv::Point text_org(center_x, center_y);
      std::ostringstream oss;
      oss << "S=" << sig;

      float font_scale = height / MAX_FONT_IMAGE_HEIGHT;

      if (height > MAX_FONT_IMAGE_HEIGHT * 0.75) {
         font_scale = 0.75;
      } else if (height < MAX_FONT_IMAGE_HEIGHT * 0.5) {
         font_scale = 0.5;
      }

      cv::putText(output, oss.str(), text_org, cv::FONT_HERSHEY_COMPLEX, font_scale, colorMap[sig]);
      cv::rectangle(output, top_left, bottom_right, colorMap[sig], 1);

      env->ReleaseIntArrayElements(arr, vals, JNI_COMMIT);
      env->DeleteLocalRef(arr);
   }

   // cv::circle(output, cv::Point(158, 104), 50, cv::Scalar(255, 0, 0, 0.4), 2);
   // cv::line(output, cv::Point(108, 104), cv::Point(208, 104), cv::Scalar(255, 0, 0, 0.4), 2);
   // cv::line(output, cv::Point(158, 54), cv::Point(158, 154), cv::Scalar(255, 0, 0, 0.4), 2);

   // Rectangle examples
   // cv::rectangle(output, cv::Point(79, 52), cv::Point(237, 156), cv::Scalar(255, 0, 0), 2);
   // cv::rectangle(output, cv::Point(79, 52), cv::Point(237, 156), cv::Scalar(255, 0, 0), 2);
   
   outputStreamStd.PutFrame(output);
   // Call resume() to resume the current program, otherwise Pixy will be left in "paused" state.
   pixy.m_link.resume();
}
