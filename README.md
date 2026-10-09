# SKRASH

**skrash** is a Scratch 3.0 (`.sb3`) project runtime and executor.

---

## Requirements

- **Libraries**: `libzip`

---

## Build & Run

### 1. Compile and run C tests

Compile the standalone SB3 loader test:
```bash
cc -std=c11 -Wall -Wextra -I. \
  tests/test_sb3_loading.c \
  skrash/vm/_native/project/sb3.c \
  skrash/vm/_native/project/asset.c \
  -lzip \
  -o /tmp/test_sb3_loading
```
Compile the standalone SB3 loader test:
```bash
cc -std=c11 -Wall -Wextra -I. \
  tests/test_opcodes_detection.c \
  skrash/vm/_native/project/sb3.c \
  skrash/vm/_native/project/asset.c \
  -lzip \
  -o /tmp/test_opcodes_detection
```

Run the executable against a test project:
```bash
/tmp/test_sb3_loading
/tmp/test_opcodes_detection
```
---

## Work in progress

### Detect ~115 standard blocks

#### Motions
- [x] move `n` steps (`motion_movesteps`)
- [x] turn `n` degrees (right) (`motion_turnright`)
- [x] turn `n` degrees (left) (`motion_turnleft`)
- [ ] go to `(random position, mouse position, a sprite position)` (`motion_goto`)
- [ ] go to `(x, y)` (`motion_gotoxy`)
- [ ] glide `n` secs to `(random position, mouse position, a sprite position)` (`motion_glideto`)
- [ ] glide `n` secs to `(x, y)` (`motion_glidesecstoxy`)
- [ ] point in direction `direction` (`motion_pointindirection`)
- [ ] point towards `(random position, a sprite position)` (`motion_pointtowards`)
- [ ] change `x` by `n` (`motion_changexby`)
- [ ] set `x` to `n` (`motion_setx`)
- [ ] change `y` by `n` (`motion_changeyby`)
- [ ] set `y` to `n` (`motion_sety`)
- [ ] if on edge bounce (`motion_ifonedgebounce`)
- [ ] set rotation style `(left-right, all around, don't rotate)` (`motion_setrotationstyle`)
##### Motions variables
- [ ] `x position` (`motion_xposition`)
- [ ] `y position` (`motion_yposition`)
- [ ] `direction` (`motion_direction`)

#### Looks
- [ ] say `message` for `n` seconds (`looks_sayforsecs`)
- [ ] say `message` (`looks_say`)
- [ ] think `message` for `n` seconds (`looks_thinkforsecs`)
- [ ] think `message` (`looks_think`)
- [ ] switch costume to `costume` (`looks_switchcostumeto`)
- [ ] next costume (`looks_nextcostume`)
- [ ] switch backdrop to `backdrop` (`looks_switchbackdropto`)
- [ ] switch backdrop to `backdrop` and wait (`looks_switchbackdroptoandwait`)
- [ ] next backdrop (`looks_nextbackdrop`)
- [ ] change size by `n` (`looks_changesizeby`)
- [ ] set size to `n` % (`looks_setsizeto`)
- [ ] change `(color, fisheye, whirl, pixelate, mosaic, brightness, ghost)` effect by `n` (`looks_changeeffectby`)
- [ ] set `(color, fisheye, whirl, pixelate, mosaic, brightness, ghost)` effect to `n` (`looks_seteffectto`)
- [ ] clear graphic effects (`looks_cleargraphiceffects`)
- [ ] show (`looks_show`)
- [ ] hide (`looks_hide`)
- [ ] go to `(front, back)` layer (`looks_gotofrontback`)
- [ ] go `(forward, backward)` `n` layers (`looks_goforwardbackwardlayers`)
##### Looks variables
- [ ] `costume (number, name)` (`looks_costumenumbername`)
- [ ] `backdrop (number, name)` (`looks_backdropnumbername`)
- [ ] `size` (`looks_size`)

#### Sounds
- [ ] play sound `sound` until done (`sound_playuntildone`)
- [ ] start sound `sound` (`sound_play`)
- [ ] stop all sounds (`sound_stopallsounds`)
- [ ] change `(pitch, pan left/right)` effect by `n` (`sound_changeeffectby`)
- [ ] set `(pitch, pan left/right)` effect to `n` (`sound_seteffectto`)
- [ ] clear sound effects (`sound_cleareffects`)
- [ ] change volume by `n` (`sound_changevolumeby`)
- [ ] set volume to `n` % (`sound_setvolumeto`)
##### Sounds variables
- [ ] `volume` (`sound_volume`)

#### Events
- [x] when green flag clicked (`event_whenflagclicked`)
- [ ] when `key` key pressed (`event_whenkeypressed`)
- [ ] when this sprite clicked (`event_whenthisspriteclicked`)
- [ ] when stage clicked (`event_whenstageclicked`)
- [ ] when backdrop switches to `backdrop` (`event_whenbackdropswitchesto`)
- [ ] when `(loudness, timer)` > `n` (`event_whengreaterthan`)
- [ ] when I receive `message` (`event_whenbroadcastreceived`)
- [ ] broadcast `message` (`event_broadcast`)
- [ ] broadcast `message` and wait (`event_broadcastandwait`)

