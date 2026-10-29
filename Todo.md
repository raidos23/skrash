# TODO / Roadmap

Detailed progress of Scratch 3.0 block support in **skrash**.

Status legend:
- `[x]` : implemented and functional
- `[~]` : partial (implemented, but behavior incomplete or not Scratch-conformant)
- `[ ]` : not implemented

---

## Motion

- [x] move `n` steps (`motion_movesteps`)
- [x] turn `n` degrees (right) (`motion_turnright`)
- [x] turn `n` degrees (left) (`motion_turnleft`)
- [ ] go to `(random position, mouse position, a sprite position)` (`motion_goto`)
- [x] go to `(x, y)` (`motion_gotoxy`)
- [ ] glide `n` secs to `(random position, mouse position, a sprite position)` (`motion_glideto`)
- [ ] glide `n` secs to `(x, y)` (`motion_glidesecstoxy`)
- [x] point in direction `direction` (`motion_pointindirection`)
- [ ] point towards `(random position, a sprite position)` (`motion_pointtowards`)
- [x] change `x` by `n` (`motion_changexby`)
- [x] set `x` to `n` (`motion_setx`)
- [x] change `y` by `n` (`motion_changeyby`)
- [x] set `y` to `n` (`motion_sety`)
- [ ] if on edge bounce (`motion_ifonedgebounce`) — center clamp only, no bounce
- [ ] set rotation style `(left-right, all around, don't rotate)` (`motion_setrotationstyle`) — unsupported (no field on SB3Target)

### Motion reporters
- [x] `x position` (`motion_xposition`)
- [x] `y position` (`motion_yposition`)
- [x] `direction` (`motion_direction`)

---

## Looks

- [ ] say `message` for `n` seconds (`looks_sayforsecs`)
- [x] say `message` (`looks_say`) — terminal only
- [ ] think `message` for `n` seconds (`looks_thinkforsecs`)
- [x] think `message` (`looks_think`) — terminal only
- [x] switch costume to `costume` (`looks_switchcostumeto`)
- [x] next costume (`looks_nextcostume`)
- [ ] switch backdrop to `backdrop` (`looks_switchbackdropto`)
- [ ] switch backdrop to `backdrop` and wait (`looks_switchbackdroptoandwait`)
- [ ] next backdrop (`looks_nextbackdrop`)
- [x] change size by `n` (`looks_changesizeby`)
- [x] set size to `n` % (`looks_setsizeto`)
- [ ] change `(color, fisheye, whirl, pixelate, mosaic, brightness, ghost)` effect by `n` (`looks_changeeffectby`)
- [ ] set `(color, fisheye, whirl, pixelate, mosaic, brightness, ghost)` effect to `n` (`looks_seteffectto`)
- [ ] clear graphic effects (`looks_cleargraphiceffects`)
- [x] show (`looks_show`)
- [x] hide (`looks_hide`)
- [ ] go to `(front, back)` layer (`looks_gotofrontback`)
- [ ] go `(forward, backward)` `n` layers (`looks_goforwardbackwardlayers`)

### Looks reporters
- [ ] `costume (number, name)` (`looks_costumenumbername`)
- [ ] `backdrop (number, name)` (`looks_backdropnumbername`)
- [ ] `size` (`looks_size`)

---

## Sounds

- [ ] play sound `sound` until done (`sound_playuntildone`)
- [ ] start sound `sound` (`sound_play`)
- [ ] stop all sounds (`sound_stopallsounds`)
- [ ] change `(pitch, pan left/right)` effect by `n` (`sound_changeeffectby`)
- [ ] set `(pitch, pan left/right)` effect to `n` (`sound_seteffectto`)
- [ ] clear sound effects (`sound_cleareffects`)
- [ ] change volume by `n` (`sound_changevolumeby`)
- [ ] set volume to `n` % (`sound_setvolumeto`)

### Sounds reporters
- [ ] `volume` (`sound_volume`)

---

## Events

- [x] when green flag clicked (`event_whenflagclicked`)
- [x] when `key` key pressed (`event_whenkeypressed`) — limited mapping (a-z, space, arrows, `any` via is_key_pressed only)
- [x] when this sprite clicked (`event_whenthisspriteclicked`)
- [x] when stage clicked (`event_whenstageclicked`)
- [ ] when backdrop switches to `backdrop` (`event_whenbackdropswitchesto`)
- [~] when `(loudness, timer)` > `n` (`event_whengreaterthan`) — timer only (loudness unsupported)
- [x] when I receive `message` (`event_whenbroadcastreceived`)
- [x] broadcast `message` (`event_broadcast`)
- [~] broadcast `message` and wait (`event_broadcastandwait`) — fired but does not block correctly

---

## Control

- [x] wait `n` seconds (`control_wait`)
- [x] repeat `n` (`control_repeat`)
- [x] forever (`control_forever`)
- [x] if `condition` then (`control_if`)
- [x] if `condition` then ... else ... (`control_if_else`)
- [ ] wait until `condition` (`control_wait_until`)
- [x] repeat until `condition` (`control_repeat_until`)
- [~] stop `(all, this script, other scripts in sprite)` (`control_stop`) — `this script` and `all` OK, `other scripts in sprite` partial
- [x] when I start as a clone (`control_start_as_clone`)
- [x] create clone of `(myself, a sprite)` (`control_create_clone_of`)
- [x] delete this clone (`control_delete_this_clone`)

---

## Sensing

- [ ] ask `question` and wait (`sensing_askandwait`)
- [ ] set drag mode `(draggable, not draggable)` (`sensing_setdragmode`)
- [ ] reset timer (`sensing_resettimer`)

