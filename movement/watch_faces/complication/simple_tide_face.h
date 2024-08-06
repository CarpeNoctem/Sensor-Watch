/*
 * MIT License
 *
 * Copyright (c) 2023 CarpeNoctem
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
    // Anything you need to keep track of, put it here!
    uint8_t unused;
} simple_tide_state_t;

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

