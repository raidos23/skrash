import skrash


def test_project_variables_mapping():
    """Variables are exposed as a name-to-Variable mapping."""
    project = skrash.load("tests/checks/data.sb3")

    compteur = project.variables["compteur"]
    assert compteur.name == "compteur"

    compteur.value = 60
    assert compteur.value == 60

    compteur.value += 5
    assert compteur.value == 65
