#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>

// Function prototypes. Don't change these.
uint8_t *read_card(char *fname, int *size);
void save_jpeg(uint8_t *data, int size, char *filename);
void recover(uint8_t *data, int size);

#define RAW_FILE "card.raw"

int main()
{
    // Read the card.raw file into an array of bytes (uint8_t)
    int card_length;
    uint8_t *card_data = read_card(RAW_FILE, &card_length);
    
    // Recover the images
    recover(card_data, card_length);
}

uint8_t *read_card(char *filename, int *size)
{
    struct stat st;
    if (stat(filename, &st) == -1)
    {
        fprintf(stderr, "Can't get info about %s\n", filename);
        exit(1);
    }
    int len = st.st_size;
    uint8_t *raw = malloc(len * sizeof(uint8_t));
    
    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        fprintf(stderr, "Can't open %s for reading\n", filename);
        exit(1);
    }
    
    fread(raw, 1, len, fp);
    fclose(fp);
    
    *size = len;
    return raw;
}

void save_jpeg(uint8_t *data, int size, char *filename)
{
    static int file_count = 0;

    file_count++;
    if (file_count >= 100)
    {
        fprintf(stderr, "Warning: Your program was terminated.\n");
        fprintf(stderr, "  To prevent a large number of files from being created,\n");
        fprintf(stderr, "  program execution was stopped after save_jpeg was called\n");
        fprintf(stderr, "  100 times. You likely have a bug in your program.\n");
        exit(1);
    }

    FILE *fp = fopen(filename, "wb");
    if (!fp)
    {
        fprintf(stderr, "Can't write to %s\n", filename);
        exit(1);
    }
    
    fwrite(data, 1, size, fp);
    fclose(fp);   
}



void recover(uint8_t *data, int size)
{
    int count = 0;
    int start = 0;
    int end = 0;
    int found = 0;
    char out[100];
    
    while (start < size - 4)
    {
        // Find start of a JPEG
        if (data[start] == 0xff && data[start + 1] == 0xd8 && data[start + 2] == 0xff &&
            (data[start + 3] == 0xe0 || data[start + 3] == 0xe1))
        {
            end = start + 4;
            while (end < size - 1)
            {
                
                if (data[end] == 0xff && data[end + 1] == 0xd9 && data[end + 2] == 0xFF) 
                {
                    sprintf(out, "image_%02d.jpg", count+1);
                    save_jpeg(&data[start], end, out); //this was the issue I was having, also the un-needed nested if statement.
                    count++;
                    break;   
                }
                end++;
            }
        }
        start++;
    }

    printf("Total JPEGs found: %d\n", count);
}

