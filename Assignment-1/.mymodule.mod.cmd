savedcmd_mymodule.mod := printf '%s\n'   mymodule.o | awk '!x[$$0]++ { print("./"$$0) }' > mymodule.mod
