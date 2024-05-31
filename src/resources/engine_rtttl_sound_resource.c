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
const float octave_4_notes[14] = {
    440.00f,    // A
    466.16f,    // A#

    493.88f,    // B
    0.0f,       // Doesn't exist, for mapping only

    261.63f,    // C
    277.18f,    // C#

    293.66f,    // D
    311.13f,    // D#

    329.63f,    // E
    0.0f,       // Doesn't exist, for mapping only

    349.23f,    // F
    369.99f,    // F#

    392.00f,    // G
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


float ENGINE_FAST_FUNCTION(rtttl_sound_resource_get_sample)(rtttl_sound_resource_class_obj_t *self, bool *complete){
    self->seconds_since_8th_beat += ENGINE_AUDIO_SAMPLE_DT;

    // Notes are played/checked at smallest time division
    // of eighth notes
    if(self->seconds_since_8th_beat >= self->seconds_per_8th_beat){
        self->seconds_since_8th_beat = 0.0f;
        self->note_beat_count++;

        // Only change notes when the current note has played
        // for enough beats
        if(self->note_beat_count >= self->note_beat_duration){
            self->note_beat_count = 0;

            // The attributes of the note we need to collect are:
            // * duration
            // * pitch (frequency/note)
            // * octave (power of 2 frequency multiplier)
            // * rest (should we actually do nothing this duration?)
            self->note_beat_duration = self->default_d;
            self->rest = false;
            float note_octave = self->default_o;

            // When/if a note is found, save it's index.
            // If a flat/# is found, add 1 to this index
            uint8_t note_index = 0;
            bool note_collected = false;

            // Duration (1, 2, 4, 8, 16, 32) and octave (4, 5, 6, 7) will both be stored here
            char duration_digits[3] = {'0', '\0', '\0'};
            char octave_digits[2] = {'0', '\0'};
            uint8_t duration_digit_index = 0;

            bool dotted = false;

            // Get the character at current cursor position
            uint8_t current_char = self->data[self->cursor];

            // While not a comma, collect note and configure tone afterwards
            while(current_char != 44){
                // Evaluate current character and set flags for state-machine

                // Is this an ASCII base-10 number/digit?
                // If not a digit, is it the letter p?
                // If not p, is it a flat/#? 
                // If not a flat, is it a dot?
                // if not a dot, is it a note?
                if(current_char >= 49 && current_char <= 57){
                    // Handle digit

                    // If we've collected a note, must be the octave
                    if(note_collected){
                        octave_digits[0] = current_char;
                    }else{
                        duration_digits[duration_digit_index] = current_char;
                        duration_digit_index++;
                    }
                }else if(current_char == 112){
                    // Handle pause
                    self->rest = true;
                }else if(current_char == 35){
                    // Handle flat/#
                    note_index += 1;
                }else if(current_char == 46){
                    // Handle dot
                    dotted = true;
                }else{
                    // Handle note
                    // Depending on if upper or lower case char,
                    // get the correct pitch from the table
                    if(current_char >= 65 && current_char <= 71){           // Upper
                        note_index = current_char - ascii_to_note_offset_large;
                    }else if(current_char >= 97 && current_char <= 103){    // Lower
                        note_index = current_char - ascii_to_note_offset_small;
                    }else{
                        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("RTTTLSoundResource: ERROR: Encountered a character that's not supported!"));
                    }

                    note_collected = true;
                }

                // Progress to the next char
                self->cursor++;

                if(self->cursor >= self->data_size){
                    break;
                }

                current_char = self->data[self->cursor];
            }


            // Now that the information is extracted, set the duration and tone
            if(duration_digit_index > 0){
                self->note_beat_duration = atoi(duration_digits);

                // A note divisor of 1 means this note gets 4 beats
                // but needs to last 4 * 8 = 32 8th notes (that's the playback rate)
                // A note divisor of 32 means this note gets 1/8th beat
                // but needs to last 4/32 * 8 = 1 8th note
                self->note_beat_duration = (4.0f / self->note_beat_duration) * 8.0f;

                // https://www.mobilefish.com/tutorials/rtttl/rtttl_quickguide_specification.html#:~:text=A%20dotted%20duration%20is%20one%20in%20which%20a%20note%20is%20given%20the%20duration%20of%20%22itself%20%2B%20half%20of%20itself.%22
                if(dotted){
                    self->note_beat_duration += self->note_beat_duration / 2;
                }
            }

            if(self->rest == false && octave_digits[0] != '0'){
                note_octave = atoi(octave_digits);
            }

            if(self->rest == false){
                float frequency = octave_4_notes[note_index];
                uint8_t note_octave_power_of_2 = note_octave - octave_base;

                frequency = frequency * powf(2.0f, note_octave_power_of_2);

                tone_sound_resource_set_frequency(self->tone, frequency);
            }

            // If we end, loop, otherwise skip comma
            if(self->cursor >= self->data_size){
                self->cursor = 0;
                *complete = true;
            }else{
                // Skip comma
                self->cursor++;
            }
        }
    }

    // Always return the current note that should be playing
    if(self->rest == false){
        return tone_sound_resource_get_sample(self->tone);
    }else{
        return 0.0f;
    }
}


void rtttl_sound_resource_set_b(rtttl_sound_resource_class_obj_t *self, uint16_t b){
    self->b = b;

    // Calculate beats per minute and divide by 8 to account for 1/32 notes:
    // https://images.app.goo.gl/J4eHkGHcoSyJuZ5W6
    self->seconds_per_8th_beat = (60.0f / (float)self->b) / 8.0f;
    self->seconds_since_8th_beat = 0.0f;
}


mp_obj_t rtttl_sound_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New RTTTLSoundResource");
    mp_arg_check_num(n_args, n_kw, 1, 1, false);

    rtttl_sound_resource_class_obj_t *self = m_new_obj_with_finaliser(rtttl_sound_resource_class_obj_t);
    self->base.type = &rtttl_sound_resource_class_type;
    self->channel = NULL;
    self->tone = tone_sound_resource_class_new(&tone_sound_resource_class_type, 0, 0, NULL);
    self->data = NULL;
    self->data_size = 0;
    self->cursor = 0;

    // https://www.mobilefish.com/tutorials/rtttl/rtttl_quickguide_specification.html#:~:text=defaults%20are%20assumed%3A-,d%3D4%2Co%3D6%2Cb%3D63.,-The%20data%20section
    self->default_d = 4;
    self->default_o = 6;
    rtttl_sound_resource_set_b(self, 63);

    self->note_beat_duration = 0;
    self->note_beat_count = 0;
    self->rest = false;

    // Open one-time file
    engine_file_open_read(0, args[0]);
    self->data_size = engine_file_size(0);

    // ### STEP 1: Don't care about the name, subtract name colon from the size ###
    // Need to subtract non-data portion of file size.
    // Subtract a byte until we find first colon or at
    // least stop if no colon is found and the data is all
    // the way subtracted
    uint8_t current_char = engine_file_seek_get_u8(0, self->cursor);
    while(current_char != 58 && self->data_size != 0){
        self->cursor++;
        self->data_size--;
        current_char = engine_file_seek_get_u8(0, self->cursor);
    }

    // Skip the colon and subtract the first colon from the size
    self->cursor++;
    self->data_size--;

    // ### STEP 2: Subtract the default value section from the size and get the default values ###
    char default_duration_digits[3] = {'0', '\0', '\0'};
    char default_octave_digits[3] = {'0', '\0', '\0'};
    char default_beats_digits[4] = {'0', '\0', '\0', '\0'};
    uint8_t duration_digit_index = 0;
    uint8_t octave_digit_index = 0;
    uint8_t beats_digit_index = 0;
    char gathering_default = ' ';

    current_char = engine_file_seek_get_u8(0, self->cursor);
    while(current_char != 58 && self->data_size != 0){  // ;
        switch(current_char){
            case 100:   // d
            {
                gathering_default = 'd';
                self->cursor++; // Skip `=`
                self->data_size--;
            }
            break;
            case 111:   // o
            {
                gathering_default = 'o';
                self->cursor++; // Skip `=`
                self->data_size--;
            }
            break;
            case 98:    // b
            {
                gathering_default = 'b';
                self->cursor++; // Skip `=`
                self->data_size--;
            }
            break;
            default:
            {
                if(current_char != 44){ // ,
                    if(gathering_default == 'd'){
                        default_duration_digits[duration_digit_index] = current_char;
                        duration_digit_index++;
                    }else if(gathering_default == 'o'){
                        default_octave_digits[octave_digit_index] = current_char;
                        octave_digit_index++;
                    }else if(gathering_default == 'b'){
                        default_beats_digits[beats_digit_index] = current_char;
                        beats_digit_index++;
                    }
                }
            }
        }

        self->cursor++;
        self->data_size--;
        current_char = engine_file_seek_get_u8(0, self->cursor);
    }

    // Actually assign the default values from the extracted strings
    if(duration_digit_index > 0){
        self->default_d = atoi(default_duration_digits);
    }

    if(octave_digit_index > 0){
        self->default_o = atoi(default_octave_digits);
    }

    if(beats_digit_index > 0){
        rtttl_sound_resource_set_b(self, atoi(default_beats_digits));
    }

    // Skip the colon and subtract the second colon from the size
    self->cursor++;
    self->data_size--;

    // Get space in ram for notes to live
    self->data = engine_resource_get_space(self->data_size, true);
    engine_resource_start_storing(self->data, true);

    // One byte at a time, copy data from LFS to scratch space
    for(uint32_t idx=0; idx<self->data_size; idx++){
        engine_resource_store_u8(engine_file_seek_get_u8(0, self->cursor));
        self->cursor++;
    }
    
    // Stop storing and close one-time file
    engine_resource_stop_storing();
    engine_file_close(0);

    // Reset cursor to play music instead of seek through file
    self->cursor = 0;
    self->data_size--;  // <- not exactly sure why one less is needed not to seg fault

    return MP_OBJ_FROM_PTR(self);
}


