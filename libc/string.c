#include "string.h"

size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

char *strcpy(char *dest, const char *src) {
    char *d = dest;
    while ((*d++ = *src++));
    return dest;
}

char *strncpy(char *dest, const char *src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i]; i++) dest[i] = src[i];
    for (; i < n; i++) dest[i] = '\0';
    return dest;
}

char *strcat(char *dest, const char *src) {
    char *d = dest + strlen(dest);
    while ((*d++ = *src++));
    return dest;
}

char *strncat(char *dest, const char *src, size_t n) {
    char *d = dest + strlen(dest);
    while (n-- && *src) *d++ = *src++;
    *d = '\0';
    return dest;
}

int strcmp(const char *a, const char *b) {
    while (*a && (*a == *b)) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

int strncmp(const char *a, const char *b, size_t n) {
    if (n == 0) return 0;
    while (--n && *a && (*a == *b)) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

char *strchr(const char *str, int c) {
    while (*str) {
        if (*str == (char)c) return (char *)str;
        str++;
    }
    return (c == '\0') ? (char *)str : NULL;
}

char *strrchr(const char *str, int c) {
    const char *last = NULL;
    while (*str) {
        if (*str == (char)c) last = str;
        str++;
    }
    return (char *)last;
}

void *memset(void *ptr, int value, size_t size) {
    uint8_t *p = (uint8_t *)ptr;
    while (size--) *p++ = (uint8_t)value;
    return ptr;
}

void *memcpy(void *dest, const void *src, size_t size) {
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    while (size--) *d++ = *s++;
    return dest;
}

void *memmove(void *dest, const void *src, size_t size) {
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    if (d < s) {
        while (size--) *d++ = *s++;
    } else {
        d += size; s += size;
        while (size--) *--d = *--s;
    }
    return dest;
}

int memcmp(const void *a, const void *b, size_t size) {
    const uint8_t *p1 = (const uint8_t *)a;
    const uint8_t *p2 = (const uint8_t *)b;
    while (size--) {
        if (*p1 != *p2) return *p1 - *p2;
        p1++; p2++;
    }
    return 0;
}

void itoa(int value, char *buf, int base) {
    char tmp[32];
    int i = 0, neg = 0;
    if (value < 0 && base == 10) { neg = 1; value = -value; }
    if (value == 0) { buf[0] = '0'; buf[1] = '\0'; return; }
    while (value) {
        int r = value % base;
        tmp[i++] = (r < 10) ? '0' + r : 'a' + r - 10;
        value /= base;
    }
    if (neg) tmp[i++] = '-';
    int j = 0;
    while (i--) buf[j++] = tmp[i];
    buf[j] = '\0';
}

void utoa(uint32_t value, char *buf, int base) {
    char tmp[32];
    int i = 0;
    if (value == 0) { buf[0] = '0'; buf[1] = '\0'; return; }
    while (value) {
        int r = value % base;
        tmp[i++] = (r < 10) ? '0' + r : 'a' + r - 10;
        value /= base;
    }
    int j = 0;
    while (i--) buf[j++] = tmp[i];
    buf[j] = '\0';
}
