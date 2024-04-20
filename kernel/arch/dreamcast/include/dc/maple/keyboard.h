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

/** \brief   Maximum number of keys the DC can read simultaneously.

    This is a hardware constant. The define prevents the magic number '6' from appearing.
**/
#define MAX_PRESSED_KEYS 6

/** \brief   Maximum number of keys a DC keyboard can have.

    This is a hardware constant. The define prevents the magic number '256' from appearing.
**/
#define MAX_KBD_KEYS 256

/** \brief   Size of a keyboard queue.

    Each keyboard queue will hold this many elements. Once the queue fills, no
    new elements will be placed on the queue. As long as you check the queue
    relatively frequently, the default of 16 should be plenty.

    \note   This <strong>MUST</strong> be a power of two.
*/
#define KBD_QUEUE_SIZE 16

/** \defgroup   kbd_mods    Modifier Keys
    \brief                  Masks for the various keyboard modifier keys

    These are the various modifiers that can be pressed on the keyboard, and are
    reflected in the modifiers field of kbd_cond_t.

    \sa kbd_mods_t

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

    This is the LEDs that can be turned on and off on the keyboard. This list
    may not be exhaustive. Think of these sorta like an extension of the
    modifiers list.

    \sa kbd_leds_t

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

/** \brief Keyboard LED States

    Typedef for an integral type which can store the LED bit flags.

    \sa kbd_leds
*/
typedef uint8_t kbd_leds_t;

