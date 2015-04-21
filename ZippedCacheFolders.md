# Introduction #

Since gpsvp 0.4.22 it is possible to zipped cache subfolders using the 7z format.

Advantages:
  * much more faster file transfer from PC to SD card (it is much faster to transfer a few big 7z files instead of a lot of small files)
  * save space (a lot of small files needs much more space than a big one due to the "cluster size" of the disk) even without real compression.

Disadvantages:
  * loading the tiles can be a little bit slower (but even on my quite old 200 Mhz smartphone it is fast enough)
  * it doesn't work with the WM2003 version of gpsvp (due to missing functions to open a picture directly from memory). It works on WM5/6/6.1/6.5 and on the desktop version.

# Details #

I decided to use the 7z-format. There might be other quicker formats, but it was the first format I found to be fast enough to handle files of about 100 MB on a smartphone and with source code available under a compatible license. I first tried using a simple zip format but it was really slow even on a desktop PC.

How to do it?
  * You will need the 7-zip compressor: http://www.7-zip.org/
  * I advise to use the not solid and not compressed format for best performances (The code can also cope with "solid" and "compressed" 7z files but "solid" files will probably be really slow and compressed file doesn't have a real advantage because png or jpg files are already compressed). Command line syntax: "7z.exe a -t7z -mx0 -ms=off" but of course you can use the interactive version of 7-Zip too.
  * You can compress subfolders in the cache at any level (Single limitation: mapcfg.ini should not be zipped). gpsvp 0.4.22 first looks if a tile exists as normal file, then looks along the path if the tile exists in a 7z file and then eventually try to download it.
  * If you let gpsvp download tiles, it won't compress them automatically (You have to create the 7z file manually on the pc). Downloaded tile will be stored as normal files. gpsvp has no problem having some of the tiles as normal files and other in a 7z file.

# Example #

I like to pack all tiles (from Zoom\_00=10 to Zoom=16) corresponding to a single tile of Zoom\_00=10 in a subfolder so that I can easily copy just some parts of a map on my smartphone. A Z10 tile has approximately the coverage of a day hike. In the worst case (= if a hike it at the border between several Z10 tiles) I just need 4 Z10-tiles-tree to cover a whole day.

To do that with the map from Open Cycle Map, I use this configuration:
```
[Tiled MAP]
gpsVPVersionMin=0.4.15
Filename=%Y.png
Subpath=%10QKEY/%ZOOM_00/%X
URL=http://tile.opencyclemap.org/cycle/%ZOOM_00/%X/%Y.png
```

My cache folder looks then like that:
```
OpenCycleMap\mapcfg.ini
OpenCycleMap\1200023312\10
OpenCycleMap\1200023312\11
OpenCycleMap\1200023312\12
OpenCycleMap\1200023312\13
OpenCycleMap\1200023312\14
OpenCycleMap\1200023330\10
OpenCycleMap\1200023330\11
OpenCycleMap\1200023330\12
OpenCycleMap\1200023330\13
OpenCycleMap\1200023330\14
...
```

I usually compress the subfolders with the zoom levels:
  * Right click on OpenCycleMap\1200023312\14
  * Choose 7-ZIP/Add to archive...
  * Select Archive format = 7z and Compression level = Store
  * Click OK, you will get a new file 14.7z
  * Erase the folder OpenCycleMap\1200023312\14

But you could also compress each of the subfolder under OpenCycleMap (for instance OpenCycleMap\1200023312).
If you put the configuration file with gpsvp.exe (subfolder MapConfig) you could also compress all "OpenCycleMap" in a single file.