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

#include <stdlib.h>
#include <string.h>
#include "simple_tide_face.h"
#include "watch_utility.h"

#if __EMSCRIPTEN__
#include <emscripten.h>
#endif

// To-do: Add settings page & functionality for tide events (off, hi, lo, both)

void simple_tide_face_setup(movement_settings_t *settings, uint8_t watch_face_index, void ** context_ptr) {
    (void) settings;
    (void) watch_face_index;
    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(simple_tide_state_t));
        memset(*context_ptr, 0, sizeof(simple_tide_state_t));
        // Do any one-time tasks in here; the inside of this conditional happens only at boot.
    }
    // Do any pin or peripheral setup here; this will be called whenever the watch wakes from deep sleep.
}

// Don't recalculate tide countdowns more than once per minute
// To-do: May or may not need this capability for this face.
// static void _simple_tide_set_expiration(simple_tide_state_t *state, watch_date_time next_rise_set) {
//    uint32_t timestamp = watch_utility_date_time_to_unix_time(next_rise_set, 0);
//    state->rise_set_expires = watch_utility_date_time_from_unix_time(timestamp + 60, 0);
//}

static watch_duration_t _simple_tide_get_duration_until_high(movement_settings_t *settings, simple_tide_state_t *state) {
    if (state->next_high_tide_time.reg == NULL) {
        return watch_utility_seconds_to_duration(0);
    }
    int remaining_secs_to_high = watch_utility_date_time_to_unix_time(state->next_high_tide_time, settings->bit.time_zone) - watch_utility_date_time_to_unix_time(watch_rtc_get_date_time(), settings->bit.time_zone); 
    watch_duration_t retval = watch_utility_seconds_to_duration(remaining_secs_to_high);
    printf("Remaining duration to high tide: %d:%02d\n", retval.hours,retval.minutes); //To-do: remove debug
    // return watch_utility_seconds_to_duration(remaining_secs_to_high);
    return retval;
}

static watch_duration_t _simple_tide_get_duration_until_low(movement_settings_t *settings, simple_tide_state_t *state) {
    if (state->next_low_tide_time.reg == NULL) {
        return watch_utility_seconds_to_duration(0);
    }
    int remaining_secs_to_low = watch_utility_date_time_to_unix_time(state->next_low_tide_time, settings->bit.time_zone) - watch_utility_date_time_to_unix_time(watch_rtc_get_date_time(), settings->bit.time_zone); 
    watch_duration_t retval = watch_utility_seconds_to_duration(remaining_secs_to_low);
    printf("Remaining duration to low tide: %d:%02d\n", retval.hours, retval.minutes); //To-do: remove debug
    // return watch_utility_seconds_to_duration(remaining_secs_to_low);
    return retval;
}

// To-do: Check this against python test code to make it smarter, and make sure to clean up function references to those we've already defined.
// Predict new tide times
static void _simpletide_check_and_update_next_tide_times(movement_settings_t *settings, simple_tide_state_t *state) {
    int now_unix_time = watch_utility_date_time_to_unix_time(watch_rtc_get_date_time(), settings->bit.time_zone);
    int next_high_unix_time = watch_utility_date_time_to_unix_time(state->next_high_tide_time, settings->bit.time_zone);
    int next_low_unix_time = watch_utility_date_time_to_unix_time(state->next_low_tide_time, settings->bit.time_zone);

    // If the next high tide time is in the past, calculate the next high tide time based on the current time and the known time difference between high and low tide.
    while (next_high_unix_time < now_unix_time) {
        next_high_unix_time += 12 * 3600 + 25 * 60; // 12 hours 25 minutes
        state->next_high_tide_time = watch_utility_date_time_from_unix_time(next_high_unix_time, settings->bit.time_zone);
    }

    // If the next low tide time is in the past, calculate the next low tide time based on the current time and the known time difference between high and low tide.
    while (next_low_unix_time < now_unix_time) {
        next_low_unix_time += 12 * 3600 + 25 * 60; // 12 hours 25 minutes
        state->next_low_tide_time = watch_utility_date_time_from_unix_time(next_low_unix_time, settings->bit.time_zone);
    }
}

