#labels Featured
**Q: How do I install gpsVP?**

A: Usual procedure is to download zip archive, extract cab file from it, copy it to your device, open with some file manager and then follow the instructions.


**Q: Where do I get maps for gpsVP?**

A1: Search free maps or read how to download worldmap.

A2: If enabled, raster maps are automatically downloaded, displayed and saved to cache folder.


**Q: How to start a raster maps bulk downloading?**

A: Choose the raster map type and zoom in/out to see the whole map fragment you intend to download. Then select Maps->Raster maps->Download maps->Add current view.
Select "Download all lower levels" from the same submenu if you need less detailed levels too.
Zoom in to the desired detail level. Then select "Start with current zoom" from the same submenu. Gpsvp will ask you if the required tiles count is suitable for you. Hit "Ok" and observe the "Download queue" monitor value during the download process ("Data total", "Data in", "Internet" monitors are also informative).
Please note that the download queue is nod saved across gpsvp sessions.
_**Hint**: download maps on your Windows PC. You can resize an application window to contain exactly the desired map fragment and nothing more. Needless to say it is much faster._


**Q: I tried it on my HTC P3300 (ASUS 696). It installed and it runs. It displays a map with a crosshair and a symbol in the upper right corner on it, but then it stops. I cannot enter the menu.**

A: Actually, HTC P3300 isn’t running WM Smartphone. It’s running WM PocketPC Phone Edition, just another operating system. I have a version for this platform available too from download page.


**Q: My device uses COM4 for the GPS. Is this a setting I can change somewhere?**

A: Port may be selected in Menu->Setup->Settings


**Q: Is it possible to save your tracks in a google earth compatible format?**

A: There are many tools for converting tracks, e.g. http://www.gpsvisualizer.com/map?form=googleearth


**Q: I’ve opened a map with gpsVP but I cannot see it.**

A1:Try to close all maps, open this one (gpsVP centers on opened map) and try to zoom in/out. gpsVP shows a map only when detail level of map is close to displayed detail level.

A2: Make sure that your map isn’t locked. gpsVP doesn’t support locked Garmin maps.

A3:If you have created your map with PatchIMG, you should use separate maps instead. Maps created with PatchIMG require more sophisticated algorithm to find out what to show.


**Q: I encounter an "internet error", how to solve it?**

A: Was it with gpsVP 0.4.14? Upgrade to gpsVP 0.4.16! gpsVP used to wait 60 seconds when an network error occurred (and network errors occur quit often on some server). gpsVP 0.4.16 retries much more quickly (There are still cases where it waits 60s but only if several network errors occurred in a short time).

**Q: Can I use a scanned map?**

A: Yes you can but you need to prepare it: Georeference it and split it in tiles. I have created a new wiki page about that: UsingScannedMaps

**Q: How to read the scale bar?**

A: The scale bar is composed of several segments of different lengths. The numbers on the scale bar correspond to the length of ONE segment (and not of the complete scale bar).

![http://gpsvp.googlecode.com/svn/wiki/img/Screen-XP-Scalebar-LongBeach.png](http://gpsvp.googlecode.com/svn/wiki/img/Screen-XP-Scalebar-LongBeach.png)

_The Long Beach Island is as long as "segment 5 km" + "segment 10 km", so the length of the island is about 15km._

**Q: How do I prevent my photo album from finding and listing all those map and satellite images.**

A: Give the directories and images a S+H (system+hidden) attribute. If you place a file called 'attrib' with said attributes in the root of the cache folder, any tile downloaded from that point will copy the attributes, so it is automatic.