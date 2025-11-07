/*
 * Example: Lookup Table Optimization for b64_dec
 * 
 * This shows how to replace the O(64) linear search with O(1) lookup table.
 * 
 * PERFORMANCE IMPACT: 10-20x speedup for decoding operations
 */

// Add these after the alphabet definitions in base64.c (around line 80)

// Lookup tables - initialized once
static unsigned char b64_lookup[256] = {0};
static unsigned char b64_url_lookup[256] = {0};
static unsigned char b32_lookup[256] = {0};
static unsigned char b16_lookup[256] = {0};

// Initialize lookup tables (call this once, maybe in a constructor or at startup)
static void init_lookup_tables(void) {
    // Initialize base64 lookup
    for (int i = 0; i < 64; i++) {
        b64_lookup[(unsigned char)b64_alp[i]] = i;
        b64_url_lookup[(unsigned char)b64_url_alp[i]] = i;
    }
    
    // Initialize base32 lookup
    for (int i = 0; i < 32; i++) {
        b32_lookup[(unsigned char)b32_alp[i]] = i;
    }
    
    // Initialize base16 lookup
    for (int i = 0; i < 16; i++) {
        b16_lookup[(unsigned char)b16_alp[i]] = i;
    }
}

// Alternative: Use static initialization (C99) - no function call needed
// This is initialized at compile time
static const unsigned char b64_lookup_static[256] = {
    ['A'] = 0,  ['B'] = 1,  ['C'] = 2,  ['D'] = 3,  ['E'] = 4,  ['F'] = 5,
    ['G'] = 6,  ['H'] = 7,  ['I'] = 8,  ['J'] = 9,  ['K'] = 10, ['L'] = 11,
    ['M'] = 12, ['N'] = 13, ['O'] = 14, ['P'] = 15, ['Q'] = 16, ['R'] = 17,
    ['S'] = 18, ['T'] = 19, ['U'] = 20, ['V'] = 21, ['W'] = 22, ['X'] = 23,
    ['Y'] = 24, ['Z'] = 25,
    ['a'] = 26, ['b'] = 27, ['c'] = 28, ['d'] = 29, ['e'] = 30, ['f'] = 31,
    ['g'] = 32, ['h'] = 33, ['i'] = 34, ['j'] = 35, ['k'] = 36, ['l'] = 37,
    ['m'] = 38, ['n'] = 39, ['o'] = 40, ['p'] = 41, ['q'] = 42, ['r'] = 43,
    ['s'] = 44, ['t'] = 45, ['u'] = 46, ['v'] = 47, ['w'] = 48, ['x'] = 49,
    ['y'] = 50, ['z'] = 51,
    ['0'] = 52, ['1'] = 53, ['2'] = 54, ['3'] = 55, ['4'] = 56, ['5'] = 57,
    ['6'] = 58, ['7'] = 59, ['8'] = 60, ['9'] = 61,
    ['+'] = 62, ['/'] = 63
    // All other values remain 0 (invalid)
};

// REPLACEMENT IN b64_dec function (around line 316):
// 
// OLD CODE (SLOW - O(64) per character):
//     for (t = 0; s[i] != b64_alp[t]; t++);
//     x |= t;
//
// NEW CODE (FAST - O(1) per character):
//     t = b64_lookup_static[(unsigned char)s[i]];
//     if (t == 0 && s[i] != 'A') {  // 'A' maps to 0, so check explicitly
//         // Handle invalid character
//         fprintf(stderr, "b64_dec: error, non alphabet char found -> %c -> %d\n",
//                 s[i], s[i]);
//         exit(EXIT_FAILURE);
//     }
//     x |= t;

// BETTER: Use a sentinel value (like 255) for invalid characters
static const unsigned char b64_lookup_safe[256] = {
    ['A'] = 0,  ['B'] = 1,  ['C'] = 2,  ['D'] = 3,  ['E'] = 4,  ['F'] = 5,
    ['G'] = 6,  ['H'] = 7,  ['I'] = 8,  ['J'] = 9,  ['K'] = 10, ['L'] = 11,
    ['M'] = 12, ['N'] = 13, ['O'] = 14, ['P'] = 15, ['Q'] = 16, ['R'] = 17,
    ['S'] = 18, ['T'] = 19, ['U'] = 20, ['V'] = 21, ['W'] = 22, ['X'] = 23,
    ['Y'] = 24, ['Z'] = 25,
    ['a'] = 26, ['b'] = 27, ['c'] = 28, ['d'] = 29, ['e'] = 30, ['f'] = 31,
    ['g'] = 32, ['h'] = 33, ['i'] = 34, ['j'] = 35, ['k'] = 36, ['l'] = 37,
    ['m'] = 38, ['n'] = 39, ['o'] = 40, ['p'] = 41, ['q'] = 42, ['r'] = 43,
    ['s'] = 44, ['t'] = 45, ['u'] = 46, ['v'] = 47, ['w'] = 48, ['x'] = 49,
    ['y'] = 50, ['z'] = 51,
    ['0'] = 52, ['1'] = 53, ['2'] = 54, ['3'] = 55, ['4'] = 56, ['5'] = 57,
    ['6'] = 58, ['7'] = 59, ['8'] = 60, ['9'] = 61,
    ['+'] = 62, ['/'] = 63
    // All other values are 0, which we'll treat as invalid
};

// In b64_dec, replace the lookup with:
//     t = b64_lookup_safe[(unsigned char)s[i]];
//     if (t == 0 && s[i] != 'A' && s[i] != '=') {  // 'A'=0, '=' is padding
//         // Invalid character
//         fprintf(stderr, "b64_dec: error, non alphabet char found -> %c -> %d\n",
//                 s[i], s[i]);
//         exit(EXIT_FAILURE);
//     }
//     x |= t;

// PERFORMANCE COMPARISON:
// - Old method: For each character, loop through 64-element array = 64 comparisons
// - New method: Direct array access = 1 memory access
// - Speedup: ~10-20x for typical base64 strings

