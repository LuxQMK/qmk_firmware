# LuxQMK Core Engine
SRC += luxqmk.c

# Automatically include the matching board module based on $(KEYBOARD)
ifneq ($(filter gmmk/gmmk3%,$(KEYBOARD)),)
    SRC += boards/gmmk3.c
else ifneq ($(filter gmmk/gmmk2%,$(KEYBOARD)),)
    SRC += boards/gmmk2.c
else
    SRC += boards/generic.c
endif
