/* KallistiOS ##version##

   dc/maple/keyboard.h
   Copyright (C) 2000-2002 Jordan DeLong and Megan Potter
   Copyright (C) 2012 Lawrence Sebald
   Copyright (C) 2024 Falco Girgis

*/

/** \file    dc/maple/keyboard.h
    \brief   Definitions for using the keyboard device.
    \ingroup kbd

    This file contains the definitions needed to access the Maple keyboard
    device. Obviously, this corresponds to the MAPLE_FUNC_KEYBOARD function
    code.

    \todo
        - French/AZERTY keyboard keymaps
        - Italian/QWERTY keyboard keymaps
        - Error key states are not handled properly
        - Remove legacy "global queue" API
        - Fix caps lock ASCII conversions
        - Error mechanisms:
            * unknown region
            * hardware key press limit exceeded
            * queue buffer overflow

    \author Jordan DeLong
    \author Megan Potter
    \author Lawrence Sebald
    \author Falco Girgis
*/

#ifndef __DC_MAPLE_KEYBOARD_H
#define __DC_MAPLE_KEYBOARD_H

#include <sys/cdefs.h>
__BEGIN_DECLS

#include <arch/types.h>
#include <dc/maple.h>

#include <stdint.h>
#include <stdbool.h>

/** \defgroup kbd   Keyboard
    \brief          Driver for the Dreamcast's Keyboard Input Device
    \ingroup        peripherals

    @{
*/


/** \defgroup   kbd_mods    Modifier Keys
    \brief                  Masks for the various keyboard modifier keys

    These are the various modifiers that can be pressed on the keyboard, and are
    reflected in the modifiers field of kbd_cond_t.

    @{
*/
#define KBD_MOD_LCTRL       (1 << 0)
#define KBD_MOD_LSHIFT      (1 << 1)
#define KBD_MOD_LALT        (1 << 2)
#define KBD_MOD_S1          (1 << 3)
#define KBD_MOD_RCTRL       (1 << 4)
#define KBD_MOD_RSHIFT      (1 << 5)
#define KBD_MOD_RALT        (1 << 6)
#define KBD_MOD_S2          (1 << 7)
/** @} */

/** \brief  Keyboard Modifier Keys

    Typedef for a bitmask of modifier keys on the DC keyboard.

    \sa kbd_mods
*/
typedef uint8_t kbd_mods_t;

/** \defgroup   kbd_leds    LEDs
    \brief                  Values for the different keyboard LEDs
    \ingroup                kbd

    This is the LEDs that can be turned on and off on the keyboard. This list
    may not be exhaustive. Think of these sorta like an extension of the
    modifiers list.

    @{
*/
#define KBD_LED_NUMLOCK     (1 << 0)
#define KBD_LED_CAPSLOCK    (1 << 1)
#define KBD_LED_SCRLOCK     (1 << 2)
#define KBD_LED_UNKNOWN1    (1 << 3)
#define KBD_LED_UNKNOWN2    (1 << 4)
#define KBD_LED_KANA        (1 << 5)
#define KBD_LED_POWER       (1 << 6)
#define KBD_LED_SHIFT       (1 << 7)
/** @} */

typedef uint8_t kbd_leds_t;

/** \defgroup   kbd_keys    Keys
    \brief                  Values representing the various keyboard keys

    This is the list of keys that are on the keyboard that may be pressed. The
    keyboard returns keys in this format.

    These are the raw keycodes returned by the US keyboard, and thus only cover
    the keys on US keyboards.
*/
enum {
    KBD_KEY_NONE            = 0x00,
    KBD_KEY_ERROR           = 0x01, /* ERROR_ROLLOVER */
    KBD_KEY_ERR2            = 0x02,
    KBD_KEY_ERR3            = 0x03,
    KBD_KEY_A               = 0x04,
    KBD_KEY_B               = 0x05,
    KBD_KEY_C               = 0x06,
    KBD_KEY_D               = 0x07,
    KBD_KEY_E               = 0x08,
    KBD_KEY_F               = 0x09,
    KBD_KEY_G               = 0x0a,
    KBD_KEY_H               = 0x0b,
    KBD_KEY_I               = 0x0c,
    KBD_KEY_J               = 0x0d,
    KBD_KEY_K               = 0x0e,
    KBD_KEY_L               = 0x0f,
    KBD_KEY_M               = 0x10,
    KBD_KEY_N               = 0x11,
    KBD_KEY_O               = 0x12,
    KBD_KEY_P               = 0x13,
    KBD_KEY_Q               = 0x14,
    KBD_KEY_R               = 0x15,
    KBD_KEY_S               = 0x16,
    KBD_KEY_T               = 0x17,
    KBD_KEY_U               = 0x18,
    KBD_KEY_V               = 0x19,
    KBD_KEY_W               = 0x1a,
    KBD_KEY_X               = 0x1b,
    KBD_KEY_Y               = 0x1c,
    KBD_KEY_Z               = 0x1d,
    KBD_KEY_1               = 0x1e,
    KBD_KEY_2               = 0x1f,
    KBD_KEY_3               = 0x20,
    KBD_KEY_4               = 0x21,
    KBD_KEY_5               = 0x22,
    KBD_KEY_6               = 0x23,
    KBD_KEY_7               = 0x24,
    KBD_KEY_8               = 0x25,
    KBD_KEY_9               = 0x26,
    KBD_KEY_0               = 0x27,
    KBD_KEY_ENTER           = 0x28,
    KBD_KEY_ESCAPE          = 0x29,
    KBD_KEY_BACKSPACE       = 0x2a,
    KBD_KEY_TAB             = 0x2b,
    KBD_KEY_SPACE           = 0x2c,
    KBD_KEY_MINUS           = 0x2d,
    KBD_KEY_PLUS            = 0x2e,
    KBD_KEY_LBRACKET        = 0x2f,
    KBD_KEY_RBRACKET        = 0x30,
    KBD_KEY_BACKSLASH       = 0x31,
    KBD_KEY_SEMICOLON       = 0x33,
    KBD_KEY_QUOTE           = 0x34,
    KBD_KEY_TILDE           = 0x35,
    KBD_KEY_COMMA           = 0x36,
    KBD_KEY_PERIOD          = 0x37,
    KBD_KEY_SLASH           = 0x38,
    KBD_KEY_CAPSLOCK        = 0x39,
    KBD_KEY_F1              = 0x3a,
    KBD_KEY_F2              = 0x3b,
    KBD_KEY_F3              = 0x3c,
    KBD_KEY_F4              = 0x3d,
    KBD_KEY_F5              = 0x3e,
    KBD_KEY_F6              = 0x3f,
    KBD_KEY_F7              = 0x40,
    KBD_KEY_F8              = 0x41,
    KBD_KEY_F9              = 0x42,
    KBD_KEY_F10             = 0x43,
    KBD_KEY_F11             = 0x44,
    KBD_KEY_F12             = 0x45,
    KBD_KEY_PRINT           = 0x46,
    KBD_KEY_SCRLOCK         = 0x47,
    KBD_KEY_PAUSE           = 0x48,
    KBD_KEY_INSERT          = 0x49,
    KBD_KEY_HOME            = 0x4a,
    KBD_KEY_PGUP            = 0x4b,
    KBD_KEY_DEL             = 0x4c,
    KBD_KEY_END             = 0x4d,
    KBD_KEY_PGDOWN          = 0x4e,
    KBD_KEY_RIGHT           = 0x4f,
    KBD_KEY_LEFT            = 0x50,
    KBD_KEY_DOWN            = 0x51,
    KBD_KEY_UP              = 0x52,
    KBD_KEY_PAD_NUMLOCK     = 0x53,
    KBD_KEY_PAD_DIVIDE      = 0x54,
    KBD_KEY_PAD_MULTIPLY    = 0x55,
    KBD_KEY_PAD_MINUS       = 0x56,
    KBD_KEY_PAD_PLUS        = 0x57,
    KBD_KEY_PAD_ENTER       = 0x58,
    KBD_KEY_PAD_1           = 0x59,
    KBD_KEY_PAD_2           = 0x5a,
    KBD_KEY_PAD_3           = 0x5b,
    KBD_KEY_PAD_4           = 0x5c,
    KBD_KEY_PAD_5           = 0x5d,
    KBD_KEY_PAD_6           = 0x5e,
    KBD_KEY_PAD_7           = 0x5f,
    KBD_KEY_PAD_8           = 0x60,
    KBD_KEY_PAD_9           = 0x61,
    KBD_KEY_PAD_0           = 0x62,
    KBD_KEY_PAD_PERIOD      = 0x63,
    KBD_KEY_S3              = 0x65
};

