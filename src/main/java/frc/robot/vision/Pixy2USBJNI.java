package frc.robot.vision;


public class Pixy2USBJNI implements Runnable {
    static {
       System.loadLibrary("pixy2_usb");
    }
  
    // Declare an instance native method sayHello() which receives no parameter and returns void
    private native void pixy2USBInit();

    private native void pixy2USBGetVersion();

    private native void pixy2USBLampOn();

    private native void pixy2USBLampOff();

    private native void pixy2USBStartCameraServer();

    private Pixy2USBJNI pixy2USBJNI;

    @Override
    public void run() {
        pixy2USBJNI = new Pixy2USBJNI();
        pixy2USBJNI.pixy2USBInit();
        pixy2USBJNI.pixy2USBGetVersion();
        pixy2USBJNI.pixy2USBLampOn();

        // System.out.println("Starting C++ CameraServer...");
        pixy2USBJNI.pixy2USBStartCameraServer();
    }
}