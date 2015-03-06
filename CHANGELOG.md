#CHANGES

## 1.0.2 - 06/03/2015
- Fixed ResetISR position from Relocator + 0x4000 to Relocator + 0x4001 because
    of the thum instructions (file startup.asm).
    
- Added fail report when somenthing goes wrong.

- Remove the TARGET_IS_CC3200 symbol (using ROM driverlib makes the bootloader
  get stuck).

## 1.0.0 - 01/29/2015
- Initial Commit.
