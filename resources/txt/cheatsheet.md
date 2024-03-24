# Cheatsheet

Here's a quick overview of how text files are processed before
the contents are displayed.

For some examples, see any of the text files in `resources/txt/game_screens`.
Together they cover pretty much all use cases.


## Preprocessing

Preprocessing is the first step in the pipeline. It mostly consists of
reading in _directives_ such as `{foo}` and performing actions in response,
using a lookup table of named _parameters_.


### Substitution

| Syntax          | Description                                       |
| --------------- | ------------------------------------------------- |
| `{foo}`         | Substitute a parameter named `foo` into the text. |

### Conditions

| Syntax          | Description                                        |
| --------------- | -------------------------------------------------- |
| `{!if p}`       | Begin an "if" block. `p` must be a boolean; the block will be executed when `p` is true. |
| `{!if x < 5}`   | Use a comparison instead. `x` must be an integer.  |
| `{!if 5 > x}`   | Each side can be either a parameter or a constant. |
| `{!if x = y}`   | It's also possible to check for equality.          |
| `{!else_if q}`  | Continue an "if" block with another condition.     |
| `{!else}`       | Continue an "if" block with the default path.      |
| `{!end}`        | End an "if" block.                                 |

### Loops

| Syntax          | Description     |
| --------------- | --------------- |
| `{!list stuff}` | Begin a loop. `stuff` must be an array; the text inside the block will be repeated for each member of `stuff` |
| `{!end}`        | End a loop.     |

### Comments

| Syntax          | Description                                  |
| --------------- | -------------------------------------------- |
| `{-Note 1}`     | A comment, stripped from the text.           |
| `{--- Note 2}`  | Anything that begins with `{-` is a comment. |


## Text formatting

Text formatting is the second (and final) step in the pipeline.

| Syntax          | Meaning               |
| --------------- | --------------------- |
| `_Proper Noun_` | _italics_             |
| `*important*`   | **bold**              |
| `` `cmd` ``     | `monospaced` typeface |
| `=Main Menu=`   | Title of screen       |
| `$How to play$` | Help text             |
| `$Help message` | The second '$' sign is optional. Newlines won't interrupt the formatting. |
| `~fancy~`       | Flavour text          |

The second delimiter (e.g. '$' sign) is optional. Also, newlines won't
interrupt the formatting. So, for example, you might see this at the end of
the text if a help message is needed:

```
$This is a long help message.

It's split over two lines.
```


## Escape sequences

Any of the characters mentioned above as having a special meaning can be
escaped using a backslash, `\`. This will cause the character to be printed,
with no other effect.

The complete list of escape sequences is as follows:
 * `\\`          -- backslash
 * `\_`          -- underscore
 * `\*`          -- asterisk
 * `` \` ``      -- backtick
 * `\=`          -- equals sign
 * `\$`          -- dollar sign
 * `\~`          -- tilde
 * `\{`          -- left curly brace
 * `\}`          -- right curly brace
 * `\<newline>`  -- don't display this newline in the output
