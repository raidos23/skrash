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
        """Display a speech bubble."""
        ...


class Project:
    """A loaded Scratch 3 project."""

    def start(self) -> None:
        """Start the Scratch runtime."""
        ...

    def step(self) -> bool:
        """Execute one runtime step.

        Returns:
            True if the runtime is still active, False when it has
            finished or cannot execute another step.
        """
        ...

    def run(self) -> None:
        """Run the project inside an SDL2 window.

        Blocks until the window is closed (Escape key or close button).
        """
        ...

    def get_sprite(self, name: str) -> Sprite:
        """Return the sprite with the given name.

        Raises:
            KeyError: if no sprite matches `name`.
        """
        ...


def load(path: str) -> Project:
    """Load a Scratch 3 project from an .sb3 file.

    Raises:
        RuntimeError: if the file cannot be loaded or parsed.
    """
    ...

def get_variable(self, name: str) -> Variable:
        """Return a variable by name.
        
        Raises:
            KeyError: if the variable does not exist.
        """
        ...

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

