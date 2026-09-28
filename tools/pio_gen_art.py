# Hook do PlatformIO: regenera src/art/ a partir de art/*.art antes de compilar.
import os
import subprocess

Import("env")  # noqa: F821 (injetado pelo SCons do PlatformIO)

root = env.subst("$PROJECT_DIR")  # noqa: F821
subprocess.check_call([env.subst("$PYTHONEXE"), os.path.join(root, "tools", "gen_art.py")])  # noqa: F821
