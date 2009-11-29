import javax.microedition.midlet.*;
import javax.microedition.lcdui.*;
import javax.microedition.lcdui.game.*;
import java.util.*;
import javax.microedition.io.file.FileConnection;
import javax.microedition.io.Connector;
import java.io.InputStream;


public class GPSCanvas extends Canvas implements GPSListener {
	public int level = 3;

    private int WIDTH, HEIGHT;
    private GPS4Mobile midlet = null;
	Log log = null;
	private IntPoint curpos = null;
	private GPSPoint curgpspos = null;
	private IntPoint curcenter = null;
	private WayPoint nearest = null;
	public IntPoint destination = null;
	private Route route = null;

	TileFactory factory = new OSMFactory();
	long manual = System.currentTimeMillis();
	boolean fullscreen = false;

	TileDownloader downloader = null;

	void drawOutlinedString(Graphics g, int color, String s, int x1, int y1, int flags) {
		g.setColor(0xFFFFFF);
		g.drawString(s, x1 + 1, y1, flags);
		g.drawString(s, x1 - 1, y1, flags);
		g.drawString(s, x1, y1 + 1, flags);
		g.drawString(s, x1, y1 - 1, flags);
		g.setColor(color);
		g.drawString(s, x1, y1, flags);
	}

	public void ToggleFullscreen(){
		fullscreen = !fullscreen;
		setFullScreenMode(fullscreen);
        WIDTH = getWidth();
        HEIGHT = getHeight();
	}

    public GPSCanvas(GPS4Mobile m){
        midlet = m;
		log = m.log;
        WIDTH = getWidth();
        HEIGHT = getHeight();
		downloader = new TileDownloader(log, this);
		(new Thread(downloader)).start();
    }

	int FlooredDiv(int a, int b) {
		return (a >= 0) ? ((a + b - 1 ) / b) : (a / b);
	}

	protected void sizeChanged(int w, int h) {
		WIDTH = w;
		HEIGHT = h;
	}

