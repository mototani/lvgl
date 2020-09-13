LVGL_DIR      := $(shell pwd)
LVGL_DIR_NAME := lvgl

CC ?= gcc
AR ?= ar

BUILD ?= debug
OBJDIR ?= $(BUILD)

CSRCS   := main.c widget.c lv_port_disp_sdl2.c lv_port_indev_sdl2.c

DEPPATH :=

VPATH   := src:$(LVGL_DIR)/$(LVGL_DIR_NAME)/$(OBJDIR)

CFLAGS  := -I$(LVGL_DIR) -I$(LVGL_DIR)/$(LVGL_DIR_NAME)
CFLAGS  += $(shell pkg-config --cflags sdl2 libpng freetype2)

LDFLAGS :=

LIBS    := $(LVGL_DIR)/$(LVGL_DIR_NAME)/$(OBJDIR)/liblvgl.a
LIBS    += $(shell pkg-config --libs sdl2 libpng freetype2)

OBJS := $(addprefix $(OBJDIR)/,$(CSRCS:.c=.o))


all: $(OBJDIR)/demo

$(OBJDIR)/demo: $(OBJS) | $(OBJDIR)
	@echo ---- $@
	$(CC) $^ $(LDFLAGS) $(LIBS) -o $@

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	@echo ---- $@
	$(CC) -c $< $(CFLAGS) $(TARGET_ARCH) -o $@

$(OBJDIR):
	@echo ---- $@
	mkdir -p $@

clean:
	rm -rf $(OBJDIR) *~ src/*~

