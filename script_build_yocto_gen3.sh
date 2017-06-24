#!/bin/bash

if [ ! -d meta-renesas ] ; then
	echo "ERROR: Please extract or git clone meta-renesas"
	return -1
fi

tput reset
BINARIES_DIR=$1

# "1" is Enable; "0" is Disable
BUILD_BSP_POKY_CONF="0"
BUILD_BSP_LINARO_CONF="0"
BUILD_GFX_CONF="0"
BUILD_MMP_CONF="1"

MMP_ENABLE="0"
USE_V4L2="1"

BUILD_H3="1"
BUILD_M3="1"
BUILD_H3ULCB="0"
BUILD_M3ULCB="0"

# "1" is online; "0" is offline
NETWORK="0"

export CURRENT_DIR=`pwd`

set_up_meta_data() {
	git clone /shsv/RCarSW/rvc_git_repo/yocto/poky.git
	cd poky
	git checkout -b temp cca8dd15c8096626052f6d8d25ff1e9a606104a3
	cd ..

	git clone /shsv/RCarSW/rvc_git_repo/yocto/meta-linaro.git
	cd meta-linaro
	git checkout -b temp f865492472ad39065365586ed667415741baff5b
	cd ..

	git clone /shsv/RCarSW/rvc_git_repo/yocto/meta-openembedded.git
	cd meta-openembedded
	git checkout -b temp 851a064b53dca3b14dd33eaaaca9573b1a36bf0e
	cd ..
}

if [ ! -d meta-openembedded -o ! -d poky -o ! -d meta-linaro ] ; then
	set_up_meta_data
fi

bitbake_all(){
	echo "bitbake core-image-weston-sdk -c fetchall"
	bitbake core-image-weston-sdk -c fetchall
	echo "bitbake core-image-weston"
	bitbake core-image-weston
	echo "bitbake core-image-weston-sdk"
	bitbake core-image-weston-sdk
	echo "bitbake core-image-weston -c populate_sdk"
	bitbake core-image-weston -c populate_sdk
	echo "bitbake core-image-weston-sdk -c populate_sdk"
	bitbake core-image-weston-sdk -c populate_sdk
}