// Class methods
STATIC mp_obj_t rtttl_sound_resource_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("RTTTLSoundResource: Deleted (freeing sound data)");

    rtttl_sound_resource_class_obj_t *self = self_in;
    audio_channel_class_obj_t *channel = self->channel;

    // This is very important! Need to make sure to set channel source this source is
    // related to NULL. Otherwise, even though this source gets collected it will not
    // be set to NULL and the audio ISR will try to access invalid memory!!!
    if(channel != NULL){
        audio_channel_stop(channel);
    }

    // Free the resource fast space
    #if defined(__unix__)
        free(self->data);
    #elif (__arm__)
        m_free(self->data);
    #else
        #error "TextureResource: Unknown platform"
    #endif

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(rtttl_sound_resource_class_del_obj, rtttl_sound_resource_class_del);


/*  --- doc ---
    NAME: RTTTLSoundResource
    ID: RTTTLSoundResource
    DESC: Can be used to play a music in a ringtone format (TODO: better docs): https://en.wikipedia.org/wiki/Ring_Tone_Text_Transfer_Language
    PARAM:  [type=string]   [name=filepath] [value=any]
    ATTR:   [type=int]      [name=tempo]    [value=any positive value]                                                                                                                                                                  
*/ 
STATIC void rtttl_sound_resource_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
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
            case MP_QSTR_tempo:
                destination[0] = mp_obj_new_int(self->b);   // beats per minute
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_tempo:
                self->b = mp_obj_get_int(destination[1]);
            break;
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t rtttl_sound_resource_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(rtttl_sound_resource_class_locals_dict, rtttl_sound_resource_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    rtttl_sound_resource_class_type,
    MP_QSTR_RTTTLSoundResource,
    MP_TYPE_FLAG_NONE,

    make_new, rtttl_sound_resource_class_new,
    attr, rtttl_sound_resource_class_attr,
    locals_dict, &rtttl_sound_resource_class_locals_dict
);