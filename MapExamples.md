

See the page MapConfiguration for a complete description of the syntax.

# Simple tile server: Open Street Map #

[OpenStreetMap](http://www.openstreetmap.org/) is a free editable map of the whole world.

![http://gpsvp.googlecode.com/svn/wiki/img/Screen-SP-OSM1.png](http://gpsvp.googlecode.com/svn/wiki/img/Screen-SP-OSM1.png)
<= Mapnik  ;  Osmarender =>
![http://gpsvp.googlecode.com/svn/wiki/img/Screen-SP-OSM2.png](http://gpsvp.googlecode.com/svn/wiki/img/Screen-SP-OSM2.png)


The Mapnik version already exists in gpsVP but it is a good simple example:
```
[Tiled MAP]
gpsVPVersionMin=0.4.15
Filename=x=%X&y=%Y.png
URL=http://tile.openstreetmap.org/%ZOOM_00/%X/%Y.png
```

The Osmarender version is the same map with another rendering. To display it in gpsVP use following configuration file:
```
[Tiled MAP]
gpsVPVersionMin=0.4.15
Filename=x=%X&y=%Y.png
URL=http://tah.openstreetmap.org/Tiles/tile/%ZOOM_00/%X/%Y.png
```


# WMS-Server with 2 maps depending on the zoom level: Bavaria hiking map #

The "Bayerische Vermessungsverwaltung" has a [WMS-Server with topographic maps](http://www.gdi.bayern.de/Geowebdienste/geowebdienste.htm) (1:50 000 and 1:500 000) of Bavaria that are free for personal use.


![http://gpsvp.googlecode.com/svn/wiki/img/Screen-SP-TopoBavaria2.png](http://gpsvp.googlecode.com/svn/wiki/img/Screen-SP-TopoBavaria2.png)
<= Map 1:500 000  ;  Map 1:50 000 =>
![http://gpsvp.googlecode.com/svn/wiki/img/Screen-SP-TopoBavaria1.png](http://gpsvp.googlecode.com/svn/wiki/img/Screen-SP-TopoBavaria1.png)

With the following configuration file you can use the maps in gpsVP:
```
[Tiled MAP]
gpsVPVersionMin=0.4.15
ZoomOne10=UK500
ZoomOne11=UK500
ZoomOne12=UK500
ZoomOne13=UK500
ZoomOne14=UK500
ZoomOne15=TOP50
ZoomOne16=TOP50
ZoomOne17=TOP50
ZoomOne18=TOP50
DemoPointLon=11.04791164
DemoPointLat=47.43018866
DemoPointZoomOne=16

[TOP50]
Filename=top50-x=%X&y=%Y&zoom=%ZOOM_17.jpg
URL=http://www.geodaten.bayern.de/ogc/getogc.cgi?&REQUEST=GetMap&SERVICE=WMS&VERSION=1.1.1&LAYERS=TK50&STYLES=default&FORMAT=image/jpeg&BGCOLOR=0xFFFFFF&TRANSPARENT=TRUE&SRS=EPSG:4326&BBOX=%LONG1,%LAT1,%LONG2,%LAT2&WIDTH=256&HEIGHT=256&reaspect=false

[UK500]
Filename=uk500-x=%X&y=%Y&zoom=%ZOOM_17.jpg
URL=http://www.geodaten.bayern.de/ogc/getogc.cgi?&REQUEST=GetMap&SERVICE=WMS&VERSION=1.1.1&LAYERS=UK500&STYLES=default&FORMAT=image/jpeg&BGCOLOR=0xFFFFFF&TRANSPARENT=TRUE&SRS=EPSG:4326&BBOX=%LONG1,%LAT1,%LONG2,%LAT2&WIDTH=256&HEIGHT=256&reaspect=false
```

For more explanations see [UsingWMSServers](UsingWMSServers.md).


# File based map: Using the cache of Googleak #

NB: gpsVP can also download tiles, see _How to start a raster maps bulk downloading_ in the [FAQ](FAQ.md).

[Googleak](http://www.trekbuddy.net/forum/viewtopic.php?p=7610) (or [the new .NET version](http://www.trekbuddy.net/forum/viewtopic.php?t=3772)) is a software with a quite user friendly interface to download tiles on your PC. The tiles in the cache of Googleak can be used in gpsVP simply by adding a short configuration file.

For example to use the Yahoo Maps cache of Googleak:
  * Copy the Googleak folder `tiles\yahoo\map` to your gpsVP cache.
  * Rename it to `YahooMaps`
  * Add the following file called `mapcfg.ini` to the folder:
```
[Tiled MAP]
gpsVPVersionMin=0.4.15
Filename=%Y
Subpath=%ZOOM_01/%X
URL=  
```
  * That's it. Now just start gpsVP and choose the raster map `YahooMaps`.


# File based map: JTileDownloader #

[JTileDownloader](http://wiki.openstreetmap.org/wiki/JTileDownloader) is a tile downloader for [OpenStreetMap](http://www.openstreetmap.org/).

Fridolin reported on the discussion group that following configuration file is needed to use the downloaded tiles with gpsVP:
```
[Tiled MAP]
gpsVPVersionMin=0.4.15
Filename=%Y.png
Subpath=%ZOOM_00/%X
```


# File based map: Using the cache of PDA Tilemanager #

[PDA TileManager](http://www.amberhome.de/pdatilemanager.html) is a software which can download tiles along a gpx track, within a circular area or within a manually entered area. It works directly on the PDA or on the PC. The maps are configurable through an xml file.

Here the configuration file needed to use one of these maps with gpsvp:
```
[Tiled MAP]
gpsVPVersionMin=0.4.17
Filename=%Y.png
Subpath=%ZOOM_00/%X
```

NB: you can also configure an URL to be able to download tiles as well with pda tilemanager as with gpsvp (take the urlPattern of pda tilemanager and replace $z through %ZOOM\_00, $x trough %X and $y through %Y)


# File based map: Mobile Atlas Creator #

[Mobile Atlas Creator](http://trekbuddyatlasc.sourceforge.net/index.html) is a powerfull tiles downloader with a user friendly interface. It can generate atlases for different programs. gpsVP isn't in the list, but you can use some of the other avaible formats and simply add a configuration file.

Here the configuration file needed to use the Mobile Trail format:
```
[Tiled MAP]
gpsVPVersionMin=0.4.15
Subpath=%ZOOM_00/%X
Filename=%Y.png
```
You need to put the configuration file in the subfolder named like the name of the map and copy this subfolder into the cache of gpsVP.

Other formats like Maverick atlas format should also be usable especially if you want to avoid having a lot of image files on your storage card (it uses another file extension, probably to avoid some media managers automatically indexing the tiles)

# Tile Server with "qrst"-URL: NOAA nautical charts #

The NOAA raster nautical charts can apparently be downloaded [here as GeoTiff](http://www.charts.noaa.gov/RNCs/RNCs.shtml) for personal use, so that it should possible to cut them for use with gpsVP using GDAL2tiles.

![http://gpsvp.googlecode.com/svn/wiki/img/Screen-SP-Nautical1.png](http://gpsvp.googlecode.com/svn/wiki/img/Screen-SP-Nautical1.png)
<= two different scales of the map =>
![http://gpsvp.googlecode.com/svn/wiki/img/Screen-SP-Nautical2.png](http://gpsvp.googlecode.com/svn/wiki/img/Screen-SP-Nautical2.png)

But there are also available as Google Maps Overlay [here](http://www.geogarage.com/main/examples.html.en#noaa) or [here](http://demo.geogarage.com/noaa).

Following configuration permits to display the overlay in gpsVP:

**Second version, which works with gpsVP 0.4.18 or newer but which works with all zoom levels (the first version couldn't display anything finer than zoom 11)**
```
[Tiled MAP]
gpsVPVersionMin=0.4.17
Filename=%QRST.png
URL=http://tiles5.geogarage.com/noaa/%6QRST/%QRST.png
ZoomOne12=NOAA_12_18
ZoomOne13=NOAA_12_18
ZoomOne14=NOAA_12_18
ZoomOne15=NOAA_12_18
ZoomOne16=NOAA_12_18
ZoomOne17=NOAA_12_18
ZoomOne18=NOAA_12_18
DemoPointLon=-75.10237
DemoPointLat=38.79779
DemoPointZoomOne=17

[NOAA_12_18]
Filename=%QRST.png
URL=http://tiles5.geogarage.com/noaa/%6QRST/%6,7QRST/%QRST.png
```
(I put a "demo" point on the east coast. If you can't find the map just click context menu/raster map/go to demo point. But note that the map also cover the west coast of the USA)