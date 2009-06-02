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
}

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
}