/**\brief Raw Keyboard Key Identifiers

    This is the list of keys that are on the keyboard that may be pressed. The
    keyboard returns keys in this format.

    \note
    These are the raw keycodes returned by the US keyboard, and thus only cover
    the keys on US keyboards.

    \sa kbd_key_t
*/
enum {
    KBD_KEY_NONE         = 0x00, /**< \brief No key */
    KBD_KEY_ERROR        = 0x01, /**< \brief ERROR_ROLLOVER */
    KBD_KEY_ERR2         = 0x02, /**< \brief Unknown error */
    KBD_KEY_ERR3         = 0x03, /**< \brief Unknown error */
    KBD_KEY_A            = 0x04, /**< \brief A key */
    KBD_KEY_B            = 0x05, /**< \brief B key */
    KBD_KEY_C            = 0x06, /**< \brief C key */
    KBD_KEY_D            = 0x07, /**< \brief D key */
    KBD_KEY_E            = 0x08, /**< \brief E key */
    KBD_KEY_F            = 0x09, /**< \brief F key */
    KBD_KEY_G            = 0x0a, /**< \brief G key */
    KBD_KEY_H            = 0x0b, /**< \brief H key */
    KBD_KEY_I            = 0x0c, /**< \brief I key */
    KBD_KEY_J            = 0x0d, /**< \brief J key */
    KBD_KEY_K            = 0x0e, /**< \brief K key */
    KBD_KEY_L            = 0x0f, /**< \brief L key */
    KBD_KEY_M            = 0x10, /**< \brief M key */
    KBD_KEY_N            = 0x11, /**< \brief N key */
    KBD_KEY_O            = 0x12, /**< \brief O key */
    KBD_KEY_P            = 0x13, /**< \brief P key */
    KBD_KEY_Q            = 0x14, /**< \brief Q key */
    KBD_KEY_R            = 0x15, /**< \brief R key */
    KBD_KEY_S            = 0x16, /**< \brief S key */
    KBD_KEY_T            = 0x17, /**< \brief T key */
    KBD_KEY_U            = 0x18, /**< \brief U key */
    KBD_KEY_V            = 0x19, /**< \brief V key */
    KBD_KEY_W            = 0x1a, /**< \brief W key */
    KBD_KEY_X            = 0x1b, /**< \brief X key */
    KBD_KEY_Y            = 0x1c, /**< \brief Y key */
    KBD_KEY_Z            = 0x1d, /**< \brief Z key */
    KBD_KEY_1            = 0x1e, /**< \brief 1 key */
    KBD_KEY_2            = 0x1f, /**< \brief 2 key */
    KBD_KEY_3            = 0x20, /**< \brief 3 key */
    KBD_KEY_4            = 0x21, /**< \brief 4 key */
    KBD_KEY_5            = 0x22, /**< \brief 5 key */
    KBD_KEY_6            = 0x23, /**< \brief 6 key */
    KBD_KEY_7            = 0x24, /**< \brief 7 key */
    KBD_KEY_8            = 0x25, /**< \brief 8 key */
    KBD_KEY_9            = 0x26, /**< \brief 9 key */
    KBD_KEY_0            = 0x27, /**< \brief 0 key */
    KBD_KEY_ENTER        = 0x28, /**< \brief Enter key */
    KBD_KEY_ESCAPE       = 0x29, /**< \brief Escape key */
    KBD_KEY_BACKSPACE    = 0x2a, /**< \brief Backspace key */
    KBD_KEY_TAB          = 0x2b, /**< \brief Tab key */
    KBD_KEY_SPACE        = 0x2c, /**< \brief Space key */
    KBD_KEY_MINUS        = 0x2d, /**< \brief - key */
    KBD_KEY_PLUS         = 0x2e, /**< \brief + key */
    KBD_KEY_LBRACKET     = 0x2f, /**< \brief [ key */
    KBD_KEY_RBRACKET     = 0x30, /**< \brief ] key */
    KBD_KEY_BACKSLASH    = 0x31, /**< \brief \ key */
    KBD_KEY_SEMICOLON    = 0x33, /**< \brief ; key */
    KBD_KEY_QUOTE        = 0x34, /**< \brief " key */
    KBD_KEY_TILDE        = 0x35, /**< \brief ~ key */
    KBD_KEY_COMMA        = 0x36, /**< \brief , key */
    KBD_KEY_PERIOD       = 0x37, /**< \brief . key */
    KBD_KEY_SLASH        = 0x38, /**< \brief Slash key */
    KBD_KEY_CAPSLOCK     = 0x39, /**< \brief Caps Lock key */
    KBD_KEY_F1           = 0x3a, /**< \brief F1 key */
    KBD_KEY_F2           = 0x3b, /**< \brief F2 key */
    KBD_KEY_F3           = 0x3c, /**< \brief F3 key */
    KBD_KEY_F4           = 0x3d, /**< \brief F4 key */
    KBD_KEY_F5           = 0x3e, /**< \brief F5 key */
    KBD_KEY_F6           = 0x3f, /**< \brief F6 key */
    KBD_KEY_F7           = 0x40, /**< \brief F7 key */
    KBD_KEY_F8           = 0x41, /**< \brief F8 key */
    KBD_KEY_F9           = 0x42, /**< \brief F9 key */
    KBD_KEY_F10          = 0x43, /**< \brief F10 key */
    KBD_KEY_F11          = 0x44, /**< \brief F11 key */
    KBD_KEY_F12          = 0x45, /**< \brief F12 key */
    KBD_KEY_PRINT        = 0x46, /**< \brief Print Screen key */
    KBD_KEY_SCRLOCK      = 0x47, /**< \brief Scroll Lock key */
    KBD_KEY_PAUSE        = 0x48, /**< \brief Pause key */
    KBD_KEY_INSERT       = 0x49, /**< \brief Insert key */
    KBD_KEY_HOME         = 0x4a, /**< \brief Home key */
    KBD_KEY_PGUP         = 0x4b, /**< \brief Page Up key */
    KBD_KEY_DEL          = 0x4c, /**< \brief Delete key */
    KBD_KEY_END          = 0x4d, /**< \brief End key */
    KBD_KEY_PGDOWN       = 0x4e, /**< \brief Page Down key */
    KBD_KEY_RIGHT        = 0x4f, /**< \brief Right Arrow key */
    KBD_KEY_LEFT         = 0x50, /**< \brief Left Arrow key */
    KBD_KEY_DOWN         = 0x51, /**< \brief Down Arrow key */
    KBD_KEY_UP           = 0x52, /**< \brief Up Arrow key */
    KBD_KEY_PAD_NUMLOCK  = 0x53, /**< \brief Keypad Numlock key */
    KBD_KEY_PAD_DIVIDE   = 0x54, /**< \brief Keypad Divide key */
    KBD_KEY_PAD_MULTIPLY = 0x55, /**< \brief Keypad Multiply key */
    KBD_KEY_PAD_MINUS    = 0x56, /**< \brief Keypad Minus key */
    KBD_KEY_PAD_PLUS     = 0x57, /**< \brief Keypad Plus key */
    KBD_KEY_PAD_ENTER    = 0x58, /**< \brief Keypad Enter key */
    KBD_KEY_PAD_1        = 0x59, /**< \brief Keypad 1 key */
    KBD_KEY_PAD_2        = 0x5a, /**< \brief Keypad 2 key */
    KBD_KEY_PAD_3        = 0x5b, /**< \brief Keypad 3 key */
    KBD_KEY_PAD_4        = 0x5c, /**< \brief Keypad 4 key */
    KBD_KEY_PAD_5        = 0x5d, /**< \brief Keypad 5 key */
    KBD_KEY_PAD_6        = 0x5e, /**< \brief Keypad 6 key */
    KBD_KEY_PAD_7        = 0x5f, /**< \brief Keypad 7 key */
    KBD_KEY_PAD_8        = 0x60, /**< \brief Keypad 8 key */
    KBD_KEY_PAD_9        = 0x61, /**< \brief Keypad 9 key */
    KBD_KEY_PAD_0        = 0x62, /**< \brief Keypad 0 key */
    KBD_KEY_PAD_PERIOD   = 0x63, /**< \brief Keypad Period key */
    KBD_KEY_S3           = 0x65  /**< \brief S3 key */
};

