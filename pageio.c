// pageio.c

#include "pageio.h"
#include "webpage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>

int32_t pagesave(webpage_t *pagep, int id, char *dirnm) {
    // Construct the filename
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/%d", dirnm, id);

    // Open the file for writing
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file %s for writing.\n", filename);
        return -1;
    }

    // Write the URL
    fprintf(file, "%s\n", webpage_getURL(pagep));

    // Write the depth
    fprintf(file, "%d\n", webpage_getDepth(pagep));

    // Write the length of the HTML
    fprintf(file, "%d\n", webpage_getHTMLlen(pagep));

    // Write the HTML content
    fprintf(file, "%s\n", webpage_getHTML(pagep));

    // Close the file
    fclose(file);

    return 0;
}
