<div>

<h2>Define your own color preferences</h2>

<p>You might have specific needs or preferences for changing the default colors and layout of one or more objects shown in gpsVP. This can be done by just using the  "Menu->Display->Open color scheme" menu item. A color settings file has a ".vpc" extension and is a plain text file. Records in it describe the visual representation of map objects.</p>

<p>The look and feel of the following gpsVP rendered map is e.g. quite similar as on a real Garmin device.</p>

<a href='http://gpsvp.googlecode.com/svn/wiki/img/ColSamp2.JPG'>http://gpsvp.googlecode.com/svn/wiki/img/ColSamp2.JPG</a>

You can get such a modified screen layout by creating a "MyColor.vpc" file with the following content:<br>
<br>
# Color Scheme File for gpsVP <br>
#  <br>
# Look and Feel more like a real Garmin device  <br>
#  <br>
# Major highway -> OSM = motorway  <br>
40 01 00 00 00 0 9 30 00 FF 0 7  <br>
#  <br>
# Principal highway -> OSM = primay  <br>
40 02 00 00 00 0 7 CC 00 33 0 5  <br>
#  <br>
# Other highway road -> OSM = primary_link  <br>
40 03 00 00 00 0 6 FF 33 33 0 4  <br>
#  <br>
# Major Road -> OSM = secondary  <br>
40 04 00 00 00 0 9 FF 66 00 0 7  <br>
#  <br>
# Regional Road -> OSM = tertiary  <br>
40 05 00 00 00 0 7 FF 99 33 0 5  <br>
#  <br>
# Local Road > OSM = residential  <br>
40 06 00 00 00 0 5 FF FF 99 0 3  <br>
#  <br>
# Service Road -> OSM = service  <br>
40 07 00 00 00 0 4 FF FF FF 0 2  <br>
#  <br>
# Path -> OSM = track (grade1-5)  <br>
40 0A BF 7F 7F 0 2  <br>
#  <br>
# Footway -> OSM = footway  <br>
40 0B BF 7F 7F 1 2  <br>
# <br>
# Cursor color  <br>
02 FF 33 00	 <br>
# Navigation Line  <br>
40 200 FF 33 00 0 3 00 00 00 2 1  <br>
# Old (loaded) track  <br>
40 FC FF FF 00 0 5 00 00 00 2 1  <br>
<br>
<p>All lines which start with # are just comments. So in the sample the look and feel for major highways is defined like:</p>
# Major highway<br>
40 01 00 00 00 0 9 CC 00 33 0 7<br>

<p>In this sample major highways are highlighted as a thick black polylines and a thinner red inner line. You find a detailed explanation about how this format is defined in the next paragraphs. For the particular sample "40 01" says that the entry is for overwriting the color for all loaded polylines with the "Major Highway" attribute. Then "00 00 00" says RBG color black, "0 9" a normal thick(9) line having a red "CC 00 33" (RBG) inner polyline with width 7.<p>

<h2>Useful tools and samples</h2>

<p>For getting the three double digit hexadecimal RGB color values you might use color picking tools like: <a href='http://bumblemonkey.com/picker/picker.html'>http://bumblemonkey.com/picker/picker.html</a> <br><br>
Here is the default color scheme file: <a href='http://code.google.com/p/gpsvp/source/browse/trunk/src/Resources/Normal.vpc'>http://code.google.com/p/gpsvp/source/browse/trunk/src/Resources/Normal.vpc</a> <br>
You might use this file for a reference and just take the items to your individual files you really want to modify.<br><br>
A good reversal of the normal settings are in <a href='http://code.google.com/p/gpsvp/source/browse/trunk/src/Resources/ZoltanNight_v2.vpc'>http://code.google.com/p/gpsvp/source/browse/trunk/src/Resources/ZoltanNight_v2.vpc</a> <br>
This one gives you basically a dark background and bright color foreground objects, like you might want to prefer it in a dark environment.<br>

<h2>Detail explanation</h2>

