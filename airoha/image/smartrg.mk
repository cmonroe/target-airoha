.NOTPARALLEL:

SRGRUN:= TARGET_DIR=$(TARGET_DIR) KDIR=$(KDIR) STAGING_DIR=$(STAGING_DIR_HOST) BIN_DIR=$(BIN_DIR) PACKAGE_DIR=$(PACKAGE_DIR) $(TARGET_DIR)/../flash-images/files/srg-image.sh
BINNAME:=$(IMG_PREFIX)-dragontail-root.squashfs
VERNAME:=$(VERSION_NUMBER)-$(subst DEVICE_,,$(PROFILE))

define Device/dragontail
  KERNEL_LOADADDR = 0x80200000
  KERNEL_SUFFIX := -fit-multi.itb
  KERNEL_INSTALL := 1
  KERNEL_NAME := Image
  KERNEL = kernel-bin | lzma | SrgFit
  FILESYSTEMS := squashfs
  KERNEL_INITRAMFS :=
  DEVICE_VENDOR := SmartRG
  DEVICE_MODEL := SmartRG Target
  DEVICE_DTS := an7581-smartrg-evb-emmc
  DEVICE_DTS += an7581-smartrg-SDG-8716v
  DEVICE_DTS += an7581-smartrg-SDG-8736v
  DEVICE_DTS_DIR := ../dts
  DEVICE_PACKAGES := kmod-i2c-an7581
  ARTIFACT/preloader.bin := an7581-preloader smartrg_dragontail
  ARTIFACT/bl31-uboot.fip := an7581-bl31-uboot smartrg_dragontail
  ARTIFACT/bl2-bl31-uboot.bin := an7581-emmc-bl2-bl31-uboot smartrg_dragontail
  ARTIFACTS := bl2-bl31-uboot.bin preloader.bin bl31-uboot.fip
  DTC_FLAGS += -@
  IMAGES := root.squashfs img img.run
  IMAGE/root.squashfs := SrgDisk
  IMAGE/img := srgImage
  export DEVICE_DTS
endef
TARGET_DEVICES := dragontail

#define Device/polecat
#  KERNEL_LOADADDR = 0x43200000
#  KERNEL_SUFFIX := -fit-multi.itb
#  KERNEL_INSTALL := 1
#  KERNEL_NAME := Image
#  KERNEL = kernel-bin | lzma | SrgFit
#  FILESYSTEMS := squashfs
#  KERNEL_INITRAMFS :=
#  DEVICE_VENDOR := SmartRG
#  DEVICE_MODEL := SmartRG Target
#  DEVICE_PACKAGES := kmod-hwmon-pwmfan kmod-i2c-gpio kmod-sfp kmod-usb-ohci kmod-usb2 kmod-usb3 kmod-ata-ahci-mtk e2fsprogs f2fsck mkf2fs
#  DEVICE_DTS := mt7622-smartrg-srbpi
#  DEVICE_DTS += mt7622-smartrg-834-5
#  DEVICE_DTS += mt7622-smartrg-841-t6
#  DEVICE_DTS += mt7622-smartrg-854-6
#  DEVICE_DTS += mt7622-smartrg-854-6-sfp
#  DEVICE_DTS += mt7622-smartrg-854-v6
#  DEVICE_DTS += mt7622-smartrg-854-v6-sfp
#  DEVICE_DTS += mt7622-smartrg-834-v6
#  DEVICE_DTS += mt7986a-smartrg-bpi-r3
#  DEVICE_DTS += mt7986a-smartrg-SDG-8612
#  DEVICE_DTS += mt7986a-smartrg-SDG-8614
#  DEVICE_DTS += mt7986a-smartrg-SDG-8622
#  DEVICE_DTS += mt7986a-smartrg-SDG-8632
#  DEVICE_DTS += mt7981b-smartrg-SDG-8610
#  DEVICE_DTS += mt7988a-smartrg-SDG-8733
#  DEVICE_DTS += mt7988a-smartrg-SDG-8733v
#  DEVICE_DTS += mt7988a-smartrg-SDG-8734
#  DEVICE_DTS += mt7988a-smartrg-SDG-8734v
#  DEVICE_DTS += mt7988d-smartrg-SDG-8733A
#  DEVICE_DTS += mt7988a-smartrg-SDG-8713
#  DEVICE_DTS += mt7988a-smartrg-SDG-8713v
#  DEVICE_DTS += mt7987a-smartrg-SDG-8712
#  DEVICE_DTS += mt7987a-smartrg-SDG-8712v
#  DEVICE_DTS += mt7987a-smartrg-SDG-8732
#  DEVICE_DTS += mt7987a-smartrg-SDG-8732v
#  DEVICE_DTS += mt7988a-smartrg-SDG-9000
#  DEVICE_DTS += mt7988d-smartrg-SDG-9732i
#  DEVICE_DTS += mt7988d-smartrg-SDG-9712o
#  DEVICE_DTS_DIR := ../dts
#  ARTIFACTS := emmc-preloader.bin emmc-bl31-uboot.fip
#  ARTIFACT/emmc-preloader.bin := mt7986-bl2 emmc-ddr4
#  ARTIFACT/emmc-bl31-uboot.fip := mt7986-bl31-uboot smartrg_bonanza
#  # Need stuff here
#  DTC_FLAGS += -@
#  IMAGES := root.squashfs img img.run
#  IMAGE/root.squashfs := SrgDisk
#  IMAGE/img := srgImage
#  export DEVICE_DTS
#endef
#TARGET_DEVICES := polecat 

