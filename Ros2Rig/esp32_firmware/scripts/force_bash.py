# scripts/force_bash.py
from SCons.Script import Import
Import("env")
GIT_BIN = r"C:\Program Files\Git\bin"
GIT_USR_BIN = r"C:\Program Files\Git\usr\bin"
env['ENV']['SHELL'] = rf"{GIT_BIN}\bash.exe"
env['ENV']['PATH']  = GIT_BIN + ";" + GIT_USR_BIN + ";" + env['ENV']['PATH']
env['ENV'].pop('ComSpec', None)  # cmd ausschalten
