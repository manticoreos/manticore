/*
   The Multiboot header is a special block in the OS image that is used
   by boot loaders to load the OS image.

   This header follows the Multiboot 1 format, which is is specified in
   the "Multiboot Specification":

   https://www.gnu.org/software/grub/manual/multiboot/multiboot.html
 */

#define MULTIBOOT1_HEADER_MAGIC		0x1BADB002
#define MULTIBOOT_HEADER_FLAGS		0x00
#define MULTIBOOT_HEADER_CHECKSUM	-(MULTIBOOT1_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

.section .multiboot_header, "a"

.align 4
.long MULTIBOOT1_HEADER_MAGIC
.long MULTIBOOT_HEADER_FLAGS
.long MULTIBOOT_HEADER_CHECKSUM