<p>There are only three types of records in color scheme files - for polygons, polylines and POIs.</p>
<h4>Polygons</h4>
<p>Polygon description is simple:<br />
<pre><code>80 [type] [color]</code></pre><br />
Type is hexadecimal number - code of type in Garmin format. Color is three hexadecimal numbers in 00-ff range meaning red, green and blue color intensity. Sample:<br />
<pre><code>80 50 BF FF BF</code></pre><br />

represent light green (BF FF BF) woods (50) polygon (80).</p>
<h4>Polylines</h4>
<p>Polyline description is a little bit more complex:<br />
<pre><code>40 [type] [color] [dotted flag] [width]</code></pre><br />
or<br />
<pre><code>40 [type] [color 1] [dotted flag 1] [width 1] [color 2] [dotted flag 2] [width 2]</code></pre><br />
Type and color have the same meaning as in polyline case. Width means line width and dotted flag tells whether line is dotted (1) or solid (0). Dotted flag works only when width is 1. When description contains two groups of parameters the second is painted over the first.<br />
Simple sample:<br />
<pre><code>40 02 EF 00 00 0 3</code></pre><br />

represent solid (0) red (EF 00 00) polyline (40) for principal highway (02) of width 3 (3).<br />
Another sample:<br />
<pre><code>40 14 00 00 00 0 3 FF FF FF 1 1</code></pre><br />
represents polyline (40) for railroad (14). It is wide (3) solid (0) black (00 00 00) line with narrow (1) dotted (1) white (FF FF FF) line inside it.</p>
<h4>POI</h4>
<p>POIs are described even simpler:<br />
<pre><code>10 [code] [icon]</code></pre><br />
Icon now is one of icons compiled into gpsVP. Here is the list of currently present icons:<br />
Airport, Amusement, Bank, Boats, Bridge, Car, Casino, CITY1, CITY2, CITY3, CITY4, CITY5, CITY6, Church, Cinema, Civil, Concert, Computer, DOT, Dining, Empty, Exit, Facilities, Flag, Fuel, Hospital, Information, Lodging, Marina, Museum, Park, Parking, Pharmacy, Photo, Picnic, Police. Post. Repair. Restroom, School, Service, Shop, Sports, Swimming, Telephone, Toll, Tower, Transportation. Unknown, Water, Weight.<br />

Sample:<br />
<pre><code>10 6408 "Hospital"</code></pre></p>
<h4>Custom POI icons</h4>
<p>You can also use your own dll with icons (see old icons sample below). To switch from internal icons you need to add a line with only resource dll name to file. All POIs described under this line will have icons from the dll.<br />
Sample:<br />
<pre><code>"oldicons.dll"</code></pre><br />
gpsVP uses 32x32 icons with any color depth. To edit icons I use <a href='http://www.gimp.org/'>GIMP</a>. To edit resources in a dll you can use any free resource editor, e.g. <a href='http://www.wilsonc.demon.co.uk/d10resourceeditor.htm'>XN Resource Editor</a>. If the editor cannot assign names to icon resources (like XN) you can write quoted numbers instead of string names:<br />

<pre><code>10 6408 "6408"</code></pre></p>
<h4>Undesribed types</h4>
<p>If a type is not described, polygon is painted in red, polyline is solid red line of width one and POI is red question mark. You can see the type of a polyline or a POI from context menu.</p>
<h2>Downloads</h2>
<p>You can download some sample files for this post:</p>
<ul>
<li><a href='/download/Normal.vpc'>Default color scheme</a></li>
<li><a href='/download/Night.vpc'>My attempt to create night color scheme</a></li>
<li><a href='/download/Zoltan_v1.vpc'>"More pastel color scheme" from Zoltan</a></li>
<li><a href='/download/ZoltanNight_v2.vpc'>Night scheme from Zoltan</a></li>
<li><a href='/download/oldicons.zip'>Old 16-color icon set</a></li>
</ul>
<p>Feel free to send me your color schemes so that I could publish them.</p>