typedef uint8_t kbd_key_t;

/** \defgroup   kbd_regions Region Codes
    \brief                  Region codes for the Dreamcast keyboard
    \ingroup                kbd

    This is the list of possible values for the "region" field in the
    kbd_state_t structure.
    @{
*/
typedef enum kbd_region {
    KBD_REGION_JP = 1,  /**< \brief Japanese keyboard */
    KBD_REGION_US = 2,  /**< \brief US keyboard */
    KBD_REGION_UK = 3,  /**< \brief UK keyboard */
    KBD_REGION_DE = 4,  /**< \brief German keyboard */
    KBD_REGION_FR = 5,  /**< \brief French keyboard (not supported yet) */
    KBD_REGION_IT = 6,  /**< \brief Italian keyboard (not supported yet) */
    KBD_REGION_ES = 7  /**< \brief Spanish keyboard */
} kbd_region_t;
/** @} */

/** \defgroup   key_states  Key States
    \brief                  States each key can be in.
    \ingroup                kbd

    These are the different 'states' each key can be in. They are stored in
    kbd_state_t->matrix, and manipulated/checked by kbd_check_poll.

    none-> pressed or none
    was pressed-> pressed or none
    pressed-> was_pressed
    @{
*/

#define KEY_PACK_STATE(isDown, wasDown) \
    ((!!(isDown)) | ((!!(wasDown)) << 0x1))

#define KEY_FLAG_IS_PRESSED  0x1
#define KEY_FLAG_WAS_PRESSED 0x2
#define KEY_FLAG_ALL         0x3

enum {
    KEY_STATE_HELD_UP    = KEY_PACK_STATE(false, false),
    KEY_STATE_TAPPED     = KEY_PACK_STATE(true, false),
    KEY_STATE_RELEASED   = KEY_PACK_STATE(false, true),
    KEY_STATE_HELD_DOWN  = KEY_PACK_STATE(true, true),
};

typedef uint8_t key_state_t;

/** @} */

/** \brief   Maximum number of keys the DC can read simultaneously.
    \ingroup kbd
    This is a hardware constant. The define prevents the magic number '6' from appearing.
**/
#define MAX_PRESSED_KEYS 6

/** \brief   Maximum number of keys a DC keyboard can have.
    \ingroup kbd
    This is a hardware constant. The define prevents the magic number '256' from appearing.
**/
#define MAX_KBD_KEYS 256

/** \brief   Size of a keyboard queue.
    \ingroup kbd

    Each keyboard queue will hold this many elements. Once the queue fills, no
    new elements will be placed on the queue. As long as you check the queue
    relatively frequently, the default of 16 should be plenty.

    \note   This <strong>MUST</strong> be a power of two.
*/
#define KBD_QUEUE_SIZE 16


