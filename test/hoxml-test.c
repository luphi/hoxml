#include <stdio.h> /* FILE, fopen(), fprintf(), fread(), fseek(), ftell(), printf(), SEEK_END, SEEK_SET, stderr */
#include <stdlib.h> /* atoi(), EXIT_FAILURE, EXIT_SUCCESS, free() malloc(), NULL */

#define HOXML_IMPLEMENTATION
/* #define HOXML_DEBUG */
#include "hoxml.h"

#define NUM_DOCUMENTS 20
#define NUM_INVALID_DOCUMENTS 9
#define CONTENT_BUFFER_LENGTH 75 /* Small, odd number to force reallocation and to trigger "unexpected EoF" errors */
                                 /* halfway through UTF-16 characters */

int main(int argc, char** argv) {
    char* documents[NUM_DOCUMENTS];
    int from, to, document_index;

    /* These documents are expected to return errors */
    documents[0]  = "invalid_doctype.xml";
    documents[1]  = "invalid_document_declaration.xml";
    documents[2]  = "invalid_encoding.xml";
    documents[3]  = "invalid_syntax_cdata.xml";
    documents[4]  = "invalid_syntax_close_and_self_close.xml";
    documents[5]  = "invalid_syntax_name_char.xml";
    documents[6]  = "invalid_syntax_quotation_mismatch.xml";
    documents[7]  = "invalid_syntax_reference.xml";
    documents[8]  = "invalid_tag_mismatch.xml";
    /* These documents are expected to be parsed successfully */
    documents[9]  = "valid_basic.xml";
    documents[10] = "valid_cdata.xml";
    documents[11] = "valid_comments.xml";
    documents[12] = "valid_doctype.xml";
    documents[13] = "valid_encoding_utf8.xml";
    documents[14] = "valid_encoding_utf16be.xml";
    documents[15] = "valid_encoding_utf16le.xml";
    documents[16] = "valid_little_bit_of_everything.xml";
    documents[17] = "valid_nesting.xml";
    documents[18] = "valid_references.xml";
    documents[19] = "valid_tilemap.tmx";

    from = 0;
    to = NUM_DOCUMENTS - 1;
    if (argc > 1) /* If a specific index was passed as a CLI argument */
        from = to = atoi(argv[1]); /* No sanitation here. You're a programmer. Be smart. */

    for (document_index = from; document_index <= to; document_index++) {
        FILE* file;
        size_t content_length, hoxml_buffer_length, bytes_read;
        void* hoxml_buffer;
        hoxml_context_t hoxml_context;
        const char* content_pointer;
        char content[CONTENT_BUFFER_LENGTH], content_copy[CONTENT_BUFFER_LENGTH];

        if ((file = fopen(documents[document_index], "r")) == NULL) {
            fprintf(stderr, "Couldn't open document \"%s\"\n", documents[document_index]);
            return EXIT_FAILURE;
        }

        fseek(file, 0, SEEK_END); /* Seek to the end of the file */
        content_length = ftell(file); /* Take the position in the file, the end, as the length */
        fseek(file, 0, SEEK_SET); /* Seek back to the beginning of the file to iterate through it */
        printf("\n\n\n  --------- Parsing XML document \"%s\" of length %lu\n", documents[document_index],
            (unsigned long)content_length);

        hoxml_buffer_length = content_length / 8; /* Use a small length to force multiple line reads */
        hoxml_buffer = malloc(hoxml_buffer_length);
        hoxml_init(&hoxml_context, hoxml_buffer, hoxml_buffer_length);
        printf("  --- Using an initial buffer length of %lu\n", (unsigned long)hoxml_buffer_length);

        content_pointer = content;
        while ((bytes_read = fread(content, 1, CONTENT_BUFFER_LENGTH - 1, file)) != 0) {
            int is_done_with_document;

            content[bytes_read] = '\0';
            memcpy(content_copy, content, bytes_read);
            content_pointer = content_pointer == content ? content_copy : content;
            is_done_with_document = 0;
            while (is_done_with_document == 0) {
                hoxml_code_t code;
                
                code = hoxml_parse(&hoxml_context, content_pointer, bytes_read);
                if (code < HOXML_END_OF_DOCUMENT) { /* If an error was returned */
                    if (code == HOXML_ERROR_UNEXPECTED_EOF) { /* Recoverable error */
                        /* Recover by exiting this loop leading to more XML content being read from disk */
                        printf("  --- Parsed to end of the current content buffer - continuing to next string...\n");
                        break;
                    } else if (code == HOXML_ERROR_INSUFFICIENT_MEMORY) { /* Recoverable error */
                        void* new_buffer;

                        /* Recover by doubling the size of the buffer and telling hoxml to use it */
                        printf("  --- Ran out of memory - increasing buffer from %lu to %lu\n",
                            (unsigned long)hoxml_buffer_length, (unsigned long)hoxml_buffer_length * 2);
                        hoxml_buffer_length *= 2;
                        new_buffer = malloc(hoxml_buffer_length);
                        hoxml_realloc(&hoxml_context, new_buffer, hoxml_buffer_length);
                        free(hoxml_buffer);
                        hoxml_buffer = new_buffer;
                    } else { /* NOT a recoverable error */
                        /* Some of the test documents are invalid and errors are expected. For them, just continue to */
                        /* the next document. For the rest, the valid documents, the error can't go ignored. */
                        if (document_index < NUM_INVALID_DOCUMENTS) { /* The document was invalid, this is expected */
                            printf("  --- Document \"%s\" returned error code %d on line %u, column %u as expected. "
                                "Pass.\n", documents[document_index], code, hoxml_context.line, hoxml_context.column);
                            is_done_with_document = 1;
                            break;
                        } else { /* The document was valid and something is wrong */
                            fprintf(stderr, "\n\n Error on line %d, column %d: %d\n", hoxml_context.line,
                                hoxml_context.column, code);
                            fclose(file);
                            free(hoxml_buffer);
                            return EXIT_FAILURE;
                        }
                    }
                } else if (code == HOXML_END_OF_DOCUMENT) {
                    /* Hopefully, this is one of the valid documents where we expect to eventually receive this code. */
                    /* But if this is not one of those documents, the test has failed. */
                    if (document_index < NUM_INVALID_DOCUMENTS) { /* The document was invalid, an error was expected */
                        fprintf(stderr, "\n\n  Parsing of document \"%s\" completed successfully but was expected to "
                            "fail\n", documents[document_index]);
                        return EXIT_FAILURE;
                    } else { /* The document was valid and parsed to the end of it so we're done with it */
                        printf("  --- Parsing of document %s completed without error. Pass.\n",
                            documents[document_index]);
                        is_done_with_document = 1;
                        break;
                    }
                } else {
                    switch (code) {
                    case HOXML_ELEMENT_BEGIN:
                        printf("  element  begin: \"%s\"\n", hoxml_context.tag);
                        break;
                    case HOXML_ELEMENT_END:
                        printf("  element    end: \"%s\"\n", hoxml_context.tag);
                        if (hoxml_context.content != NULL) {
                            int is_empty;
                            char* iterator;

                            /* Check the content to see if it only contains whitespace */
                            is_empty = 1;
                            iterator = hoxml_context.content;
                            while (*iterator != 0) {
                                if (!HOXML_IS_WHITESPACE(*iterator)) {
                                    is_empty = 0;
                                    break;
                                }
                                iterator++;
                            }
                            /* If the content string contains more than just whitespace */
                            if (is_empty == 0)
                                printf("         content: \"%s\"\n", hoxml_context.content);
                        }
                        break;
                    case HOXML_ATTRIBUTE:
                        printf("  attribute name: \"%s\"\n", hoxml_context.attribute);
                        printf("           value: \"%s\"\n", hoxml_context.value);
                        printf("      of element: \"%s\"\n", hoxml_context.tag);
                        break;
                    case HOXML_PROCESSING_INSTRUCTION_BEGIN:
                        printf("  PI      target: \"%s\"\n", hoxml_context.tag);
                        break;
                    case HOXML_PROCESSING_INSTRUCTION_END:
                        printf("  PI     content: \"%s\"\n", hoxml_context.content);
                        break;
                    default: break;
                    }
                }
            }

            if (is_done_with_document == 1)
                break;
        }

        fclose(file);
        free(hoxml_buffer);
    }

    printf("\n\n\n  PASS\n");
    return EXIT_SUCCESS;
}