static void _simple_tide_face_update(movement_settings_t *settings, simple_tide_state_t *state) {
    char buf[12];
    // double rise, set, minutes, seconds;
    // bool show_next_match = false;

    printf("Display is: %d\n", state->display_type); //To-do: remove debug
    printf("Tide event index is: %d\n", state->tide_event_index); //To-do: remove debug

    if (state->next_high_tide_time.reg == NULL) {
    // if (false && state->next_high_tide_time.reg == NULL) {
        printf("Next high tide reg: %d\n", state->next_high_tide_time.reg);
        watch_display_string("TI  no Set", 0);
        state->display_type = TIDE_DISPLAY_TIDE_TIME;
        return;
    }

    _simpletide_check_and_update_next_tide_times(settings, state);

    // To-do: split out a function here to check whether either of the next tide times are in the past,
    // and if so, calculate the next tide times based on the current time and the known time difference between high and low tide.
    // watch_date_time next_high = watch_rtc_get_date_time();
    // watch_date_time next_high = state->next_high_tide_time;
    // watch_date_time next_low = state->next_low_tide_time;
    // This is all clunky test code that needs to get cleaned up.
    // int remaining_secs_to_high = watch_utility_date_time_to_unix_time(state->next_high_tide_time,settings->bit.time_zone) - watch_utility_date_time_to_unix_time(watch_rtc_get_date_time(),settings->bit.time_zone); 
    // watch_duration_t duration_until_high = watch_utility_seconds_to_duration(remaining_secs_to_high);

    // Show the bell icon if we've enabled the chime for low or/and high tide.
    if (state->alert_setting != TIDE_CHIME_OFF) {
        watch_set_indicator(WATCH_INDICATOR_BELL);
    } else {
        watch_clear_indicator(WATCH_INDICATOR_BELL);
    }

    watch_set_colon();
    switch(state->display_type) {
        case TIDE_DISPLAY_TIDE_TIME:
            if (state->tide_event_index == 0) {
                sprintf( buf, "TI  %2d%02dHI", state->next_high_tide_time.unit.hour, state->next_high_tide_time.unit.minute);
            } else {
                sprintf( buf, "TI  %2d%02dLO", state->next_low_tide_time.unit.hour, state->next_low_tide_time.unit.minute);
            }
            watch_display_string(buf,0);
            break;
        case TIDE_DISPLAY_TIDE_COUNTDOWN:
            if (state->tide_event_index == 0) {
                watch_duration_t duration_until_high = _simple_tide_get_duration_until_high(settings, state);
                sprintf( buf, "CD  %2d%02dHI", duration_until_high.hours, duration_until_high.minutes);
            } else {
                watch_duration_t duration_until_low = _simple_tide_get_duration_until_low(settings, state);
                sprintf( buf, "CD  %2d%02dLO", duration_until_low.hours, duration_until_low.minutes);
            }
            watch_display_string(buf,0);
            break;
        case TIDE_DISPLAY_TIDE_GRAPH:
            watch_clear_colon();
            watch_display_string("TI1n (((((",0);
            // watch_display_string("TIOt (((((",0);
            break;
    }

    // watch_date_time date_time = watch_rtc_get_date_time(); // the current local date / time
    // watch_date_time utc_now = watch_utility_date_time_convert_zone(date_time, movement_timezone_offsets[settings->bit.time_zone] * 60, 0); // the current date / time in UTC
    // watch_date_time scratch_time; // scratchpad, contains different values at different times
    // scratch_time.reg = utc_now.reg;
}

