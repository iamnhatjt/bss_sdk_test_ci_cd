package com.selex.bssStation.service

/**
 * An event class that encapsulates a message string.
 *
 * @property message the message string.
 */
data class MessageEvent(val message: String, val data: String? = null)

enum class AppEvent {
    STATUS_BSS,
    NUMBER_PIN,
    DOOR_CHANGE
}