define Build/SrgFit

	# lzma compress the dtb files
	$(eval $(foreach S,$(DEVICE_DTS),$(shell $(STAGING_DIR_HOST)/bin/lzma e -lc1 -lp2 -pb2 $(KDIR)/image-$(S).dtb $(KDIR)/image-$(S).dtb.lzma)))

	srg-mkits.sh -o $@.its -A $(LINUX_KARCH)  -v $(LINUX_VERSION) \
	   	-i "k1" -k $@ -a $(KERNEL_LOADADDR) -e $(if $(KERNEL_ENTRY),$(KERNEL_ENTRY),$(KERNEL_LOADADDR)) -C lzma -h "crc32" -h "sha1" \
		-i "rdisk" -r $(STAGING_DIR_IMAGE)/$(IMG_PREFIX)-initramfs.cpio.gz -h "crc32" -h "sha1" \
		-i "SDG-an7581-rfb" -d $(KDIR)/image-an7581-smartrg-evb-emmc.dtb -h "crc32" -h "sha1" \
		-i "SDG-8716v" -d $(KDIR)/image-an7581-smartrg-SDG-8716v.dtb -h "crc32" -h "sha1" \
		-i "SDG-8736v" -d $(KDIR)/image-an7581-smartrg-SDG-8736v.dtb -h "crc32" -h "sha1" \
		-c "303" -K k1 -R rdisk -D "SDG-an7581-rfb" \
		-c "600" -K k1 -R rdisk -D "SDG-8716v" \
		-c "601" -K k1 -R rdisk -D "SDG-8736v"

	PATH=$(LINUX_DIR)/scripts/dtc:$(PATH) mkimage -f $@.its $@.new
	@mv -f $@.new $@

endef

