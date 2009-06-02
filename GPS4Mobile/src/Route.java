import org.xml.sax.helpers.DefaultHandler;
import org.xml.sax.Attributes;
import javax.xml.parsers.SAXParserFactory;
import javax.xml.parsers.SAXParser;
import javax.microedition.io.Connector;
import javax.microedition.io.ContentConnection;
import java.util.*;
import java.io.InputStream;

import javax.microedition.lcdui.*;


class Route extends DefaultHandler implements Runnable
{
	String url;
	Vector points = new Vector();
	Log log;
	GPSCanvas canvas;
	
	Route(GPSPoint from, GPSPoint to, Log l, GPSCanvas c)
	{
		log = l;
		canvas = c;
		try {
			url = "http://cooper.gpsvp.com:6789/way.xml?" + from.lat + "," + from.lng + "," + to.lat + "," + to.lng;
			log.write("URL is " + url);
			(new Thread(this)).start();
		} catch (Exception e) {
			log.write(e);
		}
	}

	public void startElement(java.lang.String uri, java.lang.String localName, java.lang.String qName, Attributes attributes) {
		if (localName.equals("point")) {
			String lat = attributes.getValue(attributes.getIndex("", "lat"));
			String lng = attributes.getValue(attributes.getIndex("", "lng"));
			points.addElement(new IntPoint(new GPSPoint(Double.parseDouble(lat), Double.parseDouble(lng))));
		}
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
			canvas.repaint();
		} catch (Exception e) {
			log.write(e);
		}
	}

	void Paint(Graphics g, IntPoint curpoint, int zoom, int dx, int dy) {
		try {
			if (points.size() == 0)
				return;
			int x1, x2, y1, y2;
			x1 = 0;
			y1 = 0;
			int i = 0;
			boolean first = true;
			++i;
			Enumeration en = points.elements();
			while (en.hasMoreElements()) {
				IntPoint p = (IntPoint)en.nextElement();
				x2 = (p.x - curpoint.x) / zoom + dx;
				y2 = -(p.y - curpoint.y) / zoom + dy;
				if (!first) {
					g.drawLine( x1 + 1, y1, x2 + 1, y2);
					g.drawLine( x1, y1 + 1, x2, y2 + 1);
					g.drawLine( x1 - 1, y1, x2 - 1, y2);
					g.drawLine( x1, y1 - 1, x2, y2 - 1);
					g.drawLine( x1, y1, x2, y2);
				}
				x1 = x2;
				y1 = y2;
				first = false;
			}
		} catch (Exception e) {
			log.write(e);
		}
	}
}