// static void _simple_tide_face_update_settings_display(movement_event_t event, simple_tide_state_t *state) {
static void _simple_tide_face_update_settings_display(movement_settings_t *settings, simple_tide_state_t *state) {
    // printf("Display type: %d\n", state->display_type); //To-do: remove debug
    // printf("Active digit: %d\n", state->active_time_setting_slot); //To-do: remove debug
    char buf[12];

    watch_clear_display();

    // To-do: Add some logic to get the remaining time to the next high and low tides.
    //        We'll want a dedicated function for this.
    // watch_duration_t duration_until_high = _simple_tide_get_duration_until_high(state);
    // watch_duration_t duration_until_low = _simple_tide_get_duration_until_low(state);
    
    switch (state->display_type) {
        case TIDE_DISPLAY_SET_HIGH_TIDE:
            printf("in high tide event case.\n"); //To-do: remove debug
            printf("high tide reg: %d\n", state->next_high_tide_time.reg); //To-do: remove debug
            if (state->next_high_tide_time.reg == NULL) {
                state->next_high_tide_time = watch_rtc_get_date_time();
                printf("we just grabbed this time from rtc: %2d%02d\n", state->next_high_tide_time.unit.hour, state->next_high_tide_time.unit.minute); //To-do: remove debug
            }
            watch_set_colon();
            // watch_duration_t duration_until_high = _simple_tide_get_duration_until_high(settings, state);
            //sprintf(buf, "HI  %2d%02d", duration_until_high.hours, duration_until_high.minutes);
            sprintf(buf, "HI  %2d%02d", state->next_high_tide_time.unit.hour, state->next_high_tide_time.unit.minute);
            watch_display_string(buf,0);
            break;
        case TIDE_DISPLAY_SET_LOW_TIDE:
            printf("in low tide event case.\n"); //To-do: remove debug
            printf("low tide reg: %d\n", state->next_low_tide_time.reg); //To-do: remove debug
            if (state->next_low_tide_time.reg == NULL) {state->next_low_tide_time = watch_rtc_get_date_time(); }
            watch_set_colon();
            // watch_duration_t duration_until_low = _simple_tide_get_duration_until_low(settings, state);
            // sprintf(buf, "LO  %2d%02d", duration_until_low.hours, duration_until_low.minutes);
            sprintf(buf, "LO  %2d%02d", state->next_low_tide_time.unit.hour, state->next_low_tide_time.unit.minute);
            watch_display_string(buf,0);
            break;
        case TIDE_DISPLAY_SET_ALERT:
            printf("in alert setting case.\n"); //To-do: remove debug
            watch_clear_colon();
            // Show the bell icon if we've enabled the chime for low or/and high tide.
            if (state->alert_setting != TIDE_CHIME_OFF) {
                watch_set_indicator(WATCH_INDICATOR_BELL);
            } else {
                watch_clear_indicator(WATCH_INDICATOR_BELL);
            }
            switch (state->alert_setting) {
                case 0:
                    watch_display_string("CH    OFF",0);
                    break;
                case 1:
                    watch_display_string("CH   HIGH", 0);
                    break;
                case 2:
                    watch_display_string("CH   LO", 0);
                    break;
                case 3:
                    watch_display_string("CH   BOTH", 0);
                    break;
            }
            break;
    }
    // To-do have a look at using this when setting the tide time.
    // if (event.subsecond % 2) {
    //     buf[state->active_time_setting_slot + 4] = ' ';
    // }
    // watch_display_string(buf, 0);
}

void simple_tide_face_activate(movement_settings_t *settings, void *context) {
    (void) settings;
    simple_tide_state_t *state = (simple_tide_state_t *)context;

    // Handle any tasks related to your watch face coming on screen.

    //watch_date_time time_now = watch_rtc_get_date_time();
    // scratch_time.unit.hour += 3;
    // scratch_time.unit.minute += 30;
    // state->next_high_tide_time = time_now; // To-do: remove this mockup
    // state->next_low_tide_time = time_now; // To-do: remove this mockup

    // See https://github.com/CarpeNoctem/Sensor-Watch/blob/french_revolutionary_face/movement/watch_faces/clock/simple_clock_face.c#L54
    if (watch_tick_animation_is_running()) watch_stop_tick_animation();

}

