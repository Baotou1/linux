KERNELDIR := /home/baotou/linux/atk_mp135/ATK_MP135/alientek_linux/linux-5.15.24
CURRENT_PATH := $(shell pwd)
#EXTRA_CFLAGS += -I/home/baotou/linux/atk_mp135/Drivers/05_list_dev

obj-m := led_drv.o

build: kernel_modules

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean
