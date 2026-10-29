import skrash

project = skrash.load("tests/checks/testall.sb3")
cat = project.sprites["Sprite1"]

@skrash.when_green_flag_clicked
def setup():
    cat.x = 100
    cat.y = 50
    cat.move(10)
    cat.say("Hello depuis Python !")

project.run()