

# Introduction #
_January 14th, 2009: This feature is now available in the official version of gpsVP (from version 0.4.16)._

Through configuration files it is possible to define new raster maps in gpsVP. This can be used in two ways:
  * For a **web based map**, i.e. to access a map which is on a web server. It’s a map like the ones which already exists in gpsVP (OpenStreetMap, ...).
  * For a **file based map**, i.e. a map where all the parts are stored in files. It can be a map which you downloaded on your PC, a scanned map, ...

# Web based map #

## Configuration file ##
For a web based map gpsVP will download parts of the map when needed. The parts ("tiles") are then stored in a cache, exactly like for the already existing maps.
  * Each map should have its own configuration file.
  * The configuration files should be stored in a subfolder `MapConfigs` with the executable (i.e. probably `\Program files\gpsVP\MapConfigs` if gpsVP is installed directly on the device).
  * The file extension must be `ini`. The file name is free but will be reused to create a subdirectory in the cache and will be displayed in the map menu.
  * The map configurations are loaded during the start of gpsVP. You need to restart gpsVP after adding or editing configurations.

## First simple example ##
Create a text file called `OpenStreetMapViaIni.ini` in the ` MapConfigs` with the following text:
```
[Tiled MAP]
gpsVPVersionMin=0.4.16
Filename=x=%X&y=%Y.png
URL=http://tile.openstreetmap.org/%ZOOM_00/%X/%Y.png
```

Ok it’s not very interesting because this map already exist in gpsVP. But you can just change the URL to access other maps. For example the Cycle map of OpenStreetMap should accessible at `http://andy.sandbox.cloudmade.com/tiles/cycle/%ZOOM_00/%X/%Y.png`.

NB: In its default configuration windows hides all known extensions and will probably hides the ".ini" part of the file name. To be able to create an ini file with the file explorer, you will probably need to activate the option to display all extensions. Alternatively it might be possible to copy an existing ini file.

## Other uses ##
This can be also used to access some WMS maps, see examples in MapExamples.

# File based map #

## Configuration file ##
A file based map must be cut in tiles to be used with gpsVP. Assuming that you already have such tiles:
  * Each map has its own subfolder that you will copy in the cache folder of gpsVP.
  * The configuration file should be placed in this subfolder and must be called `mapcfg.ini`.
  * The file syntax is the same as for a web based map. Of course you don’t need to specify an URL.
  * I recommend creating 2 levels of folders to store the tiles. The first level should correspond to each zoom level. The second level to each X (or Y) value. Of course you could put all the tiles directly in the cache subfolder or use only a single sublevel but it could be quite slow: storage cards can become really slow if some folders have too many files.