bool simple_tide_face_loop(movement_event_t event, movement_settings_t *settings, void *context) {
    simple_tide_state_t *state = (simple_tide_state_t *)context;

    watch_date_time now = watch_rtc_get_date_time();

    switch (event.event_type) {
        // See https://github.com/CarpeNoctem/Sensor-Watch/blob/french_revolutionary_face/movement/watch_faces/clock/simple_clock_face.c#L109

        case EVENT_ACTIVATE:
            // Show your initial UI here.
            watch_clear_display();
            watch_display_string("TIDE", 5);
            // To-do: sleep 1 sec
            state->display_type = TIDE_DISPLAY_TIDE_TIME;
            state->tide_event_index = 0;
            _simple_tide_face_update(settings, state);
            break;
        case EVENT_TICK:
            // If needed, update your display here.
            break;
        case EVENT_LIGHT_BUTTON_UP:
            // To-do: swap Light button and Alarm button functionality when setting tides
            // You can use the Light button for your own purposes. Note that by default, Movement will also
            // illuminate the LED in response to EVENT_LIGHT_BUTTON_DOWN; to suppress that behavior, add an
            // empty case for EVENT_LIGHT_BUTTON_DOWN.
            // Unless we're in a settings page, advance the display type
            if (state->display_type > TIDE_DISPLAY_SET_ALERT) { 
                state->display_type += 1;
                if (state->display_type > TIDE_DISPLAY_TIDE_GRAPH) { state->display_type = TIDE_DISPLAY_TIDE_TIME; }
                printf("Display type: %d\n", state->display_type); //To-do: remove debug
                _simple_tide_face_update(settings, state);
                break;
            } else if (state->display_type == TIDE_DISPLAY_SET_ALERT) {
                state->alert_setting += 1;
                if (state->alert_setting > 3) { state->alert_setting = 0; }
                _simple_tide_face_update_settings_display(settings, state);
                break;
            } else {
                // To-do: ?
                if (state->display_type == TIDE_DISPLAY_SET_HIGH_TIDE) {
                    if (state->active_time_setting_slot == 0) {
                        printf("Light pushed. Updating hour from %d to %d\n", state->next_high_tide_time.unit.hour, state->next_high_tide_time.unit.hour+1);
                        state->next_high_tide_time.unit.hour = (state->next_high_tide_time.unit.hour + 1) % 24;
                    } else {
                        state->next_high_tide_time.unit.minute = (state->next_high_tide_time.unit.minute + 1) % 60;
                    }
                } else {
                    if (state->active_time_setting_slot == 0) {
                        state->next_low_tide_time.unit.hour = (state->next_low_tide_time.unit.hour + 1) % 24;
                    } else {
                        state->next_low_tide_time.unit.minute = (state->next_low_tide_time.unit.minute + 1) % 60;
                    }
                }
                _simple_tide_face_update_settings_display(settings, state);
                break;
            }
            break;
        case EVENT_ALARM_BUTTON_UP:
            switch (state->display_type) {
                case TIDE_DISPLAY_SET_HIGH_TIDE:
                case TIDE_DISPLAY_SET_LOW_TIDE:
                    // Advance to the next slot in setting the tide times
                    // Then advance to the next setting
                    state->active_time_setting_slot += 1;
                    if (state->active_time_setting_slot > 1) {
                        state->active_time_setting_slot = 0;
                        state->display_type += 1;
                    }
                    // If the next tide times have already passed today, we assume the user means that time tomorrow.
                    // The 300 seconds in this condition is to try to avoid someone setting the next tide time for now, but then having that end up in the past and triggering it to roll over to tomorrow.
                    if (watch_utility_date_time_to_unix_time(state->next_high_tide_time, settings->bit.time_zone) < watch_utility_date_time_to_unix_time(now, settings->bit.time_zone) - 300) {
                        state->next_high_tide_time.unit.day = now.unit.day + 1;
                    } else {
                        state->next_high_tide_time.unit.day = now.unit.day; // Help protect against system date changes / give user a way reset these
                    }
                    if (watch_utility_date_time_to_unix_time(state->next_low_tide_time, settings->bit.time_zone) < watch_utility_date_time_to_unix_time(now, settings->bit.time_zone) - 300) {
                        state->next_low_tide_time.unit.day = now.unit.day + 1;
                    } else {
                        state->next_low_tide_time.unit.day = now.unit.day;
                    }
                    _simple_tide_face_update_settings_display(settings, state);
                    break;
                case TIDE_DISPLAY_SET_ALERT:
                    // Close settings and return to the main display
                    state->display_type = TIDE_DISPLAY_TIDE_TIME;
                    _simple_tide_face_update(settings,state);
                    break;
                case TIDE_DISPLAY_TIDE_TIME:
                case TIDE_DISPLAY_TIDE_COUNTDOWN:
                    // Advance to the next tide event time
                    // Or advance tide countdown to next tide event
                    state->tide_event_index = (state->tide_event_index + 1) % 2;
                    _simple_tide_face_update(settings, state);
                    break;
                // This button does nothing when viewing the tide graph.
            }
            break;
        case EVENT_ALARM_LONG_PRESS:
            if (state->display_type > TIDE_DISPLAY_SET_ALERT) { // If not already in the settings,
                state->display_type = TIDE_DISPLAY_SET_HIGH_TIDE; // Go to the first setting
                //state->display_type = TIDE_DISPLAY_SET_ALERT; // To-Do testing
                state->active_time_setting_slot = 0;
                // watch_clear_display(); // To-do: remove? (is it also happening in _simple_tide_face_update_settings_display?)
                // movement_request_tick_frequency(4); // For more responsive setting feedback
                _simple_tide_face_update_settings_display(settings,state);
            }
            break;
        case EVENT_TIMEOUT:
            // Your watch face will receive this event after a period of inactivity. If it makes sense to resign,
            // you may uncomment this line to move back to the first watch face in the list:
            movement_move_to_face(0);
            break;
        case EVENT_LOW_ENERGY_UPDATE:
            // If you did not resign in EVENT_TIMEOUT, you can use this event to update the display once a minute.
            // Avoid displaying fast-updating values like seconds, since the display won't update again for 60 seconds.
            // You should also consider starting the tick animation, to show the wearer that this is sleep mode:
            // watch_start_tick_animation(500);
            break;
        default:
            // Movement's default loop handler will step in for any cases you don't handle above:
            // * EVENT_LIGHT_BUTTON_DOWN lights the LED
            // * EVENT_MODE_BUTTON_UP moves to the next watch face in the list
            // * EVENT_MODE_LONG_PRESS returns to the first watch face (or skips to the secondary watch face, if configured)
            // You can override any of these behaviors by adding a case for these events to this switch statement.
            return movement_default_loop_handler(event, settings);
    }

    // return true if the watch can enter standby mode. Generally speaking, you should always return true.
    // Exceptions:
    //  * If you are displaying a color using the low-level watch_set_led_color function, you should return false.
    //  * If you are sounding the buzzer using the low-level watch_set_buzzer_on function, you should return false.
    // Note that if you are driving the LED or buzzer using Movement functions like movement_illuminate_led or
    // movement_play_alarm, you can still return true. This guidance only applies to the low-level watch_ functions.
    return true;
}

void simple_tide_face_resign(movement_settings_t *settings, void *context) {
    (void) settings;
    (void) context;

    // handle any cleanup before your watch face goes off-screen.
}

