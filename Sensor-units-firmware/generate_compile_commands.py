import os
Import("env")

from os get env
# Include the toolchain paths (std libraries, Arduino.h, etc)
env.Replace(COMPILATIONDB_INCLUDE_TOOLCHAIN=True)

# Force the file to be generated in the project root
env.Replace(COMPILATIONDB_PATH=os.path.join("$PROJECT_DIR", "compile_commands.json"))
