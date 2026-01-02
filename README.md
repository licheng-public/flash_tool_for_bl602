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

${\color{red} UPDATE \space 11/12/2025}$  **Thanks to chatgpt, a Makefile is added so that
you can run 'make' directly to build all executables into bin directory.**

1./ create partition image
    The partition configuration is specified to include all partition entries
    in the format of 'toml', e.g. partition_cfg_2M.toml. It might also define
    the number of partitions at different locations. The generated image(s)
    are  'partition.bin@addr'.
```
    $./partition_gen -i ../image_and_config/partition_cfg_2M.toml -o partition.bin
```

2./ Generate boot2 image
    The inputs for generating boot2 image are raw blsp_boot.bin, and the
    configuration file which specifies the boot/efuse configuration, e.g.
    efuse_bootheader_cfg.conf. This procedure actually generates boot header,
    and packs the original blsp_boot.bin with it.
```
    $./img_gen -i ../image_and_config/efuse_bootheader_cfg.conf \
      -b ../image_and_config/blsp_boot2.bin  -o ./boot2image.bin -s 0x2000
```

3./ Generate FW image
    Similiarly, this step packs the firmware image with the boot header.
```
    $./img_gen -i ../image_and_config/efuse_bootheader_cfg.conf \
      -b ./sdk_app_helloworld.bin -o ./fw2.bin -s 0x1000
```

4./ Generate the device tree image in dtb
    The dts source file shared by Buffalolab is an almost DTS compliant file.
    That might be the reason that the open source bl flash tool comes with an
    additional python code 'dts2dtb.py'. For example,
    bl_factory_params_IoTKitA_40M.dts, '/include/ "bl602_base.dtsi";' need to
    remove as no such files exist. In fact, the dts file can be compiled into
    dtb with device tree compiler (dtc) after the following patch:
```
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
```

Flash the images
----------------

The command of flashing, and an example of log is

```
$ ./flash --uart /dev/ttyUSB0 --rate 230400 --partition ./partition.bin@0xe000 ./partition.bin@0xf000 \
  --fw ./fw2.bin --dtb ./ro_params.dtb --eflash ./eflash_loader_40m.bin --boot2 ./boot2image.bin

hand shake with rate 230400
Received (2 bytes): OK
SUCCEED: hand shake

received [24] bytes: OK
boot_rom_ver: 0x1
opt_info dump:
0x00 0x00 0x00 0x00 0x03 0x00 0x00 0x00
0xb3 0xab 0x52 0x29 0xd8 0xac 0x18 0x00

SUCCEED: get boot_info

received [2] bytes: OK
SUCCEED: load boot header

received [20] bytes: OK
SUCCEED: load segment header

received [2] bytes: OK
SUCCEED: load segment (2048) bytes data[0]
received [2] bytes: OK
SUCCEED: load segment (2048) bytes data[1]
received [2] bytes: OK
SUCCEED: load segment (2048) bytes data[2]
received [2] bytes: OK
SUCCEED: load segment (2048) bytes data[3]
received [2] bytes: OK
SUCCEED: load segment (2048) bytes data[4]
received [2] bytes: OK
SUCCEED: load segment (2048) bytes data[5]
received [2] bytes: OK
SUCCEED: load segment (2048) bytes data[6]
received [2] bytes: OK
SUCCEED: load segment (2048) bytes data[7]
received [2] bytes: OK
SUCCEED: load segment (2048) bytes data[8]
received [2] bytes: OK
SUCCEED: load segment (2048) bytes data[9]
received [2] bytes: OK
SUCCEED: load segment (2048) bytes data[10]
received [2] bytes: OK
SUCCEED: load segment (2048) bytes data[11]
received [2] bytes: OK
SUCCEED: load segment (2048) bytes data[12]
received [2] bytes: OK
SUCCEED: load segment (2048) bytes data[13]
received [2] bytes: OK
SUCCEED: load segment (400) bytes data[14]
SUCCEED: load segment data

received [2] bytes: OK
SUCCEED: check image

received [2] bytes: OK
SUCCEED: run image

hand shake with rate 230400
Received (2 bytes): OK
SUCCEED: hand shake

flashing *** ./fw2.bin ***
received [2] bytes: OK
SUCCEED: erase storage [0x00010000, 0x000162c0]

start to flash data [25280] bytes
received [2] bytes: OK
succeed: flash (8192) bytes data[0] to addr 0x00010000
received [2] bytes: OK
succeed: flash (8192) bytes data[1] to addr 0x00012000
received [2] bytes: OK
succeed: flash (8192) bytes data[2] to addr 0x00014000
received [2] bytes: OK
succeed: flash (704) bytes data[3] to addr 0x00016000
received [2] bytes: OK
SUCCEED: ack flash ok

received [36] bytes: OK
SUCCEED: SHA256 verificatin pass

flashing *** ./ro_params.dtb ***
received [2] bytes: OK
SUCCEED: erase storage [0x001f8000, 0x001f9627]

start to flash data [5671] bytes
received [2] bytes: OK
succeed: flash (5671) bytes data[0] to addr 0x001f8000
received [2] bytes: OK
SUCCEED: ack flash ok

received [36] bytes: OK
SUCCEED: SHA256 verificatin pass

flashing *** ./boot2image.bin ***
received [2] bytes: OK
SUCCEED: erase storage [0x00000000, 0x0000b990]

start to flash data [47504] bytes
received [2] bytes: OK
succeed: flash (8192) bytes data[0] to addr 0x00000000
received [2] bytes: OK
succeed: flash (8192) bytes data[1] to addr 0x00002000
received [2] bytes: OK
succeed: flash (8192) bytes data[2] to addr 0x00004000
received [2] bytes: OK
succeed: flash (8192) bytes data[3] to addr 0x00006000
received [2] bytes: OK
succeed: flash (8192) bytes data[4] to addr 0x00008000
received [2] bytes: OK
succeed: flash (6544) bytes data[5] to addr 0x0000a000
received [2] bytes: OK
SUCCEED: ack flash ok

received [36] bytes: OK
SUCCEED: SHA256 verificatin pass

flashing *** ./partition.bin@0xe000 ***
received [2] bytes: OK
SUCCEED: erase storage [0x0000e000, 0x0000e110]

start to flash data [272] bytes
received [2] bytes: OK
succeed: flash (272) bytes data[0] to addr 0x0000e000
received [2] bytes: OK
SUCCEED: ack flash ok

received [36] bytes: OK
SUCCEED: SHA256 verificatin pass

flashing *** ./partition.bin@0xf000 ***
received [2] bytes: OK
SUCCEED: erase storage [0x0000f000, 0x0000f110]

start to flash data [272] bytes
received [2] bytes: OK
succeed: flash (272) bytes data[0] to addr 0x0000f000
received [2] bytes: OK
SUCCEED: ack flash ok

received [36] bytes: OK
SUCCEED: SHA256 verificatin pass

SUCCEED: flash completed
```

