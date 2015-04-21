

# Introduction #

To use a scanned map with gpsVP, you need to prepare it:
  1. [Georeference](http://en.wikipedia.org/wiki/Georeference) the map.
  1. Split the georeferenced map in tiles (images of 256x256 pixels).
  1. Add a small "file based MapConfiguration" file to the tiles.


# Georeferencing a map #

_it will be good to list a few tools to georeference maps here. Meanwhile you can make a search on Google ;-) and it would be nice if you could report of your experiences here!_

  * With [Map Cruncher](http://research.microsoft.com/en-us/um/redmond/projects/mapcruncher/) you can georeference the map and generate tiles in one step. It is quite easy to use, the georeferencing is done with the help of satellite images. It's perfect to use with gpsVP if your map isn't already georeferenced.


# Splitting a map in tiles #

Here are a few possibilities to split a georeferenced map in tiles:
  * [GDAL2Tiles](http://www.klokan.cz/projects/gdal2tiles/): Command line application (included in GDAL version 1.6, older versions won't do it).
  * [Maptiler](http://www.maptiler.org/): beta version available. I tried it to split a map, which first needed to be reprojected and I was impressed by the quality of the generated tiles. The user interface of the beta version isn't very stable, but as soon as the calculations are started it works great.
  * Commercial applications like [Global Mapper](http://www.globalmapper.com/) seems to be able to do it too. There is also a free limited-feature version of called [dlgv32 Pro](http://mcmcweb.er.usgs.gov/drc/dlgv32pro/), where the web formats export functions seems to be functional.
  * With [Map Cruncher](http://research.microsoft.com/en-us/um/redmond/projects/mapcruncher/) you can georeference the map and generate tiles in one step.


# Configuration file for the tiles #

The map configuration file depends on the software used to generate the tiles. The principe is to copy the tiles to a subfolder of the cache folder of gpsVP and to add a small text file called `mapcfg.ini` to the folder (More here: MapConfiguration).

## Map Cruncher ##
Tested with Map Cruncher 3.2.4: I made a copy of the folder `Layer_NewLayer` to gpsVP cache and added the following configuration file:
```
[Tiled MAP]
gpsVPVersionMin=0.4.15
Filename=%QKEY.png
SubPath=/
```

## GDAL2Tiles ##
It uses TMS tiles coordinates. The following file should do it:
```
[Tiled MAP]
gpsVPVersionMin=0.4.15
Filename=%TMSY.png
Subpath=%ZOOM_00/%TMSX
```

## Maptiler ##
Same as for GDAL2Tiles, eventually using jpg instead of png.

# Tiles vs single image #

Some GPS software allow you to use georeferenced maps without splitting them in tiles for each zoom level. This is currently not possible with gpsVP.

The use of tiles has several advantages:
  * Maps can cover bigger regions because the size limit is the size of the storage card instead of the size of the main memory.
  * You can use different scanned maps depending on the zoom level. The finest zoom level can have more details.
  * Display speed is the same at all zoom levels.

But there are also a few disadvantages:
  * The "ideal" zoom level of the scanned map might not be one of the tiles zoom level.
  * The resulting files take more space on the storage card because each zoom level is precalculated.
  * There is currently no way in gpsVP to pack the tiles in a single file so that you lose storage space depending on the [cluster](http://en.wikipedia.org/wiki/Cluster_(file_system)) size of the storage card.