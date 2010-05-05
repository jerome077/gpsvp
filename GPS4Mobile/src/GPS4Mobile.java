import javax.microedition.midlet.*;
import javax.microedition.lcdui.*;
import javax.bluetooth.*;
import java.util.*;
import java.io.*;
import javax.microedition.io.*;
import javax.microedition.io.file.*;
import javax.microedition.rms.*;

public class GPS4Mobile extends MIDlet implements CommandListener, Runnable {

    private Display display;

    public final static Command CMD_EXIT = new Command("Exit", Command.EXIT, 1);
    private final static Command CMD_CANCEL = new Command("Cancel", Command.CANCEL, 1);
    private final static Command CMD_SELECT = new Command("GPS", Command.SCREEN, 1);
    public final static Command CMD_ADDPOINT = new Command("Mark", Command.SCREEN, 1);
    public final static Command CMD_DELPOINT = new Command("Delete", Command.ITEM, 1);
    private final static Command CMD_OPENTRACK = new Command("Open track", Command.SCREEN, 1);
	public final static Command CMD_EDIT = new Command("Edit", Command.ITEM, 1);
	public final static Command CMD_WAYPOINTS = new Command("Waypoints", Command.SCREEN, 1);
	public final static Command CMD_NAVIGATE = new Command("Navigate", Command.ITEM, 1);
	public final static Command CMD_DONTNAVIGATE = new Command("Don't navigate", Command.ITEM, 1);
	public final static Command CMD_FULLSCREEN = new Command("Full screen", Command.SCREEN, 1);
	public final static Command CMD_MINIMIZE = new Command("Minimize", Command.BACK, 1);
	public final static Command CMD_MAP = new Command("Map view", Command.SCREEN, 1);
	public final static Command CMD_SATELLITE = new Command("Satellite view", Command.SCREEN, 1);
	public final static Command CMD_YANDEX = new Command("Yandex UGC", Command.SCREEN, 1);
	public final static Command CMD_SEARCH = new Command("Search", Command.SCREEN, 1);
	public final static Command CMD_DISCONNECT = new Command("Disconnect", Command.SCREEN, 1);
	public final static Command CMD_CONNECT = new Command("Connect", Command.SCREEN, 1);
	//public final static Command CMD_ROUTE = new Command("Route", Command.SCREEN, 1);
	//public final static Command CMD_SEARCHRESULTS = new Command("Search results", Command.SCREEN, 1);

	public Log log = new Log();
    private GPSCanvas canvas = new GPSCanvas(this);
    private List deviceList;
	private List trackList;
    private BTSearch btSearch;
	public GPSParser parser = null;
	public Points points = new Points(this);
    private Vector devices;
    private boolean bSearchBTDevices = true;
	private String url = "";
	public Track track2 = new Track(true, log);
	public Track track = new Track(false, track2, log);
	public Vector tracks = new Vector();
	public TrackWriter writer = new TrackWriter(this);
    
    public GPS4Mobile(){
        display = Display.getDisplay(this);
    }

	void Init()
	{
        try {
			log.write("Setting center to (0,0)");
			canvas.SetGpsCenter(new GPSPoint(0, 0));
			log.write("Opening record store");
            RecordStore options = RecordStore.openRecordStore("options", true);
            if (options.getNumRecords() != 0) {
				IntPoint center = null;
                byte[] data = options.getRecord(1);
				log.write("Closing record store");
                options.closeRecordStore();
                ByteArrayInputStream bais = new ByteArrayInputStream(data);
                DataInputStream dis = new DataInputStream(bais);
                url = dis.readUTF();
				log.write("url is " + url);
				int x = dis.readInt();
				int y = dis.readInt();
				
				if (x != 0 || y != 0)
					center = new IntPoint(x, y);
				int zoom = dis.readInt();
				int level = dis.readInt();
				// canvas.zoom = zoom;
				canvas.level = level;
				boolean navigate = dis.readBoolean();
				if (navigate) {
					x = dis.readInt();
					y = dis.readInt();
					canvas.gpsdestination = new GPSPoint(new IntPoint(x, y), canvas.factory);
				}
				String maptype = dis.readUTF();
				log.write("map type is " + maptype);
				if (maptype.equals("mssat")) {
					canvas.factory = new MSSatFactory();
					track.factory = canvas.factory;
				} else if (maptype.equals("yandex")) {
					canvas.factory = new YandexFactory();
					track.factory = canvas.factory;
				} else {
					canvas.factory = new OSMFactory();
					track.factory = canvas.factory;
				}
				if (center != null)
					canvas.SetCenter(center);
            } else {
				log.write("No records");
			}		 
        } catch(Exception e) {
			log.write(e);
        }
	}

