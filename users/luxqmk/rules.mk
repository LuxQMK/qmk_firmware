# LuxQMK Core Engine
SRC += luxqmk.c \
       luxqmk_gradients.c \
       luxqmk_eeprom.c \
       luxqmk_protocol.c \
       luxqmk_reactive.c

# Enable VIA Dynamic Keymap & WebHID Protocol across all LuxQMK builds
VIA_ENABLE ?= yes

# Custom Dynamic Eager Debounce Engine
DEBOUNCE_TYPE = custom

# Automatically include the matching board module based on $(KEYBOARD)
ifneq ($(filter gmmk/gmmk3%,$(KEYBOARD)),)
    SRC += boards/gmmk3.c
else ifneq ($(filter gmmk/gmmk2%,$(KEYBOARD)),)
    SRC += boards/gmmk2.c
else ifneq ($(filter keychron%,$(KEYBOARD)),)
    SRC += boards/keychron.c
else
    SRC += boards/generic.c
endif
