import henson.midp.Float11;

class Tile {
	public String filename;
	public String imageurl;	
	Tile(String i, String f) {
		imageurl = i;
		filename = f;
	}
}

interface TileFactory {
	public Tile CreateTile(int x, int y, int level);
	public String GetType();
	public int lng2x(double lng);
	public int lat2y(double lat);
	public double x2lng(int x);
	public double y2lat(int y);
	public int resolution();
}

class OSMFactory implements TileFactory {
	public Tile CreateTile(int x, int y, int level) {
		String imageurl, filename;
		imageurl = "http://tile.openstreetmap.org/";
		imageurl += level - 1;
		imageurl += "/";
		imageurl += x;
		imageurl += "/";
		imageurl += y;
		imageurl += ".png";
		
		filename = "file:///e:/GPS4Mobile/osm/level=" + level + "/x=" + x;
		filename += "/osm-x=" + x + "&y=" + y;
		filename += "&zoom=" + (18 - level) + ".png";

		return new Tile(imageurl, filename);
	}
	public String GetType() {
		return "osm";
	}

	public int resolution() {
		return 256 * 256 * 256 * 2; // 2 ^ (18 - 1 + 8) = 2 ^ 25
	}

	public int lng2x(double lng) {
		return (int)((lng / 180) * resolution());
	}

	public int lat2y(double lat) {
		double t1 = lat / 180 * Math.PI;
		double t2 = Float11.log((1 + Math.sin(t1))/(1 - Math.sin(t1))) / 2;
		return (int)( t2 / Math.PI * resolution());
	}

	public double x2lng(int x) {
		return (double)x * 180 / resolution();
	}

	public double y2lat(int y) {
		double t1 = (double)y * Math.PI / resolution();
		double t2 = 2 * Float11.atan(Float11.exp(t1)) - Math.PI / 2;
		return t2 * 180 / Math.PI;
	}

}

class YandexFactory implements TileFactory {
	public Tile CreateTile(int x, int y, int level) {
		String imageurl, filename;
		imageurl = "http://wvec.maps.yandex.net/?x=";
		imageurl += x;
		imageurl += "&y=";
		imageurl += y;
		imageurl += "&z=";
		imageurl += level - 1;
		
		filename = "file:///e:/GPS4Mobile/yandex/level=" + level + "/x=" + x;
		filename += "/yandex-x=" + x + "&y=" + y;
		filename += "&zoom=" + (18 - level) + ".png";

		return new Tile(imageurl, filename);
	}
	public String GetType() {
		return "yandex";
	}

	public int resolution() {
		return 256 * 256 * 256 * 2; // 2 ^ (18 - 1 + 8) = 2 ^ 25
	}

	public int lng2x(double lng) {
		return (int)((lng / 180) * resolution());
	}

	public int lat2y(double lat) {
		double t1 = lat / 180 * Math.PI;
		double t2 = Float11.log((1 + Math.sin(t1))/(1 - Math.sin(t1))) / 2;
		int usual = (int)( t2 / Math.PI * resolution());
		
		lat = lat / 180 * Math.PI;
		double lng = 0;
		/* Radius of reference ellipsoid, default to WGS 84 */
		double Rn = resolution() / Math.PI; // 6378137.0;    
		double e  = 0.0818191908426;
        double M_PI = Math.PI;
		
		double esinLat = e * Math.sin(lat);

		double tan_temp = Math.tan(M_PI / 4.0 + lat / 2.0);
		double pow_temp = Float11.pow( Math.tan(M_PI / 4.0 + Float11.asin(esinLat) / 2), e);
		double U = tan_temp / pow_temp;
		
		double x = Rn * lng;
		double y = Rn * Float11.log(U);
		return (int)y;
	}

	public double x2lng(int x) {
		return (double)x * 180 / resolution();
	}

