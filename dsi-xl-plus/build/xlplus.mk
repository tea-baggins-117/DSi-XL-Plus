# DSi XL+ opt-in foundation. Included before the component derives CXXFLAGS.
# Anchor to the original ARM9 Makefile, including recursive build/ invocations.
XLPLUS ?= 0
ifneq ($(XLPLUS),0)
ifneq ($(XLPLUS),1)
$(error XLPLUS must be 0 or 1)
endif
endif
XLPLUS_SOURCES := ../../dsi-xl-plus
ifeq ($(XLPLUS),1)
SOURCES += $(XLPLUS_SOURCES)/src/core $(XLPLUS_SOURCES)/src/nds
INCLUDES += $(XLPLUS_SOURCES)/include
ifeq ($(CURRENT_SCREEN_MODE),1)
SOURCES += $(XLPLUS_SOURCES)/src/nds/settings
endif
CFLAGS += -DDSIXLPLUS_FOUNDATION=1
else
CFLAGS += -DDSIXLPLUS_FOUNDATION=0
endif
