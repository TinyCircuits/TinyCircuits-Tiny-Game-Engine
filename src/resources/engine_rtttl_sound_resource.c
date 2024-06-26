#include "engine_rtttl_sound_resource.h"
#include "engine_tone_sound_resource.h"
#include "audio/engine_audio_channel.h"
#include "audio/engine_audio_module.h"
#include "debug/debug_print.h"
#include "resources/engine_resource_manager.h"
#include "math/engine_math.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>


#include "../lib/cglm/include/cglm/util.h"


// https://producelikeapro.com/blog/wp-content/uploads/2022/01/Understanding-Note-Frequency-Charts-And-Why-You-Should-Be-Using-One_2.jpg
const float octave_4_notes[7] = {
    440.00f,    // A
    493.88f,    // B
    261.63f,    // C
    293.66f,    // D
    329.63f,    // E
    349.23f,    // F
    392.00f,    // G
};

const float octave_4_notes_flat[7] = {
    466.16f,    // A#
    0.0f,       // B# Doesn't exist, for mapping only
    277.18f,    // C#
    311.13f,    // D#
    0.0f,       // E# Doesn't exist, for mapping only
    369.99f,    // F#
    415.30f     // G#
};

// When we get a letter that is within the large
// or small letter ASCII ranges, need to offset
// into `octave_4_notes` by subtracting that
// letter's int representation by these
const uint8_t ascii_to_note_offset_large = 65;
const uint8_t ascii_to_note_offset_small = 97;

// When we get a number for an octave (4, 5, 6, or 7)
// Need to know what power of two to multiply the base
// by. For example, if we get note A at octave 7:
//      freq = 440Hz
//      pwr_of_2 = 2^(7 - 4) = 2^3 = 8
//      freq = 440 * pwr_of_2 = 440*8 = 3520Hz (works!)
const uint8_t octave_base = 4;

// if the beats per minute is 112[b/m] then that
// means the beats per second is 112[b/m] * 1[m]/60[s]
// = 1.86[b/s]. This also means that the seconds per
// beat is 1/1.86[b/s] = 0.54[s/b], therefore, the
// interrupt samples per beat is 22050[samples/s] * 0.54[s/b] = 11854[samples/b]
//
//  https://www.pinterest.co.uk/pin/understanding-notes-and-rests-in-music-notation-reference-sheet--519039925783242366/
//
// Therefore, interrupt samples per 8th beat = 11864[samples/b] * 1[b]/8[8thb] = 1483[samples/8thb]
//
// Now, for various note durations, the actual duration a note lasts during a full beat (8 8th beats) is
//  * note duration = 1  = (1/1)  * 4[b] = 4[b]     -> 8th_beats_per_this_note = 4[b]
//  * note duration = 2  = (1/2)  * 4[b] = 2[b]     -> 8th_beats_per_this_note = 2[b]
//  * note duration = 4  = (1/4)  * 4[b] = 1[b]     -> 8th_beats_per_this_note = 1[b]
//  * note duration = 8  = (1/8)  * 4[b] = 0.5[b]   -> 8th_beats_per_this_note = 0.5[b]
//  * note duration = 16 = (1/16) * 4[b] = 0.25[b]  -> 8th_beats_per_this_note = 0.25[b]
//  * note duration = 32 = (1/32) * 4[b] = 0.125[b] -> 8th_beats_per_this_note = 0.125[b]
//
// If b=25 then 
//      seconds_per_beat = 25[b/m] * 1[m]/60[s] = 0.42[b/s] -> 1/0.42[b/s] = 2.4[s/b] = seconds_per_beat
//  ->  interrupt_samples_per_beat = 22050[samples/s] * 2.4[s/b] = 52920[samples/b] = interrupt_samples_per_beat
//  ->  note duration = 1 -> interrupt_samples_this_note = 4[b] * 52920[samples/b] = 211680[samples]


float ENGINE_FAST_FUNCTION(rtttl_sound_resource_get_sample)(rtttl_sound_resource_class_obj_t *self, bool *complete){
    if(self->interrupt_samples_counting >= self->interrupt_samples_until_next){
        // Reset counter and cast data to something more accessible
        self->interrupt_samples_counting = 0;
        uint8_t *data = ENGINE_BYTEARRAY_OBJ_TO_DATA(self->data);

        // Copy data into types from track
        uint32_t index = self->note_cursor * 8;
        uint32_t duration = 0;
        float frequency = 0.0f;

        memcpy(&duration, data+index, 4);
        memcpy(&frequency, data+index+4, 4);

        // Use the data to set the duration this
        // note will return samples for and
        // then set the frequency
        self->interrupt_samples_until_next = duration;
        tone_sound_resource_set_frequency(self->tone, frequency);

        // Loop if reach end of track
        self->note_cursor++;
        if(self->note_cursor >= self->note_count){
            self->note_cursor = 0;
        }
    }

    self->interrupt_samples_counting++;
    return tone_sound_resource_get_sample(self->tone);
}