/** \brief Keyboard Key Type

    Typedef which can hold a valid keyboard key ID.
*/
typedef uint8_t kbd_key_t;

/** \brief Region Coes for the Dreamcast keyboard

    This is the list of possible values for the "region" field in the
    kbd_state_t structure.
*/
typedef enum kbd_region {
    KBD_REGION_JP = 1, /**< \brief Japanese keyboard */
    KBD_REGION_US = 2, /**< \brief US keyboard */
    KBD_REGION_UK = 3, /**< \brief UK keyboard */
    KBD_REGION_DE = 4, /**< \brief German keyboard */
    KBD_REGION_FR = 5, /**< \brief French keyboard */
    KBD_REGION_IT = 6, /**< \brief Italian keyboard (not supported yet) */
    KBD_REGION_ES = 7  /**< \brief Spanish keyboard */
} kbd_region_t;

/** \defgroup   key_flags   Key State Flags
    \brief                  Bit flags comprising current key state

    A key_state_t is a combination of two flags:
        1) KEY_FLAG_IS_PRESSED:  whether the key is currently pressed
                                 this frame.
        2) KEY_FLAG_WAS_PRESSED: whether the key was previously pressed
                                 last frame.

    Between these two flags, you can know whether a key state transition
    event has occurred (when the two flags have different values).

    @{
*/
#define KEY_FLAG_IS_PRESSED  0x1 /**< \brief If key is currenty pressed */
#define KEY_FLAG_WAS_PRESSED 0x2 /**< \brief If key was previously pressed */
#define KEY_FLAG_ALL         0x3 /**< \brief If key is OR was pressed */
/** @} */

/** \brief Creates a packed key_state_t

    This macro is used to pack two frames worth of key state information
    into a key_state_t.
*/
#define KEY_PACK_STATE(isDown, wasDown) \
    ((!!(isDown)) | ((!!(wasDown)) << 0x1))

/** \brief key_state_t values

    Enumerates each of the 4 different states a key can be in,
    by combining two frames worth of key pressed information
    into two bits.

    \sa key_state_t, key_flags_t
*/
enum {
    /** \brief Key has been in an up state for at least the last two frames */
    KEY_STATE_HELD_UP   = KEY_PACK_STATE(false, false),
    /** \brief Key transitioned from up to pressed this frame */
    KEY_STATE_TAPPED    = KEY_PACK_STATE(true, false),
    /** \brief Key transitioned from down to released this frame */
    KEY_STATE_RELEASED  = KEY_PACK_STATE(false, true),
    /** \brief Key has been held down for at least the last two frames */
    KEY_STATE_HELD_DOWN = KEY_PACK_STATE(true, true),
};

/** \brief Keyboard Key State

    Stores the "state" value for a keyboard key (which is composed
    of two frames worth of up/down bits).

    \sa key_flags_t
*/
typedef uint8_t key_state_t;

/** \brief   Keyboard status structure.

    This structure holds information about the current status of the keyboard
    device. This is what maple_dev_status() will return.
*/
typedef struct kbd_state {
    /** \brief Key states */
    key_state_t keys[MAX_KBD_KEYS];

    /** \brief Modifier keys statuses */
    kbd_mods_t modifiers;

    /** \brief LED statuses */
    kbd_leds_t leds;

    /** \brief Keyboard type/region */
    kbd_region_t region;

    /** \brief Repeated key press data */
    struct {
        kbd_key_t key;    /**< \brief Last key held which will repeat. */
        uint64_t timeout; /**< \brief Time the next repeat will trigger. */
    } repeater;
} kbd_state_t;

