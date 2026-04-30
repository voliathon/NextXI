# Unicode Library

The `unicode` library handles instantaneous, C-level string translations. Final Fantasy XI is a legacy application that does not natively understand modern UTF-8 text encoding. If standard Lua strings are injected into packets or the chat log, they will render as garbage characters. 

This library bridges the engine's Shift-JIS translator to guarantee safe text rendering.

---

## Functions

### `unicode.to_shift_jis(utf8_string)`
Translates a standard, human-readable UTF-8 string into FFXI's native Shift-JIS byte format. This is strictly required before injecting custom strings into outgoing server packets or the local chat log.
* **Parameters:**
  * `utf8_string` *(string)*: The standard Lua string to translate.
* **Returns:**
  * `shift_jis_string` *(string)*: The translated binary string.
  * `length` *(integer)*: The byte length of the translated string.

### `unicode.from_shift_jis(shift_jis_string)`
Translates an incoming FFXI Shift-JIS string back into readable UTF-8. This is strictly required when reading player names, chat messages, or item descriptions from incoming server packets.
* **Parameters:**
  * `shift_jis_string` *(string)*: The raw binary string intercepted from the game.
* **Returns:**
  * `utf8_string` *(string)*: The readable Lua string.
  * `length` *(integer)*: The byte length of the translated string.

---

## Properties

### `unicode.symbols`
A globally accessible table containing specialized FFXI rendering characters that cannot be easily typed on a standard keyboard.
* **Contains:** Elemental icons (Fire, Ice, Light, Dark, etc.), Auto-Translate dictionary brackets (Red/Green framing), and system glyphs.