### Sensing reporters
- [~] touching `(mouse-pointer, edge, a sprite)` ? (`sensing_touchingobject`) — mouse and sprite OK (coarse AABB), edge unreliable (center clamp)
- [ ] touching color `color` ? (`sensing_touchingcolor`)
- [ ] color `color` is touching `color` ? (`sensing_coloristouchingcolor`)
- [ ] distance to `(mouse-pointer, a sprite)` (`sensing_distanceto`)
- [ ] `answer` (`sensing_answer`)
- [x] key `key` pressed? (`sensing_keypressed`)
- [x] mouse down? (`sensing_mousedown`)
- [x] `mouse x` (`sensing_mousex`)
- [x] `mouse y` (`sensing_mousey`)
- [ ] `loudness` (`sensing_loudness`)
- [ ] `timer` (`sensing_timer`)
- [ ] `(attribute)` of `(sprite, stage)` (`sensing_of`)
- [ ] `current (year, month, date, day of week, hour, minute, second)` (`sensing_current`)
- [ ] `days since 2000` (`sensing_dayssince2000`)
- [ ] `username` (`sensing_username`)

---

## Operators

- [x] `num1` + `num2` (`operator_add`)
- [x] `num1` - `num2` (`operator_subtract`)
- [x] `num1` * `num2` (`operator_multiply`)
- [x] `num1` / `num2` (`operator_divide`)
- [x] pick random `min` to `max` (`operator_random`)
- [x] `val1` > `val2` (`operator_gt`) — numeric only
- [x] `val1` < `val2` (`operator_lt`) — numeric only
- [x] `val1` = `val2` (`operator_equals`) — numeric only (string comparison not implemented)
- [x] `condition1` and `condition2` (`operator_and`)
- [x] `condition1` or `condition2` (`operator_or`)
- [x] not `condition` (`operator_not`)
- [x] join `string1` `string2` (`operator_join`)
- [x] letter `n` of `string` (`operator_letter_of`)
- [x] length of `string` (`operator_length`)
- [x] `string` contains `substring` ? (`operator_contains`)
- [x] `num1` mod `num2` (`operator_mod`)
- [x] round `n` (`operator_round`)
- [x] `(abs, floor, ceiling, sqrt, sin, cos, tan, asin, acos, atan, ln, log, e ^, 10 ^)` of `n` (`operator_mathop`)

---

## Variables & Lists

### Variables
- [x] set `variable` to `value` (`data_setvariableto`) — numeric storage only (`double`)
- [x] change `variable` by `n` (`data_changevariableby`)
- [x] show variable `variable` (`data_showvariable`) — no-op
- [x] hide variable `variable` (`data_hidevariable`) — no-op
- [x] `variable` reporter (`data_variable`)

### Lists
- [x] add `item` to `list` (`data_addtolist`) — truncated at 512 characters
- [x] delete `n` of `list` (`data_deleteoflist`)
- [x] delete all of `list` (`data_deletealloflist`)
- [x] insert `item` at `n` of `list` (`data_insertatlist`)
- [x] replace item `n` of `list` with `item` (`data_replaceitemoflist`)
- [x] item `n` of `list` (`data_itemoflist`)
- [x] item # of `item` in `list` (`data_itemnumoflist`)
- [x] length of `list` (`data_lengthoflist`)
- [x] `list` contains `item` ? (`data_listcontainsitem`)
- [x] show list `list` (`data_showlist`) — no-op
- [x] hide list `list` (`data_hidelist`) — no-op
- [x] `list` reporter (`data_listcontents`)

---

## My Blocks

- [ ] define `custom block` (`procedures_definition`)
- [ ] `custom block (arguments)` call (`procedures_call`)
- [ ] string / number argument reporter (`argument_reporter_string_number`)
- [ ] boolean argument reporter (`argument_reporter_boolean`)

---

## Renderer / Runtime (non-block)

- [x] SDL2 window + basic stage rendering
- [x] SVG costume rendering (nanosvg)
- [x] Bitmap costumes (stb_image)
- [ ] Graphic effects (ghost, brightness, etc.)
- [ ] Layer ordering
- [ ] Sound engine
- [x] Mouse / keyboard input events
- [x] Clones rendering
- [ ] Say overlay in window (terminal only)
- [ ] Batch rendering / sprite batching
- [ ] Correct fence (clamp to real edges, not center)

---

## Python API

- [x] `skrash.load(path)`
- [x] `Project.start()` / `Project.step()`
- [x] `Project.run()` (opens SDL2 window)
- [x] `Project.get_sprite(name)`
- [x] `Sprite.x` / `Sprite.y` / `Sprite.direction` (read/write)
- [x] `Sprite.move(steps)`
- [x] `Sprite.say(message)`
- [x] `Project.get_variable(name)`
- [ ] `@when_green_flag_clicked` decorator
- [ ] `@when_receive("msg")` decorator
- [ ] `wait(seconds)` (blocking for now, greenlets later)
- [ ] `Sprite.costume` / `Sprite.size` / `Sprite.visible` (read/write)
- [ ] `Project.targets` (list all sprites)

---

## Known Issues / Limitations

- Variables and lists: `double`-only storage for variables, `char*` truncated at 512 for lists. Text values in `set [var] to (join ...)` are lost (stored as 0).
- `operator_equals`, `gt`, `lt`: numeric comparison only, no string fallback.
- Clones: share blocks and costumes with the original (correct), but local-variable copy semantics are applied via deep copy in `sb3_runtime_create_clone` (see commits).
- Renderer: no layer sorting, no graphic effects, no say overlay.
- `when_timer >`: rising-edge latch implemented (`greater_than_latched`).