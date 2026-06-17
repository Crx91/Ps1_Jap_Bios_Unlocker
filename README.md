# Ps1_Jap_Bios_Unlocker
Universal chip add-on unlocker for **all** JAP ps1 BIOSes protection.

This mod patches ps1 JAP bios protection allowing playing PAL and USA region games.
My method uses a different approach instead of the one made by postal v8 PsNee (and kalymos upgrades).

Postal fix aims to patch a few bites at "ns" precision time at console boot, but have different disadvantages:
- Each bios needs a different time patching and approach;
- On some BIOSes, the fix corrupt the bios "CD player" or "MC manager", causing console crash if you try to enter in those menus;
- Wire length are very strict (due the timing involved), a few mm longer wire (propagation delay) and the fix will fail;
- Combinations of different Atmega328p and crystals can cause ISR jitter and speed patching issues.
  BTW kudos to kalymnos who is working hard to make the code more stable, you're great!

My method instead, patches on the fly the bios line only when the game is recognised, so we have:
- Universal patching method for all BIOSes;
- Wire length aren't an issue any more;
- It uses internal calibrated RC Osc of Ch32v003 and it isn't strictly time dependent like postal one!

I'm just a hobbyist programmer, so please forgive any coding mistakes, syntax errors, or other issues.
Code is written using greats ch32fun libs!

## Features

- Fully compatible with ALL Japanese BIOSes versions!
- No Bios menu crash!
- Can be associated with ALL type of mods (PsNee, mayumi, etc)
- Optional Led patching status output on chip PIN 1 (You need a 1K resistor).
- No atmega328p+16mhz crystal or other Arduino UNO like "big" board, simply a 8-pin ch32v003j4m6 chip to solder that can be easily fitted in your console!
- Bin files should be compatible even on other ch32v003 package (SOP-16, TSSOP-20 and QFN-20) and valuation boards, provided that you remove the external osc!

## Supported Playstations
- All PlayStation models with Japanese Bios.
- For SCPH-102 models, you can use my [OneNee_ch32v003](https://github.com/carmax91/OneNee_Ch32v003) port which "unlocks" all regions backup.
- For others USA and PAL models you can use my [PsNee_ch32v003](https://github.com/Crx91/PsNee-CH32V003/tree/main) port.
- Or you can use the latest [PsNee_Aio](https://github.com/Crx91/PsNee_Aio) wich have PsNee+Jap_BiosUnlocker integrated in one chip!


## Prerequisites
- [WCH-LinkE](https://github.com/Crx91/Ps1_Jap_Bios_Unlocker/blob/main/Imgs/LinU2.png) programmer (pay attention to the E).
- WCH LinkUtility software.

## HowTo
- Download this repository.
- Install the WCH LinkUtility software and the relative drivers (all the the needed software/files are zipped in the "tool" directory).
- Connect the LinkE programmer and check if is in RISCV mode (Blue LED should be always off when idle).
- If not, disconnect and reconnect the programmer holding down the "Mode-S" button.
- Open LinkUtility and check if the tool sees the programmer.
- Set the core (RISC-V) and the series (CH32V003).
- Click on the folder icon (or press ALT+F1).
- Select the BIN compiled files (you can find the bin files in the [BIN](https://github.com/Crx91/Ps1_Jap_Bios_Unlocker/tree/main/BIN) folder).
- Connect the chip to the programmer.
- Program the chip via Target -> Program (or press F10).
![done](https://github.com/Crx91/Ps1_Jap_Bios_Unlocker/blob/main/Imgs/LinU2.png)
- Now you have your Jap_bios_Unlocker on WCH CH32V003 chip!
- For the installation, follow the wiring diagrams in the [Install](https://github.com/Crx91/Ps1_Jap_Bios_Unlocker/tree/main/Install) folder based on your console motherboard.

## Some info about the Led status and code
Status Led otuput meaning:
- Off: Fix OFF or disabled.
- ON: Fix triggered, patching.
- Blinking: Waiting for trigger. (After 20 sec. if is not triggered, the Fix will be deactivated automatically (turns OFF), reset the console to reneable (turn ON) again)

## Thanks to:
ramapcsx2, kalymos, SpenceKonde, oldcrow, mayumi, arduino community, ch32fun community, Infrid and lots of people that can't remeber now.
