# REQUIRES clang-format
# Formats the entire project files inside 'src' directory based on options in '.clang-format' file

import os
from pathlib import Path

fileList = list(Path('./src/').glob('**/*.cpp')) + list(Path('./src/').glob('**/*.h'))

for file in fileList:
	command = f'clang-format --style=file -i {file}'
	os.system(command)