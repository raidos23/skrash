# Skrash Python API

Python bindings for the Skrash Scratch 3 runtime.

## Quick start

```python
import skrash

project = skrash.load("game.sb3")
player = project.sprites["Player"]

@skrash.when_green_flag_clicked
def setup():
    player.x = 0
    player.y = -100
    player.show()

project.run()
```

## `skrash.load(path)`

Load a Scratch 3 project from an `.sb3` file.

**Arguments:**
- `path` (`str`): path to the `.sb3` file.

**Returns:** `Project`

**Raises:**
- `FileNotFoundError`: if `path` does not exist.
- `RuntimeError`: if the file is not a valid Scratch 3 project.

```python
project = skrash.load("my_game.sb3")
```

---

## `Project`

### `Project.sprites`

Property returning a `dict[str, Sprite]` of sprites by name (stage excluded).

Returns a fresh dict on each access. Sprite wrappers are recreated on each access.

```python
for name, sprite in project.sprites.items():
    print(name, sprite.x, sprite.y)
```

### `Project.variables`

Property returning a `dict[str, Variable]` of variables by name.

If two variables share a name (one global, one local), the later one
overwrites the earlier in the dict.

### `Project.start()`

Start the Scratch runtime and fire green-flag callbacks.

Raises `RuntimeError` if the project failed to start (e.g. no scripts
on the green flag).

### `Project.step()`

Execute one runtime step.

**Returns:** `bool` — `True` if the runtime is still active, `False` otherwise.

```python
project.start()
while project.step():
    pass
```

### `Project.run()`

Open an SDL2 window and run the project until it's closed (Escape key
or close button).

**Blocks** the calling thread. Internally calls `start()`, so do not
call both `start()` and `run()` on the same project.

```python
project.run()
```

---

## `Sprite`

### `Sprite.x` / `Sprite.y`

Horizontal / vertical position (read/write, `float`). Scratch coordinates:
origin at the center, `x` in `[-240, 240]`, `y` in `[-180, 180]`, `y` pointing up.

```python
sprite.x = 100
sprite.y = -50
```

### `Sprite.direction`

Direction in degrees (read/write, `float`). Scratch convention: `0` = up,
`90` = right.

### `Sprite.move(steps)`

Move the sprite forward by `steps` units along its current direction.

### `Sprite.say(message)`

Print a message to stdout (not rendered in the SDL window).

```python
sprite.say("Hello")
```

### `Sprite.show()` / `Sprite.hide()`

Make the sprite visible / invisible.

---

## `Variable`

### `Variable.name`

Variable name (read-only, `str`).

### `Variable.value`

Current numeric value (read/write, `float`).

```python
score = project.variables["score"]
score.value = 42
print(score.value)   # 42.0
```

---

## `@when_green_flag_clicked`

Decorator that registers a function to be called when the project starts.

The callback is stored in a **global registry**, consumed by the next
`Project.start()` or `Project.run()` call. The registry is cleared after
consumption, so only the first project to start sees the callbacks.

```python
@skrash.when_green_flag_clicked
def setup():
    print("started!")

project.run()
```


