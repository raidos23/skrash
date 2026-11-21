import skrash


def test_python_api_without_renderer():
    """Test Python callbacks and sprite controls without opening an SDL window."""
    project = skrash.load("tests/checks/testall.sb3")
    cat = project.sprites["Sprite1"]

    @skrash.when_green_flag_clicked
    def setup():
        cat.x = 100
        cat.y = 50
        cat.move(10)
        cat.say("Hello depuis Python !")

    project.start()

    assert cat.x == 110
    assert cat.y == 50

    steps = 0
    while project.step():
        steps += 1
        assert steps < 1000, "Scratch runtime did not finish"
