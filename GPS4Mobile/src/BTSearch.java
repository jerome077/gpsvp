import javax.microedition.midlet.*;
import javax.microedition.lcdui.*;
import javax.bluetooth.*;
import java.util.*;

public class BTSearch implements Runnable, DiscoveryListener  {
    
    private DiscoveryAgent discoveryAgent;
    private LocalDevice localDevice;

    private Vector devices = new Vector(); /* RemoteDevice */ 
    private String []deviceNames = new String[20];
    private int index = 0;

    private int []attrSet = {0x4321};
    private UUID []uuidSet = {new UUID(0x1101)};

    private int transactionID = 0;

    RemoteDevice remoteDevice;
    public String url = "";

	private boolean isSearching = false;
    private boolean doneSearching = false;
    private boolean doneServiceSearching = false;

	Thread t;
	Log log;
    
    public BTSearch(Log l) {
		log = l;
        t = new Thread(this);
        t.start();
    }

    // Connect to a specifyed index.
    public void connect(int i){
        url = "";
        discoveryAgent.cancelInquiry(this);
        remoteDevice = (RemoteDevice)devices.elementAt(i);
        
        synchronized(this){ // resume the thread.
            this.notify();
        }
    }
    
    public void startSearch(){
        discoveryAgent.cancelServiceSearch(transactionID);
        Thread t = new Thread(this);
        t.start();
    }    

    public String[] getDeviceNames(){
        return deviceNames;
    }

    public String getUrl(){
        return url;
    }
    
    public void run(){
        devices = null;
        devices = new Vector();

        deviceNames = null;
        deviceNames = new String[20];

        index=0;
        remoteDevice = null;
        doneServiceSearching = false;
        doneSearching = false;
        url = "";
        
        try {
            LocalDevice localDevice = LocalDevice.getLocalDevice();
            discoveryAgent = localDevice.getDiscoveryAgent();
            
			isSearching = true;
 
            // start searching for remote devices. If a device is found the deviceDiscovered method will be called.
            discoveryAgent.startInquiry(DiscoveryAgent.GIAC, this);

            // Pause the thread until the user selects another bt device to connect to.
            synchronized(this){
                try{
                    wait();
                }catch(Exception e){
					log.write(e);
                }
            }

            // Searsh for services on the remote bt device.
            transactionID = discoveryAgent.searchServices(null, uuidSet, remoteDevice, this);
        } catch (Exception e) {
			log.write(e);
            // System.err.println("Can't initialize bluetooth: " + e);
		}
    }


    public void deviceDiscovered(RemoteDevice btDevice, DeviceClass cod) {
         try{
            deviceNames[index] = btDevice.getFriendlyName(false); // Store the name of the device
            // System.out.println(index + " : " + deviceNames[index]);
            index++; // keep track on how many devices are found.
            devices.addElement(btDevice); // store the device
        }catch(Exception e){
			log.write("deviceDiscovered");
			log.write(e);
        }
   }
    
    public void servicesDiscovered(int transID, ServiceRecord[] servRecord) {
   		try {
	        url = servRecord[0].getConnectionURL(ServiceRecord.NOAUTHENTICATE_NOENCRYPT, true);
    	    // System.out.println("servicesDiscovered url: " + url);
        }catch(Exception e){
			log.write("servicesDiscovered");
			log.write(e);
        }
    }
    
    public void serviceSearchCompleted(int transID, int respCode) {
        // System.out.println("serviceSearchCompleted");
        doneServiceSearching = true;
    }
    
    public void inquiryCompleted(int discType){
        // System.out.println("inquiryCompleted");
        doneSearching = true;
    }

	public boolean isSearchingDevices(){
		return isSearching;
	}

    public boolean doneSearchingDevices(){
        return doneSearching;
    }

    public boolean doneSearchingServices(){
        return doneServiceSearching;
    }
    
	void stop(){
		if (discoveryAgent != null) {
			if (!doneSearching) {
				discoveryAgent.cancelInquiry(this);
				return;
			}
			if (!doneServiceSearching)
				discoveryAgent.cancelServiceSearch(transactionID);
			discoveryAgent = null;
		}
		if (t != null) {
			try {
				t.join();
			} catch (Exception e) {
				log.write(e);
			}
			t = null;
		}
	}
}
