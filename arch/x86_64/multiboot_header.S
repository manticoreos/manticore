/*
   The Multiboot header is a special block in the OS image that is used
   by boot loaders to load the OS image.

   This header follows the Multiboot 2 format, which is is specified in
   the "The Multiboot Specification version 1.6":

   http://nongnu.askapache.com/grub/phcoder/multiboot.pdf
 */

.section .multiboot_header, "a"

#define MULTIBOOT2_MAGIC	0xe85250d6
#define MULTIBOOT_ARCH_I386	0
#define MULTIBOOT_HEADER_LEN	(multiboot2_header_end - multiboot2_header_start)
#define MULTIBOOT_CHECKSUM	-(MULTIBOOT2_MAGIC + MULTIBOOT_ARCH_I386 + MULTIBOOT_HEADER_LEN)

#define MULTIBOOT_TAG_END 0
#define MULTIBOOT_TAG_INFO_REQUEST 1
#define MULTIBOOT_TAG_INFO_REQUEST_LEN (multiboot2_info_request_end - multiboot2_info_request_start)

#define MULTIBOOT_TAG_FRAMEBUFFER 5

#define MULTIBOOT_TAG_MEMORY_MAP 6

.align 8
multiboot2_header_start:
.long MULTIBOOT2_MAGIC
.long MULTIBOOT_ARCH_I386
.long MULTIBOOT_HEADER_LEN
.long MULTIBOOT_CHECKSUM

.align 8
multiboot2_info_request_start:
.short MULTIBOOT_TAG_INFO_REQUEST
.short 0
.long MULTIBOOT_TAG_INFO_REQUEST_LEN
.long MULTIBOOT_TAG_MEMORY_MAP
multiboot2_info_request_end:

.align 8
.short MULTIBOOT_TAG_FRAMEBUFFER
.short 0
.long 20
.long 0
.long 0
.long 0

.align 8
.short MULTIBOOT_TAG_END
.short 0
.long 8
multiboot2_header_end:
