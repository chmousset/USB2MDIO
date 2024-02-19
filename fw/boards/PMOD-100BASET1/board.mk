# List of all the board related files.
BOARDSRC = $(CHIBIOS)/../boards/PMOD-100BASET1/board.c

# Required include directories
BOARDINC = $(CHIBIOS)/../boards/PMOD-100BASET1

# Shared variables
ALLCSRC += $(BOARDSRC)
ALLINC  += $(BOARDINC)
