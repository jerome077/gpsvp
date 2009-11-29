import java.io.*;
import javax.microedition.io.*;
import javax.microedition.io.file.*;
import java.util.Date;
import javax.microedition.lcdui.*;
import java.util.*;

class GetFileList implements Runnable {
	List list;
	Log log;
	GetFileList(List li, Log lo) {
		list = li;
		log = lo;
	}
	public void run() {
		TrackWriter.GetFileList(list, log);
	}
}

class TrackWriter implements GPSListener {
    private static final String m_sPath = "file:///e:/GPS4Mobile/";
    public String m_sFilePLT = "";
    public boolean m_bWritePLT = true;
	private boolean newsegment = true;
    private OutputStream osFilePLT; 
    private FileConnection fcFilePLT;
	private Log log;
	int buffered = 0;
	GPSPoint last;

	TrackWriter(GPS4Mobile m) {
		log = m.log;
	}
	
	static Track read(String name, Log log) {
		int iData = 0;
		String sWord = "";
		char c = 'x';
		String []data = new String[100];
		
		try {
			log.write("Reading track");

			Track track = new Track(true, log);
			FileConnection fcFile = (FileConnection) Connector.open(m_sPath + name);

			if(!fcFile.exists()) {
				log.write("No waypoint file");
				return null;
			}
			InputStream stream = fcFile.openInputStream();
			InputStreamReader reader = new InputStreamReader(stream);

			int i;
			int skip = 6;
			do{
				i = reader.read(); // read one byte at the time.
				c = (char)i;
				switch(c){
				case '\n':
					data[iData] = sWord.trim();
					++iData;
					if (skip > 0)
						--skip;
					else {
						if (iData > 2) {
							if (data[2] == "1")
								track.brk();
							track.add(null, null, new GPSPoint(Double.parseDouble(data[0]), Double.parseDouble(data[1])));
						}
					}
					iData = 0;
					sWord = "";
					break;
				case ',':
					data[iData] = sWord.trim();
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
			log.write("Done");
			return track;
		}catch(Exception e){
			String message = "iData = ";
			message += iData;
			message += "; sWord = ";
			message += sWord;
			message += "; c = ";
			message += c;
			if (iData > 0) {
				message += "data[0] = ";
				message += data[0];
			}
			log.write(message);
			log.write(e);
		}
		return null;
	}

	public void add(String d, String t, GPSPoint g) {
		if (last != null && !newsegment && last.Distance(g) < 10)
			return;
		CreatePlt(d, t);
		if(m_bWritePLT){
			String sLine = "";
			Date dt = new Date();
			sLine = g.lat + "," +
				g.lng + "," + 
				(newsegment ? "1," : "0,") +
				"-777," +
				dt.toString() +"," +
				",,";
			try{                
				osFilePLT.write(sLine.getBytes());
				osFilePLT.write('\n');
			}catch(Exception e){
				log.write(e);
			}
			newsegment = false;
			++buffered;
			if (buffered > 100) {
				try {
					osFilePLT.flush();
				} catch (Exception e) {
					log.write(e);
				}
				buffered = 0;
			}
		}
	}

	public void brk() {
		newsegment = true;
	}
	
    public void CreatePlt(String d, String t) {
        if(m_bWritePLT && osFilePLT == null){
            try{
                CreateDirectory();
				if (m_sFilePLT == "")
					m_sFilePLT = d + "_" + t + ".plt";
                boolean bAddPLTHeader = false;
                FileConnection fcFile = (FileConnection) Connector.open(m_sPath + m_sFilePLT);
                if(!fcFile.exists()){
                    fcFile.create();
                    bAddPLTHeader = true;
                }                
                osFilePLT = fcFile.openOutputStream();
                if(bAddPLTHeader){
                    String sLine = "OziExplorer Track Point File Version 2.1\n";
                    osFilePLT.write(sLine.getBytes());
                    sLine = "WGS 84\n";
                    osFilePLT.write(sLine.getBytes());
                    sLine = "Altitude is in Feet\n";
                    osFilePLT.write(sLine.getBytes());
                    sLine = "Reserved 3\n";
                    osFilePLT.write(sLine.getBytes());
                    sLine = "0,3,8421504,GPS4Mobile.plt,0,0,0,8421504\n";
                    osFilePLT.write(sLine.getBytes());                    
                    sLine = "1000\n";
                    osFilePLT.write(sLine.getBytes());                   
                }
				fcFile.close();
            }catch(Exception e){
				log.write(e);
                m_bWritePLT = false;
            }
        }
    }

	public static void GetFileList(List list, Log log) {
        try {
            FileConnection fcDir = (FileConnection) Connector.open(m_sPath);
            if (!fcDir.exists())
				return;
			Enumeration e = fcDir.list("*.plt", false);
			while (e.hasMoreElements()) {
				String name = (String)e.nextElement();
				int i = 0;
				for (i = 0; i < list.size(); ++i) {
					if (name.compareTo(list.getString(i)) < 0)
						break;
				}
				FileConnection fcFile = (FileConnection) Connector.open(m_sPath + name);
				name += " (" + fcFile.fileSize() + ")";
				fcFile.close();
				list.insert(i, name, null);
			}
            fcDir.close();
        }catch(Exception e){
			log.write(e);
        }
	}

    public void CreateDirectory() {
        try {
            FileConnection fcDir = (FileConnection) Connector.open(m_sPath);
            if (!fcDir.exists())
                fcDir.mkdir();
            fcDir.close();
        }catch(Exception e){
			log.write(e);
        }
    }
}
