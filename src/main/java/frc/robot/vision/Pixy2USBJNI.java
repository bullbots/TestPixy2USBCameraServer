package frc.robot.vision;

import java.util.concurrent.atomic.AtomicBoolean;

import org.opencv.core.CvType;
import org.opencv.core.Mat;

import edu.wpi.cscore.CvSource;
import edu.wpi.cscore.UsbCamera;
import edu.wpi.first.cameraserver.CameraServer;

public class Pixy2USBJNI implements Runnable {
    static {
       System.loadLibrary("pixy2_usb");
    }
  
    private native int pixy2USBInit();

    private native void pixy2USBGetVersion();

    private native void pixy2USBLampOn();

    private native void pixy2USBLampOff();

    private native byte[] pixy2GetMatDataRGBFrame();

    private static final int PIXY2_RAW_FRAME_WIDTH = 316;
    private static final int PIXY2_RAW_FRAME_HEIGHT = 208;

    private Pixy2USBJNI pixy2USBJNI;

    public AtomicBoolean toggleLamp = new AtomicBoolean(false);
    private boolean lampOn = false;

    public void toggleLamp() {
        if (lampOn) {
            System.out.println("Turning Lamp Off");
            pixy2USBJNI.pixy2USBLampOff();
            lampOn = false;
        } else {
            System.out.println("Turning Lamp On");
            pixy2USBJNI.pixy2USBLampOn();
            lampOn = true;
        }
    }

    @Override
    public void run() {
        // Uncomment these if you want extra, "regular" USB cameras
        // UsbCamera camera = CameraServer.getInstance().startAutomaticCapture(0);
        // camera.setResolution(640, 480);
        // UsbCamera camera1 = CameraServer.getInstance().startAutomaticCapture(1);
        // camera1.setResolution(640, 480);

        pixy2USBJNI = new Pixy2USBJNI();
        int init_result = pixy2USBJNI.pixy2USBInit();
        if (init_result == 0) {
            pixy2USBJNI.pixy2USBGetVersion();
            pixy2USBJNI.pixy2USBLampOn();
            lampOn = true;
    
            CvSource outputStream = CameraServer.getInstance().putVideo("Target Reticle", PIXY2_RAW_FRAME_WIDTH, PIXY2_RAW_FRAME_HEIGHT);
            Mat output = new Mat(PIXY2_RAW_FRAME_HEIGHT, PIXY2_RAW_FRAME_WIDTH, CvType.CV_8UC3);

            while(true) {
                if (toggleLamp.get()) {
                    toggleLamp();
                    toggleLamp.set(false);
                }
                byte[] matDataRGBFrame = pixy2USBJNI.pixy2GetMatDataRGBFrame();
                if (matDataRGBFrame != null) {
                    output.put(0, 0, matDataRGBFrame);
                    outputStream.putFrame(output);
                }
            }
        } else {
            System.err.println("WARNING: is the Pixy2 plugged in???");
        }
    }
}