/** \brief   Keyboard status structure.
    \ingroup kbd

    This structure holds information about the current status of the keyboard
    device. This is what maple_dev_status() will return.

    \headerfile dc/maple/keyboard.h
*/
typedef struct kbd_state {
    /** \brief  Key array.

        This array lists the state of all possible keys on the keyboard. It can
        be used for key repeat and debouncing. This will be non-zero if the key
        is currently being pressed.

        \see    kbd_keys
    */
    key_state_t keys[MAX_KBD_KEYS];

    /** \brief  Modifier key status. */
    kbd_mods_t modifiers;

    kbd_leds_t leds;

    /** \brief  Keyboard type/region. */
    kbd_region_t region;

    kbd_key_t repeat_key;           /**< \brief Key that is repeating. */
    uint64_t repeat_timeout;        /**< \brief Time that the next repeat will trigger. */
} kbd_state_t;

typedef void (*kbd_key_callback_t)(maple_device_t *dev, key_state_t state,
                                   kbd_key_t key, kbd_mods_t mods, kbd_leds_t leds, void *ud);

void kbd_set_event_handler(kbd_key_callback_t callback, void *user_data);
void kbd_set_repeat_timing(uint16_t start, uint16_t interval);

int kbd_set_leds(maple_device_t *dev, kbd_leds_t leds);

key_state_t kbd_key_state(maple_device_t *dev, kbd_key_t key);

char kbd_key_to_ascii(kbd_region_t region, kbd_key_t key, kbd_mods_t mods, kbd_leds_t leds);



/** \brief   Activate or deactivate global key queueing.
    \ingroup kbd
    \deprecated

    This function will turn the internal keyboard queueing on or off. Note that
    there is only one queue for the whole system, no matter how many keyboards
    are attached, and the queue is of fairly limited length. Turning queueing
    off is useful (for instance) in a game where individual keypresses don't
    mean as much as having the keys up or down does.

    You can clear the queue (without popping all the keys off) by setting the
    active value to a different value than it was.

    The queue is by default on, unless you turn it off.

    \param  active          Set to non-zero to activate the queue.
    \note                   The global queue does not account for non-US
                            keyboard layouts and is deprecated. Please use the
                            individual queues instead for future code.
*/
void kbd_set_queue(int active) __attribute__((deprecated));

/** \brief   Pop a key off the global keyboard queue.
    \ingroup kbd
    \deprecated

    This function pops the front off of the keyboard queue, and returns the
    value to the caller. The value returned will be the ASCII value of the key
    pressed (accounting for the shift keys being pressed).

    If a key does not have an ASCII value associated with it, the raw key code
    will be returned, shifted up by 8 bits.

    \return                 The value at the front of the queue, or -1 if there
                            are no keys in the queue or queueing is off.
    \note                   This function does not account for non-US keyboard
                            layouts properly (for compatibility with old code),
                            and is deprecated. Use the individual keyboard
                            queues instead to properly account for non-US
                            keyboards.
    \see                    kbd_queue_pop()
*/
int kbd_get_key(void) __attribute__((deprecated));

/** \brief   Pop a key off a specific keyboard's queue.
    \ingroup kbd

    This function pops the front element off of the specified keyboard queue,
    and returns the value of that key to the caller.

    If the xlat parameter is non-zero and the key represents an ISO-8859-1
    character, that is the value that will be returned from this function.
    Otherwise if xlat is non-zero, it will be the raw key code, shifted up by 8
    bits.

    If the xlat parameter is zero, the lower 8 bits of the returned value will
    be the raw key code. The next 8 bits will be the modifier keys that were
    down when the key was pressed (a bitfield of KBD_MOD_* values). The next 3
    bits will be the lock key status (a bitfield of KBD_LED_* values).

    \param  dev             The keyboard device to read from.
    \param  xlat            Set to non-zero to do key translation. Otherwise,
                            you'll simply get the raw key value. Raw key values
                            are not mapped at all, so you are responsible for
                            figuring out what it is by the region.

    \return                 The value at the front of the queue, or -1 if there
                            are no keys in the queue.
*/
int kbd_queue_pop(maple_device_t *dev, bool xlat);

/* \cond */
/* Init / Shutdown */
void kbd_init(void);
void kbd_shutdown(void);
/* \endcond */

/** @} */

__END_DECLS

#endif  /* __DC_MAPLE_KEYBOARD_H */
