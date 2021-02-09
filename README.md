# ICOM_UXFF_RevT
Source and support for the UXFFront module, Rev-T (TX/RX) MCU software design.  UXFFront is a replacement
board for the "Front Unit" used in the ICOM IC-900/IC-901 UX radio modules.  This replacement board allows
the modules to be configured as stand alone radios.  The UXFFront will interface with the UX-19, UX-59,
UX-29, UX-39, UX-49, and UX-129 modules.

This repo contains the 8051 C source for the MCU TX/RX application (there are RX only, interposer, and
Front Unit replacement build options as well, but the code for these is maintained separately).
See the UXFFconfigurator repo for the configuration support application (WIN10 console application)
and support files.
