# Python API Ideas

This document is a design roadmap for the proposed Skrash Python API. It records architectural decisions, proposed interfaces, and questions that should be resolved before implementation.

Its purpose is to separate API design from implementation: the API should be thought through as a coherent whole before code is written around individual use cases.

The guiding principle is:

> Python should remain Python, Scratch should remain Scratch, and Skrash should connect the two naturally.

**Important:** Everything in this document is a proposal or design direction unless explicitly marked as implemented elsewhere. Examples illustrate the intended API; they do not claim that these methods or behaviors are currently available.

## Design status

Use these terms when discussing the roadmap:

- **Proposed:** an idea worth considering; details may still change.
- **Designed:** the intended behavior and interface have been specified.
- **Validated:** the design has been reviewed for consistency and is ready to guide implementation.

A proposal does not automatically become an implementation task.

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
│   ├── create(name, value)
│   └── [name]                 # Global variables only
│
├── lists
│   ├── create(name)
│   └── [name]                 # Global lists only
│
├── sprites
│   └── [name]                 # Access existing sprites
│
├── stage
│   ├── variables              # Stage-local variables only
│   └── lists                  # Stage-local lists only
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

Each sprite exposes its own local collections:

```text
Sprite
├── variables                  # This sprite's local variables only
├── lists                      # This sprite's local lists only
└── costumes
```

There must be no implicit fallback from a target's local collection to the project's global collection.

## 2. Runtime Control

The `Project` object provides methods to control project execution.

| Method | Intended behavior |
|---|---|
| `project.start()` | Starts the project's scripts. |
| `project.step()` | Advances the runtime by one frame. |
| `project.run()` | Starts the runtime execution loop. |

The exact lifecycle and interaction between these methods should remain consistent with the runtime's behavior.

## 3. Variable Access and Scope

The API should follow Scratch's ownership model while making each scope explicit.

### Global variables

Global variables belong to the project and are accessed only through `project.variables`.

```python
project.variables.create("score", 0)

score = project.variables["score"]
```

### Sprite-local variables

A sprite's collection contains only variables local to that sprite.

```python
player = project.sprites["Player"]

player.variables.create("health", 100)
health = player.variables["health"]
```

### Stage-local variables

The stage has its own local variables.

```python
stage = project.stage

stage.variables.create("level", 1)
level = stage.variables["level"]
```

### Scope rules

| Collection | Ownership and visibility |
|---|---|
| `project.variables` | Global variables only |
| `sprite.variables` | Local variables of that sprite only |
| `stage.variables` | Stage-local variables only |

A global variable and a local variable may have the same name without being treated as the same variable. Access is always explicit through the owning object. No collection silently combines global and local variables.

Collections should support both creating data and retrieving data that already exists.

## 4. List Access

Lists should follow the same ownership and scope rules as variables.

### Global lists

```python
project.lists.create("players")

players = project.lists["players"]
```

### Sprite-local lists

```python
player = project.sprites["Player"]

player.lists.create("inventory")
inventory = player.lists["inventory"]
```

### Stage-local lists

```python
stage = project.stage

stage.lists.create("unlocked_levels")
unlocked_levels = stage.lists["unlocked_levels"]
```

### Scope rules

| Collection | Ownership and visibility |
|---|---|
| `project.lists` | Global lists only |
| `sprite.lists` | Lists local to that sprite only |
| `stage.lists` | Stage-local lists only |

List operations (item access, insertion, deletion, clearing, and iteration) should be specified consistently in a later design pass. The ownership rules should not depend on those operations.

## 5. Sprite Creation and Extension

The API should support two distinct workflows:

1. Create a new Scratch sprite from Python.
2. Access and optionally extend a sprite that already exists in a loaded Scratch project.

These workflows must not be confused: extending an existing sprite must not accidentally create a duplicate Scratch sprite.

### 5.1 Creating a new sprite from Python

A Python class that directly inherits from `Sprite` represents a new sprite.

```python
class Player(Sprite):
    def __init__(self):
        super().__init__()

        self.costumes.add("player.png")

        self.variables.create("health", 100)
        self.lists.create("inventory")
```

Intended rules:

- The class name becomes the Scratch sprite's name. In this example, the sprite is named `Player`.
- The user should not have to repeat the same name in a separate `name="Player"` argument.
- A newly created sprite must have at least one costume before it is finalized and added to the project.
- Its variables and lists belong to that sprite and remain local to it.
- A name conflict in the project should produce an explicit error. Skrash should not silently rename the sprite.
- Sprite initialization and registration should have a clearly defined lifecycle. In particular, the design should allow the costume to be supplied before the sprite is finalized.

