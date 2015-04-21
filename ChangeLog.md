# Introduction #

Here is the list of the changes since gspvp became opensource. For a more detailed change log see the code source log.

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

# January 23th 2011 - Version 0.4.22 #
CAUTION: NOT PROPERLY WORKING support for yandex.ru Map is present in this version (Yandex uses different projection and that's why tiles with the same coords are shifted against google ones). Everything else should work as fine as in the previous version.

Here is the change list since version 0.4.20:
  * Geoid correction matrix: Updated values Atlantic & east America's. Matrix is now complete (cybermaus)
  * Altitude correction into setup menu (cybermaus)
  * Google Maps now use language identifier from GetLocaleInfo. So it respects regional settings in user's host.  (vlysenkov)
  * Slide right/left on a monitors row in map mode will switch to previous/next monitors row. (vlysenkov)
  * Map configuration: Partial variables (Syntax %n,m) now also possible for %X, %Y, %TMSX, %TMSY (jerome077)
  * New monitors to display the current zoom level of the raster map: ZOOM\_00, ZOOM\_01 and ZOOM\_17 (jerome077)
  * Save flash card health for those ones who does refresh cache. Raster->Maintenance->Refresh... routine now compares old and new contents for each downloaded file to prevent file rewrite with the same data. (vlysenkov)
  * Enlarge screen buttons again (for easier finger operation) (vlysenkov)
  * New function "Show available tiles" based on the same current view as for downloading tiles but which draw a new track with the outline of all available tiles in the view. (jerome077)
  * Zipped subfolders in the cache: gpsvp can read tiles out of zipped folders in the cache (Not available on WM2003). Format should be 7z. For good performance please use the not solid and not compressed format (7z.exe a -t7z -mx0 -ms=off). It still saves lot of place and tranfering file from a PC to an SD card is much faster. (jerome077)
  * Experimental feature to export a map (reassembling tiles using gdal) (jerome077)
  * Other improvements

# March 02 2010 - Version 0.4.20 #
Here is the change list since version 0.4.18:
  * Geoid altitude correction: Project headers (cybermaus)
  * Use GPS time (instead of system time) and subsecond time accuracy in the recorded track (Alexey.Kruglov)
  * Show Sun azimuth (Alexey.Kruglov)
  * Made screen buttons larger for easier finger operation (vlysenkov)
  * Track analysis (length, altitude differences) (jerome077)
  * Routing (route editor, route following, distance along the route) (jerome077)
  * "Home only" mode to avoid internet connection while roaming (vlysenkov)
  * [Issue 62](https://code.google.com/p/gpsvp/issues/detail?id=62) (Enhancement): Remember Monitor Line settings (vlysenkov)
  * "Relative course" monitor (vlysenkov)
  * Improved Speed for raster maps when using "Prefer Google zoom levels" (jerome077)
  * Bugfix [Issue 71](https://code.google.com/p/gpsvp/issues/detail?id=71): new url to search in OpenStreetMap.org (jerome077)
  * [Issue 62](https://code.google.com/p/gpsvp/issues/detail?id=62) (Enhancement): Function to refresh the cache (inside a region or whole) (vlysenkov)
  * Other improvements (vlysenkov, ...)

# May 29 2009 - Version 0.4.18 #
Here is the change list since version 0.4.16 (I hope I didn't forgot anything):
  * Support for proxy server (alexeikasatkin)
  * Higher precision for internal coordinates format (Alexey.Kruglov)
  * Improved GPS read cycle (cybermaus)
  * Support for GPX waypoints and track files (jerome077)
  * Support for UTM coordinate format (jerome077)
  * Export of waypoints to Open Street Map format with free defined OSM tags (jerome077)
  * Possibility to save the tiles as hidden file, to avoid image-album applications picking up the map tile cache (cybermaus)
  * Extension of some variables for user configured raster maps (jerome077)
  * New languages files (Matobo, brotbuexe, vsevolod)
  * Bugfix: `SubPath` now configurable for web based user maps (jerome077)
  * Bugfix [Issue 52](https://code.google.com/p/gpsvp/issues/detail?id=52): Waypoint coordinates unstability (jerome077)
  * Bugfix [Issue 39](https://code.google.com/p/gpsvp/issues/detail?id=39): Leave full screen in monitor mode on Touch only devices (cybermaus)
  * Bugfix [Issue 89](https://code.google.com/p/gpsvp/issues/detail?id=89): screen buttons for user configured map (jerome077)
  * Bugfix [Issue 90](https://code.google.com/p/gpsvp/issues/detail?id=90): Deleting "broken" tiles, only if internet & downloading are enabled (jerome077)
  * Bugfix: Garmin POI label show (hawordg)
  * Other improvements (vlysenkov, vsevolod.shorin, Alexey.Kruglov, cmkgroup, ...)

# Jan 11 2009 - Version 0.4.16 #
This is the first release of gpsVP since it became opensource and I'm very glad that it is really team work now. We skip version 0.4.15 to keep clear what source was used for what version. Now it's 0.4.16. I can't tell for sure, from what source 0.4.14 was built so some improvements made may be missing from this list.

  * Configurable tile sources (jerome077)
  * Option to invert satellite images. Looks better in direct sunlight (vlysenkov)
  * Google topo maps (vlysenkov)
  * Better font colors (jozo.hovan)
  * Improve Garmin labels support (jozo.hovan)
  * Option to show only polygon outline when combining raster and vector maps (cybermaus)
  * Better NMEA support (chris)
  * Fix problem with closing sockets on Windows (jerome077)
  * Keep memory low for raster maps (jerome077)
  * Danish language
  * Many other improvements (jerome077, vlysenkov, vsevolod.shorin)