Columns CE -- A Columns clone for the TI-84 CE
==================================================================
Warning:
* **This only works on the TI-84+ CE.**
* This will **not** work on the TI-84+ CSE.
* This will **not** work on any TI-84+ (SE)

Motivation
----------
Because I enjoyed the game on the Sega Genesis when I was much younger
and I wanted something like it on the CE.

Building the Game From Source
-----------------------------
* If you don't have it, install the latest version of the CE C SDK
  from here: https://github.com/CE-Programming/toolchain/releases
* Run `make gfx` from the project's root directory to build the graphical assets.
* Run `make` from the project's root directory to build the project.
* If all is successful, a `bin` folder should have appeared with at least one
  file named `COLUMNCE.8xp` in it. This is the file you want to send to your
  calculator.
  
Running the Game
----------------
* Send `COLUMNCE.8xp` to your TI-84 CE graphing calculator by using your
  favorite computer-to-calculator link software (e.g. TiLP, TI-Connect CE).
* If you do not have the 'Standard' CE libraries (`GRAPHX`, `FILEIOC`, etc.),
  you should download them from
  https://tiny.cc/clibs and also send them to
  your calculator.
* Run the program...
  * ...by using your favorite shell software (e.g. DoorsCE9, Cesium, etc.)
    * Follow the instructions provided by that software on how to run Columns CE.
  * ...by invoking it on the homescreen as an ASM program. To do this:
	* If you aren't on the homescreen or aren't sure, push <kbd>2nd</kbd>, then
	  <kbd>mode</kbd> to exit back out to the homescreen.
    * Push <kbd>clear</kbd> to clear the homescreen
	* Bring up the catalog by pushing <kbd>2nd</kbd>, then <kbd>0</kbd>
	* Arrow down until you select `Asm(`, then push <kbd>enter</kbd>
	* Push <kbd>prgm</kbd> and (if needed) scroll down until you select `COLUMNCE`,
	  then push <kbd>enter</kbd>
	* You should see on the homescreen `Asm(prgmCOLUMNCE`. Push <kbd>enter</kbd> to run.
	
Troubleshooting
---------------
* The project doesn't build
  * Did you (properly) install the latest version of the CE C SDK?
  * Did you check to see if your command prompt / cmd.exe is open on the project's
    root directory? (Columns-CE-master)
* The project appears to build but I can't find `COLUMNCE.8XP` in the `bin` folder.
  * Try running `make clean` in the command prompt at the project's root directory.
* I can't send `COLUMNCE.8XP` to my calculator.
  * Try downloading the project again or rebuilding the game.
  * Try searching online for troubleshooting steps for the computer-to-calculator
    link software you are using.
* I can't run the game or it's giving errors.
  * Did you follow the steps outlined in how to run the game exactly?
  * Did you download and install all the libraries that the onscreen prompt
    asked you to download? (`GRAPHX`, `FILEIOC`, etc.)
* The sound doesn't work!
  * That's because it's not implemented, and might never be. Audio on the CE,
    at this time, is a bit of a stretch. The menu options that hint that there
	might be music are placeholders if this game gets ported to a platform
	that has an easy way to play sounds.
* The game is bugged or is causing unwanted behavior.
  * Try again and see if what is happening can be repeated.
  * Post your concerns here: https://www.cemetech.net/forum/viewtopic.php?t=14614
  * Or post your concerns in GitHub's issue tracker here: https://github.com/Iambian/Columns-CE/issues

  
Controls
--------
In the menu:

| Keys     |  Function         |
|---------:|:------------------|
|<kbd>mode</kbd>    | Go back/Quit      |
|<kbd>2nd</kbd>     | Select option     |
|Arrow keys| Change menu option|

During gameplay:

| Keys       |  Function                |
|-----------:|:-------------------------|
|<kbd>mode</kbd>      | Forfeit/Quit             |
|<kbd>2nd</kbd>       | Shift the jewels         |
|<kbd><</kbd>/<kbd>></kbd>| Move jewels left or right|
|<kbd>v</kbd>      | Quickly drop the jewels  |

License and Copyright
---------------------
* The graphics are copyrighted to SEGA. Used without permission.
* The program is covered under the terms of the MIT license, found in `LICENSE`

Credits
-------
* Tim (geekboy1011) Keller - Provider of delicious cherries and maintainer of sanity
* jcgter777 - readme formatting assistance
* Cemetech forum and IRC channel (irc.efnet.org/#cemetech) - A place to be and a wonderful community
