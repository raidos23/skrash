class Project:
    """A loaded Scratch 3 project."""

    def start(self) -> None:
        """Start the Scratch runtime."""
        ...

    def step(self) -> bool:
        """Execute one runtime step.

        Returns:
            True if a runtime step was executed, False when the
            runtime has finished or cannot execute another step.
        """
        ...


def load(path: str) -> Project:
    """Load a Scratch 3 project from an .sb3 file."""
    ...