define Build/SrgDiskSquashfs
	@echo "Creating SRG squashfs Image"
	mkdir -p $(TARGET_DIR)/mnt/FLASH
	mkdir -p $(TARGET_DIR)/mnt/boot
	mkdir -p $(TARGET_DIR)/FLASH
	mkdir -p $(TARGET_DIR)/boot
	mkdir -p $(TARGET_DIR)/Boot
	$(STAGING_DIR_HOST)/bin/mksquashfs4 $(TARGET_DIR) $(KDIR)/root.squashfs.run \
		-nopad -noappend -root-owned \
		-comp $(SQUASHFSCOMP) $(SQUASHFSOPT)
	$(CP) $(KDIR)/root.squashfs.run $(KDIR)/root.squashfs.run.bin 
	dd if=/dev/zero bs=128k count=1 >> $(KDIR)/root.squashfs.run.bin
	sha256sum  $(KDIR)/root.squashfs.run.bin  | cut -d ' ' -f 1 | xargs echo -n  >> $(KDIR)/root.squashfs.run.bin
	$(CP) $(KDIR)/root.squashfs.run.bin $(KDIR)/$(BINNAME).run.bin

	sha256sum  $(BIN_DIR)/$(IMG_PREFIX)-dragontail-fit-multi.itb | cut -d ' ' -f 1 | xargs echo -n  >> $(BIN_DIR)/$(IMG_PREFIX)-dragontail-fit-multi.itb
	$(CP) $(BIN_DIR)/$(IMG_PREFIX)-dragontail-fit-multi.itb $(TARGET_DIR)/Boot/fit-multi.itb
	$(STAGING_DIR_HOST)/bin/mksquashfs4 $(TARGET_DIR) $(KDIR)/root.squashfs \
		-nopad -noappend -root-owned \
		-comp $(SQUASHFSCOMP) $(SQUASHFSOPT)
	$(CP) $(KDIR)/root.squashfs $(KDIR)/root.squashfs.bin 
	dd if=/dev/zero bs=128k count=1 >> $(KDIR)/root.squashfs.bin
	sha256sum  $(KDIR)/root.squashfs.bin  | cut -d ' ' -f 1 | xargs echo -n  >> $(KDIR)/root.squashfs.bin
	$(CP) $(KDIR)/root.squashfs.bin $(KDIR)/$(BINNAME).bin
	$(CP) $(KDIR)/root.squashfs.bin $(BIN_DIR)/
	ln -sfr $(BIN_DIR)/root.squashfs.bin $(BIN_DIR)/root.squashfs.img
	ln -sfr $(BIN_DIR)/$(IMG_PREFIX)-dragontail-fit-multi.itb $(BIN_DIR)/fit-multi.itb
endef

define Build/SrgDisk
    $(call Build/SrgDiskSquashfs)
endef

define Build/srgImage
	@echo "Build generic image and .run image"
	bash -c "$(SRGRUN) SRGImages $(BINNAME).bin $(VERNAME)"  
	bash -c "CDT= $(SRGRUN) RUNIMG $(BINNAME) $(VERNAME) $(IMG_PREFIX)"  
	bash -c "CDT=factory $(SRGRUN) RUNIMG $(BINNAME) $(VERNAME) $(IMG_PREFIX)"  
endef