bool rtttl_sound_resource_get_substr(void *substr, uint8_t substr_max_len, const char *start, uint8_t start_len, const char *end, uint8_t end_len){
    uint32_t file_size = engine_file_size(0);
    
    // Search for start and end substrings
    uint32_t start_offset = engine_file_seek_until(0, start, start_len);
    uint32_t end_offset = engine_file_seek_until(0, end, end_len);

    // If start not found, return false
    if(start_offset == file_size){
        return false;
    }

    // Correct end offset to refer to end of substring
    // as long as not at end of file, in which case,
    // the end offset is assumed to be correct
    if(end_offset != file_size) end_offset -= end_len;

    // If size of substring is too large, return false
    uint32_t substr_len = end_offset - start_offset;
    if(substr_len > substr_max_len){
        return false;
    }

    // Made it through all the checks, read the substring
    engine_file_seek(0, start_offset, MP_SEEK_SET);
    engine_file_read(0, substr, substr_len);

    return true;
}


bool rtttl_sound_resource_extract_default(uint16_t *output, const char *start, const char *end){
    // Always look from start
    engine_file_seek(0, 0, MP_SEEK_SET);

    char buffer[4] = {'0', '\0', '\0', '\0'};
    if(rtttl_sound_resource_get_substr(buffer, 3, start, 2, end, 1)){
        *output = atoi(buffer);
        return true;
    }

    return false;
}


void rtttl_sound_resource_get_defaults(uint8_t *note_duration, uint8_t *octave, uint16_t *beats_per_minute){
    // Try to extract default values. Try twice since,
    // depending on order, the substring could end with
    // `,` or `:`
    bool found_duration = rtttl_sound_resource_extract_default((uint16_t*)note_duration, "d=", ",");
    bool found_octave = rtttl_sound_resource_extract_default((uint16_t*)octave, "o=", ",");
    bool found_beats_per_minute = rtttl_sound_resource_extract_default(beats_per_minute, "b=", ",");

    if(found_duration == false)         found_duration = rtttl_sound_resource_extract_default((uint16_t*)note_duration, "d=", ":");
    if(found_octave == false)           found_octave = rtttl_sound_resource_extract_default((uint16_t*)octave, "o=", ":");
    if(found_beats_per_minute == false) found_beats_per_minute = rtttl_sound_resource_extract_default(beats_per_minute, "b=", ":");

    // https://www.mobilefish.com/tutorials/rtttl/rtttl_quickguide_specification.html#:~:text=defaults%20are%20assumed%3A-,d%3D4%2Co%3D6%2Cb%3D63.,-The%20data%20section
    if(found_duration == false)         *note_duration = 4;
    if(found_octave == false)           *octave = 6;
    if(found_beats_per_minute == false) *beats_per_minute = 63;
}


uint16_t rtttl_sound_resource_count_notes(){
    // Always look from start
    engine_file_seek(0, 0, MP_SEEK_SET);

    // Get to note data
    engine_file_seek_until(0, ":", 1);
    engine_file_seek_until(0, ":", 1);

    // Start with one noted counted since
    // last note does not end with comma
    uint16_t note_count = 1;

    // The current character
    char c = ' ';

    // Read until the end of the file
    while(engine_file_read(0, &c, 1) != 0){
        if(c == ',') note_count++;
    }

    return note_count;
}


