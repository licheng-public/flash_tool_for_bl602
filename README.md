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

Still work in progress....