## Where to get tiles for a file based map? ##
A few possibilities:
  * Download them using the PC version of gpsVP.
  * Download them with [Googleak](http://www.linuxtechs.net/kruch/tb/forum/viewtopic.php?t=833&highlight=google+tiles).
  * Split a geotiff map using [gdal\_translate](http://www.gdal.org/gdal_translate.html) (Not easy because you probably don’t want to enter the coordinates of each tile manually. But for the one who knows how to write a script: it works, I have already done it).
  * Split a map using [GDAL2Tiles](http://www.klokan.cz/projects/gdal2tiles/) from the lastest version 1.6 of GDAL.
  * Split a map using [Maptiler](http://www.maptiler.org/) (which should be released “soon”).

See also UsingScannedMaps.

# Syntax of a configuration file #
It’s an ini file with at least a section called `[Tiled MAP]`.

## Section `[Tiled MAP]` ##
Following entries are available:
| **Entry** | **Example** | **Description** |
|:----------|:------------|:----------------|
| `gpsVPVersionMin` | `0.4.16` | Versions 0.4.16 and newer support the syntax of this configuration file. A warning will be displayed if the map is loaded with an older version of gpsVP. |
| `Filename` | `x=%X&y=%Y.png` | Filename of a tile in the cache. Variables allowed. Just make sure that there won’t be two tiles with the same name in the same folder. |
| `URL` |  | URL of a tile. Variables allowed (of  course!). |
| `Subpath` | `level=%ZOOM_01/x=%X` | Path to the tile relative to the cache folder of the map. Variables allowed. Omit it for web based map, it will used the example as default value. |
| `DemoPointLon` | `11.04791164` | Longitude of a demo point for the map. Useful for map that doesn’t cover the whole Earth. It should be a point where the map is visible. A menu entry in the context menu of gpsVP allows to center the map on this point. |
| `DemoPointLat` | `47.43018866` | Latitude of the demo point. |
| `DemoPointZoomOne` | `16` | Zoom level of the demo point from 1 for the whole Earth to currently max 18. |
| `ZoomOne1` | `RefMap1` | Reference on another section for a different URL for this zoom level. Override the default configuration. Allow the configuration composite maps. |
| ... |  | ... |
| `ZoomOne18` |  | See `ZoomOne1`. |

## Variables ##
Following variables are available for the entries `Filename`, `URL` and `Subpath`:
| **Variable** | **Description** | **Required version** |
|:-------------|:----------------|:---------------------|
| `%LONG1` | Longitude of the west side of the tile. Can be used to get tiles from a WMS Server. | 0.4.16 |
| `%LAT1` | Latitude of the south side of the tile. | 0.4.16 |
| `%LONG2` | Longitude of the east side of the tile. | 0.4.16 |
| `%LAT2` | Latitude of the north side of the tile. | 0.4.16 |
| `%X` | X-Coordinate as Google maps tile (Origin in the upper-left corner). | 0.4.16 |
| `%Y` | Y-Coordinate as Google maps tile (Origin in the upper-left corner). | 0.4.16 |
| `%ZOOM_17` | Zoom level, value 17 for the whole Earth. | 0.4.16 |
| `%ZOOM_01` | Zoom level, value 1 for the whole Earth. | 0.4.16 |
| `%ZOOM_00` | Zoom level, value 0 for the whole Earth. | 0.4.16 |
| `%TMSX` | X-Coordinate as TMS tile (Origin in the lower-left corner). | 0.4.16 |
| `%TMSY` | Y-Coordinate as TMS tile (Origin in the lower-left corner). | 0.4.16 |
| `%QRST` | Tile reference as for Google Satellite (quadtree string using the characters q, r, s and t). | 0.4.16 |
| `%QKEY` | Tile reference as for Microsoft Live Satellite (quadtree string using the characters 0, 1, 2 and 3). | 0.4.16 |
| `%EPSG_3785_LONG1` | Longitude of the west side of the tile using EPSG:3785 (= EPSG:900913) coordinates. | 0.4.24 |
| `%EPSG_3785_LAT1` | Latitude of the south side of the tile using EPSG:3785 (= EPSG:900913) coordinates. | 0.4.24 |
| `%EPSG_3785_LONG2` | Longitude of the east side of the tile using EPSG:3785 (= EPSG:900913) coordinates. | 0.4.24 |
| `%EPSG_3785_LAT2` | Latitude of the north side of the tile using EPSG:3785 (= EPSG:900913) coordinates. | 0.4.24 |

It is also possible to use only a part of some variables:
| `%`**`n`**`VAR` | Part of a `%VAR`. Replace **`n`** through the desired length. Example `%5QRST` returns the first 5 characters of a `%QRST` tile reference. |
|:----------------|:-------------------------------------------------------------------------------------------------------------------------------------------|
| `%`**`n,m`**`VAR` | Part of a `%VAR`. Replace **`n`** through the desired length and **`m`** through the first character to use. Example `%6,7QRST` returns the 6 characters starting at the 7th one. |
| `%`**`n,m,z`**`VAR` | `%VAR` but calculated for a zoom level of z (Zoom\_00) instead of the current zoom level. Can be used with or without **`n`** and **`m`** like in `%,,10X` |

| **Variable** | **Required version** |
|:-------------|:---------------------|
| `%`**`n`** for `%QRST`, `%QKEY` | 0.4.16 |
| `%`**`n,m`** for `%QRST`, `%QKEY` | 0.4.18 |
| `%`**`n`** or `%`**`n,m`** for `%X`, `%Y`, `%TMSX`, `%TMSY` | 0.4.22 |
| `%`**`n,m,z`** | 0.4.24 (not yet published) |

Details for `%ZOOM_00`:
  * Zoom level as in the version 2 of the Google Map API.
  * ZOOM\_00 = 0 => whole Earth in one tile, 1 => in 4 tiles …
  * Coarsest ZOOM\_00 in gpsVP 0.4.14 = 1 (whole Earth in 4 tiles)
  * Finest ZOOM\_00 in gpsVP 0.4.14 = 17 (whole Earth in 2<sup>17</sup>x2<sup>17</sup> tiles)

Details for `%ZOOM_01`:
  * ZOOM\_00 plus 1
  * ZOOM\_01 = 1 => whole Earth in one tile, 2 => in 4 tiles …
  * Corresponds to the "level=" in the cache subdirectories of gpsVP 0.4.14
  * Coarsest ZOOM\_01 in gpsVP 0.4.14 = 2 (whole Earth in 4 tiles)
  * Finest ZOOM\_01 in gpsVP 0.4.14 = 18

Details for `%ZOOM_17`:
  * Zoom level as in the version 1 of the Google Map API.
  * ZOOM\_17 = 17 => whole Earth in one tile, 16 => in 4 tiles …
  * Corresponds to "data.level" in the code source of gpsvp
  * Coarsest ZOOM\_17 in gpsVP 0.4.14 = 16 (whole Earth in 4 tiles)
  * Finest ZOOM\_17 in gpsVP 0.4.14 = 0 


## Referenced sections ##
The entries `ZoomOne1` to `ZoomOne18` in `[Tiled MAP]` allow the definition of composite maps. For each value there should a corresponding "referenced" section in the ini file. The same value can be used for several zoom levels.

The referenced sections must have exactly the same name as in the entry (Example: `[RefMap1]`) and can use the following entries: `Filename`, `URL`, `Subpath`.

"`ZoomOne`" in the entry name has the same zoom definition as the variable `%ZOOM_01`, i.e. `ZoomOne1` corresponds to the Earth in 1 tile.


## Character encoding ##
You can use ANSI or UTF-8.
  * ANSI is the default characters set under Windows.
  * UTF-8 permits to use Unicode characters.

The Windows XP version of Notepad can save file using ANSI or UTF-8. For UTF-8 files it will automatically add a [mark called BOM](http://de.wikipedia.org/wiki/Byte_Order_Mark) at the beginning of the file. If you use another text editor, make sure that it add a BOM at the beginning of the file if you want to use UTF-8.

NB: You can use Unicode characters for file or folder names but it probably won't work for the URLs.

# More examples #
See MapExamples.