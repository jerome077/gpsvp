import java.util.*;
import javax.microedition.midlet.*;
import javax.microedition.io.*;
import javax.microedition.lcdui.*;

import java.io.*;
import java.io.*;

interface GPSListener {
	public void add(String date, String time, GPSPoint g, String altitude);
	public void brk();
}

public class GPSParser implements Runnable{

	private StreamConnection conn = null;
	Thread th = null;
    public String m_sUTC_Time = "";
    public String m_sLatitude = "";
    public String m_sNS_Indicator = "";
    public String m_sLongitude = "";
    public String m_sEW_Indicator = "";
    public String m_sAltitude = "";
    public String m_sDate = "";
    public String debug = "";
	public boolean disconnect = false;

    public int [] sat = new int[12];
    public int [] satid = new int[12];
    
	private Track track = null;
	private Log log = null;
	private GPSCanvas canvas = null;
	private GPS4Mobile midlet = null;
	private TrackWriter writer = null;
	private boolean exiting = false;
	private long lasttime = System.currentTimeMillis();

    private String url = "";
	private Vector listeners = new Vector();
	
	private static final int DATA_LEN = 64;
	public boolean connected = false;

	String []data = new String[DATA_LEN];
	int iData = 0;

	public void addListener(GPSListener l) {
		listeners.addElement(l);
	}

    public GPSParser(String u, Log l, GPSCanvas c, Track t, TrackWriter w, GPS4Mobile m) {
        url = u; // Connection string to the bluetooth device.
		log = l;
		track = t;
		canvas = c;
		writer = w;
		midlet = m;
        th = new Thread(this);
        th.start();
		addListener(track);
		addListener(writer);
		addListener(canvas);
    }
	
	private void brk() {
		Enumeration e = listeners.elements();
		while (e.hasMoreElements()) {
			// log.write("Tick");
			GPSListener l = (GPSListener)(e.nextElement());
			if (l != null)
				l.brk();
			else
				log.write("l == null");
			// log.write("Tack");
		}
	}

	private void add(String date, String time, GPSPoint g, String altitude) {
		Enumeration e = listeners.elements();
		while (e.hasMoreElements()) {
			GPSListener l = (GPSListener)(e.nextElement());
			l.add(date, time, g, altitude);
		}
		try {
			Class.forName("com.nokia.mid.ui.DeviceControl");
			com.nokia.mid.ui.DeviceControl.setLights(0,100);
		}
		catch(Exception ex){
			log.write(ex);
		}
	}

    public void run() {
        InputStream is = null;
        String err = ""; // used for debugging
        int errors = 0;

		while (!exiting) {
			String sWord = "";
			char c = 'x';
			try{
				++errors;
				if (errors > 10) {
					log.write("Too many errors. Disconnecting");
					midlet.commandAction(GPS4Mobile.CMD_EXIT, null);
				}
				log.write("Opening " + url);
				conn = (StreamConnection)Connector.open(url);
				log.write("Getting input stream");
				is = conn.openInputStream();            
				
				int i=0;
				String s = "";
				byte btCheckSum = 0, btRecivedCS = 0;
				byte cs1, cs2;
				
				brk();
				log.write("Reading data");
				connected = true;
				canvas.repaint();
				// Start reading the data from the GPS
				do{
					i = is.read(); // read one byte at the time.
					c = (char)i;
					errors = 0;
					switch(c){
					case '$':
						iData = 0;
						sWord = "";
						btCheckSum = 0;
						break;
					case ',':
						try {
							data[iData] = sWord;
				        }catch(Exception e){
							log.write("data[iData] = sWord (120)");
							log.write(e);
							throw e;
				        }
						sWord = "";
						btCheckSum ^= c;
						iData++;
						if(iData==1){
							try {
								if(data[0].compareTo("GPGGA") !=0 && data[0].compareTo("GPRMC") !=0 && data[0].compareTo("GPGSV") !=0) {
									while( !exiting && ((char)is.read()) != '$' );
									iData = 0;
									sWord = "";
									btCheckSum = 0;
								}                  
					        }catch(Exception e){
								log.write("if(data[0].compareTo(\"GPGGA\") !=0 && data[0].compareTo(\"GPRMC\") !=0 && data[0].compareTo(\"GPGSV\") !=0)");
								log.write(e);
								throw e;
					        }
                        }
                        break;
					case '*':
						cs1 = (byte)is.read();
						if( (cs1 - (int)'0') <= 9){
							btRecivedCS = (byte)((cs1 - '0') << 4);
						} else {
							btRecivedCS  = (byte)((cs1 - 'A' + 10) << 4);
						}
						
						cs2 = (byte)is.read();
						if( (cs2 - '0') <= 9){
							btRecivedCS |= (cs2 - '0');
						} else {
							btRecivedCS |= (cs2 - 'A' + 10);
						}
						if(btRecivedCS==btCheckSum) {
							parseData();
							String sLine = "";                            
						}
						break;
					default:
						btCheckSum ^= c;
						sWord += c;
					}                
				}while(!exiting && !disconnect && i != -1);
			} catch(Exception e) {
				String message = "iData = ";
				message += iData;
				message += "; sWord = ";
				message += sWord;
				message += "; c = ";
				message += c;
				if (iData > 0) {
					message += "data[0] = ";
					message += "commented out";
					// message += data[0];
				}
				log.write(message);
				log.write(e);
			}
			brk();
			log.write("Closing connection");
			connected = false;
			canvas.repaint();
			try {
				if (is != null)
					is.close();
				if (conn != null)
					conn.close();
			} catch(Exception e) {
				log.write(e);
			}
			try {
				if (disconnect) {
					while (disconnect)
						Thread.sleep(1000);
				} else {
					for (int i = 60; i > 0 && !exiting; --i)
						Thread.sleep(1000);
				}
			} catch(Exception e) {
				log.write(e);
			}
		}
    }
 
