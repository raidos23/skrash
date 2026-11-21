# Python API Ideas

This document presents ideas for the Python API, inspired by the way the official Scratch editor organizes and manages projects, sprites, the stage, and variable scopes. The goal is to bring these familiar Scratch concepts into a Python interface for controlling and inspecting projects.

## 1. Project API

The `Project` object is the central entry point for interacting with a Scratch project through Python.

### Proposed structure

```text
Project
│
├── start()
├── step()
├── run()
│
├── variables
│   └── Global variables
│
├── sprites
│   └── [name]
│       └── variables
│           ├── Global variables
│           └── Sprite-local variables
│
├── stage
│   └── variables
│       ├── Global variables
│       └── Stage-local variables
│
├── mouse
│   ├── move(x, y)
│   ├── press(button)
│   └── release(button)
│
├── click
│   ├── sprite(name)
│   └── stage()
│
└── keyboard
    ├── press(key)
    └── release(key)
```

## 2. Runtime Control

The `Project` object provides methods to control project execution.

| Method | Description |
|---|---|
| `project.start()` | Starts the project's scripts. |
| `project.step()` | Advances the runtime by one frame. |
| `project.run()` | Starts the runtime execution loop. |

## 3. Variable Access

The API follows Scratch's variable scope model: variables can be global or belong to a specific target.

### Global variables

```python
project.variables["score"]
```

Provides access to project-wide variables.

### Sprite variables

```python
player = project.sprites["Player"]

player.variables["score"]
player.variables["health"]
```

Provides access to global variables and variables local to the selected sprite.

### Stage variables

```python
project.stage.variables["score"]
project.stage.variables["level"]
```

Provides access to global variables and variables local to the stage.

## 4. Virtual Mouse

The `project.mouse` interface injects mouse events directly into the Skrash runtime.

```python
project.mouse.move(230, 23)

project.mouse.press("left")
project.mouse.release("left")
```

| Method | Description |
|---|---|
| `move(x, y)` | Simulates mouse movement inside the runtime. |
| `press(button)` | Simulates a mouse button press. |
| `release(button)` | Simulates a mouse button release. |

### Physical mouse vs. virtual mouse

The virtual mouse is separate from the user's physical operating-system mouse.

It must not:

- Move the real system cursor.
- Click outside the Skrash runtime.
- Interfere with other applications.

Virtual mouse events are injected internally into Skrash's input system. Physical mouse events and Python-generated virtual mouse events are separate input sources that reach the runtime's input system.

## 5. Targeted Clicks

The `project.click` interface provides a higher-level way to simulate clicks on specific project elements.

```python
project.click.sprite("Player")
project.click.stage()
```

| Method | Description |
|---|---|
| `sprite(name)` | Simulates a click targeting the specified sprite. |
| `stage()` | Simulates a click targeting the stage. |

This interface provides direct targeting without requiring the caller to manually move and press the virtual mouse.

## 6. Virtual Keyboard

The `project.keyboard` interface injects keyboard events directly into the Skrash runtime.

```python
project.keyboard.press("space")
project.keyboard.release("space")
```

| Method | Description |
|---|---|
| `press(key)` | Simulates a key press. |
| `release(key)` | Simulates a key release. |

Keyboard simulation remains internal to Skrash and must not generate operating-system-level keyboard input.

## 7. Design Principles

1. **Scratch-inspired model:** Reuse familiar concepts from the official Scratch editor, including projects, sprites, the stage, and variable scopes.
2. **Scratch compatibility:** Preserve Scratch's project concepts and variable scopes.
3. **Runtime isolation:** Keep simulated input events inside the Skrash runtime.
4. **Python accessibility:** Provide a straightforward interface for controlling and inspecting Scratch projects.
5. **Clear separation:** Keep virtual mouse input, targeted clicks, and virtual keyboard input distinct.
6. **Minimalism:** Expose useful runtime capabilities without unnecessarily reproducing Scratch's internal block structure.
