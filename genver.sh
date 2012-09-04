#!/bin/bash

printf "#include \"weak.h\"\n\n// DO NOT EDIT MANUALLY: Generated from script.\n\nconst char *version = \"%s\";\n" $(git describe) > version.c