    public void paint(Graphics g){
		g.setFont(Font.getFont(Font.FACE_PROPORTIONAL, Font.STYLE_BOLD, Font.SIZE_MEDIUM));
        g.setColor(0xFFFFFF);
        g.fillRect(0, 0, WIDTH, HEIGHT);

		int zoom = 0x80000 >> level;

		// nearest
		nearest = null;
		int mindistance = 10 * zoom;
		Enumeration en = midlet.points.getPoints();
		while (en.hasMoreElements()) {
			WayPoint pt = (WayPoint)en.nextElement();
			int distance = pt.p.Distance(curcenter);
			if (distance < mindistance) {
				mindistance = distance;
				nearest = pt;
			}					
		}
		if (nearest != null && mindistance != 0 && (curgpspos == null || System.currentTimeMillis() < manual)) {
			curcenter = new IntPoint(nearest.p);
		}


		// Tiles
		if (curcenter != null) {
			g.setColor(0x000000);
			int topx = FlooredDiv((curcenter.x / zoom - WIDTH / 2), 256);
			int topy = FlooredDiv((-curcenter.y / zoom - HEIGHT / 2), 256);
			int offset = FlooredDiv(IntPoint.resolution, zoom * 256);
			for (int x = topx - 1; ; ++x) {
				int x1 = x * 256 - (curcenter.x / zoom - WIDTH / 2);
				if (x1 + 256 < 0)
					continue;
				if (x1 > WIDTH)
					break;
				for (int y = topy - 1; ; ++y) {
					int y1 = y * 256 - (-curcenter.y / zoom - HEIGHT / 2);
					if (y1 + 256 < 0)
						continue;
					if (y1 > HEIGHT)
						break;
					Tile tile = factory.CreateTile(x + offset, y + offset, level);
					Image im = null;
					if (downloader.imagecache.containsKey(tile.filename)) {
						CacheElement ce = (CacheElement)downloader.imagecache.get(tile.filename);
						im = ce.image;
						if (ce.old && downloader.tile == null)
							downloader.tile = tile;
					} else {
						try {
							FileConnection file = (FileConnection) Connector.open(tile.filename);
							boolean old = false;
							if (file.lastModified() / 1000 + 60 * 60 * 24 * 28 < (new Date()).getTime() / 1000) {
								// log.write(file.lastModified() / 1000+" + "+60 * 60 * 24 * 28+" < "+(new Date()).getTime() / 1000);
								old = true;
								if (downloader.tile == null)
									downloader.tile = tile;
							}
							InputStream stream = file.openInputStream();
							im = Image.createImage(stream);
							downloader.imagecache.put(tile.filename, new CacheElement(im, old));
						} catch (Exception e) {
							downloader.tile = tile;
							downloader.imagecache.put(tile.filename, new CacheElement(null, true));
						}
					}
					downloader.imagecachecontrol.removeElement(tile.filename);
					downloader.imagecachecontrol.addElement(tile.filename);
					if (downloader.imagecachecontrol.size() > 32) {
						String out = (String)downloader.imagecachecontrol.firstElement();
						downloader.imagecachecontrol.removeElement(out);
						downloader.imagecache.remove(out);
					}						
					if (im != null)
						g.drawImage(im, x1, y1, g.TOP|g.LEFT);
				}
			}
		}

		// Old tracks
		if (curcenter != null) {
			g.setColor(0xbb00bb);
			Enumeration e = midlet.tracks.elements();
			while (en.hasMoreElements()) {
				Track tr = (Track)en.nextElement();
				if (tr != null)
					tr.Paint(g, curcenter, zoom, WIDTH / 2, HEIGHT / 2);
			}

		}

		GPSParser parser = midlet.parser;
		Track track = midlet.track;

		// Current track
		if (curcenter != null && track != null) {
			if (midlet.track2.tracklen > 0) {
				g.setColor(0xdd00dd);
				midlet.track2.Paint(g, curcenter, zoom, WIDTH / 2, HEIGHT / 2);
			}
			if (track.tracklen > 0) {
				g.setColor(0xff00ff);
				track.Paint(g, curcenter, zoom, WIDTH / 2, HEIGHT / 2);
			}
		}

		// Current route
		if (curcenter != null && route != null) {
			g.setColor(0x0000ff);
			route.Paint(g, curcenter, zoom, WIDTH / 2, HEIGHT / 2);
		}

		// Waypoints
		if (curcenter != null) {
			if (nearest != null) {
				addCommand(midlet.CMD_EDIT);
				addCommand(midlet.CMD_DELPOINT);
			} else {
				removeCommand(midlet.CMD_EDIT);
				removeCommand(midlet.CMD_DELPOINT);
			}

			g.setColor(0x00FF00);
			en = midlet.points.getPoints();
			int x1, y1;
			while (en.hasMoreElements()) {
				WayPoint pt = (WayPoint)en.nextElement();
				x1 = (pt.p.x - curcenter.x) / zoom + WIDTH / 2;
				y1 = - (pt.p.y - curcenter.y) / zoom + HEIGHT / 2;
				g.setColor(0x800000);
				g.fillArc(x1-2, y1-2, 4, 4, 0, 360);
				g.setColor(0xFFFFFF);
				g.fillArc(x1-1, y1-1, 2, 2, 0, 360);
			}

			// Nearest waypoint
			if (nearest != null) {
				x1 = (nearest.p.x - curcenter.x) / zoom + WIDTH / 2;
				y1 = - (nearest.p.y - curcenter.y) / zoom + HEIGHT / 2;
				g.setColor(0xFF0000);
				g.fillArc(x1-3, y1-3, 6, 6, 0, 360);
				g.setColor(0xFF7F7F);
				g.fillArc(x1-2, y1-2, 4, 4, 0, 360);
			} else {
				g.setColor(0x000000);
				g.drawLine(WIDTH / 2 - 2, HEIGHT / 2, WIDTH / 2 + 2, HEIGHT / 2);
				g.drawLine(WIDTH / 2, HEIGHT / 2 - 2, WIDTH / 2, HEIGHT / 2 + 2);
			} 
		}

		// Destination vector
		if (curpos != null && curcenter != null && destination != null) {
			int x1 = (int)((curpos.x - curcenter.x) / zoom + WIDTH / 2);
			int y1 = (int)(- (curpos.y - curcenter.y) / zoom + HEIGHT / 2);
			int x2 = (int)((destination.x - curcenter.x) / zoom + WIDTH / 2);
			int y2 = (int)(-(destination.y - curcenter.y) / zoom + HEIGHT / 2);
			g.setStrokeStyle(g.DOTTED);
			g.setColor(0xFF00FF);
			g.drawLine(x1, y1, x2, y2);
			g.setStrokeStyle(g.SOLID);
		}

		// Destination
		if (curcenter != null && destination != null) {
			g.setColor(0xFF00FF);
			int x1 = (int)((destination.x - curcenter.x) / zoom + WIDTH / 2);
			int y1 = (int)(-(destination.y - curcenter.y) / zoom + HEIGHT / 2);
			g.fillArc(x1 - 3, y1 - 3, 6, 6, 0, 360);
			g.setColor(0xFFFFFF);
			g.fillArc(x1 - 2, y1 - 2, 4, 4, 0, 360);
		}

		// Current location
		if (curpos != null && curcenter != null) {
			int x1 = (int)((curpos.x - curcenter.x) / zoom + WIDTH / 2);
			int y1 = (int)(- (curpos.y - curcenter.y) / zoom + HEIGHT / 2);
			int r = (WIDTH > 200) ? 5 : 4;
			g.setColor(0xFFFFFF);
			g.fillArc(x1 - r, y1 - r, 8, 8, 0, 360);
			--r;
			g.setColor(0x000000);
			g.fillArc(x1 - r , y1 - r, 6, 6, 0, 360);
		}

		// Distance and speed
		{
			String s = "";
			if (destination != null) {
				int dist = 0;
				if (curgpspos != null)
					dist = (int)curgpspos.Distance(new GPSPoint(destination));
				else {
					if (nearest != null)
						dist = (int)(nearest.g.Distance(new GPSPoint(destination)));
					else
						dist = (int)((new GPSPoint(curcenter)).Distance(new GPSPoint(destination)));
				}
				if (dist > 9999) {
					s += dist / 1000;
					s += "km";
				} else {
					s += dist;
					s += "m";
				}
			}
			s += track.GetSpeeds();
			int x1 = 0;
			int y1 = HEIGHT;
			if (!s.equals("")) {
				y1 -= g.getFont().getHeight();
				drawOutlinedString(g, 0x660066, s, x1, y1, g.TOP | g.LEFT);
			}
			if (nearest != null) {
				y1 -= g.getFont().getHeight();
				drawOutlinedString(g, 0x660000, nearest.l, x1, y1, g.TOP | g.LEFT);
			}
		}
		
		// Scale
		if (curcenter != null) {
			GPSPoint gpcenter = new GPSPoint(curcenter);
			String s = "";
			int scale = (int)(WIDTH * (20000000.0 / (IntPoint.resolution / zoom)) * Math.cos(Math.toRadians(gpcenter.lat)));
			if (scale > 9999) {
				s += scale / 1000;
				s += "km";
			} else {
				s += scale;
				s += "m";
			}
			int x1 = WIDTH / 2;
			int y1 = 1;
			drawOutlinedString(g, 0x000000, s, x1, y1, g.TOP | g.HCENTER);
		}
		
		// Connection status
		g.setColor(0x000000);
		g.fillArc(WIDTH - 7, 1, 6, 6, 0, 360);
		if (parser != null && parser.disconnect == true) {
			removeCommand(midlet.CMD_DISCONNECT);
			addCommand(midlet.CMD_CONNECT);
		}
		if (parser != null && parser.disconnect == false) {
			addCommand(midlet.CMD_DISCONNECT);
			removeCommand(midlet.CMD_CONNECT);
		}
		if (parser != null && parser.connected == true) {
			g.setColor(0x00FF00);
			g.fillArc(WIDTH - 6, 2, 4, 4, 0, 360);
		} else {
			if (parser.disconnect == true)
				g.setColor(0x000000);
			else
				g.setColor(0xFF0000);
			g.fillArc(WIDTH - 6, 2, 4, 4, 0, 360);
		}
		
		// Log alert
		if (log.bad) {
			g.setColor(0x000000);
			g.fillArc(1, 1, 8, 8, 0, 360);
			g.setColor(0xFF0000);
			g.fillArc(2, 2, 6, 6, 0, 360);
		}
	}
	
