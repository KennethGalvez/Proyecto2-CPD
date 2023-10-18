#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <openssl/des.h>

void encrypt(long key, unsigned char *plain, int len) {
    DES_cblock k;
    DES_key_schedule ks;
    for(int i = 0; i < 8; ++i){
        k[i] = (key >> (8 * (7 - i))) & 0xFF;
    }
    DES_set_odd_parity(&k);
    DES_set_key_checked(&k, &ks);
    for (int i = 0; i < len; i += 8) {
        DES_ecb_encrypt((const_DES_cblock *)(plain + i), (DES_cblock *)(plain + i), &ks, DES_ENCRYPT);
    }
}

void decrypt(long key, unsigned char *ciph, int len) {
    DES_cblock k;
    DES_key_schedule ks;
    for(int i = 0; i < 8; ++i){
        k[i] = (key >> (8 * (7 - i))) & 0xFF;
    }
    DES_set_odd_parity(&k);
    DES_set_key_checked(&k, &ks);
    for (int i = 0; i < len; i += 8) {
        DES_ecb_encrypt((const_DES_cblock *)(ciph + i), (DES_cblock *)(ciph + i), &ks, DES_DECRYPT);
    }
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    char *input_file = NULL;
    unsigned char cipher_text[256]; 
    long file_size = 0;

    long known_key = 0x0000000000000042;

    if (rank == 0) {
        if (argc != 4) {
            printf("Uso: %s <archivo_entrada.txt> <potencia_de_2> <archivo_salida.txt>\n", argv[0]);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        input_file = argv[1];
        int power_of_two = atoi(argv[2]);

        FILE *input_fp = fopen(input_file, "rb");
        if (!input_fp) {
            perror("No se pudo abrir el archivo de entrada");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        fseek(input_fp, 0, SEEK_END);
        file_size = ftell(input_fp);
        fseek(input_fp, 0, SEEK_SET);

        char plain_text[file_size];
        fread(plain_text, 1, file_size, input_fp);
        fclose(input_fp);

        // Encriptar el texto usando la clave conocida
        encrypt(known_key, plain_text, file_size);
        memcpy(cipher_text, plain_text, file_size);
    }

    MPI_Bcast(&file_size, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(cipher_text, file_size, MPI_CHAR, 0, MPI_COMM_WORLD);

    char search[] = "es una"; 

    long upper = (1L << atoi(argv[2])); 
    long range_per_node = upper / size;
    long mylower = range_per_node * rank;
    long myupper = range_per_node * (rank + 1) - 1;
    if (rank == size - 1) {
        myupper = upper;
    }

    for (long key = mylower; key <= myupper; key++) {
        unsigned char decrypted_text[file_size];
        memcpy(decrypted_text, cipher_text, file_size);
        decrypt(key, decrypted_text, file_size);

        if (strstr((char *)decrypted_text, search) != NULL) {
            printf("Proceso %d con clave %lx encontró el texto descifrado correcto:\n%s\n", rank, key, decrypted_text);
            break; 
        }
    }

    MPI_Finalize();
    return 0;
}