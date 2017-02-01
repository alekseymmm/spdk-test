CC = gcc
LINKER = gcc

APP_NAME := spdk-test 

SPDK_ROOT_DIR := /root/spdk/
DPDK_DIR := /root/spdk/dpdk-16.11/x86_64-native-linuxapp-gcc

SPDK_LIBS_DIR := $(SPDK_ROOT_DIR)/build/lib/
SPDK_LIBS := -lspdk_nvme -lspdk_util -lspdk_log -lspdk_env_dpdk

DPDK_LIBS_DIR := $(DPDK_DIR)/lib/
DPDK_LIBS := -lrte_eal -lrte_mempool -lrte_ring

RDMA_LIBS := -libverbs -lrdmacm

START_GROUP_FLAG := -Wl,--start-group -Wl,--whole-archive
END_GROUP_FLAG := -Wl,--end-group -Wl,--no-whole-archive

CFLAGS = -Wall -pthread -I $(SPDK_ROOT_DIR)/include/ \
-I $(DPDK_DIR)/include -include $(SPDK_ROOT_DIR)/config.h
 
LFLAGS = -Wall -Wl,-z,relro,-z,now -Wl,-z,noexecstack -pthread \
-L $(SPDK_LIBS_DIR) $(SPDK_LIBS) -L $(DPDK_LIBS_DIR) \
$(START_GROUP_FLAG) $(DPDK_LIBS) -ldl $(END_GROUP_FLAG) \
$(RDMA_LIBS) -lrt


OBJS := main.o

.PHONY: all $(DIRS-y)

%.o : %.c
	@$(CC) $(CFLAGS) -c $< -o $@
	
all: $(APP_NAME)

$(APP_NAME) : $(OBJS)
	@echo "Start building..."
	@$(LINKER) -o $(APP_NAME) $(OBJS) $(LFLAGS)
	@echo "Build done."



