import javax.microedition.io.file.FileConnection;
import javax.microedition.io.Connector;
import javax.microedition.io.ContentConnection;
import java.io.InputStream;
import java.io.DataInputStream;
import java.io.OutputStream;
import java.util.*;
import javax.microedition.lcdui.Image;

class CacheElement {
	Image image;
	boolean old;
	CacheElement(Image i, boolean o)
	{
		image = i;
		old = o;
	}
}

class TileDownloader implements Runnable {

	Log log;
	GPSCanvas canvas;
	public Tile tile = null;
	public Hashtable imagecache = new Hashtable();
	public Vector imagecachecontrol = new Vector();

	TileDownloader(Log l, GPSCanvas c) {
		log = l;
		canvas = c;
	}
	
	void CreateDirs(String path) {
		int pos = 9;
		try {
			while (true) {
				pos = path.indexOf('/', pos + 1);
				if (pos < 0)
					break;
				FileConnection fcDir = (FileConnection) Connector.open(path.substring(0, pos));
				if (!fcDir.exists())
					fcDir.mkdir();
				fcDir.close();
			}
		} catch (Exception e) {
			log.write("Tried to create " + path.substring(0, pos));
			log.write(e);
		}
	}

    public void run() {
        while(true){
            try{
				if (tile != null) {
					String durl = tile.imageurl;
					String dfile = tile.filename;
					ContentConnection c = null;
					DataInputStream dis = null;
					FileConnection fcFile = null;
					try {
						c = (ContentConnection)Connector.open(durl);
						// int len = (int)c.getLength();						
						dis = c.openDataInputStream();
						CreateDirs(dfile);
						fcFile = (FileConnection) Connector.open(dfile);
						if(fcFile.exists())
							fcFile.delete();
						fcFile.create();
						OutputStream stream = fcFile.openOutputStream();

						int ch;
						while ((ch = dis.read()) != -1) {
							stream.write(ch);
						}

						stream.close();
						fcFile.close();
					} catch (Exception e) {
						log.write("Downloading " + durl + " to " + dfile);
						log.write(e);
						if (fcFile != null)
							fcFile.delete();
					}
					if (dis != null)
						dis.close();
					if (c != null)
						c.close();

					imagecache.remove(dfile);
					tile = null;
					canvas.repaint();
				}
                Thread.sleep(1000);
            }catch(Exception e){
				log.write(e);
			}
        }
    }
}
