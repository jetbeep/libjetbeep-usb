#ifndef JETBEEP_BARCODE_TYPE__H
#define JETBEEP_BARCODE_TYPE__H

#include <string>

namespace JetBeep {
  enum class BarcodeType : int {
    unknown = 0,
    upca = 1,
    upce = 2,
    eanJan8 = 3,
    eanJan13 = 4,
    tf = 5,
    itf = 6,
    codabar = 7,
    code39 = 8,
    code93 = 9,
    code128 = 10,
    upcas = 11,
    upces = 12,
    upcd1 = 13,
    upcd2 = 14,
    upcd3 = 15,
    upcd4 = 16,
    upcd5 = 17,
    ean8S = 18,
    ean13S = 19,
    ean128 = 20,
    ocra = 21,
    ocrb = 22,
    code128Parsed = 23,
    gs1DataBar = 24,
    rss14 = 25,
    gs1DataBarExpanded = 26,
    rssExpanded = 27,
    gs1DataBarStackedOmnidirectional = 28,
    gs1DataBarExpandedStacked = 29,
    cca = 30,
    ccb = 31,
    ccc = 32,
    pdf417 = 33,
    maxicode = 34,
    dataMatrix = 35,
    qrCode = 36,
    microQrCode = 37,
    aztec = 38,
    microPDF417 = 39,
    other = 40
  };

  struct Barcode {
    std::string value;
    BarcodeType type;
  };
} // namespace JetBeep

#endif
