import skrash

project = skrash.load("tests/checks/data.sb3")

compteur = project.get_variable("compteur")
print("Nom :", compteur.name)
print("Valeur initiale :", compteur.value)

compteur.value = 60
print("Après modification :", compteur.value)

compteur.value += 5
print("Après += 5 :", compteur.value)

project.run()