import java.io.OutputStream;
import java.io.OutputStreamWriter;
import javax.microedition.io.file.FileConnection;
import javax.microedition.io.Connector;
import java.util.Date;

class Log {
    final static String path = "file:///e:/GPS4Mobile/";
    final static String name = "log.txt";
	
	private boolean created = false;
	private OutputStreamWriter writer;

	public boolean bad = false;

	boolean create(){
		if (!created) {
			try {
				FileConnection fcDir = (FileConnection) Connector.open(path);
				if (!fcDir.exists())
					fcDir.mkdir();
				fcDir.close();
                FileConnection fcFile = (FileConnection) Connector.open(path + name);
                if(fcFile.exists())
					fcFile.delete();
				fcFile.create();
                OutputStream stream = fcFile.openOutputStream();
				writer = new OutputStreamWriter(stream);
				created = true;
				fcFile.close();
			}catch(Exception e){
				bad = true;
				return false;
			}
		}
		return true;
	}
	void write(String s) {
		if (create()) {
			try {
				writer.write((new Date()).toString() + ": " + s + "\n");
				writer.flush();
				bad = false;
			} catch (Exception e) {
				bad = true;
			}
		}
	}
	void write(Exception e) {
		write(e.toString());
	}
}
