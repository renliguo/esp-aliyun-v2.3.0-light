/* user1.bin @ 0x1000, user2.bin @ 0x10000 */
/* Flash Map (1024KB + 1024KB), support 2MB/4MB SPI Flash */
/* |..|........................|.....|.....|..|........................|.....|....|                        */
/* ^  ^                        ^     ^     ^  ^                        ^     ^                             */
/* |_boot start(0x0000)        |     |     |_pad start(0x100000)       |     |                             */
/*    |_user1 start(0x1000)    |_user1 end    |_user2 start(0x101000)  |_user2 end                         */
/*                                   |_system param symmetric area(0xfb000)  |_system param area(0x1fb000) */
/* NOTICE: */
/* 1. You can change irom0 len, but MUST make sure user1 end not overlap system param symmetric area. */
/* 2. Space between user1 end and pad start can be used as user param area.                           */
/* 3. Space between user2 end and system param area can be used as user param area.                   */
/* 4. Don't change any other seg.                                                                     */
/* 5. user1.bin and user2.bin are same in this mode, so upgrade only need one of them.                */
/*
 *
 * Automatically generated file; DO NOT EDIT.
 * Espressif IoT Development Framework Configuration
 *
 */
MEMORY
{
  dport0_0_seg : org = 0x3FF00000, len = 0x10
  /* All .data/.bss/heap are in this segment. */
  dram0_0_seg : org = 0x3FFE8000, len = 0x18000
  /* Functions which are critical should be put in this segment. */
  iram1_0_seg : org = 0x40100000, len = 0xC000
  /* It is actually mapped to flash. */
  irom0_0_seg : org = 0x40200010 + 0x10000, len = 0xf0000 - 0x10 - 0x10000
  /* RTC memory, persists over deep sleep. */
  rtc_seg : org = 0x60001200, len = 0x200
}
