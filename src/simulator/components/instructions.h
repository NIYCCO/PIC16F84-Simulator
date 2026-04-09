#pragma once

enum Instruction {
    // Byte-orientierte Datei-Register-Operationen
    ADDWF  = 0x0700, // Addiere W und f 
    ANDWF  = 0x0500, // UND-Verknüpfung W mit f 
    CLRF   = 0x0180, // Lösche f (Opcode: 00 0001 1fff ffff) [cite: 477, 483]
    CLRW   = 0x0100, // Lösche W (Opcode: 00 0001 0xxx xxxx) [cite: 477, 483]
    COMF   = 0x0900, // Komplementiere f 
    DECF   = 0x0300, // Dekrementiere f 
    DECFSZ = 0x0B00, // Dekrementiere f, überspringe wenn 0 
    INCF   = 0x0A00, // Inkrementiere f 
    INCFSZ = 0x0F00, // Inkrementiere f, überspringe wenn 0 
    IORWF  = 0x0400, // Inklusives ODER W mit f 
    MOVF   = 0x0800, // Bewege f 
    MOVWF  = 0x0080, // Bewege W nach f (Opcode: 00 0000 1fff ffff) [cite: 477, 520]
    NOP    = 0x0000, // Keine Operation 
    RLF    = 0x0D00, // Rotiere f nach links durch Carry 
    RRF    = 0x0C00, // Rotiere f nach rechts durch Carry 
    SUBWF  = 0x0200, // Subtrahiere W von f 
    SWAPF  = 0x0E00, // Vertausche Nibbles in f 
    XORWF  = 0x0600, // Exklusives ODER W mit f 

    // Bit-orientierte Datei-Register-Operationen
    BCF    = 0x1000, // Bit Löschen in f (Opcode: 01 00bb bfff ffff) [cite: 477, 478]
    BSF    = 0x1400, // Bit Setzen in f (Opcode: 01 01bb bfff ffff) [cite: 477, 481]
    BTFSC  = 0x1800, // Bit-Test in f, überspringe wenn gelöscht 
    BTFSS  = 0x1C00, // Bit-Test in f, überspringe wenn gesetzt 

    // Literal- und Kontroll-Operationen
    ADDLW  = 0x3E00, // Addiere Literal und W (Opcode: 11 111x kkkk kkkk) 
    ANDLW  = 0x3900, // UND-Verknüpfung Literal mit W 
    CALL   = 0x2000, // Unterprogramm aufrufen (Opcode: 10 0kkk kkkk kkkk) 
    CLRWDT = 0x0064, // Watchdog-Timer löschen (Opcode: 00 0000 0110 0100) [cite: 477, 486]
    GOTO   = 0x2800, // Gehe zu Adresse (Opcode: 10 1kkk kkkk kkkk) [cite: 477, 500]
    IORLW  = 0x3800, // Inklusives ODER Literal mit W 
    MOVLW  = 0x3000, // Bewege Literal nach W (Opcode: 11 00xx kkkk kkkk) [cite: 477, 515]
    RETFIE = 0x0009, // Rückkehr vom Interrupt (Opcode: 00 0000 0000 1001) [cite: 477, 526]
    RETLW  = 0x3400, // Rückkehr mit Literal in W (Opcode: 11 01xx kkkk kkkk) [cite: 477, 534]
    RETURN = 0x0008, // Rückkehr vom Unterprogramm (Opcode: 00 0000 0000 1000) [cite: 477, 534]
    SLEEP  = 0x0063, // Wechsel in Standby-Modus (Opcode: 00 0000 0110 0011) [cite: 477, 541]
    SUBLW  = 0x3C00, // Subtrahiere W von Literal 
    XORLW  = 0x3A00  // Exklusives ODER Literal mit W 
};