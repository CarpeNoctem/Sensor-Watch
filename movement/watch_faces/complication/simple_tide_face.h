/*
 * MIT License
 *
 * Copyright (c) 2023,2024 @CarpeNoctem, Dennis Faucher, with code from Joey Castillo
 * borrowed from other parts of the project - namely sunrise_sunset, and templated code.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef SIMPLE_TIDE_FACE_H_
#define SIMPLE_TIDE_FACE_H_

#include "movement.h"

/*
 * Simple Tide Clock
 *
 * There are no tide tables for this face. It is lightweight and simple, giving you a rough indication of the high and low tide times for effectively one location.
 * This is a tide clock that operates on the general tide cycle time of 12 hours 25 minutes. Note that other factors can cause some variation in tide times,
 * and some occasional adjustment may be needed.
 * 
 * To use this face, you will need to set at least one or the other of the next high or low tide time for the location of your choice.
 * If you set only one, then the face will assume 6 hrs 12.5 mins between high and low tide. If you know and set both upcoming tide times (high and low), 
 * then the face will carry that time difference going forward.
 * 
 */

typedef struct {
    uint8_t display_type; // See simple_tide_display_type_t below.
    uint8_t tide_event_index; 
    uint8_t active_time_setting_slot; // 0 if we're setting hours, 1 if we're setting minutes
    watch_date_time next_high_tide_time;
    watch_date_time next_low_tide_time;
    uint8_t alert_setting; // 0 = off, 1 = high tide, 2 = low tide, 3 = both
    uint8_t working_hours; // Scratch space for setting the next high or low tide time
    uint8_t working_minutes; // Scratch space for setting the next high or low tide time
} simple_tide_state_t;

typedef enum {
    TIDE_DISPLAY_SET_HIGH_TIDE = 0, // Set next high time
    TIDE_DISPLAY_SET_LOW_TIDE,      // Set next low tide time
    TIDE_DISPLAY_SET_ALERT,         // Set chime for tide events: Off, High, Low, Both
    TIDE_DISPLAY_TIDE_TIME,         // Show the time of the next tide event(s)
    TIDE_DISPLAY_TIDE_COUNTDOWN,    // Show time remaining until next tide event(s)
    TIDE_DISPLAY_TIDE_GRAPH         // Show a graph of the tide coming in or going out. (Thanks, Dennis Faucher!)
} simple_tide_display_type_t;

typedef enum {
    TIDE_CHIME_OFF = 0,
    TIDE_CHIME_HIGH,
    TIDE_CHIME_LOW,
    TIDE_CHIME_BOTH
} simple_dide_alert_setting_t;

void simple_tide_face_setup(movement_settings_t *settings, uint8_t watch_face_index, void ** context_ptr);
void simple_tide_face_activate(movement_settings_t *settings, void *context);
bool simple_tide_face_loop(movement_event_t event, movement_settings_t *settings, void *context);
void simple_tide_face_resign(movement_settings_t *settings, void *context);

#define simple_tide_face ((const watch_face_t){ \
    simple_tide_face_setup, \
    simple_tide_face_activate, \
    simple_tide_face_loop, \
    simple_tide_face_resign, \
    NULL, \
})

#endif // SIMPLE_TIDE_FACE_H_

