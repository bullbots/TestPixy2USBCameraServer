package frc.robot.vision;

import java.util.Scanner;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.atomic.AtomicBoolean;

import edu.wpi.cscore.CvSource;
import edu.wpi.first.cameraserver.CameraServer;
import edu.wpi.first.hal.NotifierJNI;
import edu.wpi.first.wpilibj.RobotController;

public class Pixy2USBJNI implements Runnable {
    static {
       System.loadLibrary("pixy2_usb");
    }
  
    // Declare an instance native method sayHello() which receives no parameter and returns void
    private native int pixy2USBInit();

    private native void pixy2USBGetVersion();

    private native void pixy2USBLampOn();

    private native void pixy2USBLampOff();

    private native String pixy2USBGetBlocks();

    private native void pixy2USBInitCameraServer(int source);

    private void pixy2USBInitCameraServer(CvSource source) {
        pixy2USBInitCameraServer(source.getHandle());
    }

    // Return value is status of PutFrame
    private native int pixy2USBLoopCameraServer(int[][] blocksArray);

    private double m_expirationTime;
    private final int m_notifier = NotifierJNI.initializeNotifier();
    private final double m_period = .3; //Milliseconds?

    private Pixy2USBJNI pixy2USBJNI;

    private static final int PIXY2_RAW_FRAME_WIDTH = 316;
    private static final int PIXY2_RAW_FRAME_HEIGHT = 208;

    public AtomicBoolean toggleLamp = new AtomicBoolean(false);
    private boolean lampOn = false;
    private int cycleCounter = 0;
    private static Block[] blocks;
    private int[][] blocksArray;
    private boolean fetchFrame = true;
    public static final ArrayBlockingQueue<Block[]> blocksBuffer = new ArrayBlockingQueue<>(2);

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
            pixy2USBJNI.pixy2USBInitCameraServer(outputStream);

            m_expirationTime = RobotController.getFPGATime() * 1e-6 + m_period;
            updateAlarm();
            
            while(true) {
                long curTime = NotifierJNI.waitForNotifierAlarm(m_notifier);
                if(curTime == 0){
                    break;
                }

                m_expirationTime += m_period;
                updateAlarm();

                loopfunc();
            }
        } else {
            System.err.println("[WARNING] is the Pixy2 plugged in???");
        }
    }

    private void loopfunc() {

        if (toggleLamp.get()) {
            toggleLamp();
            toggleLamp.set(false);
        }
        
        String visionStuffs = pixy2USBJNI.pixy2USBGetBlocks();
        int arrayIndex = 0;
        blocksArray = new int[0][0];

        if (visionStuffs.equals("")) {
            // fetchFrame = true;
            if (++cycleCounter > 3) {
                cycleCounter = 0;
                if (!fetchFrame) {
                    System.out.println("[INFO] Resuming camera");
                    fetchFrame = true;
                }
                System.out.println("[INFO] No blocks detected");
            }
        } 
        else {
            System.out.println("[INFO] visionStuffs: " + visionStuffs);
            if (visionStuffs.equals("-2\n")) {
                fetchFrame = false;
            } else {
                fetchFrame = true;
                cycleCounter = 0;
                String[] visionParts = visionStuffs.split("\n");
                blocks = new Block[visionParts.length];
                blocksArray = new int[visionParts.length][7];
                
                for (String s : visionParts) {
                    if(!s.isEmpty() && !s.isBlank() && !s.equals(null) && !s.equals("")) {
                        try{
                            Scanner sc = new Scanner(s);
                            sc.next();
                            sc.next();
                            sc.next();
                            sc.next();
                            int sig = sc.nextInt();
                            sc.next();
                            int x = sc.nextInt();
                            sc.next();
                            int y = sc.nextInt();
                            sc.next();
                            int width = sc.nextInt();
                            sc.next();
                            int height = sc.nextInt();
                            sc.next();
                            int index = sc.nextInt();
                            sc.next();
                            int age = sc.nextInt();
        
                            blocks[arrayIndex] = new Block(sig, x, y, width, height, index, age);
                            blocksArray[arrayIndex] = blocks[arrayIndex].toArray();
                            arrayIndex++;
                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                    }
                }

                synchronized(blocksBuffer) {
                    if(blocksBuffer.remainingCapacity()==0) {
                        blocksBuffer.remove();
                    }
                    try {
                        blocksBuffer.put(blocks);
                    } catch (InterruptedException e){
                        e.printStackTrace();
                    }
                }
            }
        }

        if (fetchFrame) {
            // System.out.println("Sending frame...");

            pixy2USBJNI.pixy2USBLoopCameraServer(blocksArray);
        }
        // else {
        //     System.out.println("Camera paused...");
        // }
    }

    private void updateAlarm(){
        NotifierJNI.updateNotifierAlarm(m_notifier, (long) (m_expirationTime * 1e6));
    }
}