bool rtttl_sound_resource_get_next(uint32_t *note_interrupt_samples, float *note_frequency, const char *start, uint8_t default_duration, uint8_t default_octave, uint16_t bpm){
    // Longest note: 32a#7.
    char note_buffer[7] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0'};

    // Try to find note
    if(rtttl_sound_resource_get_substr(note_buffer, 6, start, 1, ",", 1) == false){
        return false;
    }

    // Setup for extracting specific data and then extract note info
    char duration_buffer[3] = {'\0', '\0', '\0'};
    char octave_buffer[2] = {'\0', '\0'};

    bool note_collected = false;        // Flag switched when note collected so that next digit is put in octave buffer (duration -> note -> octave)
    uint8_t duration_index = 0;         // Index into `duration_buffer` for where next digit should be placed
    uint8_t octave_4_index = 0;         // Index into `octave_4_notes` based on note
    bool octave_flat = false;
    bool dotted = false;                // Flag switched to indicate if duration should be increased by one half
    bool paused = false;                // Flag switched to indicate if this is just a pause
    uint8_t note_buffer_index = 0;
    char current_char = note_buffer[note_buffer_index];

    while(current_char != '\0'){
        // Is this an ASCII base-10 number/digit?
        // If not a digit, is it the letter p?
        // If not p, is it a flat/#? 
        // If not a flat, is it a dot?
        // if not a dot, is it a note?
        if((uint8_t)current_char >= 49 && (uint8_t)current_char <= 57){
            // Handle digit

            // If we've collected `duration_collected`, must be the octave
            if(note_collected){
                octave_buffer[0] = current_char;
            }else{
                duration_buffer[duration_index] = current_char;
                duration_index++;
            }
        }else if(current_char == 'p'){
            // Handle pause
            paused = true;
        }else if(current_char == '#'){
            // Handle flat/#
            octave_flat = true;
        }else if(current_char == '.'){
            // Handle dot
            dotted = true;
        }else{
            // Handle note
            // Depending on if upper or lower case char,
            // get the correct pitch from the table
            if((uint8_t)current_char >= 65 && (uint8_t)current_char <= 71){           // Upper
                octave_4_index = (uint8_t)current_char - ascii_to_note_offset_large;
            }else if((uint8_t)current_char >= 97 && (uint8_t)current_char <= 103){    // Lower
                octave_4_index = (uint8_t)current_char - ascii_to_note_offset_small;
            }else{
                mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("RTTTLSoundResource: ERROR: Encountered a character that's not supported!"));
            }

            note_collected = true;
        }

        note_buffer_index++;
        current_char = note_buffer[note_buffer_index];
    }

    // If the duration buffer was filled with something,
    // decode duration, otherwise user default (same
    // for octave)
    uint8_t rtttl_note_duration = 0;
    if(duration_buffer[0] != '\0'){
        rtttl_note_duration = atoi(duration_buffer);
    }else{
        rtttl_note_duration = default_duration;
    }

    uint8_t note_octave = 0;
    if(octave_buffer[0] != '\0'){
        note_octave = atoi(octave_buffer);
    }else{
        note_octave = default_octave;
    }

    float beats_per_second = bpm/60.0f;
    float seconds_per_beat = 1.0f/beats_per_second;
    float interrupt_samples_per_beat = ENGINE_AUDIO_SAMPLE_RATE * seconds_per_beat;

    // Based on the beats per minute, calculate how many
    // times the playback interrupt get_sample function
    // needs to be called for this note.
    // A whole note gets 4 beats and everything is a fraction
    // of that
    float note_beats = (1.0f/(float)rtttl_note_duration) * 4.0f;
    *note_interrupt_samples = (uint32_t)(note_beats * interrupt_samples_per_beat);

    // https://www.mobilefish.com/tutorials/rtttl/rtttl_quickguide_specification.html#:~:text=optional%20dotting%20(which%20increases%20the%20duration%20of%20the%20note%20by%20one%20half)
    if(dotted){
        *note_interrupt_samples = *note_interrupt_samples + (*note_interrupt_samples/2);
    }
    
    if(paused){
        *note_frequency = 0.0f;
        return true;
    }else{
        // Lookup base octave-4 frequency depending on if flat or not
        if(octave_flat){
            *note_frequency = octave_4_notes_flat[octave_4_index];
        }else{
            *note_frequency = octave_4_notes[octave_4_index];
        }
        
        // Multiply up to frequency using base and octave
        if(note_octave > octave_base){
            uint8_t note_octave_power_of_2 = note_octave - octave_base;
            *note_frequency = *note_frequency * powf(2.0f, note_octave_power_of_2);
        }
        return true;
    }
}


void rtttl_sound_resource_store_note(uint32_t note_interrupt_samples, float note_frequency){
    uint8_t data[4];

    memcpy(data, &note_interrupt_samples, 4);
    engine_resource_store_u8(data[0]);
    engine_resource_store_u8(data[1]);
    engine_resource_store_u8(data[2]);
    engine_resource_store_u8(data[3]);

    memcpy(data, &note_frequency, 4);
    engine_resource_store_u8(data[0]);
    engine_resource_store_u8(data[1]);
    engine_resource_store_u8(data[2]);
    engine_resource_store_u8(data[3]);
}


