# flash_tool_for_bl602
The flash tools designed for BL602 and written in C

After six months's break from work, I think of doing something for fun.
Pinecone provides an inexpensive platform to play with well documented guides.
https://wiki.pine64.org/wiki/PineCone

Starting with re-writing flashing tools, I plan to continue to explore the potential
of the small evaluation board (Pine64 BL602 EVB ver1.2).

inc/boot_header_info.h is copied from bl602_boot2/bl602_boot2/blsp_bootinfo.h. Idealy
the structure should be decoupled from the rest so that it can be shared with flashing tools.
The code is written with least hard code value so that it can survive the change of the data
structure in future.

The device tree file dts can be built into dtb format with Linux tool "device tree compiler"
(dtc) separately.

The flashing workflow
=====================

This is based on the document of
``https://github.com/bouffalolab/bl_docs/blob/main/BL602_ISP/en/BL602_ISP_protocol.pdf''
and the open source flash tool from bouffalolab,
``https://github.com/bouffalolab/BLOpenFlasher/tree/main''

Generate the flash image
-----------------------

1./ create partition image
    The partition configuration is specified to include all partition entries
    in the format of 'toml', e.g. partition_cfg_2M.toml. It might also define
    the number of partitions at different locations.

    The generated image(s) are  'partition.bin@addr'.

2./ Generate boot2 image
    The inputs for generating boot2 image are raw blsp_boot.bin, and the
    configuration file which specifies the boot/efuse configuration, e.g.
    efuse_bootheader_cfg.conf. This procedure actually generates boot header,
    and packs the original blsp_boot.bin with it.

3./ Generate FW image
    Similiarly, this step packs the firmware image with the boot header.

4./ Generate the device tree image in dtb
    The dts source file shared by Buffalolab is an almost DTS compliant file.
    That might be the reason that the open source bl flash tool comes with an
    additional python code 'dts2dtb.py'. For example,
    bl_factory_params_IoTKitA_40M.dts, '/include/ "bl602_base.dtsi";' need to
    remove as no such files exist. In fact, the dts file can be compiled into
    dtb with device tree compiler (dtc) after the following patch:
        diff --git a/bl602/device_tree/bl_factory_params_IoTKitA_40M.dts b/bl602/device_tree/bl_factory_params_IoTKitA_40M.dts
        index 4f02725..ddd5473 100644
        --- a/bl602/device_tree/bl_factory_params_IoTKitA_40M.dts
        +++ b/bl602/device_tree/bl_factory_params_IoTKitA_40M.dts
        @@ -1,5 +1,5 @@
         /dts-v1/;
        -/include/ "bl602_base.dtsi";
        +///include/ "bl602_base.dtsi";
         // version: 17
         // last_comp_version: 16
         // boot_cpuid_phys: 0x0
        @@ -35,8 +35,10 @@
                     feature = "button";
                     active = "Hi";
                     mode = "multipress";
        +            hbn_use = "disable";
                     button {
                         debounce = <10>;
        +                trig_level = "Hi";
                         short_press_ms {
                             start = <100>;
                             end = <3000>;
        @@ -51,9 +53,7 @@
                             start = <15000>;
                             kevent = <4>;
                         };
        -                trig_level = "Hi";
                     };
        -            hbn_use = "disable";
                 };
             };


Flash the images
----------------

Still work in progress....
