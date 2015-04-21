<div>

<h1>Introduction</h1>

I've written some tools to help organise your map collection.  I'm afraid I know as much about Visual C++ as I do about South American Folk Dancing, so these are written in my preferred (PC) language, Windows Scripting.<br>
<br>
<h1>Tools</h1>

<b>Map Layer</b>

This was written to solve a problem highlighted in the discussion groups.<br>
<br>
<a href='http://groups.google.com/group/gpsvp/browse_thread/thread/1d1649f28b63d12c'>http://groups.google.com/group/gpsvp/browse_thread/thread/1d1649f28b63d12c</a>

Instructions for use<br>
<br>
Download the latest copy of the .wsf file from <a href='http://code.google.com/p/gpsvp/downloads/list'>http://code.google.com/p/gpsvp/downloads/list</a>

Drag and drop one or more folders on to the program, or double click the program to select a folder manually.<br>
<br>
For each folder, the program will create a new folder with ' Layered' appended to the name.  Within that folder it will create subfolders named layer=2 through to layer=18.  It will then <b>copy</b> files from the original folder to the new folder and place them in the correct subfolder.<br>
<br>
<b>Blue Tile</b>

This was written to solve to reduce the size of my map folder for copying to my Windows Mobile device.  It works by deleting all the plain blue tile from a folder, and deletes any folder that contains only blue tiles. This can reduce the size of the resulting folder by up to 20% for maps that contain a lot of coastal areas.   At the moment it only works for Google and Live.com map folders.<br>
<br>
Instructions for use<br>
<br>
Download the latest copy of the .wsf file from <a href='http://code.google.com/p/gpsvp/downloads/list'>http://code.google.com/p/gpsvp/downloads/list</a>

Drag and drop one or more folders on to the program, or double click the program to select a folder manually.<br>
<br>
For each folder, the program search through all the subfolders and delete any plain blue tiles it finds.  If this results in an empty subfolder, it deletes the subfolder too.<br>
<br>
<b>Please note that this program deletes tiles directly from the folder(s) you choose.  You may want to create a new folder and copy the tiles you need to it before running this.</b>

An enhancement has been requested to export an area of map to a new folder (<a href='https://code.google.com/p/gpsvp/issues/detail?id=54'>Issue 54</a>).<br>
<br>
<a href='http://code.google.com/p/gpsvp/issues/detail?id=54'>http://code.google.com/p/gpsvp/issues/detail?id=54</a>