# TestPixy2USBCameraServer
TestPixy2USBCameraServer uses the Pixy2 USB libraries compiled specifically for the roboRIO.

This code is still an active work-in-progress and as such still has some glitchiness when it comes to deploying the code for the first time.

## Getting started

It is very important, for the time being, that you build the code first before you try to deploy it.
There is unfortunately not a hook in place yet to make sure that necessary .so JNI is built before this code is deployed.
