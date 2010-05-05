class IntPoint {
	int x = 0;
	int y = 0;

	IntPoint(GPSPoint g, TileFactory f) {
		x = f.lng2x(g.lng);
		y = f.lat2y(g.lat);
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
