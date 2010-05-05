import javax.microedition.lcdui.*;

class TrackPoint {
	GPSPoint g;
	IntPoint cachep;
	long t;
	int s;
	TileFactory factory;
	TrackPoint(GPSPoint gg, int ss) {
		g = gg;
		t = System.currentTimeMillis();
		s = ss;
	}
	IntPoint p(TileFactory f) {
		if (factory != f) {
			cachep = new IntPoint(g, f);
			factory = f;
		}
		return cachep;
	}
}

class Track implements GPSListener {
	static final int maxtrack = 600;
	final boolean compressed;
	private int step = 1;
    private TrackPoint []track = new TrackPoint[maxtrack];
    public int tracklen = 0;
	public IntPoint lastpoint;
	public TileFactory factory;
	int cursegment = 0;
	Track old = null;
	Log log;

	Track(boolean c, Log l) {
		compressed = c;
		log = l;
	}
	Track(boolean c, Track o, Log l) {
		compressed = c;
		old = o;
		log = l;
	}
	public void add(String date, String time, GPSPoint g, String altitude) {
		try {
			add(new TrackPoint(g, cursegment));	
		} catch (Exception e) {
			log.write("public void add(String date, String time, GPSPoint g)");
			log.write(e);
		}
	}
	void add(TrackPoint p) throws Exception {
		lastpoint = p.p(factory);
		if (!compressed) {
			if (tracklen > maxtrack && old != null) {
				try {
					old.add(track[tracklen % maxtrack]);
		        }catch(Exception e){
					log.write("old.add(track[tracklen % maxtrack]);");
					log.write(e);
					throw e;
       			}
			}
			try {
				track[tracklen % maxtrack] = p;
	        }catch(Exception e){
				log.write("track[tracklen % maxtrack] = p;");
				log.write(e);
				throw e;
			}
			++tracklen;
		} else {
			if (tracklen % step == 0) {
				try {
					if (tracklen % maxtrack + tracklen / maxtrack < maxtrack)
						track[tracklen % maxtrack + tracklen / maxtrack] = p;
		        }catch(Exception e){
					log.write("track[tracklen % maxtrack + tracklen / maxtrack] = p;");
					log.write(e);
					throw e;
				}
			}
			tracklen++;
			if (tracklen > maxtrack * step)
				step *= 2;
		}
	}
	public String GetSpeeds() {
		String res = "";
		try {
			if (compressed)
				return "";
			if (tracklen == 0)
				return "";
			TrackPoint to;
			try {
				to = track[(tracklen - 1) % maxtrack];
			}catch(Exception e){
				log.write("TrackPoint to = track[(tracklen - 1) % maxtrack];");
				log.write(e);
				throw e;
			}
			if (tracklen <= 10)
				return "";
			if (tracklen > 10) {
				try {
					TrackPoint from = track[(tracklen - 10 + maxtrack) % maxtrack];
					res += " " + (int)(to.g.Distance(from.g) * 3600 / (to.t - from.t));
				}catch(Exception e){
					log.write("TrackPoint from = track[(tracklen - 10 + maxtrack) % maxtrack];");
					log.write(e);
					throw e;
				}
			}
			if (tracklen > 60) {
				try {
					TrackPoint from = track[(tracklen - 60 + maxtrack) % maxtrack];
					res += "/" + (int)(to.g.Distance(from.g) * 3600 / (to.t - from.t));
				}catch(Exception e){
					log.write("TrackPoint from = track[(tracklen - 60 + maxtrack) % maxtrack];");
					log.write(e);
					throw e;
				}
			}
			if (tracklen > maxtrack) {
				try {
					TrackPoint from = track[(tracklen) % maxtrack];
					res += "/" + (int)(to.g.Distance(from.g) * 3600 / (to.t - from.t));
				}catch(Exception e){
					log.write("TrackPoint from = track[(tracklen) % maxtrack];");
					log.write(e);
					throw e;
				}
			}
		}catch(Exception e){
			log.write("GetSpeeds");
			log.write(e);
		}
		return res + "kmh";
	}
	public void brk() {
		++cursegment;
	}
	void Paint(Graphics g, IntPoint curpoint, int zoom, int dx, int dy) {
		try {
			int x1, x2, y1, y2, s1, s2;
			int i = 0;
			if (!compressed) {
				if (tracklen > Track.maxtrack)
					i = tracklen % Track.maxtrack;
				try {
					x1 = (track[i].p(factory).x - curpoint.x) / zoom + dx;
					y1 = -(track[i].p(factory).y - curpoint.y) / zoom + dy;
					s1 = track[i].s;
				}catch(Exception e){
					log.write("track[i]");
					log.write(e);
					throw e;
				}
				++i;
				i %= Track.maxtrack;
				
				while (i != (tracklen % Track.maxtrack)) {
					try {
						x2 = (track[i].p(factory).x - curpoint.x) / zoom + dx;
						y2 = -(track[i].p(factory).y - curpoint.y) / zoom + dy;
						s2 = track[i].s;
					}catch(Exception e){
						log.write("track[i] (146)");
						log.write(e);
						throw e;
					}
					if ((x1 != x2 || y1 != y2) && s1 == s2) {
						g.drawLine( x1 + 1, y1, x2 + 1, y2);
						g.drawLine( x1, y1 + 1, x2, y2 + 1);
						g.drawLine( x1 - 1, y1, x2 - 1, y2);
						g.drawLine( x1, y1 - 1, x2, y2 - 1);
						g.drawLine( x1, y1, x2, y2);
					}
					s1 = s2;
					x1 = x2;
					y1 = y2;
					++i;
					i %= Track.maxtrack;
				}
			} else {
				try {
					x1 = (track[0].p(factory).x - curpoint.x) / zoom + dx;
					y1 = -(track[0].p(factory).y - curpoint.y) / zoom + dy;
					s1 = track[0].s;
				}catch(Exception e){
					log.write("track[0]");
					log.write(e);
					throw e;
				}
				for (i = 0 + step; i < tracklen; i += step) {
					if (i % maxtrack + i / maxtrack < maxtrack)
					{
						try {
							x2 = (track[i % maxtrack + i / maxtrack].p(factory).x - curpoint.x) / zoom + dx;
							y2 = -(track[i % maxtrack + i / maxtrack].p(factory).y - curpoint.y) / zoom + dy;
							s2 = track[i % maxtrack + i / maxtrack].s;
						}catch(Exception e){
							log.write("track[i % maxtrack + i / maxtrack]");
							log.write(e);
							throw e;
						}
						if ((x1 != x2 || y1 != y2) && s1 == s2) {
							g.drawLine( x1 + 1, y1, x2 + 1, y2);
							g.drawLine( x1, y1 + 1, x2, y2 + 1);
							g.drawLine( x1 - 1, y1, x2 - 1, y2);
							g.drawLine( x1, y1 - 1, x2, y2 - 1);
							g.drawLine( x1, y1, x2, y2);
						}
						x1 = x2;
						y1 = y2;
						s1 = s2;
					}
				}
			}
		} catch (Exception e) {
			log.write("void Paint(Graphics g, IntPoint curpoint, int zoom, int dx, int dy)");
			log.write(e);
		}
	}
}