	void StartSearch(){
		if (parser != null)
			parser.stop();
		deviceList = new List("Bluetooh Devices", Choice.IMPLICIT);
		deviceList.addCommand(CMD_CANCEL);
		deviceList.setCommandListener(this);
		display.setCurrent(deviceList);
		btSearch = new BTSearch(log);
		(new Thread(this)).start();
	}
    
	void OpenTrack(){
		trackList = new List("Tracks", Choice.IMPLICIT);
		trackList.addCommand(CMD_CANCEL);
		trackList.setCommandListener(this);
		display.setCurrent(trackList);
		(new Thread(new GetFileList(trackList, log))).start();
	}

	void CheckMapMenu() {
		if (canvas.factory.GetType() == "mssat")
			canvas.removeCommand(CMD_SATELLITE);
		else
			canvas.addCommand(CMD_SATELLITE);
		if (canvas.factory.GetType() == "osm")
			canvas.removeCommand(CMD_MAP);
		else
			canvas.addCommand(CMD_MAP);
		if (canvas.factory.GetType() == "yandex")
			canvas.removeCommand(CMD_YANDEX);
		else
			canvas.addCommand(CMD_YANDEX);
	}

	void CheckNavigateMenu() {
		if (canvas.gpsdestination == null) {
			canvas.removeCommand(CMD_DONTNAVIGATE);
			canvas.addCommand(CMD_NAVIGATE);
		} else {
			canvas.addCommand(CMD_DONTNAVIGATE);
			canvas.removeCommand(CMD_NAVIGATE);
		}
	}

    public void startApp() {
		Init();
		// canvas.addCommand(CMD_WAYPOINTS);
		canvas.addCommand(CMD_MINIMIZE);
		CheckNavigateMenu();
		canvas.addCommand(CMD_FULLSCREEN);
		canvas.addCommand(CMD_OPENTRACK);
		canvas.addCommand(CMD_SELECT);
		CheckMapMenu();
		canvas.addCommand(CMD_SEARCH);
		// canvas.addCommand(CMD_ROUTE);
		canvas.addCommand(CMD_EXIT);
		canvas.setCommandListener(this);
		display.setCurrent(canvas);
		if (url != "")
			parser = new GPSParser(url, log, canvas, track, writer, this);
    }
    
    public void pauseApp() {
    }
    
    public void destroyApp(boolean unconditional) {
		SaveSettings();
    }

	void SaveSettings() {
		try {
			log.write("Saving settings");
			ByteArrayOutputStream baos = new ByteArrayOutputStream();
			DataOutputStream dos = new DataOutputStream(baos);
			dos.writeUTF(url);
			IntPoint curcenter = canvas.GetCenter();
			if (curcenter != null) {
				dos.writeInt(curcenter.x);
				dos.writeInt(curcenter.y);
			} else {
				dos.writeInt(0);
				dos.writeInt(0);
			}
			dos.writeInt(0); // canvas.zoom);
			dos.writeInt(canvas.level);
			
			dos.writeBoolean(false); // old destination

			dos.writeUTF(canvas.factory.GetType());

			if (canvas.gpsdestination != null) {
				dos.writeBoolean(true);
				dos.writeDouble(canvas.gpsdestination.lat);
				dos.writeDouble(canvas.gpsdestination.lng);
			} else {
				dos.writeBoolean(false);
			}

			RecordStore options = RecordStore.openRecordStore("options", true);
			byte[] data = baos.toByteArray();
			if (options.getNumRecords() != 0)
				options.setRecord(1, data, 0, data.length);
			else
				options.addRecord(data, 0, data.length);
			options.closeRecordStore();
			log.write("Done saving");
		} catch (Exception e) {
			log.write(e);
		}
	}