mp_obj_t rtttl_sound_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New RTTTLSoundResource");
    mp_arg_check_num(n_args, n_kw, 1, 1, false);

    rtttl_sound_resource_class_obj_t *self = mp_obj_malloc_with_finaliser(rtttl_sound_resource_class_obj_t, &rtttl_sound_resource_class_type);
    self->base.type = &rtttl_sound_resource_class_type;
    self->channel = NULL;
    self->tone = tone_sound_resource_class_new(&tone_sound_resource_class_type, 0, 0, NULL);
    self->data = NULL;
    self->interrupt_samples_until_next = 0;
    self->interrupt_samples_counting = 0;
    self->note_cursor = 0;

    // Open one-time file
    engine_file_open_read(0, args[0]);

    // Get default note duration, octave, and bpm
    uint8_t default_duration = 0;
    uint8_t default_octave = 0;
    uint16_t beats_per_minute = 0;

    rtttl_sound_resource_get_defaults(&default_duration,
                                      &default_octave,
                                      &beats_per_minute);
    
    // Get note count
    self->note_count = rtttl_sound_resource_count_notes();

    // To reduce interrupt complexity, for each note cache
    //  * Times get_sample needs to be called before moving to next sample (32-bits/4 bytes)
    //  * Complete frequency (32-bits/4 bytes)
    self->data = engine_resource_get_space_bytearray(self->note_count * (4+4), true);
    engine_resource_start_storing(self->data, true);

    // Get to first `:` to skip it
    engine_file_seek(0, 0, MP_SEEK_SET);
    engine_file_seek_until(0, ":", 1);

    uint32_t note_interrupt_samples = 0;
    float note_frequency = 0.0f;

    // Get the first note starting with `:`
    rtttl_sound_resource_get_next(&note_interrupt_samples, &note_frequency, ":", default_duration, default_octave, beats_per_minute);
    rtttl_sound_resource_store_note(note_interrupt_samples, note_frequency);

    // Get the rest of the notes starting with `,`
    while(rtttl_sound_resource_get_next(&note_interrupt_samples, &note_frequency, ",", default_duration, default_octave, beats_per_minute)){
        rtttl_sound_resource_store_note(note_interrupt_samples, note_frequency);
    }

    // Stop storing and close one-time file
    engine_resource_stop_storing();
    engine_file_close(0);

    return MP_OBJ_FROM_PTR(self);
}


// Class methods
static mp_obj_t rtttl_sound_resource_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("RTTTLSoundResource: Deleted (freeing sound data)");

    rtttl_sound_resource_class_obj_t *self = self_in;
    audio_channel_class_obj_t *channel = self->channel;

    // This is very important! Need to make sure to set channel source this source is
    // related to NULL. Otherwise, even though this source gets collected it will not
    // be set to NULL and the audio ISR will try to access invalid memory!!!
    if(channel != NULL){
        audio_channel_stop(channel);
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(rtttl_sound_resource_class_del_obj, rtttl_sound_resource_class_del);


/*  --- doc ---
    NAME: RTTTLSoundResource
    ID: RTTTLSoundResource
    DESC: Can be used to play a music in a ringtone format (TODO: better docs): https://en.wikipedia.org/wiki/Ring_Tone_Text_Transfer_Language
    PARAM:  [type=string]         [name=filepath] [value=any]
    ATTR:   [type=int]            [name=tempo]    [value=any positive value]
    ATTR:   [type=bytearray]      [name=data]     [value=bytearray consisting of values between 0 and 13 corresponding to notes in lookup table]                                                                                                                                                               
*/ 
static void rtttl_sound_resource_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing RTTTLSoundResource attr");

    rtttl_sound_resource_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
            case MP_QSTR___del__:
                destination[0] = MP_OBJ_FROM_PTR(&rtttl_sound_resource_class_del_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_tone:
                destination[0] = self->tone;
            break;
            case MP_QSTR_data:
                destination[0] = self->data;
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_data:
                mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("RTTTLSoundResource: ERROR: Setting data directly is not allowed!"));
            break; 
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
static const mp_rom_map_elem_t rtttl_sound_resource_class_locals_dict_table[] = {

};
static MP_DEFINE_CONST_DICT(rtttl_sound_resource_class_locals_dict, rtttl_sound_resource_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    rtttl_sound_resource_class_type,
    MP_QSTR_RTTTLSoundResource,
    MP_TYPE_FLAG_NONE,

    make_new, rtttl_sound_resource_class_new,
    attr, rtttl_sound_resource_class_attr,
    locals_dict, &rtttl_sound_resource_class_locals_dict
);