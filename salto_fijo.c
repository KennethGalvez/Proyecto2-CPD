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

    if (argc != 4) {
        printf("Uso: %s <archivo_entrada.txt> <archivo_salida.txt> <limite_superior>\n", argv[0]);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    long upper_limit = atol(argv[3]);
    char *input_file = NULL;
    unsigned char cipher_text[256];
    long file_size = 0;
    double start_time, end_time;

    long known_key = 12345678L;

    if (rank == 0) {
        start_time = MPI_Wtime();

        input_file = argv[1];
        
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

        encrypt(known_key, plain_text, file_size);
        memcpy(cipher_text, plain_text, file_size);
    }

    MPI_Bcast(&file_size, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(cipher_text, file_size, MPI_CHAR, 0, MPI_COMM_WORLD);

    char search[] = "es una";
    int found = 0;
    double found_time = -1.0;

    // Saltos fijos para probar las claves
    int jump = size;
    for (long key = rank; key < upper_limit && !found; key += jump) {
        unsigned char decrypted_text[file_size];
        memcpy(decrypted_text, cipher_text, file_size);
        decrypt(key, decrypted_text, file_size);

        if (strstr((char *)decrypted_text, search) != NULL) {
            printf("Proceso %d con clave %lx encontró el texto descifrado correcto:\n%s\n", rank, key, decrypted_text);
            found = 1;
            found_time = MPI_Wtime();  // Marcar el tiempo en que se encontró la solución
        }
    }

    int global_found = 0;
    double min_found_time = -1.0;

    MPI_Reduce(&found, &global_found, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&found_time, &min_found_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0 && global_found) {
        printf("Tiempo total para descifrar: %f segundos\n", min_found_time - start_time);
    }
    MPI_Finalize();
    return 0;
}
