#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <openssl/des.h>

void encrypt(char *key_str, char *plain, char *ciph, int len) {
    DES_cblock des_key;
    DES_key_schedule keysched;
    char output[len];
    
    // Set the encryption key
    DES_set_key_unchecked((const_DES_cblock *)key_str, &keysched);
    
    // Encrypt the data
    for (int i = 0; i < len; i += 8) {
        DES_ecb_encrypt((const_DES_cblock *)(plain + i), (DES_cblock *)(output + i), &keysched, DES_ENCRYPT);
    }
    
    memcpy(ciph, output, len);
}

void decrypt(char *key_str, char *ciph, char *plain, int len) {
    DES_cblock des_key;
    DES_key_schedule keysched;
    char output[len];
    
    // Set the decryption key
    DES_set_key_unchecked((const_DES_cblock *)key_str, &keysched);
    
    // Decrypt the data
    for (int i = 0; i < len; i += 8) {
        DES_ecb_encrypt((const_DES_cblock *)(ciph + i), (DES_cblock *)(output + i), &keysched, DES_DECRYPT);
    }
    
    memcpy(plain, output, len);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Uso: %s <clave> <archivo_entrada.txt> <archivo_salida.txt>\n", argv[0]);
        return 1;
    }

    char *key_str = argv[1];
    char *input_file = argv[2];
    char *output_file = argv[3];

    FILE *input_fp = fopen(input_file, "rb");
    if (!input_fp) {
        perror("No se pudo abrir el archivo de entrada");
        return 1;
    }

    fseek(input_fp, 0, SEEK_END);
    long file_size = ftell(input_fp);
    fseek(input_fp, 0, SEEK_SET);

    char *plain_text = (char *)malloc(file_size);
    if (!plain_text) {
        perror("Error de asignación de memoria para el texto plano");
        fclose(input_fp);
        return 1;
    }

    fread(plain_text, 1, file_size, input_fp);
    fclose(input_fp);

    char cipher_text[file_size];
    encrypt(key_str, plain_text, cipher_text, file_size);

    FILE *output_fp = fopen(output_file, "wb");
    if (!output_fp) {
        perror("No se pudo abrir el archivo de salida");
        free(plain_text);
        return 1;
    }

    fwrite(cipher_text, 1, file_size, output_fp);
    fclose(output_fp);

    char decrypted_text[file_size];
    decrypt(key_str, cipher_text, decrypted_text, file_size);

    printf("Texto original:\n%s\n", plain_text);
    printf("Texto cifrado:\n%s\n", cipher_text);
    printf("Texto descifrado:\n%s\n", decrypted_text);

    free(plain_text);
    
    return 0;
}
