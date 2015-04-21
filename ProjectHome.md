gpsVP works on virtually any Windows platform including mobile ones. It shows vector and raster maps, records and shows tracks, shows multiple navigation parameters, manages and shows waypoints. It aims to be as usable as possible.

[主頁 The main page of traditional Chinese wiki version](ZhtwMainPage.md)

# February 5th 2012 - Version 0.4.24 #
Here is the change list since version 0.4.22:
  * MapConfiguration: New variables %EPSG\_3785\_LONG1, %EPSG\_3785\_LAT1, %EPSG\_3785\_LONG2, %EPSG\_3785\_LAT2 + Variables %X, %Y, %QRST, %QKEY extended to be able to specify the zoom level at which the variables should be calculated (like %,,10X for %X at zoom\_00 = 10)
  * Raster maps: New possibilities to define the download/export area though upper left and lower right corner (on desktop or mobile device) or through an A4 paper format (only on desktop)
  * Sounds by reaching specific altitudes through a new configuration file Sounds.ini (ini file instead of the registry so that it is easy to backup the configuration).
  * Both already existing sounds (on GPS connect/disconnect + on waypoint proximity) can also be configured.
  * Sounds now works in the desktop version of gpsvp too.
  * New monitors to display the coordinates of the center tile: Tile X, Tile Y
  * New monitor "Calculated Speed" ([Issue 150](https://code.google.com/p/gpsvp/issues/detail?id=150))
  * [Issue 148](https://code.google.com/p/gpsvp/issues/detail?id=148): added namespace declaration in gpx files
  * yandex.ru map removed (was not properly working)
  * Bugfix for the experimental feature "map export"
  * [Issue 111](https://code.google.com/p/gpsvp/issues/detail?id=111): bugfix rightclick on Icons


For earlier changes see the ChangeLog in the wiki.