	public void route() {
		if (destination != null) {
			if (curgpspos != null) {
				route = new Route(curgpspos, new GPSPoint(destination), log, this);
			} else {
				route = new Route(new GPSPoint(curcenter), new GPSPoint(destination), log, this);
			}
		}
	}

	public void keyRepeated(int key) {
		keyPressed(key);
	}

    public void keyPressed(int key){
		int zoom = 0x80000 >> level;
        switch(getGameAction(key)){
		case GAME_B:
			if (zoom > 2) {
				zoom /= 2;
				++level;
				repaint();
			}
			break;
		case GAME_A:
			if (zoom < 256 * 256) {
				zoom *= 2;
				--level;
				repaint();
			}
			break;
		case FIRE:
			ToggleFullscreen();
			break;
		case LEFT:
			curcenter.x -= 15 * zoom;
			manual = System.currentTimeMillis() + 60000;
			repaint();
			break;
		case RIGHT:
			curcenter.x += 15 * zoom;
			manual = System.currentTimeMillis() + 60000;
			repaint();
			break;
		case UP:
			curcenter.y += 15 * zoom;
			manual = System.currentTimeMillis() + 60000;
			repaint();
			break;
		case DOWN:
			curcenter.y -= 15 * zoom;
			manual = System.currentTimeMillis() + 60000;
			repaint();
			break;                
		}
    }

	public void brk() {
		if (curgpspos != null)
			repaint();
		curgpspos = null;
		curpos = null;
	}

	public void add(String date, String time, GPSPoint g) {
		if (curgpspos != g)
			repaint();
		curgpspos = g;
		curpos = new IntPoint(g);
		if (System.currentTimeMillis() > manual)
			curcenter = new IntPoint(g);
		addCommand(midlet.CMD_ADDPOINT);
	}

	public void SetCenter(IntPoint p) {
		curcenter = p;
		repaint();
	}

	public IntPoint GetCenter() {
		return curcenter;
	}

	public GPSPoint getGPSPos() {
		return curgpspos;
	}

	public WayPoint GetNearest() {
		return nearest;
	}
}