#### Control
- [ ] wait `n` seconds (`control_wait`)
- [ ] repeat `n` (`control_repeat`)
- [ ] forever (`control_forever`)
- [ ] if `condition` then (`control_if`)
- [ ] if `condition` then ... else ... (`control_if_else`)
- [ ] wait until `condition` (`control_wait_until`)
- [ ] repeat until `condition` (`control_repeat_until`)
- [ ] stop `(all, this script, other scripts in sprite)` (`control_stop`)
- [ ] when I start as a clone (`control_start_as_clone`)
- [ ] create clone of `(myself, a sprite)` (`control_create_clone_of`)
- [ ] delete this clone (`control_delete_this_clone`)

#### Sensing
- [ ] ask `question` and wait (`sensing_askandwait`)
- [ ] set drag mode `(draggable, not draggable)` (`sensing_setdragmode`)
- [ ] reset timer (`sensing_resettimer`)
##### Sensing variables
- [ ] touching `(mouse-pointer, edge, a sprite)` ? (`sensing_touchingobject`)
- [ ] touching color `color` ? (`sensing_touchingcolor`)
- [ ] color `color` is touching `color` ? (`sensing_coloristouchingcolor`)
- [ ] distance to `(mouse-pointer, a sprite)` (`sensing_distanceto`)
- [ ] `answer` (`sensing_answer`)
- [ ] key `key` pressed? (`sensing_keypressed`)
- [ ] mouse down? (`sensing_mousedown`)
- [ ] `mouse x` (`sensing_mousex`)
- [ ] `mouse y` (`sensing_mousey`)
- [ ] `loudness` (`sensing_loudness`)
- [ ] `timer` (`sensing_timer`)
- [ ] `(attribute)` of `(sprite, stage)` (`sensing_of`)
- [ ] `current (year, month, date, day of week, hour, minute, second)` (`sensing_current`)
- [ ] `days since 2000` (`sensing_dayssince2000`)
- [ ] `username` (`sensing_username`)

#### Operators
- [ ] `num1` + `num2` (`operator_add`)
- [ ] `num1` - `num2` (`operator_subtract`)
- [ ] `num1` * `num2` (`operator_multiply`)
- [ ] `num1` / `num2` (`operator_divide`)
- [ ] pick random `min` to `max` (`operator_random`)
- [ ] `val1` > `val2` (`operator_gt`)
- [ ] `val1` < `val2` (`operator_lt`)
- [ ] `val1` = `val2` (`operator_equals`)
- [ ] `condition1` and `condition2` (`operator_and`)
- [ ] `condition1` or `condition2` (`operator_or`)
- [ ] not `condition` (`operator_not`)
- [ ] join `string1` `string2` (`operator_join`)
- [ ] letter `n` of `string` (`operator_letter_of`)
- [ ] length of `string` (`operator_length`)
- [ ] `string` contains `substring` ? (`operator_contains`)
- [ ] `num1` mod `num2` (`operator_mod`)
- [ ] round `n` (`operator_round`)
- [ ] `(abs, floor, ceiling, sqrt, sin, cos, tan, asin, acos, atan, ln, log, e ^, 10 ^)` of `n` (`operator_mathop`)

#### Variables
##### Variables
- [ ] set `variable` to `value` (`data_setvariableto`)
- [ ] change `variable` by `n` (`data_changevariableby`)
- [ ] show variable `variable` (`data_showvariable`)
- [ ] hide variable `variable` (`data_hidevariable`)
- [ ] `variable` reporter (`data_variable`)
##### Lists
- [ ] add `item` to `list` (`data_addtolist`)
- [ ] delete `n` of `list` (`data_deleteoflist`)
- [ ] delete all of `list` (`data_deletealloflist`)
- [ ] insert `item` at `n` of `list` (`data_insertatlist`)
- [ ] replace item `n` of `list` with `item` (`data_replaceitemoflist`)
- [ ] item `n` of `list` (`data_itemoflist`)
- [ ] item # of `item` in `list` (`data_itemnumoflist`)
- [ ] length of `list` (`data_lengthoflist`)
- [ ] `list` contains `item` ? (`data_listcontainsitem`)
- [ ] show list `list` (`data_showlist`)
- [ ] hide list `list` (`data_hidelist`)
- [ ] `list` reporter (`data_listcontents`)

#### My Blocks
- [ ] define `custom block` (`procedures_definition`)
- [ ] `custom block (arguments)` call (`procedures_call`)
- [ ] string / number argument reporter (`argument_reporter_string_number`)
- [ ] boolean argument reporter (`argument_reporter_boolean`)

---

## License

licensed under the MIT License.

