# HWT905 serial acceptance
CH340 /dev/ttyUSB0; final verified 230400 baud, RRATE 0x09, RSW 0x020E.
- 04-ch341: kernel module build and USB binding.
- 05-rx: baud detection and raw four-type data capture.
- 06-register-read: checksum-valid device reply, 9.04 ms.
- 07-rate: 99.89 -> 50.47 -> 100.43 Hz.
- 08-mask: quaternion 151 -> 0 -> 150 frames.
- 09-baud: 230400 -> 115200 -> 230400; register and stream verified.
Calibration and power-cycle persistence were not tested. No flash save was issued.
