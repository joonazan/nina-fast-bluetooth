PROJECT_NAME := bluetooth-coprocessor

# Currently this cannot be used
# Compilation fails in release mode
ifeq ($(RELEASE),1)
CFLAGS += -DNDEBUG -DCONFIG_FREERTOS_ASSERT_DISABLE -O2 -DLOG_LOCAL_LEVEL=0
CPPFLAGS += -DNDEBUG -O2
endif

include $(IDF_PATH)/make/project.mk
