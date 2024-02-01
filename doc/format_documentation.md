# Documentation Format Overview
Documentation about the engine API is kept alongside the C source code.

There are two stages to the documentation generator:
1. Go through and collect all docstrings and make individual markdown files out of them
2. Go through each individual markdown file and insert other markdown files based on references. References can be marked as `insert` or 'link'

Docstrings can and should live in the files near the code that implements the API, but technically can be anywhere.

When it comes to the final structure of the documentation, each docstring gets its own markdown file/page. Since markdown files can insert or link to markdown files, there really isn't a hierarchy except for `landing.md` which needs to always exist and is used to link to or insert other important markdown files (like modules).


# Documentation example