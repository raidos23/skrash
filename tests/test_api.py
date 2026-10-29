from warnings import catch_warnings
import skrash

project = skrash.load("tests/checks/testall.sb3")
cat = project.get_sprite("Sprite1")  

cat.x = 100
cat.y = 50

cat.move(10)

cat.say("Hello depuis Python !")

project.run()