add_config_for_download(){
	echo "SOURCE_MIRROR_URL ?= \"file:///shsv/RCarSW/rvc_git_repo/yocto/downloads\"" >> conf/local.conf 
	echo "INHERIT += \"own-mirrors\"" >> conf/local.conf 
	echo "BB_GENERATE_MIRROR_TARBALLS = \"1\"" >> conf/local.conf
	if [ $1 -eq 0 ] ; then
		cd ../poky
		git am /shsv/RCarSW/rvc_git_repo/yocto/patches-for-workaground/0001-gitsm-Work-around-to-build-offline.patch
		sed -i 's|git:|gitsm:|g' ../meta-renesas/meta-rcar-gen3/recipes-multimedia/gstreamer/*bbappend
		cd -
		echo "BB_NO_NETWORK = \"1\"" >> conf/local.conf
	fi
}

add_config_for_m3() {
	sed -i '/SOC_FAMILY/d' conf/local.conf 
	echo "SOC_FAMILY = \"r8a7796\"" >> conf/local.conf
}

enable_mutimedia() {
    if [ ${MMP_ENABLE} -eq 1 ]; then
        sed -i  's|#DISTRO*|DISTRO|g' conf/local.conf
        sed -i '/dddec_lib/c #DISTRO_FEATURES_append = " dddec_lib"' conf/local.conf
        sed -i '/dddec_mdw/c #DISTRO_FEATURES_append = " dddec_mdw"' conf/local.conf
    fi
    if [ ${USE_V4L2} -eq 0 ]; then 
        sed -i '/v4l2-renderer/c #DISTRO_FEATURES_append = " v4l2-renderer"' conf/local.conf
    fi
}

add_faster_build_config() {
    echo "BB_NUMBER_THREADS = \"16\"" >> conf/local.conf
    echo "PARALLEL_MAKE = \"-j 16\"" >> conf/local.conf
}

build_bsp_conf_poky() {
	echo
	echo
	echo "/ ============================ Build BSP GCC Poky for $1 ============================ /"

	if [ ! -d build_bsp_poky ] ; then
		mkdir build_bsp_poky
	fi

	cd build_bsp_poky
	cp -rf ../poky .
	cp -rf ../meta-openembedded .
	cp -rf ../meta-renesas .
	source poky/oe-init-build-env ${1}_build_bsp

	if [ $1 = "h3" -o $1 = "m3" ]; then
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/salvator-x/poky-gcc/bsp/local-wayland.conf \
			conf/local.conf
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/salvator-x/poky-gcc/bsp/bblayers.conf \
			conf/bblayers.conf
		if [ $1 = "m3" ]; then
			add_config_for_m3
		fi
	fi

	if [ $1 = "h3ulcb" ]; then
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/h3ulcb/poky-gcc/bsp/local-wayland.conf \
			conf/local.conf
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/h3ulcb/poky-gcc/bsp/bblayers.conf \
			conf/bblayers.conf
	fi

	if [ $1 = "m3ulcb" ]; then
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/m3ulcb/poky-gcc/bsp/local-wayland.conf \
			conf/local.conf
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/m3ulcb/poky-gcc/bsp/bblayers.conf \
			conf/bblayers.conf
	fi

	add_config_for_download ${NETWORK}
	#add_faster_build_config
	bitbake_all

	cd ..
	source poky/oe-init-build-env ${1}_build_bsp_minimal

	if [ $1 = "h3" -o $1 = "m3" ]; then
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/salvator-x/poky-gcc/bsp/local.conf \
			conf/local.conf
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/salvator-x/poky-gcc/bsp/bblayers.conf \
			conf/bblayers.conf
		if [ $1 = "m3" ]; then
			add_config_for_m3
		fi
	fi

	if [ $1 = "h3ulcb" ]; then
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/h3ulcb/poky-gcc/bsp/local.conf \
			conf/local.conf
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/h3ulcb/poky-gcc/bsp/bblayers.conf \
			conf/bblayers.conf
	fi

	if [ $1 = "m3ulcb" ]; then
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/m3ulcb/poky-gcc/bsp/local.conf \
			conf/local.conf
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/m3ulcb/poky-gcc/bsp/bblayers.conf \
			conf/bblayers.conf
	fi

	add_config_for_download ${NETWORK}
	#add_faster_build_config
	bitbake core-image-minimal
	bitbake core-image-minimal -c populate_sdk

	cd $CURRENT_DIR
}

build_bsp_conf_linaro() {
	echo
	echo
	echo "/ ============================ Build BSP GCC Linaro for $1 ============================ /"

	if [ ! -d build_bsp_linaro ] ; then
		mkdir build_bsp_linaro
	fi

	cd build_bsp_linaro
	cp -rf ../poky .
	cp -rf ../meta-linaro .
	cp -rf ../meta-openembedded .
	cp -rf ../meta-renesas .

	source poky/oe-init-build-env ${1}_build_bsp_weston

	if [ $1 = "h3" -o $1 = "m3" ]; then
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/salvator-x/linaro-gcc/bsp/local-wayland.conf \
			conf/local.conf
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/salvator-x/linaro-gcc/bsp/bblayers.conf \
			conf/bblayers.conf
		if [ $1 = "m3" ]; then
			add_config_for_m3
		fi
	fi

	if [ $1 = "h3ulcb" ]; then
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/h3ulcb/linaro-gcc/bsp/local-wayland.conf \
			conf/local.conf
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/h3ulcb/linaro-gcc/bsp/bblayers.conf \
			conf/bblayers.conf
	fi

	if [ $1 = "m3ulcb" ]; then
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/m3ulcb/linaro-gcc/bsp/local-wayland.conf \
			conf/local.conf
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/m3ulcb/linaro-gcc/bsp/bblayers.conf \
			conf/bblayers.conf
	fi

	add_config_for_download ${NETWORK}
	#add_faster_build_config
	bitbake_all

	cd ..
	source poky/oe-init-build-env ${1}_build_bsp_minimal

	if [ $1 = "h3" -o $1 = "m3" ]; then
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/salvator-x/linaro-gcc/bsp/local.conf \
			conf/local.conf
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/salvator-x/linaro-gcc/bsp/bblayers.conf \
			conf/bblayers.conf
		if [ $1 = "m3" ]; then
			add_config_for_m3
		fi
	fi

	if [ $1 = "h3ulcb" ]; then
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/h3ulcb/linaro-gcc/bsp/local.conf \
			conf/local.conf
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/h3ulcb/linaro-gcc/bsp/bblayers.conf \
			conf/bblayers.conf
	fi

	if [ $1 = "m3ulcb" ]; then
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/m3ulcb/linaro-gcc/bsp/local.conf \
			conf/local.conf
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/m3ulcb/linaro-gcc/bsp/bblayers.conf \
			conf/bblayers.conf
	fi

	add_config_for_download ${NETWORK}
	#add_faster_build_config
	bitbake core-image-minimal
	bitbake core-image-minimal -c populate_sdk

	cd $CURRENT_DIR
}

build_gfx_conf() {
	echo
	echo
	echo "/ ============================ Build GFX for $1 ============================ /"

	if [ ! -d build_gfx-only ] ; then
		mkdir build_gfx-only
	fi

	cd build_gfx-only
	cd meta-renesas
	cp -rf ../poky .
	cp -rf ../meta-linaro .
	cp -rf ../meta-openembedded .
	cp -rf ../meta-renesas .
	sh meta-rcar-gen3/docs/sample/copyscript/copy_proprietary_softwares.sh ${BINARIES_DIR}
	cd ..

	source poky/oe-init-build-env ${1}_build_gfx-only

	if [ $1 = "h3" -o $1 = "m3" ]; then
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/salvator-x/linaro-gcc/gfx-only/local-wayland.conf \
			conf/local.conf
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/salvator-x/linaro-gcc/gfx-only/bblayers.conf \
			conf/bblayers.conf
		if [ $1 = "m3" ]; then
			add_config_for_m3
		fi
	fi

	if [ $1 = "h3ulcb" ]; then
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/h3ulcb/linaro-gcc/gfx-only/local-wayland.conf \
			conf/local.conf
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/h3ulcb/linaro-gcc/gfx-only/bblayers.conf \
			conf/bblayers.conf
	fi

		if [ $1 = "m3ulcb" ]; then
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/m3ulcb/linaro-gcc/gfx-only/local-wayland.conf \
			conf/local.conf
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/m3ulcb/linaro-gcc/gfx-only/bblayers.conf \
			conf/bblayers.conf
	fi

	add_config_for_download ${NETWORK}
	#add_faster_build_config
	bitbake_all

	cd $CURRENT_DIR
}

build_mmp_conf() {
	echo
	echo
	echo "/ ============================ Build MMP for $1 ============================ /"

	if [ ! -d build_mmp ] ; then
		mkdir build_mmp
	fi

	cd build_mmp
	cp -rf ../poky .
	cp -rf ../meta-linaro .
	cp -rf ../meta-openembedded .
	cp -rf ../meta-renesas .
	cd meta-renesas
	sh meta-rcar-gen3/docs/sample/copyscript/copy_proprietary_softwares.sh ${BINARIES_DIR}
	cd ..

	source poky/oe-init-build-env ${1}_build_mmp

	if [ $1 = "h3" -o $1 = "m3" ]; then
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/salvator-x/linaro-gcc/mmp/local-wayland.conf \
			conf/local.conf
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/salvator-x/linaro-gcc/mmp/bblayers.conf \
			conf/bblayers.conf
		if [ $1 = "m3" ]; then
			add_config_for_m3
		fi
	fi

	if [ $1 = "h3ulcb" ]; then
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/h3ulcb/linaro-gcc/mmp/local-wayland.conf \
			conf/local.conf
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/h3ulcb/linaro-gcc/mmp/bblayers.conf \
			conf/bblayers.conf
	fi

	if [ $1 = "m3ulcb" ]; then
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/m3ulcb/linaro-gcc/mmp/local-wayland.conf \
			conf/local.conf
		cp ../meta-renesas/meta-rcar-gen3/docs/sample/conf/m3ulcb/linaro-gcc/mmp/bblayers.conf \
			conf/bblayers.conf
	fi

	enable_mutimedia
	add_config_for_download ${NETWORK}
	#add_faster_build_config
	bitbake_all

	cd $CURRENT_DIR
}

if [ ${BUILD_H3} -eq 1 ]; then
	if [ $BUILD_MMP_CONF -eq 1 ] ; then
		build_mmp_conf h3
	fi
	if [ $BUILD_GFX_CONF -eq 1 ] ; then
		build_gfx_conf h3
	fi
	if [ $BUILD_BSP_LINARO_CONF -eq 1 ] ; then
		build_bsp_conf_linaro h3
	fi
	if [ $BUILD_BSP_POKY_CONF -eq 1 ] ; then
		build_bsp_conf_poky h3
	fi
fi

if [ ${BUILD_M3} -eq 1 ]; then
	if [ $BUILD_MMP_CONF -eq 1 ] ; then
		build_mmp_conf m3
	fi
	if [ $BUILD_GFX_CONF -eq 1 ] ; then
		build_gfx_conf m3
	fi
	if [ $BUILD_BSP_LINARO_CONF -eq 1 ] ; then
		build_bsp_conf_linaro m3
	fi
	if [ $BUILD_BSP_POKY_CONF -eq 1 ] ; then
		build_bsp_conf_poky m3
	fi
fi

if [ ${BUILD_H3ULCB} -eq 1 ]; then
	if [ $BUILD_MMP_CONF -eq 1 ] ; then
		build_mmp_conf h3ulcb
	fi
	if [ $BUILD_GFX_CONF -eq 1 ] ; then
		build_gfx_conf h3ulcb
	fi
	if [ $BUILD_BSP_LINARO_CONF -eq 1 ] ; then
		build_bsp_conf_linaro h3ulcb
	fi
	if [ $BUILD_BSP_POKY_CONF -eq 1 ] ; then
		build_bsp_conf_poky h3ulcb
	fi
fi

if [ ${BUILD_M3ULCB} -eq 1 ]; then
	if [ $BUILD_MMP_CONF -eq 1 ] ; then
		build_mmp_conf m3ulcb
	fi
	if [ $BUILD_GFX_CONF -eq 1 ] ; then
		build_gfx_conf m3ulcb
	fi
	if [ $BUILD_BSP_LINARO_CONF -eq 1 ] ; then
		build_bsp_conf_linaro m3ulcb
	fi
	if [ $BUILD_BSP_POKY_CONF -eq 1 ] ; then
		build_bsp_conf_poky m3ulcb
	fi
fi