Cheat sheet
-----------
./partition_gen -i ./partition_cfg_2M.toml -o partition.bin
./img_gen -i ./efuse_bootheader_cfg.conf -b ./blsp_boot2.bin  -o ./boot2image.bin -s 0x2000
./img_gen -i ./efuse_bootheader_cfg.conf -b ./sdk_app_helloworld.bin -o ./fw2.bin -s 0x1000
 dtc -I dts -O dtb bl_factory_params_IoTKitA_40M.dts -o ./ro_params.dtb
./flash --uart /dev/ttyUSB0 --rate 230400 --partition ./partition.bin@0xe000 ./partition.bin@0xf000   --fw ./fw2.bin --dtb ./ro_params.dtb --eflash ./eflash_loader_40m.bin --boot2 ./boot2image.bin

Open issues
-----------
1. Unable to reshake hands after flashing yet. I suspend eflash does not support this.

2. Occasionally we see this error in SHA check for boot2image due to one-bit flip. No
   root-caused yet. But it seems working fine. (The SHA impelmentation passed test.)
```
ERROR: SHA256 verificatin fail, but ignore now
sha256[0] = 0xc85e11a0 bl_resp.sha256[0] = 0xc85e11a0
sha256[1] = 0x083a27f7 bl_resp.sha256[1] = 0x083a27f7
sha256[2] = 0x2f6e1bab bl_resp.sha256[2] = 0x2f6e1bab
sha256[3] = 0x9b67cac9 bl_resp.sha256[3] = 0x9b67cac9
sha256[4] = 0x8d5d31ed bl_resp.sha256[4] = 0x8d5d31ed
sha256[5] = 0x90b64b78 bl_resp.sha256[5] = 0x90b64b78
sha256[6] = 0xe6e89e07 bl_resp.sha256[6] = 0xe6e89e07
sha256[7] = 0x0d782c58 bl_resp.sha256[7] = 0x0a782c58 X
```
