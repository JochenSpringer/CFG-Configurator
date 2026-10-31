/*
 * CFGEDIT.C
 * Editor fuer 1st Word Plus GST-CFG Druckertreiber
 * Atari ST / TOS
 *
 * Kompilierung:
 *   Pure C:      Projektdatei anlegen, C-Modul hinzufuegen
 *   Cross-GCC:   make
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXREC 1024
#define MAXDAT 96
#define HDRMAX 512

typedef struct {
    int code;                       /* -1 = Trenner-Byte 00 */
    int len;
    unsigned char data[MAXDAT];
} REC;

static REC recs[MAXREC];
static int nrec = 0;

static unsigned char header[HDRMAX];
static int header_len = 0;

static char fname[160];

static void wait_key(void)
{
    printf("\nWeiter mit Taste...");
    getchar();
}

static int hexval(int c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

static int parse_hex(const char *s, unsigned char *out, int max)
{
    int n = 0;

    while (*s && n < max) {
        int hi, lo;

        while (*s == ' ' || *s == ',') s++;
        if (!*s) break;

        hi = hexval((unsigned char)*s++);
        if (hi < 0) return -1;

        while (*s == ' ' || *s == ',') s++;
        if (!*s) return -1;

        lo = hexval((unsigned char)*s++);
        if (lo < 0) return -1;

        out[n++] = (unsigned char)((hi << 4) | lo);
    }
    return n;
}

static void print_hex(const unsigned char *p, int n)
{
    int i;

    for (i = 0; i < n; i++)
        printf("%02X ", p[i]);
}

static int load_header(const char *name)
{
    FILE *f;
    unsigned char buf[65536];
    size_t size, pos;

    f = fopen(name, "rb");
    if (!f)
        return 0;

    size = fread(buf, 1, sizeof(buf), f);
    fclose(f);

    if (size < 12 || memcmp(buf, "GST-CFG:", 8) != 0)
        return 0;

    for (pos = 8; pos + 4 <= size; pos++) {
        if (buf[pos]     == 0x0A &&
            buf[pos + 1] == 0x50 &&
            buf[pos + 2] == 0x00 &&
            buf[pos + 3] == 0x00) {
            header_len = (int)(pos + 4);
            memcpy(header, buf, header_len);
            return 1;
        }
    }
    return 0;
}

static int load_cfg(const char *name)
{
    FILE *f;
    unsigned char buf[65536];
    size_t size, pos = 0;

    f = fopen(name, "rb");
    if (!f) {
        printf("\nKann Datei nicht oeffnen: %s\n", name);
        return 0;
    }

    size = fread(buf, 1, sizeof(buf), f);
    fclose(f);

    if (size < 12 || memcmp(buf, "GST-CFG:", 8) != 0) {
        printf("\nKeine gueltige GST-CFG-Datei.\n");
        return 0;
    }

    if (!load_header(name)) {
        printf("\nHeader konnte nicht erkannt werden.\n");
        return 0;
    }

    nrec = 0;
    pos = header_len;

    while (pos < size && nrec < MAXREC) {
        int len = buf[pos++];

        if (len == 0) {
            recs[nrec].code = -1;
            recs[nrec].len = 0;
            nrec++;
            continue;
        }

        if (pos + len - 1 > size) {
            printf("\nWarnung: unvollstaendiger Record bei Offset %u.\n",
                   (unsigned int)pos - 1);
            break;
        }

        recs[nrec].code = buf[pos++];
        recs[nrec].len = len - 2;
        memcpy(recs[nrec].data, buf + pos, recs[nrec].len);
        pos += recs[nrec].len;
        nrec++;
    }

    strcpy(fname, name);

    printf("\nGeladen: %s\n", name);
    printf("Header: %d Bytes\n", header_len);
    printf("Records: %d\n", nrec);

    return 1;
}

static int save_cfg(const char *name)
{
    FILE *f;
    int i;

    f = fopen(name, "wb");
    if (!f) {
        printf("\nKann Datei nicht schreiben: %s\n", name);
        return 0;
    }

    fwrite(header, 1, header_len, f);

    for (i = 0; i < nrec; i++) {
        if (recs[i].code < 0) {
            fputc(0, f);
        } else {
            fputc(recs[i].len + 2, f);
            fputc(recs[i].code, f);
            fwrite(recs[i].data, 1, recs[i].len, f);
        }
    }

    fclose(f);

    printf("\nGespeichert: %s\n", name);
    return 1;
}

static void list_records(void)
{
    int i;
    long offset = header_len;

    printf("\nNr   Offset  Code  Len  Daten\n");
    printf("--------------------------------------------\n");

    for (i = 0; i < nrec; i++) {
        if (recs[i].code < 0) {
            printf("%3d  %6ld  ----    0  <Trenner>\n", i, offset);
            offset++;
        } else {
            printf("%3d  %6ld  %04X  %3d  ",
                   i, offset, recs[i].code, recs[i].len);
            print_hex(recs[i].data, recs[i].len);
            printf("\n");
            offset += recs[i].len + 2;
        }
    }
}

static void edit_record(void)
{
    char line[256];
    unsigned char newdat[MAXDAT];
    int idx, n;

    printf("\nRecord-Nummer: ");
    if (!fgets(line, sizeof(line), stdin))
        return;

    idx = atoi(line);
    if (idx < 0 || idx >= nrec) {
        printf("Ungueltige Record-Nummer.\n");
        return;
    }

    if (recs[idx].code < 0) {
        printf("Dies ist ein Trenner-Record.\n");
        return;
    }

    printf("Aktueller Code: %02X\n", recs[idx].code);
    printf("Aktuelle Daten:  ");
    print_hex(recs[idx].data, recs[idx].len);
    printf("\n");

    printf("Neuer Code in Hex, leer = unveraendert: ");
    if (!fgets(line, sizeof(line), stdin))
        return;

    if (line[0] != '\n' && line[0] != '\0')
        recs[idx].code = (int)strtol(line, NULL, 16) & 0xFF;

    printf("Neue Daten in Hex, leer = unveraendert:\n");
    if (!fgets(line, sizeof(line), stdin))
        return;

    n = parse_hex(line, newdat, MAXDAT);

    if (n > 0) {
        memcpy(recs[idx].data, newdat, n);
        recs[idx].len = n;
        printf("Neue Laenge: %d Bytes\n", n);
    } else if (n == 0) {
        printf("Daten unveraendert.\n");
    } else {
        printf("Ungueltige Hex-Eingabe.\n");
    }
}

static void insert_record(void)
{
    char line[256];
    unsigned char newdat[MAXDAT];
    int idx, code, n;

    printf("\nEinfuegen vor Record-Nummer: ");
    if (!fgets(line, sizeof(line), stdin))
        return;

    idx = atoi(line);
    if (idx < 0 || idx > nrec || nrec >= MAXREC) {
        printf("Ungueltige Position oder kein Platz.\n");
        return;
    }

    printf("Code in Hex: ");
    if (!fgets(line, sizeof(line), stdin))
        return;

    code = (int)strtol(line, NULL, 16) & 0xFF;

    printf("Daten in Hex: ");
    if (!fgets(line, sizeof(line), stdin))
        return;

    n = parse_hex(line, newdat, MAXDAT);
    if (n < 0) {
        printf("Ungueltige Hex-Eingabe.\n");
        return;
    }

    memmove(&recs[idx + 1], &recs[idx],
            sizeof(REC) * (size_t)(nrec - idx));

    recs[idx].code = code;
    recs[idx].len = n;
    memcpy(recs[idx].data, newdat, (size_t)n);
    nrec++;

    printf("Record eingefuegt.\n");
}

static void delete_record(void)
{
    char line[256];
    int idx;

    printf("\nZu loeschende Record-Nummer: ");
    if (!fgets(line, sizeof(line), stdin))
        return;

    idx = atoi(line);
    if (idx < 0 || idx >= nrec) {
        printf("Ungueltige Record-Nummer.\n");
        return;
    }

    memmove(&recs[idx], &recs[idx + 1],
            sizeof(REC) * (size_t)(nrec - idx - 1));
    nrec--;

    printf("Record geloescht.\n");
}

int main(void)
{
    char line[256];
    int loaded = 0;

    printf("\nCFGEDIT 1.0\n");
    printf("Editor fuer 1st Word Plus GST-CFG Druckertreiber\n\n");

    while (!loaded) {
        printf("Zu ladende CFG-Datei: ");
        if (!fgets(line, sizeof(line), stdin))
            return 0;

        line[strcspn(line, "\r\n")] = '\0';

        if (!line[0])
            return 0;

        loaded = load_cfg(line);
    }

    for (;;) {
        printf("\n[L]iste  [E]dit  [I]nsert  [D]elete  [S]ave  [Q]uit\n");
        printf("Auswahl: ");

        if (!fgets(line, sizeof(line), stdin))
            break;

        switch (toupper((unsigned char)line[0])) {
        case 'L':
            list_records();
            wait_key();
            break;

        case 'E':
            edit_record();
            break;

        case 'I':
            insert_record();
            break;

        case 'D':
            delete_record();
            break;

        case 'S':
            save_cfg(fname);
            break;

        case 'Q':
            return 0;

        default:
            break;
        }
    }

    return 0;
}
