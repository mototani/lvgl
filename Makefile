LVGL_DIR      ?= $(shell pwd)/..
LVGL_DIR_NAME ?= lvgl

CC ?= gcc
AR ?= ar

BUILD ?= debug
OBJDIR ?= $(BUILD)

CSRCS   :=
DEPPATH :=
VPATH   :=
CFLAGS  ?= -Os -g
include $(LVGL_DIR)/$(LVGL_DIR_NAME)/lvgl.mk

OBJS := $(addprefix $(OBJDIR)/,$(CSRCS:.c=.o))

all: $(OBJDIR)/liblvgl.a

$(OBJDIR)/liblvgl.a: $(OBJS) | $(OBJDIR)
	@echo ---- $@
	rm -f $@
	$(AR) rv $@ $^

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	@echo ---- $@
	$(CC) -c $< $(CFLAGS) $(TARGET_ARCH) -o $@

$(OBJDIR):
	@echo ---- $@
	mkdir -p $@

clean:
	rm -rf $(OBJDIR)
