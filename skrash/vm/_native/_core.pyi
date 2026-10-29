from typing import Callable, TypeVar

F = TypeVar("F", bound=Callable[..., None])


class Variable:
    """A variable in a loaded Scratch project."""

    @property
    def name(self) -> str:
        """Variable name."""
        ...

    @property
    def value(self) -> float:
        """Current numeric value."""
        ...

    @value.setter
    def value(self, value: float) -> None: ...


class Sprite:
    """A sprite in a loaded Scratch project."""

    @property
    def x(self) -> float:
        """Horizontal position."""
        ...

    @x.setter
    def x(self, value: float) -> None: ...

    @property
    def y(self) -> float:
        """Vertical position."""
        ...

    @y.setter
    def y(self, value: float) -> None: ...

    @property
    def direction(self) -> float:
        """Direction in degrees (0 = up, 90 = right)."""
        ...

    @direction.setter
    def direction(self, value: float) -> None: ...

    def move(self, steps: float) -> None:
        """Move the sprite forward by `steps` units."""
        ...

    def say(self, message: str) -> None:
        """Display a speech bubble (currently prints to stdout)."""
        ...

    def show(self) -> None:
        """Make the sprite visible."""
        ...

    def hide(self) -> None:
        """Hide the sprite."""
        ...


class Project:
    """A loaded Scratch 3 project."""

    @property
    def sprites(self) -> dict[str, Sprite]:
        """Dict of sprites by name (stage excluded).

        Returns a fresh dict each access.
        """
        ...

    @property
    def variables(self) -> dict[str, Variable]:
        """Dict of variables by name.

        If two variables share a name (one global, one local), the
        later one overwrites the earlier in the dict.
        Returns a fresh dict each access.
        """
        ...

    def start(self) -> None:
        """Start the Scratch runtime and fire green-flag callbacks."""
        ...

    def step(self) -> bool:
        """Execute one runtime step.

        Returns True if the runtime is still active, False otherwise.
        """
        ...

    def run(self) -> None:
        """Run the project inside an SDL2 window.

        Starts the runtime, fires green-flag callbacks, then blocks
        until the window is closed (Escape or close button).
        """
        ...


def load(path: str) -> Project:
    """Load a Scratch 3 project from an .sb3 file.

    Raises RuntimeError if the file cannot be loaded or parsed.
    """
    ...


def when_green_flag_clicked(func: F) -> F:
    """Register `func` to be called when the project starts.

    Stored in a global registry, consumed by the next
    `Project.start()` or `Project.run()` call. The registry is
    cleared after consumption.
    """
    ...