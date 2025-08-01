# hoxml

Header-Only XML parser written in portable ANSI C. *hoxml* is non-validating but largely conforming (see [Limitations](#limitations)).


## Features

- Portable ANSI C (C89), tested with GCC (Windows and Linux), Clang (macOS), and MSVC
- Supports UTF-8, UTF-16BE, and UTF-16LE including their BOMs
- Supports entities (`&apos;`, `&#x74;`, etc.), `<![CDATA[]]>` sections, comments, and processing instructions
- Allows content to be passed in parts
- Does not require malloc() and allows for reallocation of the buffer
- Verifies most well-formedness constraints
- No dependencies beyond the C standard library


## Limitations

- Parses `<!DOCTYPE>` declarations only to the extent of not mistaking one for invalid syntax
- Permits multiple `<?xml?>` and `<!DOCTYPE>` delcarations, although only prior to the root element
- Permits one element to have multiple attributes of the same name


## Usage

Define the implementation before including hoxml.
``` c
#define HOXML_IMPLEMENTATION
#include "hoxml.h"
```
As usual with header-only libraries, the implementation's definition can be limited to just a single file. This will depend on your specific build configuration.

Allocate *hoxml*'s context object, which holds state and metadata information, and a buffer for hoxml to use.
``` c
hoxml_context_t hoxml_context[1];
char* buffer = (char*)malloc(1024);
hoxml_init(hoxml_context, buffer, 1024);
```
The buffer length needed will depend on the amount of XML content, its depth, and various other minor factors. As a rule of thumb, a buffer equal in length to the content is likley enough. In cases where *hoxml* runs out of memory, more may be allocated (see [Error Recovery](#error-recovery)).

Continually call the parsing function until the *end of document* code is returned or an error code is returned.
``` c
hoxml_code_t code;
while ((code = hoxml_parse(hoxml_context, content, content_length)) != HOXML_END_OF_DOCUMENT) {
    switch (code) {
        case HOXML_ERROR_SYNTAX:
            fprintf(stderr, "Syntax error on line %d, column %d. Exiting...\n", hoxml_context->line, hoxml_context->column);
            return EXIT_FAILURE;
        ...
        case HOXML_END_OF_DOCUMENT:
            return EXIT_SUCCESS;
        case HOXML_ELEMENT_BEGIN:
            printf("Opened <%s>\n", hoxml_context->tag);
            break;
        case HOXML_ELEMENT_END:
            if (hoxml_context->content != NULL)
                printf("Closed <%s> with content \"%s\"\n", hoxml_context->tag, hoxml_context->content);
            else
                printf("Closed <%s>\n", hoxml_context->tag);
            break;
        case HOXML_ATTRIBUTE:
            printf("Attribute \"%s\" of <%s> has value: %s\n", hoxml_context->attribute, hoxml_context->tag, hoxml_context->value);
            break;
        ...
    }
}
```
The return codes and what they mean are listed in [Return Codes](#return-codes).

The XML content string passed to `hoxml_parse()` may contain partial content. All that's required is the first call be done with the beginning of the document and subsequent parts be passed contiguously.
The *unexpected EoF* error code will be returned when parsing has reached the end of the current content. At that time, pass the next portion(s) of content. The pointer passed may be the same; *hoxml* will determine if the content is new based on the ability to decode the first character of the passed string. If a single character is split between two content strings, *hoxml* will know and piece it together.


## Return Codes

`HOXML_END_OF_DOCUMENT`: The root element has been closed and parsing is done.

`HOXML_ELEMENT_BEGIN`: A new element began, either with an open tag (e.g `<tag>`) or self-closing tag (e.g. `<tag/>`). Its name is available in the `tag` variable of the context object.

`HOXML_ELEMENT_END`: An element was closed, either with a close tag (e.g. `</tag>`) or self-closing tag (e.g. `<tag/>`). Its name is available in the `tag` variable of the context object. If the element had any content, including whitespace, it will be available in the `content` variable. If not, `content` will be null.

`HOXML_ATTRIBUTE`: An attribute of the open element was declared. The attribute's name is available in the context object's `attribute` variable, its value is available in the `value` variable, and the name of the element to which it belongs is available in the `tag` variable.

`HOXML_PROCESSING_INSTRUCTION_BEGIN`: A processing instruction began. Its target is available in the context object's `tag` variable.

`HOXML_PROCESSING_INSTRUCTION_END`: A processing instruction ended. Its content is available in the context object's `content` variable and its target is (still) available in the `tag` variable.

`HOXML_ERROR_INSUFFICIENT_MEMORY`: Continued parsing requires more memory. This error is one of two that can be recovered (see [Error Recovery](#error-recovery)).

`HOXML_ERROR_UNEXPECTED_EOF`: Reached the end of the XML content before the end of the document. This error is one of two that can be recovered (see [Error Recovery](#error-recovery)).

`HOXML_ERROR_SYNTAX`: Invalid syntax. The `line` and `column` variables of the context object will contain the line and column, respectively, where the error was first noticed but not necessarily where it exists.

`HOXML_ERROR_ENCODING`: Character encoding errors or contradictions. For example, a document that contains a UTF-16BE Byte-Order Marker (BOM) will trigger this if its XML document declaration indicates UTF-8 encoding.

`HOXML_ERROR_TAG_MISMATCH`: A closing tag did not match the tag that opened it. For example, `<tag>` closed with `</tga>`.

`HOXML_ERROR_INVALID_DOCUMENT_TYPE_DECLARATION`: A Document Type Declaration (DTD) (`<!DOCTYPE>`) was found after the root element.

`HOXML_ERROR_INVALID_DOCUMENT_DECLARATION`: A document declaration (`<?xml?>`) was found after the root element.


## Error Recovery

Of the possible errors, two are recoverable: `HOXML_ERROR_INSUFFICIENT_MEMORY` and `HOXML_ERROR_UNEXPECTED_EOF`.

`HOXML_ERROR_INSUFFICIENT_MEMORY` can be recovered by providing a larger buffer to hoxml through the `hoxml_realloc()` function.
``` c
hoxml_context_t hoxml_context[1];
void* buffer = malloc(1024);
hoxml_init(hoxml_context, buffer, 1024);
/* Given the above initialization, the buffer can be doubled with: */
void* new_buffer = malloc(2048);
hoxml_realloc(hoxml_context, new_buffer, 2048);
free(buffer);
buffer = new_buffer;
```
The next call to `hoxml_parse()` will continue as if nothing happened.

`HOXML_ERROR_UNEXPECTED_EOF` can be recovered by providing the continuation of the XML content to `hoxml_parse()`, assuming it exists.
``` c
const char* first_half = "<root>This is the first half of...";
const char* second_half = "...a two-part string</root>";
hoxml_code_t code;
while ((code = hoxml_parse(hoxml_context, first_half, strlen(first_half))) != HOXML_ERROR_UNEXPECTED_EOF) ;
while ((code = hoxml_parse(hoxml_context, second_half, strlen(second_half))) != HOXML_END_OF_DOCUMENT) ;
```


## Acknowledgements

*hoxml* and its state machine design were inspired by [Yxml](https://dev.yorhel.nl/yxml).
