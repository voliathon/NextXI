# Chat Library

The chat library is used to output text to the chat log and to register for text-added events.

## Usage

### print(text, [color])
Prints the given text to the chat log. If the color is not specified, it will default to 207 (white).

```lua
local chat = require('chat')

chat.print('Hello world!')
chat.print('Hello world in red!', 167)
```

### error(text)
Prints the given text to the chat log in the standard system error color (Red).

```lua
chat.error('An error has occurred!')
```

### warning(text)
Prints the given text to the chat log in the standard alert color (Orange).
```lua
chat.warning('This is a warning.')
```

### success(text)
Prints the given text to the chat log in the standard success color (Green).
```lua
chat.success('Task completed successfully.')
```

### on_text_added(callback)
Registers a callback for when text is added to the chat log.
```lua
chat.on_text_added(function(text, color)
    print('Text added: ' .. text .. ' (color: ' .. color .. ')')
end)
```