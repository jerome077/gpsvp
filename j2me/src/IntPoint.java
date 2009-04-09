import henson.midp.Float11;

class IntPoint {
	static final int resolution = 256 * 256 * 256 * 2; // 2 ^ (18 - 1 + 8) = 2 ^ 25

	int x = 0;
	int y = 0;

	public static int lng2x(double lng) {
		return (int)((lng / 180) * resolution);
	}

	public static int lat2y(double lat) {
		double t1 = lat / 180 * Math.PI;
		double t2 = Float11.log((1 + Math.sin(t1))/(1 - Math.sin(t1))) / 2;
		return (int)( t2 / Math.PI * resolution );
	}

	public static double x2lng(int x) {
		return (double)x * 180 / IntPoint.resolution;
	}

	public static double y2lat(int y) {
		double t1 = (double)y * Math.PI / IntPoint.resolution;
		double t2 = 2 * Float11.atan(Float11.exp(t1)) - Math.PI / 2;
		return t2 * 180 / Math.PI;
	}

	IntPoint(GPSPoint g) {
		x = lng2x(g.lng);
		y = lat2y(g.lat);
	}
	IntPoint(int xx, int yy) {
		x = xx;
		y = yy;
	}
	IntPoint(IntPoint p) {
		x = p.x;
		y = p.y;
	}

	int Distance(IntPoint p) {
		return Math.max(Math.abs(p.x - x), Math.abs(p.y - y));
	}
}
