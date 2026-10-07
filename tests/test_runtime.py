import skrash


PROJECT_PATH = "tests/checks/one.sb3"

def test_python_runtime_execution():
    """Test the Python API and native Scratch runtime execution."""

    print(f"[TEST] Loading project: {PROJECT_PATH}")

    project = skrash.load(PROJECT_PATH)

    assert project is not None
    print("[OK] Project loaded")

    print("[TEST] Starting runtime")

    project.start()

    print("[OK] Runtime started")

    steps = 0

    while project.step():
        steps += 1
        print(f"[OK] Runtime step {steps}")

    assert steps > 0

    print(f"[OK] Runtime finished after {steps} step(s)")


if __name__ == "__main__":
    test_python_runtime_execution()