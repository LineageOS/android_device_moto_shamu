function build_att_image {
    # Stage the OEM partition contents
    TARGET=$ANDROID_PRODUCT_OUT/att
    TARGETIMG=$ANDROID_PRODUCT_OUT/att.img
    rm -rf $TARGET $TARGETIMG
    mkdir -p $TARGET

    # Copy over staged files
    cp -R $ANDROID_BUILD_TOP/device/moto/shamu/att/staging/* $TARGET

    OEMSIZE=$(cat $ANDROID_BUILD_TOP/device/moto/shamu/BoardConfig.mk |grep "BOARD_OEMIMAGE_PARTITION_SIZE"|cut -d= -f 2|tr -d ' ')
    make_ext4fs -s -l $OEMSIZE -a oem $TARGETIMG $TARGET
}