Automatic class-name discovery is a design requirement. Python's `__init_subclass__` may be suitable for capturing the subclass name, but the final mechanism is an implementation detail and should be validated before adoption.

The costume API shown above is illustrative. The accepted costume sources, asset loading behavior, and validation rules must be specified in the design before implementation.

### 5.2 Accessing an existing Scratch sprite

A sprite already present in a loaded project can be retrieved directly:

```python
player = project.sprites["Player"]

player.variables.create("health", 100)
player.lists.create("inventory")
```

Retrieving a sprite must return an interface to that existing project entity; it must not create a second sprite.

### 5.3 Extending an existing Scratch sprite through inheritance

The proposed user-facing direction is to allow a Python class to inherit from an existing sprite:

```python
class Player(project.sprites["Player"]):
    def __init__(self):
        self.variables.create("health", 100)
```

This is a proposed syntax, not a confirmed implementation contract. The technical design still needs to establish:

- How a runtime sprite object can serve as a Python base class.
- How the derived Python class is associated with the original Scratch sprite.
- How initialization avoids duplicating or resetting the underlying Scratch entity.
- How repeated access and class construction behave.
- What happens if the same existing sprite is extended more than once.

The essential requirement is that inheritance adds a Python layer to the existing sprite rather than creating a new Scratch sprite.

## 6. Virtual Mouse

The `project.mouse` interface injects mouse events directly into the Skrash runtime.

```python
project.mouse.move(230, 23)

project.mouse.press("left")
project.mouse.release("left")
```

| Method | Intended behavior |
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

## 7. Targeted Clicks

The `project.click` interface provides a higher-level way to simulate clicks on specific project elements.

```python
project.click.sprite("Player")
project.click.stage()
```

| Method | Intended behavior |
|---|---|
| `sprite(name)` | Simulates a click targeting the specified sprite. |
| `stage()` | Simulates a click targeting the stage. |

This interface provides direct targeting without requiring the caller to manually move and press the virtual mouse.

The design should specify how targeting behaves when sprites overlap, are hidden, or are otherwise not clickable under Scratch's rules.

## 8. Virtual Keyboard

The `project.keyboard` interface injects keyboard events directly into the Skrash runtime.

```python
project.keyboard.press("space")
project.keyboard.release("space")
```

| Method | Intended behavior |
|---|---|
| `press(key)` | Simulates a key press. |
| `release(key)` | Simulates a key release. |

Keyboard simulation remains internal to Skrash and must not generate operating-system-level keyboard input.

## 9. Design Principles

The API should be evaluated against these principles before implementation:

1. **Natural Python:** prefer familiar Python constructs and avoid unnecessary ceremony.
2. **Scratch fidelity:** preserve Scratch's concepts, ownership rules, and relevant runtime behavior.
3. **Explicit ownership:** every variable and list has a clear owner.
4. **No implicit scope resolution:** global and local data are accessed through distinct collections.
5. **Two-way continuity:** Python can create new Scratch entities and work with entities already present in Scratch projects.
6. **No accidental duplication:** extending or retrieving an existing sprite must not create another one.
7. **Consistent abstractions:** variables and lists should follow parallel ownership and access patterns.
8. **Design before implementation:** resolve architectural questions and edge cases before building the API around them.

## 10. Suggested Design Roadmap

This roadmap orders the design work; it does not claim that any item is implemented.

| Area | Next design action |
|---|---|
| Project API | Confirm lifecycle and project entry points. |
| Variable Access | Specify creation, retrieval, mutation, and scope behavior. |
| List Access | Specify collection behavior and list operations. |
| Sprite Creation | Define class naming, costume requirements, finalization, and registration. |
| Sprite Extension | Resolve the inheritance and entity-association model. |
| Runtime Control | Confirm execution semantics and edge cases. |
| Virtual Mouse | Specify event behavior and button semantics. |
| Targeted Clicks | Specify hit-testing and Scratch-compatible targeting behavior. |
| Virtual Keyboard | Specify key naming and event semantics. |

The goal of this document is to provide a coherent design reference before implementation begins. It should evolve as decisions are reviewed, and uncertain proposals should remain visibly marked as such rather than being treated as settled API behavior.
