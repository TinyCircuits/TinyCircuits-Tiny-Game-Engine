CALLBACK_TEST_MOD_DIR := $(USERMOD_DIR)

# Add our source files to the respective variables.
SRC_USERMOD += $(CALLBACK_TEST_MOD_DIR)/test.c
SRC_USERMOD += $(CALLBACK_TEST_MOD_DIR)/node_test.c

# We can add our module folder to include paths if needed
# This is not actually needed in this example.
CFLAGS_USERMOD += -I$(CALLBACK_TEST_MOD_DIR)
CEXAMPLE_MOD_DIR := $(USERMOD_DIR)