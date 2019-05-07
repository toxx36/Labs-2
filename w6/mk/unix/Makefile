INCLUDE_DIRS += -I./include
SOURCE_DIRS += ./src
LIB_DIR = ./lib
LIB_SRC_DIR = ./lib_src

BUILD_HOME=../..
include $(BUILD_HOME)/Options

CFLAGS += $(INCLUDE_DIRS)

FNAME1 = zdo_start_zc
FNAME2 = zdo_start_zr

SRCS1 = $(SOURCE_DIRS)/$(FNAME1).c
OBJS1 = ${SRCS1:.c=.o}
SRCS2 = $(SOURCE_DIRS)/$(FNAME2).c
OBJS2 = ${SRCS2:.c=.o}

$(LIB_DIR)/lib%.a : $(LIB_SRC_DIR)/%/*.o
	mkdir -p $(@D)
	@echo "Linking library $@"
	$(MAKE_LIB)

$(BIN)zdo_start_zc$(EXE) : $(OBJS1) $(LIBS) $(LIB_DIR)/libled.a $(LIB_DIR)/libbulb_input.a $(LIB_DIR)/libbuttons.a 
	$(LINK_BINARY)
	$(OBJCOPY) -O ihex $(FNAME1) $(FNAME1).hex
	$(OBJCOPY) -I ihex -O binary $(FNAME1).hex $(FNAME1).bin

$(BIN)zdo_start_zr$(EXE) : $(OBJS2)  $(LIBS) $(LIB_DIR)/libbuttons.a $(LIB_DIR)/libbulb_remote.a
	$(LINK_BINARY)
	$(OBJCOPY) -O ihex $(FNAME2) $(FNAME2).hex
	$(OBJCOPY) -I ihex -O binary $(FNAME2).hex $(FNAME2).bin

BINS =  $(BIN)zdo_start_zc$(EXE)
BINS += $(BIN)zdo_start_zr$(EXE)

all: $(BINS)

c:
	rm -rf *.hex *.bin *.o core core.* vgcore.* *~ */*.o src/*~ src/*.o include/*~ zdo_start_zc zdo_start_ze zdo_start_zr  

burn_$(FNAME1): $(FNAME1).bin
	st-flash write $(FNAME1).bin 0x08000000

burn_$(FNAME2): $(FNAME2).bin
	st-flash write $(FNAME2).bin 0x08000000
