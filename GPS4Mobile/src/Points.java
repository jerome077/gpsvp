import java.util.Vector;
import java.util.Enumeration;
import javax.microedition.io.file.FileConnection;
import javax.microedition.io.Connector;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.InputStream;
import java.io.InputStreamReader;

class Points {
    final static String path = "file:///e:/GPS4Mobile/";
    final static String name = "Waypoints.wpt";
	
    private Vector points = new Vector();
	private Log log;

	boolean may_write = false;

	Points(GPS4Mobile m) {
		log = m.log;
		read();
		may_write = true;
	}
	void add(GPSPoint p, String label) {
		points.addElement(new WayPoint(p, label));
		if (may_write)
			write();
	}
	void delete(WayPoint wp) {
		points.removeElement(wp);
		write();
	}
	Enumeration getPoints() {
		return points.elements();
	}
	void read() {
		try {
			log.write("Reading waypoints");
			FileConnection fcFile = (FileConnection) Connector.open(path + name);
			int iData = 0;
			String []data = new String[100];

			if(!fcFile.exists()) {
				log.write("No waypoint file");
				return;
			}
			InputStream stream = fcFile.openInputStream();
			InputStreamReader reader = new InputStreamReader(stream, "UTF-8");

			Vector points = new Vector();
			Vector gpspoints = new Vector();

			String sWord = "";
			
			int i;
			char c;
			int count = 0;
			do{
				i = reader.read(); // read one byte at the time.
				c = (char)i;
				switch(c){
				case '\n':
					try {
						data[iData] = sWord.trim();
			        }catch(Exception e){
						log.write("data[iData] = sWord.trim();");
						log.write(e);
						throw e;
        			}
					++iData;
					if (iData > 3) {
						try {
							add(new GPSPoint(Double.parseDouble(data[2]), Double.parseDouble(data[3])), data[1]);
			    	    }catch(Exception e){
							log.write("add(new GPSPoint(Double.parseDouble(data[2]), Double.parseDouble(data[3])), data[1])");
							log.write(e);
							throw e;
	        			}
						++count;
					}
					iData = 0;
					sWord = "";
					break;
				case ',':
					try {
						data[iData] = sWord.trim();
			        }catch(Exception e){
						log.write("data[iData] = sWord.trim(); (88)");
						log.write(e);
						throw e;
        			}
					sWord = "";
					iData++;
					break;
				default:
					sWord += c;
				}                
			} while(i != -1);

			reader.close();
			stream.close();
			fcFile.close();
			log.write("Read " + count + " points");
		
		}catch(Exception e){
			log.write(e);
		}
	}
	void parseData() {
	}
	void write() {
		if (!may_write)
			return;
		try {
			log.write("Writing waypoints");
			FileConnection fcDir = (FileConnection) Connector.open(path);
			if (!fcDir.exists())
				fcDir.mkdir();
			fcDir.close();
			FileConnection fcFile = (FileConnection) Connector.open(path + name);
			if(fcFile.exists())
				fcFile.delete();
			fcFile.create();
			OutputStream stream = fcFile.openOutputStream();
			OutputStreamWriter writer = new OutputStreamWriter(stream, "UTF-8");
			writer.write("OziExplorer Waypoint File Version 1.1\n");
			writer.write("WGS 84\n");
			writer.write("Reserved 2\n");
			writer.write("garmin\n");
			Enumeration e = points.elements();
			int i = 0;
			while (e.hasMoreElements()) {
				WayPoint p = (WayPoint)e.nextElement();
				String s = "";
				s += ++i;
				s += ", ";
				s += p.l;
				s += ", ";
				s += p.g.lat;
				s += ", ";
				s += p.g.lng;
				s += "\n";
				writer.write(s);
			}
			writer.close();
			stream.close();
			fcFile.close();	
			log.write("" + i + " waypoints written");		
		}catch(Exception e){
			log.write(e);
		}
	}
}
