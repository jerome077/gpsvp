import henson.midp.Float11;

public class GPSPoint {
    public double lat = 0;
    public double lng = 0;
    
    public GPSPoint(double lat1, double lng1) {
        lat = lat1;
        lng = lng1;
	}

	public GPSPoint(IntPoint pt, TileFactory f) {
        lng = f.x2lng(pt.x);
        lat = f.y2lat(pt.y);
	}
	
	private double Sqr(double x) {
		return x * x;
	}
	
	public double Distance(GPSPoint g) {
        double rad = 6372795;

        double lat1 = Math.toRadians(lat);
        double lat2 = Math.toRadians(g.lat);
        double lng1 = Math.toRadians(lng);
        double lng2 = Math.toRadians(g.lng);

        double cl1 = Math.cos(lat1);
        double cl2 = Math.cos(lat2);
        double sl1 = Math.sin(lat1);
        double sl2 = Math.sin(lat2);
        double delta = lng2 - lng1;
        double cdelta = Math.cos(delta);
        double sdelta = Math.sin(delta);

        double p1 = Sqr((cl2*sdelta));
        double p2 = Sqr((cl1*sl2) - (sl1*cl2*cdelta));
        double p3 = Math.sqrt(p1 + p2);
        double p4 = sl1*sl2;
        double p5 = cl1*cl2*cdelta;
        double p6 = p4 + p5;
        double p7 = p3/p6;
        double anglerad = Float11.atan(p7);
        if (anglerad < 0)
                anglerad += Math.PI;
        double dist = anglerad*rad;

        return dist;
	}
}