    public void commandAction(Command c, Displayable d) {
		if (c == CMD_OPENTRACK)
			OpenTrack();
		if (c == CMD_WAYPOINTS)
			display.setCurrent(new PointsList(points, display, canvas));
		if (c == CMD_ADDPOINT)
			AddCurrentPoint();
		if (c == CMD_NAVIGATE) {
			if (canvas.GetNearest() == null)
				canvas.gpsdestination = canvas.GetGpsCenter();
			else
				canvas.gpsdestination = canvas.GetNearest().g;
			canvas.repaint();
			CheckNavigateMenu();
		}
		if (c == CMD_DONTNAVIGATE) {
			canvas.gpsdestination = null;
			canvas.repaint();
			CheckNavigateMenu();
		}
		if (c == CMD_MAP) {
			canvas.factory = new OSMFactory();
			track.factory = canvas.factory;
			CheckMapMenu();
			canvas.repaint();
		}
		if (c == CMD_SATELLITE) {
			canvas.factory = new MSSatFactory();
			track.factory = canvas.factory;
			CheckMapMenu();
			canvas.repaint();
		}
		if (c == CMD_YANDEX) {
			canvas.factory = new YandexFactory();
			track.factory = canvas.factory;
			CheckMapMenu();
			canvas.repaint();
		}
		if (c == CMD_FULLSCREEN)
			canvas.ToggleFullscreen();
		if (c == CMD_MINIMIZE)
			display.setCurrent(null);
		
		if (c == CMD_DELPOINT) {
			points.delete(canvas.GetNearest());
			canvas.repaint();
		}
		if (c == CMD_EXIT) {
			destroyApp(false);
			notifyDestroyed();
		}
		if (c == CMD_SEARCH)
			display.setCurrent(new SearchForm(display, canvas, log));
		//if (c == CMD_SEARCHRESULTS)
		//display.setCurrent(found);
		if (c == CMD_SELECT)
			StartSearch();
		if (c == CMD_DISCONNECT) {
			parser.disconnect = true;
			canvas.repaint();
		}
		if (c == CMD_CONNECT) {
			parser.disconnect = false;
			canvas.repaint();
		}
		/*
		if (c == CMD_ROUTE) {
			canvas.route();
		}
		*/
		if (c == CMD_CANCEL) {
			display.setCurrent(canvas);
			if (btSearch != null) {
				btSearch.stop();
				btSearch = null;
			}
		}
		if (c == CMD_EDIT && canvas.GetNearest() != null)
			display.setCurrent(new WayPointProperties(canvas.GetNearest(), display, canvas, points));
		if (d == deviceList) {
			if (c == List.SELECT_COMMAND) {
				btSearch.connect(((List)d).getSelectedIndex());
				while(btSearch.getUrl().trim().compareTo("") == 0) {
					try {
						Thread.sleep(100);
					} catch (Exception e) { 
						log.write(e);
					}
				}				
				url = btSearch.getUrl();
				SaveSettings();
				parser = new GPSParser(url, log, canvas, track, writer, this);
			}
		}
		if (d == trackList) {
			if (c == List.SELECT_COMMAND) {
				display.setCurrent(canvas);
				String filename = trackList.getString(trackList.getSelectedIndex());
				int end = filename.indexOf(" ");
				if (end > 0)
					filename = filename.substring(0, end);
				Track track = TrackWriter.read(filename, log);
				if (track != null) {
					if (track.lastpoint != null) {
						tracks.addElement(track);
						canvas.SetCenter(new IntPoint(track.lastpoint));
					}
				}
			}
		}
	}
     
    private void refreshDeviceList(){
    	try {
        	String []deviceNames = btSearch.getDeviceNames();
        	deviceList.deleteAll();
        	for(int i=0; i <= deviceNames.length; i++)
	            deviceList.append(deviceNames[i],null);
			deviceList.setTitle("Bluetooth devices");
        }catch(Exception e){
			log.write("refreshDeviceList");
			log.write(e);
        }
    }
    
    public void run(){
        while(!btSearch.doneSearchingDevices())
        {
			deviceList.setTitle("Searching ...");
            try{
                Thread.sleep(100);
            } catch(Exception e) {
				log.write(e);
			}
		}        
		refreshDeviceList();
    }

    public void AddCurrentPoint(){
        if (canvas.GetCenter() != null)
            points.add(new GPSPoint(canvas.GetCenter(), canvas.factory), "Waypoint");
    }
}
