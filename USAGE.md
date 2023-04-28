# User Manual

## Table of Contents
- [1 Introduction](#1-introduction)
- [2 Menus](#2-menus)
  - [2.1 Warps](#21-warps)
    - [2.1.1 Locations](#211-locations)
    - [2.1.2 Bosses](#212-bosses)
  - [2.2 Cheats](#22-cheats)
  - [2.3 Player](#23-player)
    - [2.3.1 Stats](#231-stats)
    - [2.3.2 Gear](#232-gear)
    - [2.3.3 Partners](#233-partners)
    - [2.3.4 Badges](#234-badges)
    - [2.3.5 Items](#235-items)
    - [2.3.6 Star Spirits](#236-star-spirits)
    - [2.3.7 Misc](#237-misc)
  - [2.4 File](#24-file)
  - [2.5 Practice](#25-practice)
    - [2.5.1 Trainers](#251-trainers)
    - [2.5.2 Timer](#252-timer)
  - [2.6 Camera](#26-camera)
  - [2.7 Watches](#27-watches)
  - [2.8 Debug](#28-debug)
  - [2.9 Settings](#29-settings)
- [3 Known Issues](#3-known-issues)

## 1 Introduction
The main interface for interacting with fp is the utility menu, which can be accessed by pressing `R + D-Up`. This menu can be navigated with the D-pad, and L is used to make a selection. To quickly go back a menu, press `R + D-Left`. To close the menu, press `R + D-Up` again. These button combos can also be configured (see [2.9 Settings](#29-settings)).

If starting from a new file, it's advisable to play mostly normally until at least obtaining Goombario. Until that point, the game isn't quite in a state that expects you to be able to go anywhere but the starting areas, and you'll likely run into crashing issues.

## 2 Menus

### 2.1 Warps

#### 2.1.1 Locations
The **locations** menu provides a list of every room in the game and allows you to warp directly to them. There are three different values for this:

- **area**: Rooms grouped together by location, such as Toad Town, Dry Dry Desert, Flower Fields, etc.
- **map**: The specific map in the selected area, typically accessed by a loading zone.
- **entrance**: Where the map will be accessed from. This is mostly for selecting a loading zone, but certain entrances are also used to start cutscenes. The general rule of thumb is that earlier entrances are for loading zones and later entrances are for cutscenes.

After selecting an area, map, and entrance, selecting `warp` will immediately warp you to your selected destination. This works even if you are in battle.

#### 2.1.2 Bosses
The **bosses** menu provides a list of every major, minor, and optional boss in the game. By selecting a boss, you will warp to the room that the boss is located in, and your story progress and any necessary flags will be set to allow you to fight the boss, regardless of your current state.

Also note that when warping directly to Bowser's second phase, because the game normally carries Bowser's HP over from the first phase, a menu to select his starting HP is provided.

### 2.2 Cheats
This menu allows toggling the builtin cheats on and off. The following cheats are available:

- **encounters**:
  - **no encounters**: Prevents enemies from being able to interact with you.
  - **defeat on contact**: Instantly kills enemies upon touching them.
  - **auto-win**: Upon entering battle, instantly kills all on-screen enemies.
  - **auto-runaway**: Upon entering battle, instantly causes you to run away from battle successfully.
- **hp**: Freezes your HP at its max value.
- **fp**: Freezes your FP at its max value.
- **coins**: Freezes your coins at 999.
- **star power**: Freezes your star power at the current maximum value.
- **star pieces**: Freezes your star pieces at 160.
- **peril**: Freezes your HP at 1 (note that this overrides the **hp** cheat).
- **auto mash**: Instantly fills all bars that require you to mash A or Analog Left.
- **auto action command**: Mario's action commands will automatically be successful.
- **peekaboo**: Shows enemy HP without needing the peekaboo badge.
- **brighten room**: Dark rooms will be fully lit without needing Watt's ability.
- **quizmo spawns**: Forces Chuck Quizmo to always spawn at every location he can.

### 2.3 Player

#### 2.3.1 Stats
The **stats** menu allows you to change Mario's stats, such as your hp, fp, level, etc. Note that for action commands, having the Lucky Star in your inventory does nothing. Simply marking the Lucky Star icon is sufficient.

#### 2.3.2 Gear
The **gear** menu allows you to choose which boots and hammer Mario has.

#### 2.3.3 Partners
The **partners** menu allows you to control your active partner, which partners are in your party, and the rank of each partner. Each partner's rank is denoted by the number of blue circles next to their icon.

*Warning*: Although Goompa and Goombaria are in the game as available partners, their behavior is very buggy since they were never obtainable in the final game.

#### 2.3.4 Badges
The **badges** menu lets you select which badges are in your inventory. Pressing the `full list` button gives you a menu that allows you to add anything from the full item list, including unused badges.

#### 2.3.5 Items
The **items** menu lets you edit your **items**, **stored items**, and **key items** inventories with the respective menus. To remove an item from a specific slot, select the slot and press the delete button.

*Warning*: The game makes no distinction between normal items, key items, badges, or icons. If anything is placed in a menu it can't normally go, it can cause buggy behavior.

#### 2.3.6 Star Spirits
The **star spirits** menu allows you to select how many Star Spirits have been saved and in turn, the maximum size of your star power meter. The beam upgrades are selected by cycling through the last item in the menu.

*Warning*: The beam is not acquired in the base game until all 7 Star Spirits have been saved. Selecting either Star or Peach Beam with fewer than 7 Star Spirits can cause buggy behavior.

#### 2.3.7 Misc
The **misc** menu contains menus for less-used options:

- **princess peach**: allows you to control whether you're controlling Peach instead of Mario, if Peach is transformed into a disguise, whether she can use the Sneaky Parasol, and what enemy her disguise is of.
- **merlee**: allows you to set what Merlee's next spell will be, how many more times she can cast a spell, and how many more battle turns are left until she casts her next spell.

*Warning*: Swapping to Peach or Mario in locations they can't typically get to can cause buggy behavior.

### 2.4 File
The **slot** option lets you change which of the four save slots the game will save and load from. This includes the in-game save blocks and save menus. Selecting `save` will save your current game state to the selected slot as though you had used a save block. Selecting `load` will load the save file saved to the selected slot in a very similar manner to loading a file from file select.

Save files can be saved to and loaded from an SD card with the `export` and `import` options if playing on console or Wii Virtual Console. Pressing either one will bring up the file browser. Importing a file will immediately load a save, but none of the current in-game save slots will be overwritten unless done manually afterward. Exporting will save the file selected by *save slot*, not the current state of the game. The file extension used for save files on disk is **.pmsave**, and the default filename is **file**. The filename can be changed by pressing the name field. Pressing clear will set the name field to be empty. When the name field is empty, the default filename is untitled. When saving, pressing the name of a save file in the file browser will copy that name to the name field. Pressing accept will save the file to the current folder in the file browser with the specified file name. If the file exists, you will be prompted to overwrite it.

Your `story progress` is the main byte that determines how far you've progressed in the story. See [this](https://docs.google.com/document/d/1wrIhXo5cQjnUC_RcW8gETNvN2LJHpeuriV2NhhSUBwc/edit) for a list of what each value corresponds to.

Disabling `music` disables all in-game music. `quizmo` determines how many of Chuck Quizmo's questions have been answered successfully. This byte also determines which question Quimo will ask you next, and it's used in conjunction with your story progress to determine whether Quizmo can spawn. The three `toy box` options determine what Kammy Koopa will spawn in Shy Guy's Toybox. Pressing `restore enemies` sets all overworld enemies to an undefeated state.

### 2.5 Practice
Both the **trainers** and **timer** menu have a `save settings` button. This button has the same function as the one in settings (see [2.9 Settings](#29-settings)) and saves all fp settings to the current profile.

#### 2.5.1 Trainers
The **trainers** menu contains various different helpful menus and information screens to help with practicing and learning the game. The following trainers are available:

- **action commands**: Used to help learn action command timings. Displays log messages with information on the frame data for action commands, including attacks and blocks.
- **bowser blocks**: When enabled, causes Bowser to only attack using the specified move. Works with both Hallway and Final Bowser, though note that setting lightning will make Hallway Bowser wave since he doesn't have a lightning attack.
- **clippy**: When attempting to obtain the *clippy* state by opening the partner menu while encountering an enemy and riding Lakilester, displays a log telling when you're early or late on the timing.
- **ice staircase skip**: Used to help line up in the proper position to perform the Ice Staircase Skip trick. `position` can be either **good**, **inconsistent**, or **bad**, depending on whether using Lakilester's ability will always, sometimes, or never clip properly.
- **lzs jumps**: Used to practice *Loading Zone Storage* jumps. Displays logs with information on why LZS jumps are failed. Also keeps track of the current and most consecutive jumps in the current session which can be seen by pressing the wrench button.
- **oot ace**: JP-exclusive. Helps with performing arbitrary code execution aided by The Legend of Zelda: Ocarina of Time. `effects` shows the number of active particle effects. `flags` displays whether the animation flags located before the idle timer in Mario's player struct will cause a premature crash. `frame window` displays how big the frame window for stopping the idle timer and getting a successful jump to code stored on the expansion pak is. It will most likely always be 1 unless Ocarina of Time is used to zero out the expansion pak beforehand. Pressing `practice payload` will make it so that upon a successful jump to the expansion pak, the value of the idle timer will be displayed in the bottom-left of the screen. Pressing `oot instruction` will place the same ASM instruction on the expansion pak that doing the proper OoT setup would.

#### 2.5.2 Timer
The **timer** menu provides a real-time timer that is unaffected by lag. Separately, it also displays 30 fps lag frames by taking half of the game's vertical interrupt counter and subtracting the number of game frames that have passed. Note that lag frames can decrease due to the game speeding up to account for lag. Additionally, the timer is only guaranteed to be accurate on console.

The timer has two modes, **automatic** and **manual**.

- **automatic**: In this mode, pressing `start/stop` will prime the timer to start, but it won't start right away. Instead, once primed, the timer will begin the next time a cutscene ends. A cutscene, in this context, is anything that takes the player's control away from Mario, such as entering a loading zone or opening a menu. The timer will then continue to run until the number of cutscenes that have occurred equals the configurable `cutscene count`. In this way, any arbitrary sequence of events can be timed without worrying about human error affecting the starting/stopping of the timer.
- **manual**: This mode is much simpler. Pressing `start/stop` will start the timer immediately, and it will continue to run unless `start/stop` or `reset` is pressed.

The timer will continue to function even if the utility menu is closed, but by default it will not be displayed when running due to lag concerns. Additionally, no log messages related to the timer will be displayed when the timer is running. These can be changed by toggling the `show timer` and `timer logging` options.

Note that button combinations can be configured for the `start/stop` and `reset` functions, and the position of the timer can be configured in settings (see [2.9 Settings](#29-settings)).

### 2.6 Camera
The free camera function provides full control of the game's camera. When enabled, the camera can be controlled with the joystick, C buttons, and Z trigger. These controls are disabled in-game when controlling the free camera. Press **lock** to disable the manual camera controls and restore the normal game controls. Note that changing the camera from its default will not affect how Mario controls. As far as the rest of the game knows, the camera is still where it normally would be. 

The **behavior** setting decides how the camera moves and how the controls work:

- **manual:** The camera does not move by itself. Use the joystick to look
    around, and the C buttons to move. Hold Z to move with the joystick, look
    with C-left and right, and move vertically with C-up and down. The
    **distance min** and **distance max** settings do nothing.
- **birdseye follow:** The camera automatically looks at Mario and moves
    forward and backward to stay within the specified *distance*. Controls
    are the same as for *manual*.
- **radial follow:** The camera follows Mario from a fixed viewing angle. It
    will move up, down, and sideways to keep Mario in focus, and forward and
    backward to stay within the specified *distance*. Use the joystick,
    C-left, and C-right to rotate the viewing angle, and C-up and down to move
    towards and away from the focus point. Hold Z to swap the function of C-up
    and down with the vertical joystick axis.

### 2.7 Watches
This menu lets you add custom RAM watches to observe arbitrary parts of game's
memory in real-time. Pressing the plus icon will add a new watch, and pressing
the cross next to a watch will remove that watch. After adding a watch, enter a
memory address and value type to display the value at that address. These watch
types are available:

-   **u8:** one-byte value, unsigned.
-   **s8:** one-byte value, signed.
-   **x8:** one-byte value, hexadecimal.
-   **u16:** two-byte value, unsigned.
-   **s16:** two-byte value, signed.
-   **x16:** two-byte value, hexadecimal.
-   **u32:** four-byte value, unsigned.
-   **s32:** four-byte value, signed.
-   **x32:** four-byte value, hexadecimal.
-   **f32:** four-byte value, IEEE 754 single-precision floating-point.

Pressing the wrench button next to a watch will show that address in the memory editor (see [2.8 Debug](#28-debug)). Pressing the anchor button will release the watch from the
watches menu so that it's always visible, even when the menu is closed. When a
watch is released, a positioning button will appear which lets you change the
position of the watch on the screen. Holding Z when positioning the watch will
move it faster. Pressing the wrench icon next to a watch will open the memory
editor at the address of that watch. The **visible** option can be unchecked to
hide all watches globally.

Watches can be imported from text files on an SD card by pressing the folder
icon. Press a watch file to bring up a list of all watches contained in that
file. Press a watch to import it to your watch list. When you've imported all
the watches you need, press **return** to go back to the watches menu.

### 2.8 Debug
_Note: These features are for advanced users. Be careful._

This menu contains various debug features to use for testing;

-   **flags:** Display and edit saved game flags. The flags are grouped by the
    records they are kept in. Use the arrows to cycle between flag records.
    Press a flag to toggle its state. A red flag is "off", and a green flag is
    "on". The **log** menu displays a list of recent flag events. When a flag
    changes, its record, id, and new value is inserted at the top off the list.
    The **undo** option reverts the effect of the most recent flag event and
    removes it from the log. The **clear** option removes all flag events from
    the log, but does not affect the state of the given flags. _Note:_ The flag
    log only records changes when the log menu is open. If a flag changes and
    then changes back while the log is closed, these changes will not be
    recorded.
-   **memory:** Memory editor. Use the horizontal arrows to cycle between
    memory domains, and the vertical arrows to scroll up and down between
    addresses. Holding Z while scrolling will scroll faster. You can also enter
    an address manually in the address field. To edit memory, select the
    desired data type and press a memory cell to modify it.

### 2.9 Settings
This is where most of the functionality of fp is configured.
- **profile**: Selects which profile to save and load settings to and from. When the game starts, the settings saved to profile zero are automatically loaded, if any.
- **font**: Selects the font of the menu.
- **drop shadow**: Enables/disables the menu drop shadow. Disabling drop shadow can reduce the graphical computation impact of the menu, but may also reduce readability.
- **menu position**: Moves the on-screen position of the entire menu.
- **timer position**: Moves the on-screen position of the timer display.
- **input display**: Enables/disables and moves the position of the input display. The analog mode can be set with **control stick** to `numerical`, `graphical`, and `both`. To account for different control stick ranges, **graphical range** can be adjusted to scale the maximum analog value for the *graphical* and *both* displays up to a maximum value of 127.
- **log**: Enables/disables and moves the position of all log messages.
- **commands**: See below.
- **save settings**: Saves the current settings to the currently selected profile.
- **load settings**: Loads the settings from the currently selected profile.
- **restore defaults** Restores all saved settings to their default values (Does not affect saved profiles). If the saved settings were to become corrupted in such a way that they prevent the game from starting, holding the Start button when the game is starting will load the default settings instead of loading profile zero.

The following settings are saved:

- Menu and on-screen displays appearances and settings.
- Watches.
- Command button binds.
- Activated cheats.

The **commands** menu lets you bind commands to custom button combinations and/or activate them manually. Pressing the name of a command will activate that command, and pressing the button combo in the right column will bind a button combo to the corresponding command. If you want to unbind a command, press and keep holding L when starting the binding. A button combo for any given command can contain at most four buttons. When activating a command with a button combo, the button combo must explicitly be input the way it appears in the commands menu. For example, a command with the button combo `R + A` will only be activated if you press R first and then A, or R and A at the same time. `A + R`, or `R + B + A` will not activate the corresponding command. If the set of buttons in one button combo is a subset of those in another button combo, the former will be overridden by the latter when both are active simultaneously.

The following commands are available:

- **show/hide menu:** Opens the utility menu if it's closed, closes it if
    it's opened. *Default: `R + D-Up`*
- **return from menu:** Returns to the previous menu, as if the *return*
    button was pressed. *Default: `R + D-Left`*
- **levitate**: Makes Mario fly into the air. *Default: `D-Up`*
- **turbo**: Increases Mario's running speed. *Default: `D-Down`*
- **save position**: Saves Mario's current position and orientation. *Default: `D-Left`*
- **load position**: Loads Mario's saved position and orientation. *Default: `D-Right`*
- **lzs**: Allows Mario to walk into loading zones and store them without taking them. If a menu is then opened and closed after moving somewhere else, Mario will then take the loading zone as though Loading Zone Storage jumps had been performed. *Default: `R + D-Left`*
- **reload map**: Reloads the current map with the last known entrance value. Also works when in battle. *Default: `R + D-Down`*
- **reload last warp**: Reloads the last room warped to through the **locations** or **favorite rooms** menus. *Default: `unbound`*
- **toggle watches**: Toggles whether to display the watches you have set. *Default: `R + D-Right`*
- **reimport save**: Re-imports the last save file imported from the SD card. *Default: `R + Z`*
- **save game**: Saves the game to the current save slot as though a save block had been used. *Default: `L + D-Left`*
- **load game**: Loads the save file from the selected save slot, similar to loading a file from the file select screen. *Default: `L + D-Right`*
- **start/stop timer**: Sets the timer to start after the next cutscene or starts/stops the timer, depending on the timer mode. *Default: `unbound`*
- **reset timer**: Sets the timer back to 0 and reverts it to an inactive state. *Default: `unbound`*
- **show/hide timer**: Toggles whether the timer is showing when it's active. *Default: `unbound`*
- **break free**: Attempts to break any effect that removes control of Mario. *Default: `L + D-Down`*


**_Warning:_** Unbinding the *show/hide menu* or *return from menu* commands,
or binding them to a button combination that will interfere with menu
navigation can make it impossible to use the utility menu. If this happens,
you can restore the default settings by entering the following button sequence:
`D-Up D-Up D-Down D-Down D-Left D-Right D-Left D-Right B A`.

_Note:_ Button combos that interfere with menu navigation for commands that
aren't related to menuing are disabled while the utility menu is active.

## 3 Known Issues
There are a few known issues with fp without an easy fix:

- At least one save file (even an empty one) must exist for profile saving/loading to work.
- When using the automatic timer, riding Laki or Sushie counts as one long cutscene. This means things that would normally increment the cutscene counter, such as going through a loading zone, no longer will.
- In rare circumstances when using the free camera, the game may crash.