define Build/srgImageRun
	rm -rf $(KDIR)/img_stage
	mkdir -p $(KDIR)/img_stage
	mkdir -p $(KDIR)/img_stage/cdt
	mkdir -p $(KDIR)/img_stage/allcdts
	rm -f $(KDIR)/metadata
	touch $(KDIR)/metadata
	if [ $(1) ]; then \
		if [ "$(1)" = "allcdts" ]; then\
			find $(wildcard $(PACKAGE_SUBDIRS)) -type f -name 'cdt-*.ipk' -exec cp "{}" $(KDIR)/img_stage/allcdts \; ; \
			echo "CDT=all" >> $(KDIR)/metadata; \
		else\
			CDT_IPK=`find $(wildcard $(PACKAGE_SUBDIRS)) -type f -name 'cdt-$(1)_*.ipk' -print -quit` ; \
			cp $$CDT_IPK $(KDIR)/img_stage/cdt ; \
			echo "CDT=\"$(1)\"" >> $(KDIR)/metadata; \
			echo "CDT_VERSION=\"$(CDT_VERSION)\"" >> $(KDIR)/metadata; \
		fi\
	else \
		echo "CDT=" >> $(KDIR)/metadata; \
	fi
	cat $(TARGET_DIR)/etc/openwrt_release | sed "s/'/\"/g" >> $(KDIR)/metadata
	cat $(TARGET_DIR)/../flash-images/files/scripts/arch_platforms.sh | grep PLATFORMS >> $(KDIR)/metadata
	# for debugging add
		# --keep --verbose
	$(STAGING_DIR_HOST)/bin/sos_bld_run.py \
		--lsm $(KDIR)/metadata \
		--img_type SOS_UPGRADE \
		--encrypt \
		--self_install self-upgrade.sh \
		--add_image_file $(TARGET_DIR)/usr/srg/scripts/self-upgrade.sh "." \
		--add_image_file $(KDIR)/img_stage/ "." \
		--add_image_file $(TARGET_DIR)/../flash-images/files/scripts/ check_scripts/ \
		--add_image_file $(TARGET_DIR)/Boot Boot/ \
		--add_image_file $(BIN_DIR)/$(IMG_PREFIX)-dragontail-fit-multi.itb Boot/fit-multi.itb \
		--add_image_file $(TARGET_DIR)/usr/srg/scripts/img.sh scripts/ \
		--add_image_file $(TARGET_DIR)/usr/srg/scripts/flash-manage.sh scripts/ \
		--add_image_file $(TARGET_DIR)/usr/srg/scripts/emmc-manage.sh scripts/ \
		--add_image_file $(TARGET_DIR)/usr/srg/scripts/emmc-disk-layout-1.sh scripts/ \
		--add_image_file $(TARGET_DIR)/usr/srg/scripts/emmc-disk-layout-2.sh scripts/ \
		--add_image_file $(TARGET_DIR)/usr/srg/scripts/emmc-disk-layout-info.sh scripts/ \
		--add_image_file $(TARGET_DIR)/usr/srg/scripts/console_manage.sh scripts/ \
		--add_image_file $(TARGET_DIR)/etc/openwrt_release etc/ \
		--add_image_file $(KDIR)/root.squashfs.run.bin root.squashfs.bin \
		--out_file $(KDIR)/$(BINNAME).run
	$(CP) $(KDIR)/$(BINNAME).run $(BIN_DIR)/$(BINNAME)$(if $(1),-$(1)-$(CDT_VERSION),).run
	@echo "RUNNING BuildPackage alt-os-image"
	mkdir -p $(BIN_DIR)/alt-os-images
	$(STAGING_DIR_HOST)/bin/alt-os-images/transition.sh -f plumeos -t sos -i $(BIN_DIR)/$(BINNAME)$(if $(1),-$(1)-$(CDT_VERSION),).run -d $(BIN_DIR)/alt-os-images

	# Create SOS_HOT_FIX version. Will install image on the system without shutdown/reboot. 
	# Next boot will use the installed image.
