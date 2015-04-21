# Introduction #

A [Web Map Service (WMS)](http://en.wikipedia.org/wiki/Web_Map_Service) is a server that can generate raster maps on request. The maps can be quite complicated, they can merge data from different sources and can offer several layers that can be shown or hidden.

gpsVP doesn't include a full WMS client but it can be configured to request tiles (images of 256x256 pixel) from most WMS servers.


# Example with an hiking map of Bavaria #

The "Bayerische Vermessungsverwaltung" has a [WMS-Server with topographic maps](http://www.gdi.bayern.de/Geowebdienste/geowebdienste.htm) (1:50 000 and 1:500 000) of Bavaria that are free for personal use.


![http://gpsvp.googlecode.com/svn-history/r185/wiki/img/Screen-SP-TopoBavaria2.png](http://gpsvp.googlecode.com/svn-history/r185/wiki/img/Screen-SP-TopoBavaria2.png)
<= Map 1:500 000  ;  Map 1:50 000 =>
![http://gpsvp.googlecode.com/svn-history/r185/wiki/img/Screen-SP-TopoBavaria1.png](http://gpsvp.googlecode.com/svn-history/r185/wiki/img/Screen-SP-TopoBavaria1.png)

Apparently a WMS server can automatically choose the appropriate layer when you
configure it with a list of layers (separated through a comma) instead
of just a layer. Following configuration file for gpsVP will let the server automatically choose when to use which map:
```
[Tiled MAP]
gpsVPVersionMin=0.4.15
DemoPointLon=11.04791164
DemoPointLat=47.43018866
DemoPointZoomOne=16
Filename=top50-x=%X&y=%Y&zoom=%ZOOM_17.jpg
URL=http://www.geodaten.bayern.de/ogc/getogc.cgi?&REQUEST=GetMap&SERVICE=WMS&VERSION=1.1.1&LAYERS=UK500,TK50&STYLES=default&FORMAT=image/jpeg&BGCOLOR=0xFFFFFF&TRANSPARENT=TRUE&SRS=EPSG:4326&BBOX=%LONG1,%LAT1,%LONG2,%LAT2&WIDTH=256&HEIGHT=256&reaspect=false
```

Some explanations:
  * The map isn't available for the whole Earth. To make it easy to find the map without knowing where Bavaria is, I configured a "demo" point showing a mountain in the south of Germany. Simply use Context menu/Raster map/Got to demo point to center the map on it.
  * See the page MapConfiguration for a complete description of the syntax.


If you want to choose yourself which map to use for which level you can use the `ZoomOne` settings:
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

Some explanations:
  * At a fine scale (level 18 to 15) I use the map with a scale 1:50 000 that I called `TOP50`.
  * From level 14 to 10, I use the coarser map `UK500`.
  * I haven't defined a default URL for the coarsest zoom levels (9 to 1), because at those levels you just get a white image from the server.


# How to find the right URL for the configuration file? #

Here is the way I used to configure my WMS-maps:
  1. - Go to http://www.wmsviewer.com/main.asp. This is a full WMS client that works directly in a browser (It works fine with Firefox).
  1. - Use "Edit servers" to add one of the WMS-map. You will need the base url of the WMS server. It is generally something with `GetCapabilities` in it. For example if I take [one of the free WMS Servers of "Landesvermessung Sachsen"](http://www.landesvermessung.sachsen.de/ias/basiskarte4/service/register), I could add this URL: http://www.landesvermessung.sachsen.de/ias/basiskarte4/service/SRV4TK50/WMSFREE_TK/wmsservice?REQUEST=GetCapabilities&SERVICE=WMS
  1. - Refresh the map to see the new layer listed on the left part of the screen.
  1. - Deselect all other layers so that only the new one is active and then use the button "Fit All" to adjust the view to the extend of the layer.
  1. - You still won't see anything at this zoom level, but at least you are in the right extend. Zoom in several times until you see the map.
  1. - Right click in the map and choose "properties" to see the properties of the image (It works with Firefox I don't know if it works with other browsers). In the example I got the following URL: http://www.landesvermessung.sachsen.de/ias/basiskarte4/service/SRV4TK50/WMSFREE_TK/wmsservice?REQUEST=GetMap&VERSION=1.1.1&BBOX=13.2063178298612,51.3026358333335,13.2634990798612,51.3312264583335&SRS=EPSG:4326&HEIGHT=360&WIDTH=720&FORMAT=image/png&BGCOLOR=0xFFFFFF&LAYERS=MS&STYLES=&TRANSPARENT=TRUE&EXCEPTIONS=application/vnd.ogc.se_xml
  1. - Use this URL as base to configure the map, just replace height, width and the bounding box.

Notes:
  * Make sure to set `WIDTH` and `HEIGHT` to 256 pixels to get images that have the same size as google maps tiles.
  * WMS-Servers can generates tiles using different projections. Most of them support "EPSG:4326", which gave me very good results. Try to use EPSG:4326 if possible. Other projections might gives good results at fine zoom levels, but I didn't try them.
  * Most WMS-Servers can return different formats of images. For example png or gif instead of jpg. Png or gif should be smaller for maps. Jpg for satellite images.
  * The Windows XP version of gpsVP may have some color problems with some png or gif images that only use 256 colors (See [Issue 51](https://code.google.com/p/gpsvp/issues/detail?id=51)). Just try. The problem do not occur on Windows Mobile.
  * To know more about [WMS-Servers](http://www.mapbender.org/OGC_WMS).

# Incomplete servers #

Some servers do not implement a full WMS server and might not be usable with gpsVP.

For example http://geoportal.cuzk.cz/wmsportal/

Why isn't it a real WMS server?
  1. It doesn't respond to the query `GetCapabilities`. This query is used by full WMS-clients to know about the available maps and options on the server.
  1. (and that would be a problem for gpsVP) it doesn't resize the map as requested. Instead of that it returns a part of the map where the boundaries doesn't correspond anymore to the requested boundaries except for one zoom level.