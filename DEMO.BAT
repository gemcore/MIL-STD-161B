@echo off
sc -e | rl -e | fax -decf | fax -d | rl | sc -q
