class WayPoint {
	IntPoint p;
	String l;
	GPSPoint g;
	WayPoint(GPSPoint gg, String ll) {
		p = new IntPoint(gg);
		g = gg;
		l = ll;
	}
}