#	@echo "#!/usr/bin/env bash" > $(KDIR)/post_sos_hot_fix.sh
#	@echo "echo \"Running post_sos_hot_fix.sh\"" >> $(KDIR)/post_sos_hot_fix.sh
#	@echo "exit 1" >> $(KDIR)/post_sos_hot_fix.sh
#	
#	$(STAGING_DIR_HOST)/bin/sos_bld_run.py \
		--lsm $(KDIR)/metadata \
		--img_type SOS_HOT_FIX \
		--encrypt \
		--self_install self-upgrade.sh \
		--add_image_file $(KDIR)/post_sos_hot_fix.sh "post_upgrade/" \
		--add_image_file $(TARGET_DIR)/usr/srg/scripts/self-upgrade.sh "." \
		--add_image_file $(KDIR)/img_stage/ "." \
		--add_image_file $(TARGET_DIR)/../flash-images/files/scripts/ check_scripts/ \
		--add_image_file $(TARGET_DIR)/Boot Boot/ \
		--add_image_file $(BIN_DIR)/$(IMG_PREFIX)-dragontail-fit-multi.itb Boot/fit-multi.itb \
		--add_image_file $(TARGET_DIR)/usr/srg/scripts/img.sh scripts/ \
		--add_image_file $(TARGET_DIR)/usr/srg/scripts/flash-manage.sh scripts/ \
		--add_image_file $(TARGET_DIR)/usr/srg/scripts/emmc-manage.sh scripts/ \
		--add_image_file $(TARGET_DIR)/usr/srg/scripts/emmc-disk-layout-1.sh scripts/ \
		--add_image_file $(TARGET_DIR)/usr/srg/scripts/emmc-disk-layout-2.sh scripts/ \
		--add_image_file $(TARGET_DIR)/usr/srg/scripts/emmc-disk-layout-info.sh scripts/ \
		--add_image_file $(TARGET_DIR)/usr/srg/scripts/console_manage.sh scripts/ \
		--add_image_file $(TARGET_DIR)/etc/openwrt_release etc/ \
		--add_image_file $(KDIR)/root.squashfs.run.bin root.squashfs.bin \
		--out_file $(KDIR)/$(BINNAME)_HOT_FIX.run
#	$(CP) $(KDIR)/$(BINNAME)_HOT_FIX.run $(BIN_DIR)/$(BINNAME)$(if $(1),-$(1),)_HOT_FIX.run

endef

define Image/Flash/mkflash_emmc
	@echo "BUILD FLASH image CDT : $(1)"
	CDT_IPK=`find $(wildcard $(PACKAGE_SUBDIRS)) -type f -name 'cdt-$(1)_*.ipk' -print -quit` ; \
	OUT_DIR="$(BIN_DIR)/flashprogram_bins"; \
	mkdir -p $$OUT_DIR; \
	if [ $(3) ]; then \
		IMGFLAG="-i $(3)"; \
	else \
		IMGFLAG=""; \
	fi; \
	mk_emmc_mfg_image.sh $${IMGFLAG} -e $(2) -r $(TARGET_DIR) -R $(BIN_DIR)/root.squashfs.bin -b $(BIN_DIR) -c $$CDT_IPK -d $$OUT_DIR/$(IMG_PREFIX)-dragontail-emmc-mfg-$(2)-$(1).bin
endef

#
# The following targets are called from the flash-images package Makefile
#  Each calls back to the flash-images SRGRUN=srg-image.sh
#  BINNAME and VERNAME are computed here locally.
#  CDT [ENUM] [IMGFLAG] are env variables passed on the cmdline.
#

flashme:
	@echo "Creating FLASH image"
	$(call Image/Flash/mkflash_emmc,$(CDT),$(ENUM),$(IMGFLAG))

cdt-image:
	@echo "Build CDT image $(CDT)"
	bash -c "CDT=$(CDT) $(SRGRUN) CDT $(BINNAME) $(VERNAME)"
	bash -c "CDT=$(CDT) $(SRGRUN) RUNIMG $(BINNAME) $(VERNAME)"

mini-cdt-image:
	@echo "Build Mini CDT image $(CDT)"
	bash -c "CDT=$(CDT) $(SRGRUN) miniCDT $(BINNAME) $(VERNAME)"

allcdt-image:
	@echo "Build ALL CDT image"
	bash -c "CDT=allcdts $(SRGRUN) ALLCDT $(BINNAME) $(VERNAME)"
	bash -c "CDT=allcdts $(SRGRUN) RUNIMG $(BINNAME) $(VERNAME) $(IMG_PREFIX)"

trans-image:
	@echo "Build SOS to PLOS transition image"
	bash -c "CDT=$(CDT) $(SRGRUN) TRANSIMG $(BINNAME) $(VERNAME) $(IMG_PREFIX)"


