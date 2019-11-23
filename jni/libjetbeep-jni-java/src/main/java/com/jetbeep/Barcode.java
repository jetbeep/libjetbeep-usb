package com.jetbeep;

public class Barcode {
  public enum Type {
    unknown,
    upca,
    upce,
    eanJan8,
    eanJan13,
    tf,
    itf,
    codabar,
    code39,
    code93,
    code128,
    upcas,
    upces,
    upcd1,
    upcd2,
    upcd3,
    upcd4,
    upcd5,
    ean8S,
    ean13S,
    ean128,
    ocra,
    ocrb,
    code128Parsed,
    gs1DataBar,
    rss14,
    gs1DataBarExpanded,
    rssExpanded,
    gs1DataBarStackedOmnidirectional,
    gs1DataBarExpandedStacked,
    cca,
    ccb,
    ccc,
    pdf417,
    maxicode,
    dataMatrix,
    qrCode,
    microQrCode,
    aztec,
    microPDF417,
    other;
    
    public static Type fromInt(int value) {
      switch (value) {
        case 0:
          return Type.unknown;
        case 1:
          return Type.upca;
        case 2:
          return Type.upce;
        case 3:
          return Type.eanJan8;
        case 4:
          return Type.eanJan13;
        case 5:
          return Type.tf;
        case 6:
          return Type.itf;
        case 7:
          return Type.codabar;
        case 8:
          return Type.code39;
        case 9:
          return Type.code93;
        case 10:
          return Type.code128;
        case 11:
          return Type.upcas;
        case 12:
          return Type.upces;
        case 13:
          return Type.upcd1;
        case 14:
          return Type.upcd2;
        case 16:
          return Type.upcd4;
        case 17:
          return Type.upcd5;
        case 18:
          return Type.ean8S;
        case 19:
          return Type.ean13S;
        case 20:
          return Type.ean128;
        case 21:
          return Type.ocra;
        case 22:
          return Type.ocrb;
        case 23:
          return Type.code128Parsed;
        case 24:
          return Type.gs1DataBar;
        case 25:
          return Type.rss14;
        case 26:
          return Type.gs1DataBarExpanded;
        case 27:
          return Type.rssExpanded;
        case 28:
          return Type.gs1DataBarStackedOmnidirectional;    
        case 29:
          return Type.gs1DataBarExpandedStacked;
        case 30:
          return Type.cca;
        case 31:
          return Type.ccb;
        case 32:
          return Type.ccc;
        case 33:
          return Type.pdf417;
        case 34:
          return Type.maxicode;
        case 35:
          return Type.dataMatrix;        
        case 36:
          return Type.qrCode;
        case 37:
          return Type.microQrCode;
        case 38:
          return Type.aztec;
        case 39:
          return Type.microPDF417;
        case 40:
          return Type.other;
        default:
          return Type.unknown;
      }
    }
  }

  public String value;
  public Barcode.Type type;

  Barcode(String value, Barcode.Type type) {
    this.value = value;
    this.type = type;
  }
}