    private void parseData(){
		// log.write("Parsing data ...");
		if (iData == 0) {
			log.write("No data");
			return;
		}
		try
		{
	        if(data[0].compareTo("GPGGA") == 0 && iData > 5){
				// log.write("Parsing GPGGA");
        	    m_sLatitude = data[2];
            	m_sNS_Indicator = data[3];
	            m_sLongitude = data[4];
    	        m_sEW_Indicator = data[5];
    	        m_sAltitude = data[9];
				if (m_sLongitude.length() > 2 && m_sLongitude.length() > 3 && m_sUTC_Time != "") {
					// log.write("Adding point " + m_sLatitude + "-" + m_sNS_Indicator + "-" + m_sLatitude + "-" + m_sEW_Indicator);
					GPSPoint g = new 
						GPSPoint(Double.parseDouble(m_sLatitude.substring(0, 2)) + 
								 Double.parseDouble(m_sLatitude.substring(2)) / 60,
								 Double.parseDouble(m_sLongitude.substring(0, 3)) + 
								 Double.parseDouble(m_sLongitude.substring(3)) / 60);
					long newtime = System.currentTimeMillis();
					if (newtime > lasttime + 5000) {
						writer.brk();
					}
					lasttime = newtime;
					add(m_sDate, m_sUTC_Time, g, m_sAltitude);
				} else {
					// log.write("Breaking");
					brk();
				}
	        } else if (data[0].compareTo("GPRMC") == 0 && iData > 9) {
				// log.write("Parsing GPRMC");
    	        m_sUTC_Time = data[1];
        	    m_sLatitude = data[3];
            	m_sNS_Indicator = data[4];
            	m_sLongitude = data[5];
	            m_sEW_Indicator = data[6];            
    	        m_sDate = data[9];
        	} else if (data[0].compareTo("GPGSV") == 0 && iData > 18) {
				// log.write("Parsing GPGSV");
    	        int iMessageNumber = Integer.parseInt(data[2]);
        	    try{
            	    for(int i = 0; i < 4 && i < iData;++i){
                	    satid[i+(iMessageNumber-1)*4] = Integer.parseInt(data[3+4*i]);
                    	sat[i+(iMessageNumber-1)*4] = Integer.parseInt(data[6+4*i]);
	                }
    	        }catch(java.lang.NumberFormatException e){}
        	     
        	}
        } catch(Exception e) {
			log.write("parseData");
			log.write(e);
        }
		// log.write("Parsing done");
    }
    
	void stop()
	{
		exiting = true;
		if (th != null) {
			try {
				log.write("Joining thread");
				th.join();
			} catch (Exception e) {
				log.write(e);
			}
			th = null;
		}
	}
}
