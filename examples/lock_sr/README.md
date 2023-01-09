# SR lock Example (currently only for XMC)

This patch will lock the SR permanently or until power-cycle. Please read the comments in the header, and use it under guidance of Espressif.

CAUTION: This patch is not compatible with the HFM mode (> 80Mhz). Using HFM mode needs to modify the DC bits in SR3 when required. Furthermore, locking the SR permanently may forbid using the HFM mode forever. Only use the patch in this example when you are sure this will not run above 80MHz.

Supported on:

Targets:
- ESP32
- ESP32-S2
- ESP32-S3
- ESP32-C3

Branches
- v5.0 and later
- v4.4.1+ (release/v4.4)
- v4.3.3+ (release/v4.3)
- v4.2.4+ (release/v4.2)
- v4.1.4+ (release/v4.1)

(More details to be updated)
