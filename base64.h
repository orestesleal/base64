/* encoding modes used by utilities functions */
#define BASE64 1
#define BASE32 2
#define BASE16 3

void base16_encoder(char *s, char b[]);
void base16_decoder(char *b16, char b[]);
void base64_enc(char *s, char b[]);
void base64_dec(char *s, char b[]);
void base64url_enc(char *s, char b[]);
void base64url_dec(char *s, char b[]);
unsigned char get_token_pos(char tk, unsigned char len, char alp[]);
void base64_enc(char *s, char b[]);
void b64_enc(unsigned const char *s, char b[], unsigned int len);
unsigned int b64_dec(unsigned char *s, char b[], unsigned int len);
unsigned int get_data_size(char *s, unsigned int len);
void b32_enc(unsigned char *s, unsigned char *b, unsigned int len);
unsigned int b32_dec(unsigned char *s, char *b, unsigned int len);
void b16_enc(const char *s, char *b, unsigned int len);
unsigned int b16_dec(const char *s, char *b, unsigned int len);
void encode_wr_file(const char *src, const char *dst, unsigned char mode);
void decode_rd_file(const char *src, const char *dst, unsigned char mode);
struct finfo *get_file(const char *f);
char *alloc(unsigned int size);

struct finfo {  /* used by 'get_file' to return file information */
	char *addr;  /* file is loaded here */
	size_t size; /* size of file is returned here */
};

