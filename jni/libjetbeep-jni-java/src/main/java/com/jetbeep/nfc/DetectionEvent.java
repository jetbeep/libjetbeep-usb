package com.jetbeep.nfc;

public class DetectionEvent {
  public enum Event {
    DETECTED,
    REMOVED;
  }

  public DetectionEvent.Event event;

  /* Available only for DETECTED event */
  public CardInfo cardInfo = null;

  DetectionEvent(DetectionEvent.Event event, CardInfo cardInfo) {
    this.event = event;
    this.cardInfo = cardInfo;
  }

  DetectionEvent(DetectionEvent.Event event) {
    this.event = event;
    this.cardInfo = null;
  }
}