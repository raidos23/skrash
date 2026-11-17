import json
from zipfile import ZipFile

import pytest

import skrash


@pytest.mark.parametrize("initial_size", [None, 37.5])
def test_size_reporter_tracks_sprite_size(tmp_path, initial_size):
    """Read size before and after Scratch's set-size and change-size blocks."""
    blocks = {
        "flag": {
            "opcode": "event_whenflagclicked",
            "next": "initial",
            "parent": None,
            "inputs": {},
            "fields": {},
            "topLevel": True,
            "shadow": False,
        },
        "set_size": {
            "opcode": "looks_setsizeto",
            "next": "after_set",
            "parent": "initial",
            "inputs": {"SIZE": [1, [4, "150.5"]]},
            "fields": {},
        },
        "change_size": {
            "opcode": "looks_changesizeby",
            "next": "after_change",
            "parent": "after_set",
            "inputs": {"CHANGE": [1, [4, "-25.25"]]},
            "fields": {},
        },
    }
    for name, parent, next_block in [
        ("initial", "flag", "set_size"),
        ("after_set", "set_size", "change_size"),
        ("after_change", "change_size", None),
    ]:
        reporter = f"{name}_size"
        blocks[name] = {
            "opcode": "data_setvariableto",
            "next": next_block,
            "parent": parent,
            "inputs": {"VALUE": [3, reporter, [4, "0"]]},
            "fields": {"VARIABLE": [name, name]},
        }
        blocks[reporter] = {
            "opcode": "looks_size",
            "next": None,
            "parent": name,
            "inputs": {},
            "fields": {},
        }

    sprite = {
        "isStage": False,
        "name": "Sprite1",
        "variables": {name: [name, -1] for name in ("initial", "after_set", "after_change")},
        "lists": {},
        "blocks": blocks,
        "costumes": [],
    }
    if initial_size is not None:
        sprite["size"] = initial_size

    path = tmp_path / "looks_size.sb3"
    with ZipFile(path, "w") as archive:
        archive.writestr("project.json", json.dumps({"targets": [sprite]}))

    project = skrash.load(str(path))
    project.start()
    for _ in range(100):
        if not project.step():
            break
    else:
        pytest.fail("size reporter script did not finish")

    assert project.variables["initial"].value == (100 if initial_size is None else initial_size)
    assert project.variables["after_set"].value == 150.5
    assert project.variables["after_change"].value == 125.25