	public double y2lat(int y) {
		int x = 0;
        /* Radius of reference ellipsoid, default to WGS 84 */
        double R= resolution() / Math.PI; // 6378137.0;
         /* Isometric to geodetic latitude parameters, default to WGS 84 */
        double ab = 0.00335655146887969400;
        double bb = 0.00000657187271079536;
        double cb = 0.00000001764564338702;
        double db = 0.00000000005328478445;           
        double M_PI_2 = Math.PI / 2;     
        double M_PI = Math.PI;
                
        /* Isometric latitude*/        
        double xphi = Math.PI / 2.0   - 2.0 * Float11.atan(1.0 / Float11.exp(y /R));        
        
        double latitude = xphi + ab * Math.sin(2.0 * xphi) + bb * Math.sin(4.0 * xphi) + cb * Math.sin(6.0 * xphi) + db * Math.sin(8.0 * xphi);
        double longitude = x/R;                
        
        latitude      =  Math.abs( latitude )      > M_PI_2  ?  M_PI_2   :   latitude;
        longitude  =  Math.abs( longitude)  > M_PI ? M_PI     :   longitude;
        
        return latitude / Math.PI * 180;
	}
}

/*
class KSFactory implements TileFactory {
	public Tile CreateTile(int x, int y, int level) {
		String imageurl, filename;
		// 2, 0, 3 -> base = 4, x = 2 - 2 * ..., y = 2 - 0 * ... 
		int base = 1 << (level - 1);
		imageurl = "http://www.kosmosnimki.ru/printtilesrescaled/?t=image_&zt=n&x=";
		imageurl += (x - base / 2 + 1) * (1 << (26 - level));
		imageurl += "&y=";
		imageurl += (base / 2 - y) * (1 << (26 - level)) + 1;
		imageurl += "&z=";
		imageurl += level - 1;

		filename = "file:///e:/GPS4Mobile/ks/level=" + level + "/x=" + x;
		filename += "/ks-x=" + x + "&y=" + y;
		filename += "&zoom=" + (18 - level) + ".png";

		return new Tile(imageurl, filename);
	}
	public String GetType() {
		return "ks";
	}
}
*/

class MSSatFactory implements TileFactory {

	private String GetBlockName(int x, int y, int l) {
		int d = 1 << (l - 1);
		
		if ((x < 0) || (x > (d - 1))) {
			x = x % d;
			while (x < 0) {
				x += d;
			}
		}
		
		String res = "";

		for (int nPos = 0; nPos < (l - 1); nPos++) {
			d /= 2;
			if (y < d) {
				if (x < d) {
					res += '0';
				} else {
					res += '1';
					x -= d;
				}
			} else {
				if (x < d) {
					res += '2';
				} else { 
					res += '3';
					x -= d;
				}
				y -= d;
			}
		}
		return res;
	}

	public Tile CreateTile(int x, int y, int level) {
		String imageurl, filename;
		imageurl = "http://a0.ortho.tiles.virtualearth.net/tiles/a";
		imageurl += GetBlockName(x, y, level);
		imageurl += ".jpeg?g=45";

		filename = "file:///e:/GPS4Mobile/mssat/level=" + level + "/x=" + x;
		filename += "/a" + GetBlockName(x, y, level) + ".jpeg";

		return new Tile(imageurl, filename);
	}

	public String GetType() {
		return "mssat";
	}

	public int resolution() {
		return 256 * 256 * 256 * 2; // 2 ^ (18 - 1 + 8) = 2 ^ 25
	}

	public int lng2x(double lng) {
		return (int)((lng / 180) * resolution());
	}

	public int lat2y(double lat) {
		double t1 = lat / 180 * Math.PI;
		double t2 = Float11.log((1 + Math.sin(t1))/(1 - Math.sin(t1))) / 2;
		return (int)( t2 / Math.PI * resolution());
	}

	public double x2lng(int x) {
		return (double)x * 180 / resolution();
	}

	public double y2lat(int y) {
		double t1 = (double)y * Math.PI / resolution();
		double t2 = 2 * Float11.atan(Float11.exp(t1)) - Math.PI / 2;
		return t2 * 180 / Math.PI;
	}
}
