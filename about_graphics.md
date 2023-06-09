It's a nightmare to find to sprite sheet for this game before I know this.<br/>
Game animations cost me a 2/3 time making the game.<br/>

# For pvz 1 sprite sheets:
download .fla files here: https://drive.google.com/file/d/1zztUOZWna5Hy2xI3tju1d7Hjtp59J4dI/view

# For pvz 2: 

## Method 1:
https://www.youtube.com/watch?v=RvYLWEBOPG0<br/>
use SPCUtil to decode .obb (which contains all graphics in game). Now you can get:<br/>
	.rton: use SPCUtil to decode to .json (contains game stats, the way to split sprite sheets)<br/>
	.rsgp: contains sprite sheets, animations in game<br/>

## Method 2: 
Thanks for [8A Gaming - Youtube](https://www.youtube.com/@8AGamingOfficial/featured) with his free web: https://8auniverse.com/ and his useful tools to extract game.<br/>
MANIFESTGROUP tool: download the .json to split a texture to images<br/>
Texture Downloader Tool: download the texture<br/>
Simple Sprite Splitter Tool: split texture to images<br/>
1BSR2PNGTool: .obb to textures.<br/>

# Make GIF
Now we get the .fla files<br/>
Open .fla by Adobe Animate<br/>

For .fla pvz 2: get the texture decomposited into many images, then copy them to library of the .fla. After that, copy all layers from Object A_main to the main. Then hide the animation layer. delete all costume if necessary. export to gif and enjoy.

# Make sprite sheets from GIFs
To create a sprite sheet, use the website https://onlinegiftools.com/convert-gif-to-sprite-sheet to convert from gif.