/** \brief Keyboard Event Handler Callback

    Function type which can be registered to listen to keyboard key events.

    \param  dev     The maple device the event originated from
    \param  state   The new (transition) state of the key in question:
                        1) KEY_STATE_TAPPED: key pressed event
                        2) KEY_STATE_RELEASED: key released event
    \param  key     The raw key ID whose state change triggered the event
    \param  mods    Flags containing the states of all modifier keys
    \param  leds    Flags containing the states of all LEDs
    \param  ud      Arbitrary user-pointer which was registered with this handler

    \sa kbd_set_event_handler
*/
typedef void (*kbd_event_handler_t)(maple_device_t *dev, key_state_t state,
                                    kbd_key_t key, kbd_mods_t mods,
                                    kbd_leds_t leds, void *ud);

/** \brief Registers an Event Handler

    This function registers a function pointer as the keyboard event callback
    handler, also taking a generic user data pointer which gets passed back
    to the handler upon invocation.

    The callback will be invoced any time a key state transition occurs on any
    connected keyboard. That is a key has either gone from released to pressed
    or from pressed to released.

    \param  callback    Pointer to the function to be called upon key
                        transition events
    \param  user_data   Generic pointer which is stored internally and gets
                        passed back to \p callback upon an event firing
*/
void kbd_set_event_handler(kbd_event_handler_t callback, void *user_data);

/** \brief Configures held key auto-repeat intervals

    This function is used to configure the specific timing behavior for how the
    internal queue treats a key which is being held down. Giving non-zero values
    for both parameters will cause the held key to be re-enqueued every
    \p interval milliseconds after it has been held for the initial \p start time
    in milliseconds.

    Specifying a value of zero for the two parameters disables this repeating key
    behavior.

    \note
    By default, the \p start time is 600ms while the repeating \p interval is 20ms.

    \param  start       The duration after which the held key starts to
                        register as repeated key presses (or zero to disable
                        this behavior).
    \param  interval    The duration between subsequent key repeats after the
                        initial \p start time has elapsed.

    \sa kbd_queue_pop()
*/
void kbd_set_repeat_timing(uint16_t start, uint16_t interval);

/** \brief Converts kbd_key_t value into its corresponding ASCII value

    This function attempts to convert \p key to its ASCII representation
    using an internal translation table and additional keyboard state context.

    \param  key         The raw key type to convert to ASCII
    \param  region      The region type of the keyboard containing the key.
    \param  mods        The modifier flags impacting the key
    \param  leds        The LED state flags impacting the key

    \returns            The ASCII value corresponding to \p key or NULL if
                        the translation was unsuccessful.
*/
char kbd_key_to_ascii(kbd_key_t key, kbd_region_t region,
                      kbd_mods_t mods, kbd_leds_t leds);

/** \brief   Activate or deactivate global key queueing.
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

    This function pops the front element off of the specified keyboard queue,
    and returns the value of that key to the caller.

    If the xlat parameter is true and the key represents an ISO-8859-1
    character, that is the value that will be returned from this function.
    Otherwise if the key cannot be converted into a valid ISO-8859-1 character,
    it will return the raw key code, shifted up by 8 bits.

    If the xlat parameter is false, the lower 8 bits of the returned value will
    be the raw key code. The next 8 bits will be the modifier keys that were
    down when the key was pressed (kbd_mods_t). The next 8 bits will be the
    lock key/LED statuses (kbd_leds_t).

    \param  dev             The keyboard device to read from.
    \param  xlat            Set to true to do key translation. Otherwise,
                            you'll simply get the raw key value plus modifier
                            and LED state information. Raw key values are not
                            mapped at all, so you are responsible for
                            figuring out what it is by the region.

    \return                 The value at the front of the queue, or -1 if there
                            are no keys in the queue.
*/
int kbd_queue_pop(maple_device_t *dev, bool xlat);

/** \cond Init / Shutdown */
void kbd_init(void);
void kbd_shutdown(void);
/** \endcond */

/** @} */

__END_DECLS

#endif  /* __DC_MAPLE_KEYBOARD_H */
