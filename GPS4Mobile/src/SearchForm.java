import javax.microedition.lcdui.*;
import org.xml.sax.helpers.DefaultHandler;
import org.xml.sax.Attributes;
import javax.microedition.io.Connector;
import javax.microedition.io.ContentConnection;
import java.io.InputStream;
import javax.xml.parsers.SAXParserFactory;
import javax.xml.parsers.SAXParser;
import java.util.Vector;
import java.io.ByteArrayOutputStream;
import java.io.OutputStreamWriter;

class SearchResults extends DefaultHandler implements Runnable, CommandListener {
    private final static Command CMD_CANCEL = new Command("Cancel", Command.CANCEL, 1);

	List found;
	String url;
	int stack = 0;
	Vector points = new Vector();
	Log log;
	GPSCanvas canvas;
	Display display;
	public SearchResults(GPSCanvas c, String u, Log lo, Display d) {
		canvas = c;
		url = u;
		log = lo;
		display = d;
		found = new List("Searching ...", Choice.IMPLICIT);
		display.setCurrent(found);
		found.addCommand(CMD_CANCEL);
		found.setCommandListener(this);
	}
	public void startElement(java.lang.String uri, java.lang.String localName, java.lang.String qName, Attributes attributes) {
		++stack;
		if (localName.equals("named") && 2 == stack) {
			String name = attributes.getValue(attributes.getIndex("", "name"));
			String lat = attributes.getValue(attributes.getIndex("", "lat"));
			String lng = attributes.getValue(attributes.getIndex("", "lon"));
			found.append(name, null);
			points.addElement(new GPSPoint(Double.parseDouble(lat), Double.parseDouble(lng)));
		}
	}
	public void endElement(java.lang.String uri, java.lang.String localName, java.lang.String qName) {
		--stack;
	}
	public GPSPoint GetPoint(int i) {
		return (GPSPoint)(points.elementAt(i));
	}

	public void run() {
		try {
			ContentConnection c = (ContentConnection)Connector.open(url);
			InputStream is = c.openInputStream();
			SAXParserFactory saxFactory = SAXParserFactory.newInstance();
			saxFactory.setNamespaceAware(true);
			saxFactory.setValidating(false);
			SAXParser saxParser = saxFactory.newSAXParser();
			saxParser.parse(is, this);
		} catch (Exception e) {
			log.write(e);
		}
		found.setTitle("Found");
	}
    public void commandAction(Command c, Displayable d) {
		if (c == CMD_CANCEL)
			display.setCurrent(canvas);
		if (c == found.SELECT_COMMAND) {
			display.setCurrent(canvas);
			canvas.SetCenter(new IntPoint(GetPoint(((List)d).getSelectedIndex())));
		}
	}
}

class SearchForm extends Form implements CommandListener {
    private final static Command CMD_CANCEL = new Command("Cancel", Command.CANCEL, 1);
	private final static Command CMD_SEARCH = new Command("Search", Command.OK, 1);

	SearchResults searchResults;
	GPSCanvas canvas;
	Display display;
	TextField text;
	Log log;
	
	SearchForm(Display d, GPSCanvas c, Log l) {
		super("Search");
		canvas = c;
		display = d;
		log = l;
		text = new TextField("Name:", "", 100, TextField.ANY);
		append(text);
		addCommand(CMD_CANCEL);
		addCommand(CMD_SEARCH);
		setCommandListener(this);
	}
    public void commandAction(Command c, Displayable d) {
		if (c == CMD_CANCEL)
			display.setCurrent(canvas);
		if (c == CMD_SEARCH) {
			try {
				log.write("Searching " + text.getString());
				ByteArrayOutputStream str = new ByteArrayOutputStream();
				OutputStreamWriter writer = new OutputStreamWriter(str, "UTF-8");
				writer.write(text.getString());
				int len = str.size();
				byte[] data = str.toByteArray();
				String url = "http://www.frankieandshadow.com/osm/search.xml?find=";
				log.write("URL is " + url);
				for (int i = 0; i < len; ++i) {
					try {
						int v = data[i];
						if (v < 0)
							v += 256;
						String s = Integer.toHexString(v);
						if (s.length() < 2)
							s = "0" + s;
						s = "%" + s;
						log.write("Adding " + s);
						url += s;
			        }catch(Exception e){
						log.write("int v = data[i];");
						log.write(e);
						throw e;
        			}
				}
				str.close();
				log.write("URL is " + url);
				// append(new TextField("URL:", url, 200, TextField.ANY));
				searchResults = new SearchResults(canvas, url, log, display);
				(new Thread(searchResults)).start();
				// canvas.addCommand(GPS4Mobile.CMD_SEARCHRESULTS);
			} catch (Exception e) {
				log.write(e);
			}
		}
	}
}
