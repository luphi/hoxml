#include <stdio.h> /* fprintf(), printf(), stderr */
#include <stdlib.h> /* EXIT_FAILURE, EXIT_SUCCESS, free(), malloc(), NULL */

#define HOXML_IMPLEMENTATION
#include "hoxml.h"

int main(int argc, char** argv) {
    /* XML content to parse. A string constant is used to keep this simple but content from disk would be typical. */
    const char* content = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                          "<the_lord_of_the_rings>\n"
                          "    <book>The Fellowship of the Ring</book>\n"
                          "    <book>The Two Towers</book>\n"
                          "    <book>The Return of the King</book>\n"
                          "</the_lord_of_the_rings>";
    size_t content_length = strlen(content);

    /* hoxml initialization. Allocate the context object and buffer then call hoxml_init(). */
    hoxml_context_t hoxml_context[1];
    char* buffer = (char*)malloc(content_length * 2);
    hoxml_init(hoxml_context, buffer, content_length * 2);

    /* Loop until the "end of document" code is returned */
    hoxml_code_t code;
    int exit_status = EXIT_SUCCESS;
    while ((code = hoxml_parse(hoxml_context, content, content_length)) != HOXML_END_OF_DOCUMENT &&
           exit_status == EXIT_SUCCESS) {
        /* Various codes will be returned as hoxml_parse() finds tags, attributes, content, etc. or runs into an */
        /* error. Parsing is finished when an (unrecoverable) error code or the "end of document" code is returned. */
        switch (code) {
        /* Error code cases: */
        case HOXML_ERROR_INVALID_INPUT:
            fprintf(stderr, " A parameter passed to hoxml_parse() was unacceptable\n");
            exit_status = EXIT_FAILURE;
            break;
        case HOXML_ERROR_INTERNAL:
            fprintf(stderr, " hoxml ran into an internal error\n");
            exit_status = EXIT_FAILURE;
            break;
        case HOXML_ERROR_INSUFFICIENT_MEMORY:
            fprintf(stderr, " Not enough memory\n");
            exit_status = EXIT_FAILURE;
            break;
        case HOXML_ERROR_UNEXPECTED_EOF:
            fprintf(stderr, " Unexpected end of file\n");
            exit_status = EXIT_FAILURE;
            break;
        case HOXML_ERROR_SYNTAX:
            fprintf(stderr, " Syntax error: line %d, column %d\n", hoxml_context->line, hoxml_context->column);
            exit_status = EXIT_FAILURE;
            break;
        case HOXML_ERROR_ENCODING:
            fprintf(stderr, " Encoding error: line %d, column %d\n", hoxml_context->line, hoxml_context->column);
            exit_status = EXIT_FAILURE;
            break;
        case HOXML_ERROR_TAG_MISMATCH:
            fprintf(stderr, " Start tag did not match end tag on line %d\n", hoxml_context->line);
            exit_status = EXIT_FAILURE;
            break;
        case HOXML_ERROR_INVALID_DOCUMENT_TYPE_DECLARATION:
            fprintf(stderr, " XML document type declaration found before a root element\n");
            exit_status = EXIT_FAILURE;
            break;
        case HOXML_ERROR_INVALID_DOCUMENT_DECLARATION:
            fprintf(stderr, " XML document declaration found before a root element\n");
            exit_status = EXIT_FAILURE;
            break;
        /* Element, attribute, etc. code cases: */
        case HOXML_PROCESSING_INSTRUCTION_BEGIN:
            printf(" Processing instruction target: %s\n", hoxml_context->tag);
            break;
        case HOXML_PROCESSING_INSTRUCTION_END:
            printf(" Processing instruction content: %s\n", hoxml_context->content);
            break;
        case HOXML_ELEMENT_BEGIN:
            printf(" Opened <%s>\n", hoxml_context->tag);
            break;
        case HOXML_ELEMENT_END:
            if (hoxml_context->content != NULL) {
                /* Check the content to see if it only contains whitespace */
                int is_empty = 1;
                char* iterator = hoxml_context->content;
                while (*iterator != '\0') {
                    /* If not a whitespace or newline character */
                    if (*iterator != 0x20 && *iterator != 0x09 && *iterator != 0x0A && *iterator != 0x0D) {
                        is_empty = 0;
                        break;
                    }
                    iterator++;
                }
                /* If the content string contains more than just whitespace */
                if (is_empty == 0)
                    printf(" Closed <%s> with content \"%s\"\n", hoxml_context->tag, hoxml_context->content);
            } else
                printf(" Closed <%s>\n", hoxml_context->tag);
            break;
        case HOXML_ATTRIBUTE:
            printf(" Attribute \"%s\" of <%s> has value: %s\n", hoxml_context->attribute,
                hoxml_context->tag, hoxml_context->value);
            break;
        default:
            printf(" Unhandled code %d\n", code);
            break;
        }
    }

    free(buffer);
    if (exit_status == EXIT_SUCCESS)
        printf("\n\n Parsed to the end of the document\n");
    return exit_status;
}
