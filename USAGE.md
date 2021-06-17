# User Manual

## Table of Contents
- [1 Introduction](#1-introduction)
- [2 Menus](#2-menus)
  - [2.1 Warps](#21-warps)
    - [2.1.1 Locations](#211-locations)
    - [2.1.2 Bosses](#212-Bosses)
    - [2.1.3 Favorite Rooms](#213-favorite-rooms)
  - [2.2 Cheats](#22-cheats)
  - [2.3 Player](#23-player)
    - [2.3.1 Status](#231-status)
    - [2.3.2 Partners](#232-partners)
    - [2.3.3 Items and Badges](#233-items-and-badges)
    - [2.3.4 Star Power](#234-star-power)
    - [2.3.5 Princess Peach](#235-princess-peach)
    - [2.3.6 Merlee](#236-merlee)
  - [2.4 File](#24-file)
  - [2.5 Practice](#25-practice)
    - [2.5.1 Tricks](#251-tricks)
    - [2.5.2 Trainers](#252-trainers)
    - [2.5.3 Timer](#253-timer)
  - [2.6 Debug](#26-debug)
  - [2.7 Settings](#27-settings)
- [3 Known Issues](#3-known-issues)

## 1 Introduction
The main interface for interacting with fp is the utility menu, which can be accessed by pressing `R + D-Up`. This menu can be navigated with the D-pad, and L is used to make a selection. To quickly go back a menu, press `R + D-Left`. To close the menu, press `R + D-Up` again. These button combos can also be configured (see [2.7 Settings](#27-settings)).

## 2 Menus

### 2.1 Warps

#### 2.1.1 Locations
The **locations** menu provides a list of every room in the game and allows you to warp directly to them. There are three different values for this:

- **group**: Rooms grouped together by location, such as Toad Town, Dry Dry Desert, Flower Fields, etc.
- **room**: The specific map in the selected group, typically accessed by a loading zone
- **entrance**: Where the room will be accessed from. This is mostly for selecting a loading zone, but certain entrances are also used to start cutscenes. The general rule of thumb is that earlier entrances are for loading zones and later entrances are for cutscenes.

After selecting a group, room, and entrance, selecting `warp` will immediately warp you to your selected destination. This works even if you are in battle. Do note though that there are certain states where attempting to warp would cause a crash, such as when a menu is open, so generally fp will disallow you from doing this with a warning message.

#### 2.1.2 Bosses
The **bosses** menu provides a list of every major, minor, and optional boss in the game. By selecting a boss, you will warp to the room that the boss is located in, and your story progress and any necessary flags will be set to allow you to fight the boss, regardless of your current state.

Also note that when warping directly to Bowser's second phase, because the game normally carries Bowser's HP over from the first phase, a menu to select his starting HP is provided.

#### 2.1.3 Favorite Rooms
Since there are hundreds of rooms, and you'll likely want to go to many of the same rooms more often than others, the **favorite rooms** menu allows you to save up to 10 rooms for easy access. These can be saved along with the rest of your settings to persist across multiple play sessions (see [2.7 Settings](#27-settings)).

Each row is a different room, defined by a group, room, and entrance. These can either be set manually, or by selecting `current` to populate the selected row with your current location. Selecting `warp` will warp you to the selected row. Selecting `save settings` will save all your settings to the current profile, including the displayed favorite locations.

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
- **star pieces**: Freezes your star pieces at 160.
- **peril**: Freezes your HP at 1 (note that this overrides the **hp** cheat).
- **break free**: Allows you to move during cutscenes.
- **auto mash**: Instantly fills all bars that require you to mash A.
- **quizmo spawns**: Forces Chuck Quizmo to always spawn at every location he can.

### 2.3 Player

#### 2.3.1 Status
The **status** menu allows you to change things about Mario's status, such as your boots and hammer, hp and fp, etc. Note that for action commands, having the Lucky Star in your inventory does nothing. Simply marking the checkbox is sufficient.

#### 2.3.2 Partners
The **partners** menu allows you to control your active partner, which partners are in your party, and the rank of each partner.

*Warning*: Although Goompa and Goombaria are in the game as available partners, their behavior is very buggy since they were never obtainable in the final game.

#### 2.3.3 Items and Badges
The **items**, **stored items**, **key items**, and **badges** menus let you edit your item and badge inventories. Items can be selected either by ID or by name. Arrows can be used to rearrange items.

*Warning*: The game makes no distinction between normal items, key items, badges, or icons. If anything is placed in a menu it can't normally go, it can cause buggy behavior.

#### 2.3.4 Star Power
The **star power** menu allows you to edit how many Star Spirits have been saved, how full the star power meter is, and what beam upgrade you have is. `star spirits saved` determines both which star spirits are available in battle and your max star power. `partial bar` increases in increments of 0x20 and has 8 different levels.

*Warning*: 7 is the max `star spirits saved` that can be obtained in game. Setting this to anything higher can cause buggy behavior.

*Warning*: `beam` can never be anything other than **none** in the base game until all 7 Star Spirits have been saved. Setting it to anything else with fewer than 7 Star Spirits can cause buggy behavior.

#### 2.3.5 Princess Peach
The **princess peach** menu allows you to control whether you're controlling Peach instead of Mario, if Peach is transformed into a disguise, whether she can use the Sneaky Parasol, and what enemy her disguise is of.

*Warning*: Swapping to Peach or Mario in instances where they're not typically able to be can cause buggy behavior.

#### 2.3.6 Merlee
The **merlee** menu allows you to set what Merlee's next spell will be, how many more times she can cast a spell, and how many more battle turns are left until she casts her next spell.

### 2.4 File
The **save slot** option lets you change which of the four save slots the game will save and load from. This includes the in-game save blocks and save menus. Selecting `save` will save your current game state to the selected slot as though you had used a save block. Selecting `load` will load the save file saved to the selected slot in a very similar manner to loading a file from file select.

Your `story progress` is the main byte that determines how far you've progressed in the story. See [this](https://pastebin.com/tYtE2xbm) for a list of what each value corresponds to.

Disabling `music` disables all in-game music. `quizzes answered` determines how many of Chuck Quizmo's questions have been answered successfully. This byte determines which question Quimo will ask you next, and it's used in conjunction with your story progress to determine whether Quizmo can spawn. Pressing `restore enemies` sets all overworld enemies to an undefeated state. Pressing `restore letters` resets all flags related to collecting letters in the overworld.

### 2.5 Practice

#### 2.5.1 Tricks
The **tricks** menu allows easy access for practicing every speedrun trick in the game. By selecting a trick, all necessary setup required will be executed, followed by a warp to the location of the trick. For Record Skip, for instance, Story Progress is set to before the Weight is obtained, your active partner is set to Bombette for NPC pushing the Boo, any instances of the Weight are removed from your Key Item inventory, and you're warped to the room in Boo's Mansion with the Weight chest.

To reload the most recently loaded trick, press `R + Z`. This button combo can also be configured (see [2.7 Settings](#27-settings)).

#### 2.5.2 Trainers
The **trainers** menu contains various different helpful menus and information screens to help with practicing and learning the game. The following trainers are available:

- **bowser blocks**: When enabled, causes Bowser to only attack using the specified move. Works with both Hallway and Final Bowser, though note that setting lightning will make Hallway Bowser wave since he doesn't have a lightning attack.
- **ice staircase skip**: Used to help line up in the proper position to perform the Ice Staircase Skip trick. `position` can be either **good**, **inconsistent**, or **bad**, depending on whether using Lakilester's ability will always, sometimes, or never clip properly.
- **oot ace**: Helps with performing arbitrary code execution aided by The Legend of Zelda: Ocarina of Time. `effects` shows the number of active particle effects. `flags` displays whether the animation flags located before the idle timer in Mario's player struct will cause a premature crash. `frame window` displays how big the frame window for stopping the idle timer and getting a successful jump to code stored on the expansion pak is. It will most likely always be 1 unless Ocarina of Time is used to zero out the expansion pak beforehand. Pressing `practice payload` will make it so that upon a successful jump to the expansion pak, the value of the idle timer will be displayed in the bottom-left of the screen. Pressing `oot instruction` will place the same ASM instruction on the expansion pak that doing the proper OoT setup would.

#### 2.5.3 Timer
The **timer** menu provides a real-time timer that is unaffected by lag. Separately, it also displays 30 fps lag frames by taking half of the game's vertical interrupt counter and subtracting the number of game frames that have passed. Note that lag frames can decrease due to the game speeding up to account for lag.

The timer has two modes, **automatic** and **manual**.

- **automatic**: In this mode, pressing `start/stop` will prime the timer to start, but it won't start right away. Instead, once primed, the timer will begin the next time a cutscene ends. A cutscene, in this context, is anything that takes the player's control away from Mario, such as entering a loading zone or opening a menu. The timer will then continue to run until the number of cutscenes that have occurred equals the configurable `cutscene count`. In this way, any arbitrary sequence of events can be timed without worrying about human error affecting the starting/stopping of the timer.
- **manual**: This mode is much simpler. Pressing `start/stop` will start the timer immediately, and it will continue to run unless `start/stop` or `reset` is pressed.

The timer will continue to function even if the utility menu is closed, but by default it will not be displayed when running due to lag concerns. Additionally, no log messages related to the timer will be displayed when the timer is running. These can be changed by toggling the `show timer` and `timer logging` options.

Note that button combinations can be configured for the `start/stop` and `reset` functions, and the position of the timer can be configured in settings (see [2.7 Settings](#27-settings)).

### 2.6 Debug
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

### 2.7 Settings
This is where most of the functionality of fp is configured. The **profile** option selects which profile to save and load settings to and from. When the game starts, the settings saved to profile zero are automatically loaded, if any. The appearance of the menu can be configured with the **font** and **drop shadow** options. Disabling drop shadow can reduce the graphical computation impact of the menu, but may also reduce readability. The visibility of the on-screen display elements can be configured with the **input display** and **log** options. The screen position of the utility menu, input display, log, and timer can be configured by their respective positioning buttons. Holding Z when positioning an element will move it faster. **save settings** and **load settings** will save and load the current settings to the currently selected profile. **restore defaults** will restore all saved settings to their default values (Does not affect saved profiles). If the saved settings were to become corrupted in such a way that they prevent the game from starting, holding the Start button when the game is starting will load the default settings instead of loading profile zero. The following settings are saved:

-   Menu and on-screen displays appearances and settings.
-   Saved positions and the currently selected position slot number.
-   Command button binds.
-   Activated cheats.
-   Favorite rooms.

The **commands** menu lets you bind commands to custom button combinations and/or activate them manually. Pressing the name of a command will activate that command, and pressing the button combo in the right column will bind a button combo to the corresponding command. If you want to unbind a command, press and keep holding L when starting the binding. A button combo for any given command can contain at most four buttons. When activating a command with a button combo, the button combo must explicitly be input the way it appears in the commands menu. For example, a command with the button combo `R + A` will only be activated if you press R first and then A, or R and A at the same time. `A + R`, or `R + B + A` will not activate the corresponding command. If the set of buttons in one button combo is a subset of those in another button combo, the former will be overridden by the latter when both are active simultaneously.

The following commands are available:

-   **show/hide menu:** Opens the utility menu if it's closed, closes it if
    it's opened. *Default: `R + L`*
-   **return from menu:** Returns to the previous menu, as if the *return*
    button was pressed. *Default: `R + D-Left`*
-   **levitate**: Makes Mario fly into the air. *Default: `D-Up`*
-   **turbo**: Increases Mario's running speed. *Default: `D-Down`*
-   **save position**: Saves Mario's current position and orientation. *Default: `D-Left`*
-   **load position**: Loads Mario's saved position and orientation. *Default: `D-Right`*
-   **lzs**: Allows Mario to walk into loading zones and store them without taking them. If a menu is then opened and closed after moving somewhere else, Mario will then take the loading zone as though Loading Zone Storage jumps had been performed. *Default: `R + D-Left`*
-   **reload room**: Reloads the current room with the last known entrance value. Also works when in battle. *Default: `R + D-Down`*
-   **reload last warp**: Reloads the last room warped to through the **locations** or **favorite rooms** menus. *Default: `unbound`*
-   **show coordinates**: Displays your `x`, `y`, and `z` coordinates on screen.
-   **load trick**: Reloads the last trick selected from the **tricks** menu. *Default: `R + Z`*
-   **save game**: Saves the game to the current save slot as though a save block had been used. *Default: `L + D-Left`*
-   **load game**: Loads the save file from the selected save slot, similar to loading a file from the file select screen. *Default: `L + D-Right`*
-   **start/stop timer**: Sets the timer to start after the next cutscene or starts/stops the timer, depending on the timer mode. *Default: `unbound`*
-   **reset timer**: Sets the timer back to 0 and reverts it to an inactive state. *Default: `unbound`*
-   **show/hide timer**: Toggles whether the timer is showing when it's active. *Default: `unbound`*


**_Warning:_** Unbinding the *show/hide menu* or *return from menu* commands,
or binding them to a button combination that will interfere with menu
navigation can make it impossible to use the utility menu. If this happens,
you can restore the default settings by entering the following button sequence:
`D-Up D-Up D-Down D-Down D-Left D-Right D-Left D-Right B A`.

_Note:_ Button combos that interfere with menu navigation for commands that
aren't related to menuing are disabled while the utility menu is active.

## 3 Known Issues
There are a few known issues with fp without an easy fix:

-   Attempting to warp when a Super Block menu is open will cause a crash. All known ways to prevent this also prevent warping in other valid states.
-   At least one save file (even an empty one) must exist for profile saving/loading to work.
-   When using the automatic timer, riding Laki or Sushie counts as one long cutscene. This means things that would normally increment the cutscene counter, such as going through a loading zone, no longer will.