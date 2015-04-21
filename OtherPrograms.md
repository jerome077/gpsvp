

# Software for GPS #

  * [VirtualGPS](http://www.kamlex.com/index.php?option=com_content&view=article&id=49&Itemid=55) from Kamlex
    * Mobile phone (WM2003-WM6.5)
    * Freeware (lite version)
    * If you use a bluetooth gps and that you don't always take it with you, you still can get your approximated position with the help of VirtualGPS. It uses the cell-IDs from the cellular network. You will need an internet connection on your mobile phone. The transferred amount of data for one localization isn't very important (as far as I saw, about 0.5kB for one localization)

  * [GPS Mod Driver](http://forum.xda-developers.com/showthread.php?t=571266) from Mach2003
    * Mobile phone (WM2003-WM6.5)
    * Freeware
    * Intermediate driver, between hardware port (or GPSID) and virtual GPS port.
    * Corrects lag by prediction next position
    * Multiple programs reading one GPS port
    * Injects direction data from hardware compass sensor when available.
    * Corrects altitude from WGS84 to EGM2008 Sealevel
    * When installed, uses Cell-ID from cellular netowork as position.

  * [BT747](http://www.bt747.org/) (or [development site](http://sourceforge.net/projects/bt747/))
    * Mobile phone or desktop (Java/Superwaba)
    * Open source (GPL)
    * If you use a bluetooth gps with MTK chip you can use BT747 to download recorded tracks on your PC or directly on your mobile phone and display then them in gpsVP. It can also be used to change the settings of the gps directly from the mobile phone.

# Software for raster maps #

  * [Googleak](http://www.trekbuddy.net/forum/viewtopic.php?p=7610) (or [the new .NET version](http://www.trekbuddy.net/forum/viewtopic.php?t=3772))
    * Desktop (Windows)
    * Freeware
    * It is a tiles downloader with a user friendly interface. See MapExamples for how to configure gpsVP to use downloaded tiles.

  * [PDA TileManager](http://www.amberhome.de/pdatilemanager.html)
    * Mobile phone or desktop (.NET CF 2.0)
    * Freeware
    * It is tiles downloader, which can download tiles along a gpx track, within a circular area or within a manually entered area. The maps are configurable through an xml file. See MapExamples for how to configure gpsVP to use downloaded tiles.

  * [JTileDownloader](http://wiki.openstreetmap.org/wiki/JTileDownloader)
    * Desktop (Java)
    * Open source (GPL)
    * It is a tiles downloader for [OpenStreetMap](http://www.openstreetmap.org/). See MapExamples for how to configure gpsVP to use downloaded tiles.

  * [Map Cruncher](http://research.microsoft.com/en-us/um/redmond/projects/mapcruncher/) von Microsoft
    * Desktop (Windows)
    * Freeware
    * Map Cruncher can georeference scanned map and generate tiles in one step. It is quite easy to use, the georeferencing is done with the help of satellite images. See UsingScannedMaps for how to configure gpsVP to use generated tiles.

  * [Maptiler](http://www.maptiler.org/) (or [development site](http://code.google.com/p/maptiler/)) a graphical interface for [GDAL2Tiles](http://www.klokan.cz/projects/gdal2tiles/)
    * Desktop (Windows/OS X/Linux)
    * Open source
    * Maptiler or GDAL2Tiles can be used to split a georeferenced scanned map into tiles. See UsingScannedMaps for how to configure gpsVP to use generated tiles.
    * Seems to generate tiles from very good quality even if the map needs to be reprojected.

  * [Mobile Atlas Creator](http://trekbuddyatlasc.sourceforge.net/index.html)
    * Desktop (Java => Windows/OS X/Linux)
    * Open source (GPL)
    * It is a tiles downloader with a user friendly interface. See MapExamples for how to configure gpsVP to use downloaded tiles.

  * [dlgv32 Pro](http://mcmcweb.er.usgs.gov/drc/dlgv32pro/)
    * Desktop (Windows)
    * Freeware, free limited-feature version of Global Mapper. The web format export functions seems to be functional and can be used to generate tiles.