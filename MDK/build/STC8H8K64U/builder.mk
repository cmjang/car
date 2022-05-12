CFLAGS := PRINT(.\build\STC8H8K64U\${c}.lst) OMF2 TABS(4) CODE NOCOND SYMBOLS OPTIMIZE(8,SPEED) LARGE ROM(LARGE) INCDIR(D:/keil/C51/INC/Atmel;.\..\..\Libraries\libraries;.\..\..\Libraries\seekfree_libraries;.\..\..\Libraries\seekfree_peripheral;.\..\USER\inc;.\..\USER\src;.\..\CODE;D:\keil\C51\INC) DEFINE(__UVISION_VERSION='526')
CXXFLAGS := PRINT(.\build\STC8H8K64U\${cpp}.lst) OMF2 TABS(4) CODE NOCOND SYMBOLS OPTIMIZE(8,SPEED) LARGE ROM(LARGE) INCDIR(D:/keil/C51/INC/Atmel;.\..\..\Libraries\libraries;.\..\..\Libraries\seekfree_libraries;.\..\..\Libraries\seekfree_peripheral;.\..\USER\inc;.\..\USER\src;.\..\CODE;D:\keil\C51\INC) DEFINE(__UVISION_VERSION='526')
ASMFLAGS := PRINT(.\build\STC8H8K64U\${asm}.lst) SET(LARGE) INCDIR(D:/keil/C51/INC/Atmel;.\..\..\Libraries\libraries;.\..\..\Libraries\seekfree_libraries;.\..\..\Libraries\seekfree_peripheral;.\..\USER\inc;.\..\USER\src;.\..\CODE;D:\keil\C51\INC)
LDFLAGS :=  PRINT(.\build\STC8H8K64U\MDK.map) REMOVEUNUSED
LDLIBS := 
